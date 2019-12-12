#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

int main(int argc, char**argv){
    
    if(argc < 3){
        // Too little or too many arguments when running C file
        perror("Not enough arguments\n");
		printf("DUMBclient <address> <port>\n");
        return -1;
    }

    //char* firstArg = argv[1];
    int csoc, PORT;

    PORT =  atoi(argv[2]);

    if ((csoc = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        // error
        printf("Error in creating socket!\n");
        return -1;
    }

    struct sockaddr_in sa;//gethostbyname, gethostbyaddr
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    //sa.sin_addr = gethostbyname(argv[1])->h_addr_list[0];
	sa.sin_addr.s_addr = INADDR_ANY;
	int i;
	int succ = 0;
	for (i = 0; i < 3; i++){
		if(connect(csoc, (struct sockaddr*)&sa, sizeof(sa)) != -1){
			succ = 1;
			break;
		}
	}
	
	if (succ != 1){
		printf("Error connecting to server!\n");
		close(csoc);
        return -1;
	}
	//We are connected, send hello
	char HLO[] = "HELLO";
	if (send(csoc, HLO, 6, 0) < 0){
		perror("Failed to send message!\n");
		shutdown(csoc, SHUT_RDWR);
		close(csoc);
        return -1;
	}
	printf("HELLO\n");
	char buff[4096];//make buffer
	memset(buff, 0, 4096);//initialize buffer to be empty
	if (recv(csoc, buff, 4096, 0) < 0){
		printf("Failed to recv message!\n");
		shutdown(csoc, SHUT_RDWR);
		close(csoc);
        return -1;
	}
	if (strcmp(buff, "HELLO DUMBv0 ready!") != 0){//error
		printf("Invalid server response recieved:\n");
		printf("%s\n", buff);
		shutdown(csoc, SHUT_RDWR);
		close(csoc);
		return -1;
	}
	printf("%s\n", buff);
	//proper response received, begin command loop
	char input[4096];
	int lostc = 0;

	while (1){
		printf("Enter command:\n");
		scanf("%s", input);
		if (strcmp(input, "quit") == 0){
			char msg[] = "GDBYE";
			if (send(csoc, msg, 6, 0) <= 0){
				perror("Failed to send message!\n");
				break;
			}
			printf("GDBYE\n");
			if (recv(csoc, buff, 4096, 0) > 0){//received a message erroneously 
				printf("GDBYE Failed:\n");
				printf("%s\n", buff);
			}
			else{//(server shutsdown socket, so never receive deadlock)
				close(csoc);
				printf("Success!\n");
				return 1;
			}			
		}
		else if (strcmp(input, "create") == 0){
			printf("Okay, what's the name of the box you wish to create?\n");
			printf("create:> ");
			scanf("%s", input);
			char msg[4096] = "CREAT ";
			strncat(msg, input, 4095 - 6);
			if (send(csoc, msg, strlen(msg)+1, 0) <= 0){//+1 for \0
				perror("Failed to send message!\n");
				break;
			}
			//printf("CREAT\n");
			if (recv(csoc, buff, 4096, 0) < 0){//error
				printf("No response!:\n");
				lostc = 1;
				break;
			}
			if (strcmp(buff, "OK!") == 0){//box created!
				printf("Success! Message box '%s' created.\n", input);
			}
			else if (strcmp(buff, "ER:EXIST") == 0){//box already exists
				printf("Error! Message box '%s' already exists.\n", input);
			}
			else if (strcmp(buff, "ER:WHAT?") == 0){//command failed
				printf("Error! Command failed '%s'.\n", input);
			}
			else{//received incorrect response
				perror("Invalid response from server.\n");
			}
		}
		else if (strcmp(input, "delete") == 0){
			printf("Okay, what's the name of the box you wish to delete?\n");
			printf("delete:> ");
			scanf("%s", input);
			char msg[4096] = "DELBX ";
			strncat(msg, input, 4095 - 6);
			if (send(csoc, msg, strlen(msg)+1, 0) <= 0){
				perror("Failed to send message!\n");
				break;
			}
			//printf("CREAT\n");
			if (recv(csoc, buff, 4096, 0) < 0){//error
				printf("No response!\n");//socket closed? Connection lost?
				lostc = 1;
				break;
			}
			if (strcmp(buff, "OK!") == 0){//box deleted!
				printf("Success! Message box '%s' deleted.\n", input);
			}
			else if (strcmp(buff, "ER:NEXST") == 0){//box does not exist
				printf("Error! Message box '%s' does not exist.\n", input);
			}
			else if (strcmp(buff, "ER:OPEND") == 0){//box is open
				printf("Error! Message box '%s' is open.\n", input);
			}
			else if (strcmp(buff, "ER:NOTMT") == 0){//box is not empty
				printf("Error! Message box '%s' is not empty.\n", input);
			}
			else if (strcmp(buff, "ER:WHAT?") == 0){//command failed
				printf("Error! Command failed '%s'.\n", input);
			}
			else{//received incorrect response
				perror("Invalid response from server.\n");
			}
		}
		else if (strcmp(input, "open") == 0){
			printf("Okay, what's the name of the box you wish to open?\n");
			printf("open:> ");
			scanf("%s", input);
			char msg[4096] = "OPNBX ";
			strncat(msg, input, 4095 - 6);
			if (send(csoc, msg, strlen(msg)+1, 0) <= 0){
				perror("Failed to send message!\n");
				break;
			}
			//printf("CREAT\n");
			if (recv(csoc, buff, 4096, 0) < 0){//error
				printf("No response!\n");//socket closed? Connection lost?
				lostc = 1;
				break;
			}
			if (strcmp(buff, "OK!") == 0){//box open!
				printf("Success! Message box '%s' opened.\n", input);
			}
			else if (strcmp(buff, "ER:NEXST") == 0){//box does not exist
				printf("Error! Message box '%s' does not exist.\n", input);
			}
			else if (strcmp(buff, "ER:OPEND") == 0){//box is already open		//modified such that even if %s does not exist, if client has an open box, send error
				printf("Error! Message box is already open.\n");
			}
			else if (strcmp(buff, "ER:WHAT?") == 0){//command failed
				printf("Error! Command failed '%s'.\n", input);
			}
			else{//received incorrect response
				perror("Invalid response from server.\n");
			}
		}
		else if (strcmp(input, "close") == 0){
			printf("Okay, what's the name of the box you wish to close?\n");
			printf("close:> ");
			scanf("%s", input);
			char msg[4096] = "CLSBX ";
			strncat(msg, input, 4095 - 6);
			if (send(csoc, msg, strlen(msg)+1, 0) < 0){
				perror("Failed to send message!\n");
				break;
			}
			//printf("CREAT\n");
			if (recv(csoc, buff, 4096, 0) < 0){//error
				printf("No response!\n");//socket closed? Connection lost?
				lostc = 1;
				break;
			}
			if (strcmp(buff, "OK!") == 0){//box closed!
				printf("Success! Message box '%s' closed.\n", input);
			}
			else if (strcmp(buff, "ER:NOOPN") == 0){//box is not open
				printf("Error! Message box '%s' is not open.\n", input);
			}
			else if (strcmp(buff, "ER:WHAT?") == 0){//command failed
				printf("Error! Command failed '%s'.\n", input);
			}
			else{//received incorrect response
				perror("Invalid response from server.\n");
			}
		}
		else if (strcmp(input, "next") == 0){
			//printf("Okay, what's the name of the box you wish to get the next message from?\n");//added functionality
			//printf("next:> ");
			//scanf("%s", input);
			char msg[4096] = "NXTMG";// ";
			//strncat(msg, input, 4095 - 6);
			if (send(csoc, msg, strlen(msg)+1, 0) <= 0){
				perror("Failed to send message!\n");
				break;
			}
			//printf("NXT\n");
			if (recv(csoc, buff, 4096, 0) < 0){//error
				printf("No response!\n");//socket closed? Connection lost?
				lostc = 1;
				break;
			}
			if (strcmp(buff, "ER:EMPTY") == 0){//box is empty
				//printf("Error! Message box '%s' is empty.\n", input);
				printf("Error! Message box is empty.\n");
			}
			else if (strcmp(buff, "ER:NOOPN") == 0){//box is not open
				//printf("Error! Message box '%s' is not open.\n", input);
				printf("Error! No message box opened.\n");
			}
			else if (strcmp(buff, "ER:WHAT?") == 0){//command failed
				printf("Error! Command failed '%s'.\n", input);
			}
			else{//check for message or incorrect response
				char * token;
				char *delim = "!";
				token = strtok(buff, delim);
				if (strcmp(token, "OK") == 0){//received message
					printf("Success! Here is the message:\n");
					token = strtok(NULL, delim);//length
					int length = atoi(token);
					int i;
					int offset = 3 + strlen(token) + 1;
					for (i = 0; i < length; i++){
						printf("%c", buff[i+offset]);
					}
				}
				else{
					perror("Invalid response from server.\n");
				}
			}
		}
		else if (strcmp(input, "put") == 0){
			printf("Okay, what's the message?\n");
			printf("put:> ");
			scanf("%s", input);
			int size = strlen(input);
			char pt2[64];
			sprintf(pt2, "%d\n", size);
			size = strlen(pt2) + strlen(pt2);
			char msg[4096] = "PUTMG!";//*msg = malloc(size + strlen(input));
			strcat(msg, pt2);
			char*pt3 = "!";
			strcat(msg, pt3);
			strncat(msg, input, 4095 - (strlen(pt2) + 7));//4095 because strncat copies up to n+1 for \0
			
			if (send(csoc, msg, strlen(msg)+1, 0) < 0){
				perror("Failed to send message!\n");
				break;
			}
			//printf("NXT\n");
			if (recv(csoc, buff, 4096, 0) < 0){//error
				printf("No response!\n");//socket closed? Connection lost?
				lostc = 1;
				break;
			}
			char tstr[64];
			sprintf(tstr, "OK!%s", pt2); 
			if (strcmp(buff, tstr) == 0){//server received full message
				printf("Success! Message sent!\n");
			}
			else if (strcmp(buff, "ER:NOOPN") == 0){//box is not open
				printf("Error! No message box open.\n");
			}
			else if (strcmp(buff, "ER:WHAT?") == 0){//command failed
				printf("Error! Command failed '%s'.\n", input);
			}
			else{//check for message or incorrect response
				perror("Invalid response from server.\n");
				printf("%s\n", buff);
			}
		}
		else if (strcmp(input, "help") == 0){
			printf("Commands:\nquit\ncreate\ndelete\nopen\nclose\nnext\nput\nhelp\n");
		}
		else{
			printf("That is not a command. For a command list, enter 'help'.\n");
		}
		
		if (lostc == 1){
			close(csoc);
			perror("Lost connection!\n");
			return -1;
		}

	}



    return 0;
}
