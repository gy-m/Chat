#include <stdio.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MSG_SIZE 256 
#define MAX_NAME_SIZE 256 
#define MAX_BUFF 100 
#define MAX_CLIENTS 20
#define DEFAULT_PORT 1345 
#define MAX_LISTEN_QUEUE_LEN 5


struct client
{
    char name[MAX_NAME_SIZE];
    int socket_number;
};


// return registraion 1 if registration, 0 otherwise
int check_if_registration(char* msg)
{
	char* user_name_start_ptr;
	char user_name[MAX_NAME_SIZE] = {0};

	if(strstr(msg, "registration request for:"))
	{
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[]) 
{ 

	//int listen_socket = 0, client_socket[MAX_CLIENTS] = {0}, len = 0; 
	int listen_socket = 0, len = 0; 

	struct sockaddr_in server_addr = {0};
	struct sockaddr_in client_addr = {0};
	// read data buffer
	char buff[MAX_BUFF] = {0};

	// declare timeout parameter for select function
	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	// creating set for the file descriptors, which represent the "read sockets" of the clients 
	fd_set read_fds;
	int ready;
	int max_sock;
	int comm;

	struct client clients[MAX_CLIENTS] = {0};


	// Creating listening Socket:
	if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("***Error in creating the socket*** \n"); 
		return -1; 
	} 


	// Creating Adress:
	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	//server_addr.sin_port = htons(DEFAULT_PORT); 

	// if the user enter a port as an argument
	if(argc == 2)
	{
		int port_dst = 0;
		port_dst = atoi(argv[1]);
		server_addr.sin_port = htons(port_dst);
		printf("defined port %d\n", port_dst);

	}
	// if not, use the default port
	else
	{
		server_addr.sin_port = htons(DEFAULT_PORT); 
		printf("defined port %d\n", DEFAULT_PORT);
	}


	// Binding listening socket to IP:
	if ((bind(listen_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))) != 0)
	{ 
		printf("***Error in binding socket to address***\n"); 
		return -1; 
	} 


	// Listening for connection:
	if ((listen(listen_socket, MAX_LISTEN_QUEUE_LEN)) != 0)
	{ 
		printf("***Error in listening for connection***\n"); 
		return -1; 
	}

	while(1)
	{

		// initiate the read set which will hold the file descriptor
		FD_ZERO(&read_fds);

		// adding to the "read_fds" the listen_socket
  		FD_SET(listen_socket, &read_fds);

  		// max_sock will hold the top socket (which is the top file desciptor)
  		max_sock = listen_socket;


		// adding to the set the client_sockets
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
		  	// if client socket is 0 (not sent us anything) we do not want to add him to the set
			if (clients[i].socket_number == 0)
		  		continue;

		  	// if we found the client socket which sent us something, its fd will changed and we want to add it to the set
		  	FD_SET(clients[i].socket_number, &read_fds);

		  	// msx_sock will always hold the top socke / file descriptor
		  	if (max_sock < clients[i].socket_number)
		  			max_sock = clients[i].socket_number;
		  }

  		// call selelct - when one of the sockets will be "ready", or we get timeout, we will continue the next line
		//ready = select(max_sock + 1, &read_fds, NULL, NULL, &timeout);	
		ready = select(max_sock + 1, &read_fds, NULL, NULL, NULL);	

		
		if (ready < 0)
			printf("ERROR SELECT\n");

		else if (ready == 0)
			printf("IM NOT READY\n");

		else
		{
			// check which socket is ready
			if (FD_ISSET(listen_socket, &read_fds))
			{
				// Accept the connection and create new socket:
				len = sizeof(client_addr); 
				comm = accept(listen_socket, (struct sockaddr*)&client_addr, &len);

				// check if the new connection is a client or it is an error
				if (comm < 0)
				{ 
						printf("***Error in accepting connection***\n"); 
						return -1; 
				}

				else
				{
					//printf("Adding a new socket %d\n", comm);
					printf("Adding a new socket: %d\nstarting registration\n", comm);

					// find the place in the socket array where to place the new socket
					for(int i = 0; i < MAX_CLIENTS; i++)
					{
						if(clients[i].socket_number == 0)
						{
							clients[i].socket_number = comm;
							FD_SET(clients[i].socket_number, &read_fds);
							break;
						}
					}
				}
			}

			else
			{

				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					if (FD_ISSET(clients[i].socket_number, &read_fds))
					{
						char new_user[MAX_NAME_SIZE] = {0};

						memset(buff, 0, MAX_BUFF);
						read(clients[i].socket_number, buff, sizeof(buff));


						if (check_if_registration(buff))
						{


							char *user_name_start_ptr;
							user_name_start_ptr = strstr(buff, ":");
							strcpy(clients[i].name, user_name_start_ptr+2);
							printf("new registration for: %s \n", clients[i].name);
						}

						printf("%s: %s\n", clients[i].name, buff); 

						char tmp[MAX_BUFF] = {0};
						strcpy(tmp, buff);
						memset(buff, 0, MAX_BUFF);

						// in this exercise we will won't close the server when we get "stop" because he handles with another clients
						if(!strcmp(tmp, "stop"))
						{
							printf("closing the connection of client: %d\n", clients[i].socket_number);
							FD_CLR(clients[i].socket_number, &read_fds);
							close(clients[i].socket_number);
							clients[i].socket_number = 0;	

							// initiate the user name in the array
							strcpy(clients[i].name, "\0");
						}
						else
						{
							for(int i=0; i < MAX_CLIENTS; i++)
							{
								if (clients[i].socket_number != 0)
								{
									printf("broadcasted to client: %s ; the message: %s\n", clients[i].name, tmp);
									send(clients[i].socket_number , tmp , strlen(tmp) , 0); 
								}
							}
						}
						
					}
				}
			}
			
		}
	}

	// Close Sockets
	close(listen_socket); 
	for(int i = 0; i < MAX_CLIENTS; i++)
		close(clients[i].socket_number);
}


