#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <ctype.h>


struct messageBox{
    char name[26];
    struct message * messageQ;
    int currentUser; // make this equal to the socket that requested access
    struct messageBox * nextMessageBox;
};

struct message{
    char * text;
    struct message * nextMsg;
};

char* clientCommand;

struct messageBox * messageBoxStore;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int boxAlreadyExist(struct messageBox * boxStorePointer, char * arg0){
    
    if(boxStorePointer == NULL){
        return 1;
    }

    struct messageBox * ptr  = boxStorePointer;
    while(ptr != NULL){
        if(strcmp(arg0, ptr->name) == 0){
            return 0;
        }
        ptr = ptr->nextMessageBox;
    }

    return 1;

}

void createMessageBox(char * arg0){
    
    if(messageBoxStore == NULL){
        messageBoxStore = (struct messageBox*)malloc(sizeof(struct messageBox));
        strcpy(messageBoxStore -> name, arg0);
        messageBoxStore ->currentUser = -1;
        return;
    }

    struct messageBox * ptr = messageBoxStore;
    while(ptr -> nextMessageBox!= NULL){
        ptr -> nextMessageBox;
    }

    ptr -> nextMessageBox = (struct messageBox*) malloc(sizeof(struct messageBox));
    strcpy(ptr -> nextMessageBox ->name, arg0);
    ptr->nextMessageBox->currentUser = -1;
    return;
    
}

void * requestThread(void * arg){
    int cSoc = *((int *) arg);
    
    while(1){
        recv(cSoc, clientCommand, 200, 0);
        if (strcmp(clientCommand,"") == 0){
            // User did not enter anything 
            perror("please enter a command\n");
        }else if(strcmp(clientCommand,"HELLO") ==  0){
            char * serverHello = (char*)malloc(19);
            strcpy(serverHello, "HELLO DUMBv0 ready!");
            send(cSoc, serverHello, 19, 0);
            free(serverHello);
        }else if(strcmp(clientCommand, "GDBYE")){
            return NULL;
        }else if(memcmp(clientCommand, "CREAT", 5) == 0){

            pthread_mutex_lock(&lock);
            if(clientCommand[5] != ' '){
                send(cSoc, "ER:WHAT?",9,0);
                pthread_mutex_unlock(&lock);
                return NULL;
                
            }

            char * arg0 = strtok(clientCommand, " ");
            arg0 = strtok(NULL, " ");

            if(strlen(arg0) < 5 || strlen(arg0) > 25){
                send(cSoc, "ER:WHAT?",9,0);
                pthread_mutex_unlock(&lock);
                return NULL;
                
            }

            if(!isalpha(arg0[0])){
                send(cSoc, "ER:WHAT?",9,0);
                pthread_mutex_unlock(&lock);
                return NULL;
                
            }

            if(boxAlreadyExist(messageBoxStore, arg0) == 0){
                send(cSoc, "ER:EXIST?",9,0);
                pthread_mutex_unlock(&lock);
                return NULL;
                
            }else{
                // create the new messageBoxStore
                createMessageBox(arg0);
                send(cSoc, "OK!",4,0);
                pthread_mutex_unlock(&lock);
                return NULL;

            }
        }else if(memcmp(clientCommand, "OPNBX",5) == 0){
            char * arg0 = strtok(clientCommand, " ");
            arg0 = strtok(NULL, " ");

            if(strlen(arg0) < 5 || strlen(arg0) > 25){
                send(cSoc, "ER:WHAT?",9,0);
                return NULL;
                
            }

            if(!isalpha(arg0[0])){
                send(cSoc, "ER:WHAT?",9,0);
                return NULL;
            }

            if(boxAlreadyExist(messageBoxStore, arg0) == 0){
                struct messageBox * ptr = messageBoxStore;
                while(ptr != NULL){
                    if(strcmp(ptr->name, arg0) == 0){
                        break;
                    }
                    ptr -> nextMessageBox;
                }

                if(ptr -> currentUser != cSoc){
                    send(cSoc, "ER:OPEND",9,0);
                    return NULL;
                }

                if(ptr -> currentUser == -1){
                    ptr ->currentUser = cSoc;
                    send(cSoc, "OK!", 4, 0);
                    return NULL;
                }

            }else{
                if(boxAlreadyExist(messageBoxStore,arg0) == 1){
                    send(cSoc, "ER:NEXST",9,0);
                    return NULL;
                }
            }
        }else if(strcmp(clientCommand, "NXTMG") == 0){
            // FIND THE MESSAGE BOX THAT THIS CURRENT USER HAS AND RETURN AND DELETE THE MOST CURRENT MESSAGE
            struct messageBox * ptr = messageBoxStore;
            while(ptr != NULL){
                if(ptr -> currentUser == cSoc){
                    break;
                }
                ptr -> nextMessageBox;
            }
            if(ptr == NULL){
            // user does not have a message box open
                send(cSoc, "ER:NOOPN",9,0);
                return NULL;
            }

            struct message * temp = ptr ->messageQ;
            if(temp == NULL){
                // no messageQ exists meaning there are no messages
                send(cSoc, "ER:EMPTY",9,0);
                return NULL;
            }
            char * messageCpy;
            strcpy(messageCpy, temp -> text);
            ptr -> messageQ = ptr ->messageQ->nextMsg;

            int messageCpyLength = strlen(messageCpy);
            char * res;
            strcpy(res, "OK!");
            char * ml;
            sprintf(ml, "%d\n", messageCpyLength);
            strcat(res,ml);
            strcat(res,"!");
            strcat(res,messageCpy);
            send(cSoc, res,sizeof(res),0);
            free(temp);
            free(messageCpy);
            return NULL;

        }else if(memcmp(clientCommand, "PUTMG",5) == 0){
            if(clientCommand[5] != '!'){
                send(cSoc, "ER:WHAT?",9,0);
                return NULL;
            }

            char * arg0 = strtok(clientCommand, "!");

            int sizeOfMessageToBeAdded = atoi(arg0);

            arg0 = strtok(NULL, "!");
            if(sizeOfMessageToBeAdded != strlen(arg0)){
                send(cSoc, "ER:WHAT?",9,0);
                return NULL;
            }
            char * message;
            strcpy(message,arg0);

            // now add this message to the end of the current open box
            struct messageBox * ptr = messageBoxStore;
            while(ptr != NULL){
                if(ptr -> currentUser == cSoc){
                    break;
                }
                ptr -> nextMessageBox;
            }
            if(ptr == NULL){
            // user does not have a message box open
                send(cSoc, "ER:NOOPN",9,0);
                return NULL;
            }

            struct message * qPtr = ptr ->messageQ;
            while(qPtr != NULL){
                qPtr -> nextMsg;
            }

            qPtr = (struct message *) malloc (sizeof(struct message));
            qPtr ->text = (char*) malloc(sizeOfMessageToBeAdded + 1);
            strcpy(qPtr -> text, message);
            char * res;
            strcpy(res, "OK!");
            char *sMTOA;
            sprintf(sMTOA, "%d\n", sizeOfMessageToBeAdded);
            strcat(res,sMTOA);
            send(cSoc, res, strlen(res), 0);
            return NULL;

        }else if(memcmp(clientCommand, "DELBX", 5) == 0){
            if(clientCommand[5] != ' '){
                send(cSoc, "ER:WHAT?", 9, 0);
                return NULL;
            }

            char * boxToBeDeleted = strtok(clientCommand, " ");
            boxToBeDeleted = strtok(NULL, " ");

            if(!isalpha(boxToBeDeleted[0])){
                send(cSoc, "ER:WHAT?", 9, 0);
                return NULL;
            }

            if(boxAlreadyExist(messageBoxStore,boxToBeDeleted) == 1){
                send(cSoc, "ER:NEXST", 9, 0);
                return NULL;
            }

            pthread_mutex_lock(&lock);

            // delete boxToBeDeleted from messageBoxStore
            struct messageBox * storePtr = messageBoxStore;
            struct messageBox * prev = NULL;
            while(storePtr != NULL){
                if(strcmp(boxToBeDeleted, storePtr -> name) == 0){
                    break;
                }
                prev = storePtr;
                storePtr -> nextMessageBox;
            } 

            if(storePtr == NULL){
                // NO BOX EXISTS IN THE STORE
                send(cSoc, "ER:NEXST", 9, 0);
                pthread_mutex_unlock(&lock);
                return NULL;
            }

            if(storePtr -> currentUser != -1){
                send(cSoc, "ER:OPEND", 9, 0);
                pthread_mutex_unlock(&lock);
                return NULL;
            }

            // there are no messages on the queue so you can delete this box
            if(storePtr -> messageQ != NULL){
                send(cSoc, "ER:NOTMT", 9, 0);
                pthread_mutex_unlock(&lock);
                return NULL;
            }

            // delete this box
            if(prev == NULL){
                // box to be deleted is first box
                prev = storePtr;
                storePtr = storePtr->nextMessageBox;
                free(prev);
                send(cSoc, "OK!", 4, 0);
                pthread_mutex_unlock(&lock);
                return NULL;
            }

            prev -> nextMessageBox = storePtr -> nextMessageBox;
            storePtr -> nextMessageBox = NULL;
            free(storePtr);
            send(cSoc, "OK!", 4, 0);
            pthread_mutex_unlock(&lock);
            return NULL;

        }else{
            if(memcmp(clientCommand, "CLSBX", 5) == 0){
                if(clientCommand[5] != ' '){
                    send(cSoc, "ER:WHAT?", 9, 0);
                    return NULL;
                }

                char * boxName = strtok(boxName, " ");
                boxName = strtok(NULL, " ");

                if(boxAlreadyExist(messageBoxStore,boxName) == 1){
                    send(cSoc, "ER:NEXST", 9, 0);
                    return NULL;
                }

                struct messageBox * messageBoxStorePtr = messageBoxStore;
                while(messageBoxStorePtr != NULL){
                    if(strcmp(messageBoxStorePtr->name, boxName) == 0){
                        break;
                    }
                    messageBoxStorePtr = messageBoxStorePtr->nextMessageBox;
                }
                
                if(messageBoxStorePtr == NULL){
                    send(cSoc, "ER:NOOPN",9, 0);
                    return NULL;
                }

                if(messageBoxStorePtr -> currentUser != cSoc){
                    send(cSoc, "ER:NOOPN",9, 0);
                    return NULL;
                }

                messageBoxStorePtr -> currentUser = -1;
                send(cSoc, "OK!", 4, 0);
                return NULL;
            }else{
                // command does not exist
                send(cSoc, "ER:WHAT?", 9, 0);
                return NULL;
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

    //messageBoxStore = (struct messageBox *) malloc(sizeof(struct messageBox));

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
    if((bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address))) < 0){
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