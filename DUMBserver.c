#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

int main(int argc, char**argv){
    // On start the server should be invoked with a port number

    if(argc != 2){
        perror("Error in arguments\n");
        return -1;
    }

    char** messageBoxStore;

    int server_socket, PORT;
    
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





    
    return 0;
}