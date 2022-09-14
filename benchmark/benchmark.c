#include <liburing.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include "benchmark.h"

#define COLS 				1
#define NUM_PARTIES 		3
#define DEFAULT_PORT 		8000

#define QUEUE				10
#define EVENT_TYPE_READ		1
#define EVENT_TYPE_WRITE	2

#define MAX_CONN_TRIES 		8

struct io_uring ring;

struct request {
	int event_type;
	int iovec_count;
	int socket;
	struct iovec iov[];
};

struct secrecy_config {
	unsigned int rank;
	unsigned int num_parties;
	int initialized;
	unsigned short port;
	char **ip_list;
} config;

const static char *opt_str = "r:c:p:i:h";

int succ_sock;
int pred_sock;

int main(int argc, char **argv)
{
	// Initialize the arguments
	init(argc, argv);

	const long ROWS = atol(argv[argc - 1]); 

	const int rank = get_rank();
	const int pred = get_pred();
	const int succ = get_succ();

	int len = strlen("Hello");
	char *recv_buf = calloc(len, sizeof(char));
	if(rank == 0)
	{
		char *str = (char *)"Hello";
		TCP_Send(get_socket(succ), (void *)str, len);
		TCP_Send(get_socket(pred), (void *)str, len);
	}
	else
	{
		TCP_Recv(get_socket(0), (void *)recv_buf, len);
		printf("Node %i recieved: %s\n", rank, recv_buf);
	}
}

// initialize communication, config.rank, num_parties
void init(int argc, char **argv)
{
    if (parse_opts(argc, argv))
	{
		printf("Failed to parse input options\n");
		exit(1);
	}	

	TCP_Init();
	// this protocol works with 3 parties only
	if (config.rank == 0 && config.num_parties != NUM_PARTIES)
	{
		fprintf(stderr, "ERROR: The number of processes must be %d for %s\n", NUM_PARTIES, argv[0]);
	}
}

int get_socket(unsigned int rank)
{
	if (rank == get_succ())
	{
		return succ_sock;
	}
	return pred_sock;
}

void TCP_Send(int sock, void *data, int len)
{
	struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	struct io_uring_cqe *cqe;
	struct request *req = malloc(sizeof(struct request) + sizeof(struct iovec));

	req->event_type = EVENT_TYPE_WRITE;
	req->socket = sock;
	req->iovec_count = 1;
	req->iov[0].iov_base = malloc(len);
	req->iov[0].iov_len = len;
	memcpy(req->iov[0].iov_base, data, len);

	io_uring_prep_writev(sqe, req->socket, req->iov, req->iovec_count, 0);
	io_uring_sqe_set_data(sqe, req);
	io_uring_submit(&ring);

	io_uring_wait_cqe(&ring, &cqe);
	io_uring_cqe_seen(&ring, cqe);
}

void TCP_Recv(int sock, void* buf, int len)
{
	struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	struct io_uring_cqe *cqe;
	struct request *req = malloc(sizeof(struct request) + sizeof(struct iovec));
	struct request *res;

	req->iovec_count = 1;
	req->iov[0].iov_base = malloc(len);
	req->iov[0].iov_len = len;
	req->event_type = EVENT_TYPE_READ;
	req->socket = sock;
	memset(req->iov[0].iov_base, 0, len);

	io_uring_prep_readv(sqe, req->socket, &req->iov[0], req->iovec_count, 0);
	io_uring_sqe_set_data(sqe, req);
	io_uring_submit(&ring);

	cqe = malloc(sizeof(struct io_uring_cqe) + sizeof(struct iovec));

	if(io_uring_wait_cqe(&ring, &cqe) != 0)
	{
		perror("Recieve completion failed.\n");
		exit(0);
	}

	if (cqe->res < 0) 
	{
		fprintf(stderr, "Async request failed: %s for event: %d\n",
		strerror(-cqe->res), req->event_type);
		exit(1);
	}

	res = (struct request *)io_uring_cqe_get_data(cqe);
	memcpy(buf, (char *)res->iov[0].iov_base, len);

	io_uring_cqe_seen(&ring, cqe);
}

static void print_usage(const char *name)
{
	printf("Usage: %s <opts>\n", name);
	printf("<opts>:\n");
	printf("    -r|--rank     The rank of this node (from 0 to parties - 1)\n");
	printf("    -c|--count    The count of parties participating\n");
	printf("    -p|--port     The to use for internode communication, defaults to 8000\n");
	printf("    -i|--ips      Comma delimited list of ip addresses in rank order\n");
	printf("    -h|--help     Print this message\n");
};

static int parse_opts(int argc, char **argv)
{
	int c;
	char *haystack = NULL;
	unsigned int i;

	// Set the port here, if the user specified a value this will be overwritten
	config.port = DEFAULT_PORT;
	int opt_index = 0;
	while (1)
	{
		c = getopt_long(argc, argv, opt_str, opts, &opt_index);
		if (c == -1)
			break;

		switch (c)
		{
			case 'r':
				config.rank = atoi(optarg);
				break;
			case 'c':
				config.num_parties = atoi(optarg);
				break;
			case 'p':
				config.port = atoi(optarg);
				break;
			case 'i':
				if (optarg == NULL)
				{
					printf("Missing argument to --ips switch\n");
					print_usage(argv[0]);
					return -1;
				}
				haystack = optarg;
				break;
			case 'h':
			case '?':
				print_usage(argv[0]);
				return -1;
			default:
				printf("Unknown option -%o\n", c);
				print_usage(argv[0]);
				return -1;
		}
	}

	if (haystack != NULL)
	{
		config.ip_list = calloc(config.num_parties, sizeof(char*));
		if (config.ip_list == NULL)
		{
			printf("Failed to allocate memory for ip list\n");
			return -1;
		}

		char *next = NULL;
		i = 0;
		do
		{
			config.ip_list[i] = haystack;
			i++;
			if (i >= config.num_parties)
			break;
			next = strchr(haystack, ',');
			if (next != NULL)
			{
			*next = '\0';
			haystack = next + 1;
			}
		} while(next != NULL);
	}
	
	if (config.ip_list == NULL || config.num_parties == 0)
	{
		printf("Invalid configuration, you must specify node rank, count of parties, and the ip list\n");
		print_usage(argv[0]);
		return -1;
	}

	config.initialized = 1;

	return 0;
}

void TCP_Init()
{
	if(get_rank() == 0)
	{
		TCP_Connect(get_succ());
		TCP_Accept(get_pred());
	}
	else
	{
		TCP_Accept(get_pred());
		TCP_Connect(get_succ());
	}

	io_uring_queue_init(QUEUE, &ring, IORING_SETUP_SQPOLL);
}

void TCP_Accept(int source)
{
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	int result = setsockopt(server_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(int));
	if (result)
	{
		perror("Error setting TCP_NODELAY ");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
	&opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(config.port);

	// Forcefully attaching socket to the port
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
	}

	pred_sock = new_socket;
}

int TCP_Connect(int dest)
{
	int sock = 0, option = 1;
	struct sockaddr_in serv_addr;
	int tries = 0;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(config.port);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, get_address(dest), &serv_addr.sin_addr) <= 0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	int result = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &option, sizeof(int));
	if (result)
	{
		perror("Error setting TCP_NODELAY ");
		return -1;
	}

	while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		if (tries < MAX_CONN_TRIES &&
		(errno == ECONNREFUSED || errno == EINTR || errno == ETIMEDOUT || errno == ENETUNREACH || errno == EHOSTUNREACH))
		{
			// Try and exponential back-off to wait for the other side to come up
			printf("Couldn't connect to peer, waiting %d seconds before trying again.\n", (int)pow(2, tries));
			sleep((int)pow(2, tries));
			tries++;
		}
		else
		{
			perror("Connection Failed ");
			return -1;
		}
	}

	succ_sock = sock;
	return 0;
}

int TCP_Finalize()
{
	io_uring_queue_exit(&ring);
	
	close(succ_sock);
	close(pred_sock);
}

/* get the IP address of given rank */
char *get_address(unsigned int rank)
{
	if (rank < config.num_parties)
	{
		return config.ip_list[rank];
	}
	else
	{
		printf("No such rank!");
		return NULL;
	}
	return NULL;
}

static void check_init(const char *f)
{
	if (!config.initialized)
	{
		fprintf(stderr, "ERROR: init() must be called before %s\n", f);
	}
}

int get_rank()
{
	check_init(__func__);
	return config.rank;
}

int get_succ()
{
	check_init(__func__);
	return (get_rank() + 1) % NUM_PARTIES;
}

int get_pred()
{
	check_init(__func__);
	return ((get_rank() + NUM_PARTIES) - 1) % NUM_PARTIES;
}
