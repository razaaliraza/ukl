#include <liburing.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define COLS 1

int main(int argc, char **argv)
{
	// Initialize the arguments
	init(argc, argv);

	const long ROWS = atol(argv[argc - 1]); 

	const int rank = get_rank();
	const int pred = get_pred();
	const int succ = get_succ();

	
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
	}                                                        }
}

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
				;
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
	return 0;
}

void TCP_Connect(int dest_
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

	Convert IPv4 and IPv6 addresses from text to binary form
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
#ifdef TRACE_TCP
	int i;
	struct timespec diff;
	FILE *snds = fopen("/sends.csv", "w");
	FILE *recvs = fopen("/receives.csv", "w");

	printf("Saw %ld sends and %ld receives\n", send_count, receive_count);

	fprintf(snds, "Time,Size\n");
	fprintf(recvs, "Time,Size\n");
	for (i = 0; i < send_count; i++)
	{
		calc_diff(&diff, &sends[i].end, &sends[i].start);
		fprintf(snds, "%ld.%09ld,%ld\n", diff.tv_sec, diff.tv_nsec, sends[i].size);
	}
	for (i = 0; i < receive_count; i++)
	{
		calc_diff(&diff, &receives[i].end, &receives[i].start);
		fprintf(recvs, "%ld.%09ld,%ld\n", diff.tv_sec, diff.tv_nsec, receives[i].size);
	}

#ifdef URING_TCP
	io_uring_queue_exit(ring)
#endif
	fflush(snds);
	fflush(recvs);

	free(sends);
	free(receives);

	fclose(snds);
	fclose(recvs);
#endif

	close(succ_sock);
	close(pred_sock);
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
