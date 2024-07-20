#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int socket_file_descriptor, connection, bind_status, connection_status;
#define PORT 9000
#define MESSAGE_LENGTH 8196
#define REC_FILE "/var/tmp/aesdsocketdata"

struct sockaddr_in servAddr;

struct addrinfo *clientInfo;
struct addrinfo hints;
socklen_t length;
int fd;

int main (int argc, char *argv[]){
    char buffer[MESSAGE_LENGTH];
    // Creating the Socket
    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_file_descriptor == -1){
        printf("Scoket creation failed.!\n");
        return -1;
    }

    
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    
    int retVal = inet_pton(AF_INET, "127.0.0.1", 
    &servAddr.sin_addr.s_addr);
    if (retVal == 0)
        printf("inet_pton() failed - invalid address string");
    else if (retVal < 0)
        printf("inet_pton() failed");
    servAddr.sin_port = htons(PORT);

    
    

    // Establish the connection to the echo server
    if (connect(socket_file_descriptor, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        printf("connect() failed");

    
    while(1){   
        while(1){
            printf("Waiting user input\n"); 
            fgets(buffer, MESSAGE_LENGTH, stdin);
            if(strcmp(buffer, "exit\n") == 0 ){
                break;
            }

            if(buffer[strlen(buffer)-2] == '*'){
                send(socket_file_descriptor, buffer, strlen(buffer)-2, 0);    
            }
            else{
                send(socket_file_descriptor, buffer, strlen(buffer), 0);
                break;
            }
        }

        int count = 1;
        while( count != 0){
            int nrec = recv(socket_file_descriptor, buffer, MESSAGE_LENGTH, 0);
            buffer[nrec] = '\0';
            printf("recv: %s", buffer);
            usleep(5000);
            ioctl(socket_file_descriptor, FIONREAD, &count);
        }
    }
    close(socket_file_descriptor);
    

 return 0;
}

