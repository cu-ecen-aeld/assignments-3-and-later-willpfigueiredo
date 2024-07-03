#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>


int socket_file_descriptor, connection, bind_status, connection_status;
#define PORT "9000"
#define MESSAGE_LENGTH 8196
#define REC_FILE "/var/tmp/aesdsocketdata"

struct sockaddr serveraddress, client;

struct addrinfo *servinfo;
struct addrinfo hints;
socklen_t length;
int fd;

static void signal_handler(int signal_number);

int main (int argc, char *argv[]){
    openlog(NULL, 0, LOG_USER);
    int do_fork = 0;

    if(argc == 2  && strcmp(argv[1],"-d")==0){
        do_fork = 1;
    }

    // Creating the Socket
    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_file_descriptor == -1){
        syslog(LOG_ERR, "Scoket creation failed.!\n");
        return -1;
    }

    // Protocol family
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    //getting addr info
    int info_ret = getaddrinfo(NULL, PORT, &hints, &servinfo);
    if(info_ret != 0){
        printf("getaddrinfo error: %s", gai_strerror(info_ret));
        return -1;
    }

    // Binding the newly created socket with the given Ip Address
    bind_status = bind(socket_file_descriptor, servinfo->ai_addr, sizeof(*(servinfo->ai_addr)));
    if(bind_status == -1){
        printf("Socket binding failed.!\n");
        return -1;
    }

     //fazer o fork aqui
    if(do_fork){
        pid_t p = fork();
        
            if( p < 0 ){
                printf("Fork gone wrong\n");
                return -1;
            }
            else if( p > 0){
                printf("This is the father process and will close\n");
                return 0;
            }
    }

   

    struct sigaction new_action;
    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = signal_handler;

    if(sigaction(SIGTERM, &new_action, NULL) != 0){
        printf("Error %d (%s) registering for SIGTERM", errno, strerror(errno));
        return -1;
    }
    if(sigaction(SIGINT, &new_action, NULL) != 0){
        printf("Error %d (%s) registering for SIGINT", errno, strerror(errno));
        return -1;
    }


    // Server is listening for new creation
    connection_status = listen(socket_file_descriptor, 5);
    if(connection_status == -1){
        printf("Socket is unable to listen for new connections.!\n");
        return -1;
    }else{
        printf("Server is listening for new connection: \n");
    }
    length = sizeof(client);

    fd  = open("/var/tmp/aesdsocketdata", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH );
            if(fd < 0){
                printf("Error creating the file for writing\n");
                return -1;
            }
        close(fd);
    
    
    while(1){
        printf("Waiting to accept connection: \n");
        connection = accept(socket_file_descriptor, &client, &length);
        if(connection == -1){
            printf("Server is unable to accept the data from client.!\n");
            return -1;
        }
        else{
            printf("Connection accepted\n");
        }
        
        char clntAddr[INET_ADDRSTRLEN];
        if(inet_ntop(AF_INET, &((struct sockaddr_in*)&client)->sin_addr.s_addr, clntAddr, sizeof(clntAddr))!= NULL){
            syslog(LOG_SYSLOG, "Accepted connection from %s\n", clntAddr);
        }
        else{
            printf("Unable to get client address");
        }

        
        

        
        while(1){
            int break_again = 0;
        //open file to write recevied chars
            fd  = open("/var/tmp/aesdsocketdata", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH );

            if(fd < 0){
                printf("Error opening the file for writing errno: %d\n", errno);
                return -1;
            }
            char rec_buffer[MESSAGE_LENGTH];
            ssize_t nRec = 0;
            do{
                nRec = recv(connection, rec_buffer, MESSAGE_LENGTH, 0);
                if(nRec <= 0){
                    break_again = 1;
                    break;
                }
                int bytesWritten;
                bytesWritten = write(fd, rec_buffer, nRec);
            }while(rec_buffer[nRec-1] != '\n');

            close(fd);
            if(break_again){
                syslog(LOG_SYSLOG, "Closed connection from %s\n", clntAddr);
                break;
            }

            fd  = open("/var/tmp/aesdsocketdata", O_RDONLY, 0 );
            if(fd < 0){
                printf("Error openning file for reading\n");
                return -1;
            }
            FILE *fs;
            fs = fdopen(fd, "r");
            while (fgets(rec_buffer, sizeof(rec_buffer), fs) != NULL) {
                int nSent = send(connection, rec_buffer, strlen(rec_buffer), 0);
                if(nSent < 0){
                    break_again = 1;
                    break;
                }
            }
            close(fd);

            if(break_again){
                syslog(LOG_SYSLOG, "Closed connection from %s\n", clntAddr);
                printf("Connection closed\n");
                break;
            }
        }
    }


    return 0;
}

static void signal_handler(int signal_number){
    syslog(LOG_SYSLOG,"Caught signal, exiting\n");
    close(fd);
    close(connection);
    close(socket_file_descriptor);
    remove("/var/tmp/aesdsocketdata");
    exit(0); 
    

}

