#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include <string.h>


struct messageBox{
    char name[26];
    struct message * messageQ;
    int available;
    struct messageBox * nextMessageBox;
};

struct message{
    char text[200];
    struct message * nextMsg;
};

char* clientCommand;

struct messageBox * messageBoxStore;

void * requestThread(void * arg){
    int cSoc = *((int *) arg);
    
    while(1){
        recv(cSoc, clientCommand, 200, 0);
        if (strcmp(clientCommand,NULL) == 0){
            // User did not enter anything 
            perror("please enter a command\n");
        }else if(strcmp(clientCommand,"HELLO") ==  0){
            char * serverHello = (char*)malloc(19);
            strcpy(serverHello, "HELLO DUMBv0 ready!");
            send(cSoc, serverHello, 19, 0);
            free(serverHello);
        }else if(strcmp(clientCommand, "GDBYE")){
            close(cSoc);
            return;
        }else if(memcmp(clientCommand, "CREAT", 5) == 0){

        }else if(memcmp(clientCommand, "OPNBX",5) == 0){

        }else if(strcmp(clientCommand, "NXTMG") == 0){

        }else if(memcmp(clientCommand, "PUTMG",5) == 0){

        }else if(memcmp(clientCommand, "DELBX", 5) == 0){

        }else{
            if(memcmp(clientCommand, "CLSBX", 5) == 0){

            }
        }
    }
    
} 

int main(int argc, char**argv){
    
    
    // On start the server should be invoked with a port number
    if(argc != 2){
        perror("Error in arguments\n");
        return -1;
    }

    messageBoxStore = (struct messageBox *) malloc(sizeof(struct messageBox));

    int server_socket, client_socket, PORT;
    struct sockaddr_storage serverStorage;

    if((server_socket = socket(AF_INET, SOCK_STREAM,0)) < 0){
        perror("Error in creating server socket\n");
        return -1;
    }
    PORT = atoi(argv[1]);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to our specified IP and PORT
    if((bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address))) == 0){
        perror("bind failed\n");
        return -1;
    }

    // Size connection allowed to server is set to 15
    if(listen(server_socket, 15) < 0){
        perror("listen error\n");
        return -1;
    }

    // The second parameter in the accept() is the client's address
    socklen_t addrlen = sizeof(serverStorage);
    pthread_t threads[20];
    int i = 0;
    while(1){

        // Create new socket for every request
        // serverStorage holds the address of the client
        if((client_socket = accept(server_socket, (struct sockaddr*)&serverStorage, &addrlen)) < 0 ){
            perror("Error in aceepting new socket\n");
            return -1;
        }

        // On a connection request, the server needs to create a new thread to handle all client requessts
        if(pthread_create(&threads[i], NULL, requestThread, &client_socket) != 0){
            perror("Error in creating request thread\n");
        }

        if( i >= 20){
            // we have reached the maximums threads that there are to create so now we must wait for all of them to finish
            i = 0;
            while(i < 20){
                pthread_join(threads[i++],NULL);
            }
            // we set i to 0 again so that we can continue to listen for more requests
            i = 0;
        }
    }

    close(server_socket);
    close(client_socket);
    return 0;
}