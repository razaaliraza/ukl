# Unikernel Linux
# Copyright (C) 2018-2022 Red Hat Inc., Boston University,
# Ali Raza, Tommy Unger, Eric Munson, Richard W.M. Jones.
#
# This library is free software; you can fiotribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

EXTRA_DIST = 

CLEANFILES = \
	*~ *.o *.a \
	stamp-fio-dir

clean-local:
	-cd fio && $(MAKE) clean

distclean-local:
	rm -rf fio

noinst_DATA = UKL.a

# Libraries built in the top build directory.
CRT_STARTS  = $(abs_top_builddir)/crt1.o $(abs_top_builddir)/crti.o \
	      $(abs_top_builddir)/crtbeginT.o
CRT_ENDS    = $(abs_top_builddir)/crtend.o $(abs_top_builddir)/crtn.o
C_LIB       = $(abs_top_builddir)/libc.a
PTHREAD_LIB = $(abs_top_builddir)/libpthread.a
RT_LIB      = $(abs_top_builddir)/librt.a
MATH_LIB    = $(abs_top_builddir)/libm.a
GCC_LIBS    = $(abs_top_builddir)/libgcc.a $(abs_top_builddir)/libgcc_eh.a

AM_CFLAGS   = -ggdb -mno-red-zone -mcmodel=kernel -static

# Build zlib.
libz.a: stamp-zlib-dir
	rm -f $@
	-cd zlib && $(MAKE) clean
	cd zlib && \
	CFLAGS="$(AM_CFLAGS)" ./configure --static
	$(MAKE) -C zlib
	mv zlib/libz.a $@

# Check out a local copy of zlib.
stamp-zlib-dir:
	rm -f $@
	if ! test -d zlib; then \
	    	wget https://www.zlib.net/zlib-1.2.13.tar.gz; \
		tar -zxf zlib-1.2.13.tar.gz; \
		rm -rf zlib-1.2.13.tar.gz; \
		mv zlib-1.2.13 zlib; \
	fi
	touch $@

UKL.a: stamp-fio-dir undefined_sys_hack.o math_hack.o libz.a
	rm -f $@
	$(MAKE) clean -C fio
	cd fio && ./configure --build-static --extra-cflags="$(AM_CFLAGS)" --disable-libnfs \
		--disable-tcmalloc
# XXX We expect this to fail with undefined references to
# set_bypass_limit / set_bypass_syscall (provided by UKL).
	-$(MAKE) V=1 -C fio
	cd fio && ld -r -o ../fio.o --allow-multiple-definition \
	    $(CRT_STARTS) \
	    filehash.o rate-submit.o steadystate.o filesetup.o \
	    diskutil.o fio.o fio_sem.o \
	    zbd.o eta.o gettime.o cgroup.o flow.o log.o libfio.o pshared.o blktrace.o \
	    lib/strntol.o lib/num2str.o lib/axmap.o lib/getrusage.o lib/hweight.o \
	    lib/gauss.o lib/rand.o lib/output_buffer.o lib/rbtree.o lib/prio_tree.o \
	    lib/lfsr.o lib/ieee754.o lib/memcpy.o lib/bloom.o lib/memalign.o lib/zipf.o \
	    lib/mountcheck.o lib/pattern.o lib/flist_sort.o zone-dist.o fifo.o parse.o \
	    helpers.o backend.o smalloc.o filelock.o time.o profile.o init.o options.o \
	    iolog.o debug.o stat.o server.o io_u_queue.o memory.o helper_thread.o dedupe.o \
	    trim.o engines/ftruncate.o engines/sync.o engines/mtd.o engines/nvme.o \
	    engines/splice.o engines/io_uring.o engines/net.o engines/exec.o engines/cmdprio.o \
	    engines/e4defrag.o engines/fileoperations.o engines/mmap.o engines/null.o \
	    engines/cpu.o engines/sg.o engines/falloc.o verify.o json.o cconv.o workqueue.o \
	    crc/sha256.o crc/fnv.o crc/crc64.o crc/md5.o crc/sha512.o crc/xxhash.o \
	    crc/crc32c-arm64.o crc/sha1.o crc/murmur3.o crc/crc32c-intel.o crc/sha3.o \
	    crc/crc7.o crc/crc16.o crc/crc32.o crc/crc32c.o crc/test.o oslib/libmtd.o \
	    oslib/asprintf.o oslib/libmtd_legacy.o oslib/strlcat.o oslib/linux-blkzoned.o \
	    oslib/linux-dev-lookup.o oslib/strcasestr.o oslib/strndup.o idletime.o client.o \
	    gettime-thread.o optgroup.o ioengines.o profiles/act.o profiles/tiobench.o \
	    td_error.o rwlock.o io_u.o \
	    --start-group \
	    --whole-archive ../libz.a $(MATH_LIB) $(RT_LIB) $(PTHREAD_LIB) $(C_LIB) --no-whole-archive \
	    $(GCC_LIBS) \
	    --end-group $(CRT_ENDS)
	ar cr UKL.a fio.o undefined_sys_hack.o math_hack.o
	objcopy --prefix-symbols=ukl_ UKL.a
	objcopy --redefine-syms=$(top_srcdir)/redef_sym_names UKL.a

# Check out a local copy of fio.
stamp-fio-dir:
	rm -f $@
	if ! test -d fio; then \
	    git clone -b fio-ukl git@github.com:unikernelLinux/fio.git; \
	fi
	touch $@

undefined_sys_hack.c: $(top_builddir)/undefined_sys_hack.c
	cp $< $@

# automake doesn't add this rule unless we were to specify a C program
# to compile, which we don't want to do because of the very special
# linking requirements.
.c.o:
	$(CC) $(CFLAGS) $(AM_CFLAGS) -c $< -o $@

data.tar.gz: 
	-
