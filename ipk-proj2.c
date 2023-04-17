#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>

#include <regex.h>

#define BUFSIZE 512
#define BUFSIZETCP 1024

typedef struct result{
    int result;
    bool error;
} result_t;

/// @brief Check if imput arguments are simmilar to the format "-h <host> -p <port number> -m <mode>"
/// @param argc 
/// @param argv 
/// @return 
bool checkArgs(int argc, char *argv[]){
    if(argc != 7){
        fprintf(stderr, "Wrong number of arguments!\n");
        return false;
    }
    if(argv[1][0] != '-' || argv[1][1] != 'h'){
        fprintf(stderr, "Wrong format of arguments!\n");
        return false;
    }


    if(argv[3][0] != '-' || argv[3][1] != 'p'){
        fprintf(stderr, "Wrong format of arguments!\n");
        return false;
    }

    char *endptr;
    long port = strtol(argv[4], &endptr, 10);
    if(*endptr != '\0' || port < 0 || port > 65535){
        fprintf(stderr, "Wrong format of port number!\n");
        return false;
    }

    if(argv[5][0] != '-' || argv[5][1] != 'm'){
        fprintf(stderr, "Wrong format of arguments!\n");
        return false;
    }

    if(strcmp(argv[6], "tcp") != 0 && strcmp(argv[6], "udp") != 0){
        fprintf(stderr, "Wrong format of arguments!\n");
        return false;
    }

    return true;
    
}



/// @brief check if input string has format
/// @param str
/// @return true if string has correct format
bool checkFormat(char *str){
    regex_t regex;
    //negative number is not allowed, NEED TO FIX
    char regex_pattern[] = "^\\(\\+|\\-|\\*|/\\s-?[0-9]+(\\s-?[0-9]+)+\\)$";
    int x = regcomp(&regex, regex_pattern, REG_EXTENDED);
    if (x != 0) {
        char error_message[100];
        regerror(x, &regex, error_message, sizeof(error_message));
        fprintf(stderr, "Error compiling regex: %s\n", error_message);
        return false;
    }
    regmatch_t match;
    x = regexec(&regex, str, 1, &match, 0);
    regfree(&regex);
    if(x == 0){
        return true;
    }
    else{
        return false;
    }
}


result_t resolve_command(char *str){
    int result = 0;
    char *token;
    char *rest = str;
    char *operation;
    int number;
    int i = 0;
    bool error = false;
    token = strtok(rest, " ");
    while(token != NULL){
        if(i == 0){
            operation = token;
        }
        else if(i == 1){
            result = atoi(token);
        }
        else{
            number = atoi(token);
            switch(operation[1]){
                case '+':
                    result += number;
                    break;
                case '-':
                    result -= number;
                    break;
                case '*':
                    result *= number;
                    break;
                case '/':
                    if(number == 0){
                        error = true;
                        break;
                    }
                    result /= number;
                    break;
                default:
                    error = true;
            }
        }
        i++;
        token = strtok(NULL, " ");
    }
    result_t result_struct;
    result_struct.result = result;
    result_struct.error = error;
    return result_struct;
}

void UDP_server( char *argv[] ){
    char buf[BUFSIZE];
	int server_socket, port_number, bytestx, bytesrx;
    socklen_t clientlen;
    struct sockaddr_in client_address, server_address;
    int optval;
    const char * hostaddrp;
    struct hostent *hostp;

    char *adress;
    adress = argv[2];

     
    port_number = atoi(argv[4]);
    adress = argv[2];

      
    /* Vytvoreni soketu */
	if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) <= 0)
	{
		perror("ERROR: socket");
		exit(EXIT_FAILURE);
	}

    /* potlaceni defaultniho chovani rezervace portu ukonceni aplikace */ 
    optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    /* adresa serveru, potrebuje pro prirazeni pozadovaneho portu */
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(adress);
    server_address.sin_port = htons((unsigned short)port_number);
	
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) 
    {
        perror("ERROR: binding");
        exit(EXIT_FAILURE);
    }
    
    char hepl_buf[BUFSIZE];
    while(1) 
    {   char buf_help[BUFSIZE];
        bzero(buf, BUFSIZE);
        printf("INFO: Ready UDP\n");
    	/* prijeti odpovedi a jeji vypsani */
        clientlen = sizeof(client_address);
        bytesrx = recvfrom(server_socket, buf, BUFSIZE, 0, (struct sockaddr *) &client_address, &clientlen);
        if (bytesrx < 0) 
            perror("ERROR: recvfrom:");
              
        

        strcpy(hepl_buf, buf+2);
        buf[0] = 0x01;

        buf[2] = strlen(hepl_buf)+3;
        strcpy(buf+3, hepl_buf);

        if(checkFormat(hepl_buf)){
            buf[1] = 0x00;
            result_t result_struct = resolve_command(hepl_buf);
            if(result_struct.error){
                buf[1] = 0x01;
            }
            else{
                sprintf(buf+3, "%d", result_struct.result);
                buf[2] = strlen(buf+3)+3;
            }
        }
        else{
            buf[1] = 0x01;
            sprintf(buf+3, "Invalid expression: %s", hepl_buf);
        }

        buf[2] = strlen(buf+3)+3;
        
    
        /* odeslani zpravy zpet klientovi  */        
        bytestx = sendto(server_socket, buf, buf[2], 0, (struct sockaddr *) &client_address, clientlen);
        if (bytestx < 0) 
            perror("ERROR: sendto:");
    }

}


bool send_message(int socket, char *message){
    int n = strlen(message);
    n = write(socket, message, n);
    if (n < 0) {
        perror("write failed");
        return false;
    }
    return true;
}

void handle_client(int new_socket){
    char buf[BUFSIZE], help_buf[BUFSIZE];
    int n;


    //Check incoming HELLO
    bzero(buf, BUFSIZETCP);
    n = read(new_socket, buf, BUFSIZE);
    if (n < 0) {
        perror("read failed");
        return;
    }
    if (n == 0) {
        printf("Client disconnected\n");
        return;
    }


    if(strcmp(buf, "HELLO\n") == 0){
        send_message(new_socket, "HELLO\n");
    }
    else{
        send_message(new_socket, "BYE\n");

        close(new_socket);
        return;
    }

    result_t result;
    char *token;
    //response to client
    while (1) {
        // receive data from client
        bzero(buf, BUFSIZETCP);
        n = read(new_socket, buf, BUFSIZE);
        if (n < 0) {
            perror("read failed");
            break;
        }
        if (n == 0) {
            perror("Client disconnected\n");
            break;
        }

        token = strtok(buf, " ");
        if(strcmp(token, "BYE") == 0){
            send_message(new_socket, "BYE\n");
            break;
        }
        else if(strcmp(token, "SOLVE") == 0){
            token = strtok(NULL, "");
            if(checkFormat(token)){
                result = resolve_command(token);
                if(result.error){
                    send_message(new_socket, "BYE\n");
                }
                else{
                    sprintf(help_buf, "RESULT %d\n", result.result);
                    send_message(new_socket, help_buf);
                }
            }
            else{
                send_message(new_socket, "BYE\n");
                break;
            }
        }
        else{
            send_message(new_socket, "BYE\n");
            break;
        }

    }

    // close socket
    close(new_socket);

}

void TCP_server(char *argv[]){
    char buf[BUFSIZE];
	int server_socket, port_number, bytestx, bytesrx;
    socklen_t clientlen;
    struct sockaddr_in client_address, server_address;
    int optval;
    const char * hostaddrp;
    struct hostent *hostp;
    pid_t pid;

    int news_socket;

    char *adress;
    adress = argv[2];

     
    port_number = atoi(argv[4]);
    adress = argv[2];

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
	{
		perror("ERROR: socket");
		exit(EXIT_FAILURE);
	}

    /* potlaceni defaultniho chovani rezervace portu ukonceni aplikace */ 
    optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    /* adresa serveru, potrebuje pro prirazeni pozadovaneho portu */
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(adress);
    server_address.sin_port = htons((unsigned short)port_number);
	
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) 
    {
        perror("ERROR: binding");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    
    while (1) {
        //printf("INFO: Ready TCP\n");
        // accept incoming connection
        clientlen = sizeof(client_address);
        news_socket = accept(server_socket, (struct sockaddr *)&client_address, &clientlen);
        if (news_socket < 0) {
            //perror("accept failed");
            continue;
        }


        // fork new process to handle client
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            close(news_socket);
            continue;
        }
        if (pid == 0) {  // child process
            close(server_socket);
            handle_client(news_socket);
            exit(EXIT_SUCCESS);
        }
        else {  // parent process
            close(news_socket);
        }
    }


}


int main(int argc, char *argv[]){
    if(!checkArgs(argc, argv)){
        return 1;
    }
    //test();

    if(strcmp(argv[6], "tcp") == 0){
        TCP_server(argv);
    }
    else if(strcmp(argv[6], "udp") == 0){
        UDP_server(argv);
    }
    else{
        perror("ERROR: Unknown protocol\n");
        return 1;
    }
    return 0;

    
}