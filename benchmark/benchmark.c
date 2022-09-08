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
