#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#define MESSAGE_LEN 80
#define COMMAND_SIZE 40

int listening_socket;
int is_connected;
char *p_command;

/** @brief connection with server
 * 	@param listening_socket is socket that will beconnected to ip and port
 *  @param servaddr is structure for assigning ip and port of server
 * 	@param ip we will connect
 * 	@param port we will connect
 * 	@return void
 */
int connection(int listening_socket, struct sockaddr_in* servaddr, char* ip, unsigned short int port);

/** @brief sending, recievinf and working wih messages
 * 	@param listening_socket is created socket that will help with communication
 * 	@param command is message that will be send to server
 * 	@return void
 */
void command_execution(int listening_socket, char* command);

/** @brief function for validation of inputing ip
 *  @param ip is ip that will be validate
 *  @return int
 */
int ip_validation(char* ip);

/** @brief ctrl+c catcher */
void INThandler(int dummy);

/** brief help function */
void help(void);

/** @brief close all sockets, free allocated memories, and exit the program */
void all_exit();

int main(int argc, char *argv[])
{
	// ctrl+c handler 
	signal(SIGINT, INThandler);
    
    is_connected = 0;
	struct sockaddr_in servaddr;
    char *command = (char*)malloc(COMMAND_SIZE * sizeof(char));
    p_command = command;

    if(argc != 1) {
    	exit(0);
    }

	while(1) {
		command = p_command;

		memset(command, '\0', COMMAND_SIZE);
        printf("$ ");

        fgets (command, COMMAND_SIZE, stdin);
        if(strncmp(command, "\n", 1) == 0) {
            continue; 
        }

        if(strncmp(command, "exit", 4) == 0) {
        	all_exit();
        	exit(0);
        }

        if(strncmp(command, "help", 4) == 0) {
            help();
            continue;
        }

        if(!is_connected) {
        	if(strncmp(command, "connect", 7) != 0) {
        		puts("Wrong Command!");
        		continue;
        	}

        	char *ip =(char*) malloc(16 * sizeof(char));

        	command += 7;
			while(*command == ' ') {
        		command++;
        	}

            char* pos = strchr(command, ' ');
            if(pos == NULL) {
                puts("IP or port doesn't set!");
                puts("You must write 'connect IP PORT'");
                continue;
            }
        	memcpy(ip, command, pos - command);
        	char ip_for_validate[sizeof(ip) + 1];
	            
        	memcpy(ip_for_validate, ip, sizeof(ip) + 1);
            if(!ip_validation(ip_for_validate)) {
                puts("Wrong IP!");
                continue;
            }   

        	int port;
   
			command += strlen(ip);
			while (*command == ' ') {
        		command++;
        	}

			port = atoi(command);
            if(port < 0 || port > 65535) {
                puts("Wrong port number!");
                continue;
            }

        	// Socket creation and connection to ip and port
			listening_socket = connection(listening_socket, &servaddr, ip, port);

			is_connected = 1;
			continue;
        }

        if((strncmp(command, "get_time", 8) != 0) && (strncmp(command, "disconnect", 10) != 0)) {
        		puts(command);
        		puts("Wrong Command!");
        		continue;
        }

		// Simple chat for command execution 
		command_execution(listening_socket, command);

		if ((strncmp(command, "disconnect", 10)) == 0) {
			command = p_command;
			is_connected = 0;
			close (listening_socket);
    	}
    }
	
	// Close the socket and free alocated spaces
	free(p_command);
}

int connection(int listening_socket, struct sockaddr_in* servaddr, char* ip, unsigned short int port)
{
	// socket create and verificationi
	
	if ((listening_socket = socket(AF_INET , SOCK_STREAM , 0)) < 0)
	{
		printf ("Socket creation failed!\n");
		exit(0);
	}
	printf("Socket successfully created!\n ");

	// assign IP and Port
	servaddr->sin_family = AF_INET;
	servaddr->sin_addr.s_addr = inet_addr(ip);
	servaddr->sin_port = htons(port);
	
	// Connect the client socket to server socket
	if(connect(listening_socket, (struct sockaddr*)servaddr, sizeof(*servaddr)) < 0) {
		printf ("Connection with the server failed!\n");
		exit(0);
	}
	printf("Connected to the server!\n");
	return listening_socket;
}

void command_execution(int listening_socket, char* command)
{
	int n;
	int issend, isrecv;

	//Send some data
	issend = send(listening_socket, command, COMMAND_SIZE, 0);
	if(issend < 0) {
		puts("Send failed\n");
		return ;
	}

	//Receive a reply from the server
	memset(command, '\0' , COMMAND_SIZE);
	isrecv = recv(listening_socket, command, COMMAND_SIZE , 0);
	if(isrecv < 0) {
		puts("recv failed\n");
		return ;
	}

	printf ("Server reply -> %s\n", command);
    if ((strncmp(command, "disconnect", 10)) == 0) {
		printf("Disconncted...\n");
    }
}

int ip_validation(char *ip)
{
    int num; 
    int flag = 1; 
    int counter=0; 
    char* p = strtok(ip, "."); 
     
    while (p && flag ){ 
        num = atoi(p); 
     
        if (num >=0 && num<=255 && (counter++ < 4)) { 
            flag=1; 
            p=strtok(NULL,"."); 
     
        } 
        else{ 
            flag=0; 
            break; 
        } 
    } 
     
    return counter == 4;
}

void all_exit()
{
	if (is_connected) {
    	send(listening_socket, "disconnect", 10, 0);
	    close (listening_socket);
	    is_connected = 0;
    }
	free(p_command);
}

void  INThandler(int sig)
{
	char  c;

	signal(sig, SIG_IGN);
	printf("\nOUCH, did you hit Ctrl-C?\n"
	    "Do you really want to quit? [Y/n] ");
	c = getchar();
	if (c == 'y' || c == 'Y' || c == '\n') {
		all_exit();
		exit(0);
	}
	else {
		signal(SIGINT, INThandler);
	}
    printf("$ ");
	getchar(); // Get new line character
}

void help()
{
    puts("Hey :D I will help You!\n"
         "connect 'IP' 'PORT'     connecting to mentioned IP's PORT, for example\n"
         "                        connect 127.0.0.1 8080\n"
         "get_time                get servers current time\n"
         "disconnect              disconnect from connected server\n"
         "exit                    disconnect from connected server and close the program\n"
         "help                    information about commands");
}
