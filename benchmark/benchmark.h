#ifndef BENCHMARK_H
#define BENCHMARK_H

const static struct option opts[] = {
	{ "rank",    required_argument, NULL, 'r' },
	{ "count",   required_argument, NULL, 'c' },
	{ "port",    required_argument, NULL, 'p' },
	{ "ips",     required_argument, NULL, 'i' },
	{ "help",    no_argument,       NULL, 'h' },
	{ NULL,      0,                 NULL, 0 }
};

static void check_init(const char *f);

int get_rank();
int get_succ();
int get_pred();

/* Initialize args and communication */
void init(int argc, char **argv);

/* Initialize TCP connections */
void TCP_Init();

/* Accept TCP connection */
void TCP_Accept(int source);

/* Connect to TCP socket */
void TCP_Connect(int dest);

/* Finalize TCP Connections */
int TCP_Finalize();

/* Parse the arguments */
static int parse_opts(int argc, char **argv)

#endif
