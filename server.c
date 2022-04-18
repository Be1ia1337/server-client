#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

#define MESSAGE_LEN 40
#define PORT 8080

/** @brief socket creation, binding
 * 	@param listening_socket is socket that will be accepted via server
 * 	@param client_sock is clients socket
 * 	@param servaddr for setting ip and port
 * 	@return void
 */
void connection(int listening_socket, int client_sock, struct sockaddr_in* servaddr);

/** @brief comunicating with clients
 * 	@param client_sock is adress of clients socket 
 * 	@return void*
 */
void* communicating(void* client_sock);

/** @brief viewng, editing or execution of commands grom client
 * 	@param message is pointer to clients message
 * 	@return void
 */
void command_execution(char* message);

int main ()
{
	int listening_socket, client_sock;
	struct sockaddr_in servaddr;

	// listening_socket creation, binding, assigning with IP and Port
	connection(listening_socket, client_sock, &servaddr);
   
	// After chatting close the listening_socket
	close(listening_socket);
}

void connection(int listening_socket, int client_sock, struct sockaddr_in* servaddr)
{
	int* new_sock;
	int len;
	struct sockaddr_in* client_adress;

	// listening_socket create and verification
	if ((listening_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    	printf("listening_socket creation failed...\n");
    	exit(0);
	}
	printf("listening_socket successfully created..\n");
	bzero(servaddr, sizeof(*servaddr));

	// assign IP, PORT
	servaddr->sin_family = AF_INET;
   	servaddr->sin_addr.s_addr = htonl(INADDR_ANY);
   	servaddr->sin_port = htons(PORT);

	// Binding newly created listening_socket to given IP and verification
	const int optional = 1;
	const socklen_t optional_length = sizeof(optional);
	setsockopt(listening_socket , SOL_SOCKET, SO_REUSEPORT , &optional , optional_length);
	
    if ((bind(listening_socket, (struct sockaddr *)servaddr, sizeof(*servaddr))) < 0) {
       	printf("listening_socket bind failed...\n");
       	printf("Something went wrong with read()! %s\n", strerror(errno));
       	exit(0);
	}
    printf("listening_socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(listening_socket, 5)) != 0) {
    	printf("Listen failed...\n");
   		exit(0);
	}
    printf("Server listening..\n");
   	len = sizeof(client_adress);

	// Accept the data packet from client and verification
    puts("Waiting for incoming connections...");
    int result;
    while(client_sock = accept(listening_socket, (struct sockaddr *)&client_adress, &len)) {
        pthread_t client_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if (client_sock < 0) {
	    	printf("server accept failed...\n");
		    exit(0);
    	}

        printf("server accept the client...\n");
        result = pthread_create(&client_thread, NULL, communicating ,  (void*) new_sock);
        
        if(result < 0) {
    	    puts("Can't create thread :-(");
    	    exit(0);
        }
    }
}

void* communicating(void* client_sock)
{
	int this_sock = *(int*) client_sock;
	char message[MESSAGE_LEN];
	int n;
	int isrecv, issend;
    isrecv = 0;

	//Receive a message from client
	while(this_sock > 0)
	{
		memset(message, '\0', MESSAGE_LEN);
		
		isrecv = recv(this_sock, message, MESSAGE_LEN, 0);
		if((isrecv < 0) || (strncmp(message, "disconnect", 10) == 0))  {
			send(this_sock, "disconnect", 10, 0);
			puts("Client disconnected");
			break;
		}
        
		printf("From client: %s", message);

		// Work with message
		command_execution(message);

		//Send the message back to client
		send(this_sock, message, MESSAGE_LEN, 0);
		
	}
	free(client_sock);
	return 0;
}

void command_execution(char* message)
{
	if (strncmp(message, "get_time", 8) == 0) {
	    time_t now;
	    struct tm *localnow;
	    time( &now );
	    localnow = localtime( &now );

	    // Format the struct tm to a new string.
        char formatted[40];
	    strftime(formatted, sizeof(formatted), "%Y-%m-%d %T", localnow);
		strcpy(message, formatted);
	}
}
