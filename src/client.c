#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <string.h> 
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


#define MAX_BUFF 100 
#define MAX_MSG_SIZE 256 
#define STDIN 0
#define DEFAULT_IP "127.0.0.1"  
#define DEFAULT_PORT 1345

// registration request contains 2 messages: "registration request" and user_name
void register_me(int sock, char* user_name)
{
	char reg_req[MAX_MSG_SIZE] = "registration request for: ";

	strcat(reg_req, user_name);

	// Sending registration message (contain only the user name)
	send(sock , reg_req , strlen(reg_req) , 0); 
	printf("sent registration request\n\n");
}


int main(int argc, char *argv[]) 
{ 
	// you must run the server before the client
	// you must enter a user name for the cient. for example: ./client genady

	int sock = 0; 
	struct sockaddr_in server_addr; 
	char user_msg[MAX_MSG_SIZE];
	int port_dst = 0;
	char ip_dst[15] = {0};
	char *tmp;
	char user_name[MAX_MSG_SIZE] = {0};
	char buff[MAX_BUFF] = {0};


	// creating set for the file descriptors, which represent the "read sockets" of the clients 
	fd_set read_fds;
	int max_sock;
	int ready;



	// Creating Socket:
	// int socket(int domain, int type, int protocol);
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("***Error in creating the socket*** \n"); 
		return -1; 
	} 

	// Creating Adress:
	server_addr.sin_family = AF_INET; 
	if(argc == 4)
	{
		// define port which got by the user using arguments
		port_dst = atoi(argv[2]);
		server_addr.sin_port = htons(port_dst);
		// define IP which got by the user using arguments
		if(inet_aton(argv[1], &server_addr.sin_addr) <= 0)
		{ 
			printf("***Invalid Address***\n"); 
			return -1; 
		}

		strcpy(user_name, argv[3]);
		strcpy(ip_dst, argv[1]);
	}
	else
	{
		// define port
		server_addr.sin_port = htons(DEFAULT_PORT); 
		// convert IPv4 and IPv6 addresses from text to binary form 
		if(inet_aton(DEFAULT_IP, &server_addr.sin_addr) <= 0)
		{ 
			printf("***Invalid Address***\n"); 
			return -1; 
		}

		strcpy(user_name, argv[1]);
		strcpy(ip_dst, DEFAULT_IP);
		port_dst = DEFAULT_PORT;
	} 


	printf("defined port destination %d\n", port_dst);
	printf("defined IP destination %s\n", ip_dst);
	printf("defined user name %s\n", user_name);

	// Connect the socket to the address
	if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
	{ 
		printf("**Connection Failed**\n"); 
		return -1; 
	}

	printf("Welcome %s. for stoppint the connection please enter the key word 'stop'\n", user_name);

	register_me(sock, user_name);

	while(1)
	{

		// initiate the read set which will hold the file descriptor
		FD_ZERO(&read_fds);

		// adding to the "read_fds" the listen_socket
  		FD_SET(sock, &read_fds);
  		FD_SET(STDIN, &read_fds);

  		// max_sock will hold the top socket (which is the top file desciptor)
  		if (sock > STDIN)
  			max_sock = sock;
  		else
  			max_sock = STDIN;



  		// call selelct - when one of the sockets will be "ready", or we get timeout, we will continue the next line
		//ready = select(max_sock + 1, &read_fds, NULL, NULL, &timeout);	
		ready = select(max_sock + 1, &read_fds, NULL, NULL, NULL);	


		if (ready < 0)
			printf("ERROR SELECT\n");

		else if (ready == 0)
			printf("IM NOT READY\n");

		else
		{
			if (FD_ISSET(STDIN, &read_fds))
			{
				char name_and_msg[MAX_MSG_SIZE] = {0};

				//get a user input
				printf("me - ");
                       
				//scanf("%s", user_msg);
				fgets(user_msg, MAX_MSG_SIZE, stdin);
				strcpy(name_and_msg, user_name);
				strcat(name_and_msg, ": ");
				strcat(name_and_msg, user_msg);

				// Sending Message
				send(sock, name_and_msg, strlen(name_and_msg) , 0); 
				//printf("message sent to the server: %s\n", name_and_msg);

				if(!strcmp(user_msg, "stop"))
					break;
			}
			else if (FD_ISSET(sock, &read_fds))
			{
				memset(buff, 0, MAX_BUFF);
				read(sock, buff, sizeof(buff));
				printf("%s\n", buff);
			}
		}
	}
	
	close(sock);
	return 0; 
}

