#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

int main(int argc, char**argv){
    // On start the server should be invoked with a port number

    if(argc != 2){
        printf("Error in arguments\n");
        return -1;
    }

    char** messageBoxStore;

    int server_socket, PORT;
    
    server_socket = socket(AF_INET, SOCK_STREAM,0);
    PORT = atoi(argv[1]);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to our specified IP and PORT
    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    



    
    return 0;
}