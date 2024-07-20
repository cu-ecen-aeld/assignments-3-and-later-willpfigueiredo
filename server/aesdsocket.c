
#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>


#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <sys/stat.h>
#include <syslog.h>


#include <time.h>
#include <sys/time.h>
#include <signal.h>

#include <pthread.h>


int socket_file_descriptor, bind_status, connection_status;
#define PORT "9000"
#define MESSAGE_LENGTH 128
#define REC_FILE "/var/tmp/aesdsocketdata"

void array2str(char * input, int ilen, char* out){
    for(int i = 0; i < ilen; ++i){
        out[i] = input[i];
    }
    out[ilen] = '\0';
}

void printArr(char * input, int ilen){
    for(int i = 0; i < ilen; ++i){
        printf("%c", input[i]);
    }
    printf("\n");
}

struct listNodeRcv{
    char * string;
    int length;
    struct listNodeRcv *next;
};

void addNodeRcv(struct listNodeRcv ** headRcv, struct listNodeRcv ** tailRcv, char * buffer, int length){
    struct listNodeRcv *newNode = malloc(sizeof(struct listNodeRcv));
    newNode->length = length;
    newNode->next = NULL;
    newNode->string = malloc(length);
    for(int i = 0; i < length; ++i){
        newNode->string[i] = buffer[i];
    }

    //printf("%ld: Adding node len: %d - ", pthread_self(), newNode->length);
    //printArr(newNode->string, newNode->length);
    
    if(*headRcv == NULL){
        *headRcv = newNode;
        *tailRcv = newNode;
    }
    else{
        (*tailRcv)->next = newNode;
        *tailRcv = newNode;
    }
}

void dellNodesRcv(struct listNodeRcv *headRcv){
    
    while(headRcv != NULL){
        struct listNodeRcv *temp = headRcv;
        headRcv = headRcv->next;
        free(temp->string);
        free(temp);
    }
}


struct listNode{
    pthread_t thread;
    int connection;
    int done;
    struct sockaddr cAddr;
    struct listNode *next;
};
//int nNodes;
struct listNode *head = NULL;
struct listNode *tail = NULL;

void addNode(pthread_t thread, int cnt){
    struct listNode *newNode = malloc(sizeof(struct listNode));
    newNode->thread = thread;
    newNode->connection = cnt;
    newNode->done = 0;
    newNode->next = NULL;
    
    if(head == NULL){
        head = newNode;
        tail = newNode;
    }
    else{
        tail->next = newNode;
        tail = newNode;
    }
}

void finishNode(struct listNode *node){
    close(node->connection);
    pthread_cancel(node->thread);
    pthread_join (node->thread, NULL);
    free(node);
}

void clearList(){
   
        
    struct listNode *temp = head;
    while(temp != NULL){
        if(temp->done == 1){
            //printf("Joning: %ld - ", temp->thread);
            pthread_join(temp->thread, NULL);
            temp->done = 2;
        }
        //printf("\n");
        temp = temp->next;
    }
    
    
    // while(head != NULL && head->done == 1){
    //     if(head->next != NULL){
    //         finishNode(head);
    //         head = NULL;
    //         tail = NULL;
    //         return;
    //     }
    //     else{
    //         struct listNode *temp = head->next;
    //         finishNode(head);
    //         head = temp;
    //     }
    // }
    
    // struct listNode *tempBef = head;
    // struct listNode *temp = head->next;
    
    // while(temp != NULL){
        
    //     if(temp->done == 1){
    //         tempBef->next = temp->next;
    //         if(temp->next == NULL){
    //             tail = tempBef;
    //         }
    //         finishNode(temp);
    //         temp = tempBef->next;
    //     }
    //     else{
    //         tempBef = tempBef->next;
    //         temp = tempBef->next;
    //     }
        
    // }
    
}

void markFinishedThread(pthread_t thr){
    struct listNode *temp = head;
    while(temp != NULL){
        if(temp->thread == thr){
            temp->done = 1;
        }
        temp = temp->next;
    }
}



void dellNodes(){
    
    while(head != NULL){
        struct listNode *temp = head;
        head = head->next;
        if(temp->done == 0){
            close(temp->connection);
            pthread_cancel(temp->thread);
        }
        if(temp->done != 2 ){
            close(temp->connection);
            
            pthread_join (temp->thread, NULL);
        }
        free(temp);
    }
}

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct sockaddr serveraddress;

struct addrinfo *servinfo;
struct addrinfo hints;
socklen_t length;
int fd;
pthread_t timerThread = 0;

static void signal_handler(int signal_number);
void *work_comm(void *cnt);
void * do_timestamp(void *arg);

int main (int argc, char *argv[]){
    openlog(NULL, 0, LOG_USER);
    int do_fork = 0;
    

    if(argc == 2  && strcmp(argv[1],"-d")==0){
        do_fork = 1;
    }

    // Creating the Socket
    socket_file_descriptor = socket(PF_INET, SOCK_STREAM, 0);
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
    if(info_ret == -1){
        printf("getaddrinfo error: %s", gai_strerror(info_ret));
        return -1;
    }

    int reuseAddr = 1;
    int ret_opt = setsockopt(socket_file_descriptor,
                SOL_SOCKET,
                SO_REUSEADDR,
                &reuseAddr,
                sizeof (reuseAddr));
    if(ret_opt != 0){
        printf("Error setting socket options\n");
    }

    // Binding the newly created socket with the given Ip Address
    bind_status = bind(socket_file_descriptor, servinfo->ai_addr, servinfo->ai_addrlen);
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
    memset(&new_action, 0, sizeof(new_action));
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
    connection_status = listen(socket_file_descriptor, 10);
    if(connection_status == -1){
        printf("Socket is unable to listen for new connections.!\n");
        return -1;
    }else{
        printf("Server is listening for new connection: \n");
    }
    

    fd  = open("/var/tmp/aesdsocketdata", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH );
            if(fd < 0){
                printf("Error creating the file for writing\n");
                return -1;
            }

    
    pthread_create(&timerThread, NULL, do_timestamp, NULL);
    
    while(1){
        printf("Waiting to accept connection: \n");

        struct listNode *new_node = malloc(sizeof(struct listNode));
        length = sizeof(new_node->cAddr);
        int connection = accept(socket_file_descriptor, &(new_node->cAddr), &length);
        if(connection == -1){
            printf("Server is unable to accept the data from client.!\n");
            return -1;
        }

        pthread_t commThread = 0;
        new_node->connection = connection;
        pthread_create(&commThread, NULL, work_comm, (void*)new_node);
        addNode(commThread, connection);

        clearList();

    }
        return 0;
}

void *work_comm(void *cnt){

    struct listNode *conn_info = (struct listNode*)(cnt);
    
    int connection = conn_info->connection;
    
    char clntAddr[INET_ADDRSTRLEN];
    if(inet_ntop(AF_INET, &((struct sockaddr_in*)&(conn_info->cAddr))->sin_addr.s_addr, clntAddr, INET_ADDRSTRLEN)){
        syslog(LOG_SYSLOG, "Accepted connection from %s\n", clntAddr);
        printf("%ld: Accepted connection from %s\n", pthread_self(), clntAddr);
    }

    free(conn_info);
    // else{
    //     printf("Unable to get client address");
    // }
    char rec_buffer[MESSAGE_LENGTH];

    while(1){
        int break_again = 0;
        int old_s;

        struct listNodeRcv *headRcv = NULL;
        struct listNodeRcv *tailRcv = NULL;

        
        ssize_t nRec = 0;
        do{
            nRec = recv(connection, rec_buffer, MESSAGE_LENGTH, 0);
            if(nRec <= 0){
                if(nRec < 0){
                    printf("%ld: Socket nrec: %ld errno: %d\n",pthread_self(), nRec, errno);
                }
                break_again = 1;
                break;
            }
            printf("%ld: Received len: %ld - ",pthread_self(), nRec);
            printArr(rec_buffer, nRec);
            addNodeRcv(&headRcv, &tailRcv, rec_buffer, nRec);
        }while(rec_buffer[nRec-1] != '\n');

        struct listNodeRcv *temp = headRcv;
        
        //close(fd);
        if(break_again){
            syslog(LOG_SYSLOG, "Closed connection from %s\n", clntAddr);
            break;
        }
        
        pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &old_s);
        //printf("%ld: Waiting for mutex\n", pthread_self());
        pthread_mutex_lock(&mutex);
        //printf("%ld: Locked mutex\n", pthread_self());
        while(temp != NULL){
            //printf("%ld: fwrite len %d - ", pthread_self(), temp->length);
            //printArr(temp->string, temp->length);   
            write(fd, temp->string, temp->length);
            temp = temp->next;
        }
        pthread_mutex_unlock(&mutex);
        //printf("%ld: Released mutex\n", pthread_self());

        dellNodesRcv(headRcv);
        pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, &old_s);

        

        int fdr  = open("/var/tmp/aesdsocketdata", O_RDONLY, 0 );
        if(fdr < 0){
            printf("Error openning file for reading\n");
            //return -1;
        }
        FILE *fs;
        fs = fdopen(fdr, "r");
        //printf("\n----\n");
        pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &old_s);
        //printf("%ld: Waiting for mutex yo write\n", pthread_self());
        //pthread_mutex_lock(&mutex);
        //printf("%ld: Locked mutex to write\n", pthread_self());
        while (fgets(rec_buffer, sizeof(rec_buffer), fs) != NULL) {
            int nSent = send(connection, rec_buffer, strlen(rec_buffer), 0);
            //printf("Writing: %s", rec_buffer);
            if(nSent < 0){
                break_again = 1;
                break;
            }
        }
        //pthread_mutex_unlock(&mutex);
        //printf("%ld: Released mutex to write\n", pthread_self());
        pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, &old_s);
        //printf("\n----\n");
        //close(fs);
        close(fdr);
        //liberar mutex aqui

        if(break_again){
            syslog(LOG_SYSLOG, "Closed connection from %s\n", clntAddr);
            printf("Connection closed\n");
            break;
        }
    }
    
    printf("%ld: Exiting thread\n", pthread_self());
    close(connection);
    markFinishedThread(pthread_self());
    pthread_exit(0);
}

void * do_timestamp(void* arg){
    (void)arg;
    struct timeval tv;
    struct tm* ptm;
    char time_string[50];

    while(1){
        /* Obtain the time of day, and convert it to a tm struct. */
        gettimeofday (&tv, NULL);
        ptm = localtime (&tv.tv_sec);
        /* Format the date and time, down to a single second. */
        strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);
        int len = strlen(time_string);
        time_string[len] = '\n';
        time_string[len+1] = '\0';

        /* Print the formatted time, in seconds, followed by a decimal point
        and the milliseconds. */
        pthread_mutex_lock(&mutex);
        write(fd,"timestamp:", 10);
        write(fd, time_string, len+1);
        //write(fd, '\n', 1);
        pthread_mutex_unlock(&mutex);
        printf ("%s\n", time_string);
        sleep(10);
    }
}

static void signal_handler(int signal_number){
    (void)signal_number;
    syslog(LOG_SYSLOG,"Caught signal, exiting\n");
    
    

    //close(connection);
    printf("Deleting nodes\n");
    dellNodes();
    printf("Closing socket file descriptor\n");
    close(socket_file_descriptor);
    printf("closing write file\n");
    close(fd);
    remove("/var/tmp/aesdsocketdata");

    pthread_cancel(timerThread);
    pthread_join (timerThread, NULL);

    exit(0); 
    

}

