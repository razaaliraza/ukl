#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/workqueue.h>
// #include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/delay.h>
#include <net/sock.h>
#include <net/tcp_states.h>

#include <linux/ukl.h>
#include <linux/kmain.h>
#include <asm/sections.h>
#include <asm/proto.h>
#include <asm/fsgsbase.h>
#include <linux/sched.h>

void * tls;

extern void __libc_setup_tls (unsigned long start, unsigned long tbss_start, unsigned long end);
extern void __ctype_init (void);
// extern void __init_array_start (int, char **, char **);
// extern void __init_array_end (int, char **, char **);
extern void (*__init_array_start []) (int, char **, char **);
extern void (*__init_array_end []) (int, char **, char **);
extern unsigned long get_gs_value(void);
extern int fsbringup(void);
extern void ukl_sync(void);

void printaddr(long addr){
	printk("*** Called from addr 0x%lx ***\n", addr);
}

unsigned int inet_addr2(char* ip)
{
    int a, b, c, d;
    char addr[4];
    
    sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d);
    addr[0] = a;
    addr[1] = b;
    addr[2] = c;
    addr[3] = d;
    
    return *(unsigned int *)addr;
}

void __copy_tls(void * dest, void * src, size_t n, size_t m){
	memset (memcpy (dest, src, n) + n, '\0', m);
	return;
}

void setup_mm(void){
    struct task_struct *me = current;

   printk("init pgd = 0x%lx\n", init_mm.pgd);   
   printk("current pgd = 0x%lx\n", me->active_mm->pgd);   
   // me->mm = me->active_mm;
    me->mm = mm_alloc();
    me->mm->get_unmapped_area = arch_get_unmapped_area_topdown;
    me->mm->mmap_base = 0x7f0000000000; 
    me->mm->mmap_legacy_base = 0x300000000000;
    me->mm->task_size = 0x7ffffffff000;
    me->mm->start_brk = 0x405000;
    me->mm->brk = 0x405000;

   printk("new pgd = 0x%lx\n", me->mm->pgd);   
    //me->active_mm = me->mm;
    init_mm.pgd = me->mm->pgd;
   printk("init pgd = 0x%lx\n", init_mm.pgd);   

   printk("sem->owner = 0x%lx sem->count = 0x%lx\n", &me->mm->mmap_sem.owner, &me->mm->mmap_sem.count);

    printk("thread_info->flags = %lx\n", me->thread_info.flags);

    printk("Set up of mm struct, done.\n");
}

void lib_start_kmain(void){
    struct task_struct *me = current;
    __libc_setup_tls(__tls_data_start, __tls_bss_start, __tls_bss_end);
    printk("TLS address for main thread is %lx\n", me->thread.fsbase);
    __pthread_initialize_minimal_internal(me->thread.fsbase);
    printk("Set up of TCB done. \n");
    __ctype_init ();
    printk("Set up of ctype data done. \n");

    printk("Old task struct flags = %x\n", me->flags);
    me->flags = me->flags^PF_KTHREAD;
    me->flags = me->flags^PF_NOFREEZE;
    me->flags = me->flags^PF_USED_ASYNC;
    me->flags = me->flags^PF_SUPERPRIV; 
    printk("Current task struct flags = %x\n", me->flags);
    printk("Current task struct address = %lx\n", me);
    printk("Old task struct thread_info flags = %x\n", me->thread_info.flags);
    me->thread_info.flags = 0;
    printk("Old task struct thread_info flags = %x\n", me->thread_info.flags);
}

void setup_networking(void){
    int fd = -1;
    int retioctl = -1;

    struct sockaddr_in sin; 
    struct ifreq ifr;

    fd = ukl_socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    if(fd < 0){
        printk("Problem with socket\n");
        return  -1;
    }

    strncpy(ifr.ifr_name, "eth0\0", 5);
    memset(&sin, 0, sizeof(struct sockaddr));
    sin.sin_family = AF_INET;
    sin.sin_port=htons(0);
    sin.sin_addr.s_addr = inet_addr2("10.0.2.15");
    memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));

    retioctl = ukl_ioctl(fd, SIOCSIFADDR, &ifr);
    if(retioctl < 0){
        printk("1st Ioctl failed\n");
        return  -1;
    }


    /*strncpy(ifr.ifr_name, "eth0", 4);*/
    ifr.ifr_flags |= IFF_BROADCAST;
    ifr.ifr_flags |= IFF_MULTICAST;

    retioctl = ukl_ioctl(fd, SIOCGIFFLAGS, &ifr);
    if(retioctl < 0){
        printk("2nd Ioctl failed\n");
        return  -1;
    }


    /*strncpy(ifr.ifr_name, "eth0", 4);*/
    ifr.ifr_flags |= IFF_UP;
    /*ifr.ifr_flags |= IFF_BROADCAST;*/
    ifr.ifr_flags |= IFF_RUNNING;
    /*ifr.ifr_flags |= IFF_MULTICAST;*/

    retioctl = ukl_ioctl(fd, SIOCSIFFLAGS, &ifr);
    if(retioctl < 0){
        printk("3rd Ioctl failed\n");
        return  -1;
    }

#ifndef CONFIG_PREEMPT
#ifdef CONFIG_UNIKERNEL_LINUX
    enter_ukl();
#endif
#endif
    msleep(3000);

#ifndef CONFIG_PREEMPT
#ifdef CONFIG_UNIKERNEL_LINUX
    exit_ukl();
#endif
#endif

    printk("Set up of network interface, done.\n");
}

int interface(void)
{
    printk("interface, init array start: %px\n", &__init_array_start);
    printk("interface, init array end: %px\n", &__init_array_end);
    setup_mm();
    //setup_networking();
    lib_start_kmain();

    int i = 0;

    fsbringup();

    //set_fs(MAKE_MM_SEG(0x7FFFFFFFF000));

    //tracing_on();
    printk("interface, about to call kmain");
    kmain();
    
    //ukl_sync();
    
    while(1){
        current->state = TASK_INTERRUPTIBLE;
        schedule();
       }
    
    return 0;
}
