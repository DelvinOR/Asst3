#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

int main(int argc, char**argv){
    
    if(argc != 3){
        // Too little or too many arguments when running C file
        printf("Not enough arguments\n");
        return -1;
    }

    char* firstArg = argv[1];
    int network_socket, PORT;

    PORT =  atoi(argv[2]);

    if (network_socket = socket(AF_INET, SOCK_STREAM, 0) < 0){
        // error
        printf("Error\n");
        return -1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;



    return 0;
}