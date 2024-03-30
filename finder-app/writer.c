#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>

int main (int argc, char *argv[]){

    openlog(NULL, 0, LOG_USER);

    if(argc != 3){
        printf("Use: writer [path] [string]\n");
        syslog(LOG_ERR, "Invalid number of arguments: %d", argc);
        return 1;
    }

    int fd  = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH );
    if(fd == -1){
        printf("Error while opening file!! Errno: %d\n", errno);
        syslog(LOG_ERR, "Error while opening file!! Errno: %d\n", errno);
        return 1;
    }
    syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
    int bytesWritten = write(fd, argv[2], strlen(argv[2]));

    if(bytesWritten == -1){
        printf("Error during file writing. Errno: %d\n", errno);
        syslog(LOG_ERR, "Error during file writing. Errno: %d\n", errno);
        return 1;
    }

    close(fd);
    
    

    return 0;
}