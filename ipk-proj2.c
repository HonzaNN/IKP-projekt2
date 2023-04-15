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
#include <regex.h>

#define BUFSIZE 512
#define BUFSIZETCP 1024



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
    char regex_pattern[] = "^\\( [\\+\\-\\\\\\*] \\d+( \\d+)+\\)$";
    int x = regcomp(&regex, regex_pattern, REG_EXTENDED);
    if (x != 0) {
    char error_message[100];
    regerror(x, &regex, error_message, sizeof(error_message));
    fprintf(stderr, "Error compiling regex: %s\n", error_message);
    exit(1);
    }
    x = regexec(&regex, str, 0, NULL, 0);
    if(x == 0){
        return true;
    }
    else{
        return false;
    }


    
}


void UDP_server( char *argv[] ){
    char buf[BUFSIZE];
	int server_socket, port_number, bytestx, bytesrx;
    socklen_t clientlen;
    struct sockaddr_in client_address, server_address;
    int optval;
    const char * hostaddrp;
    struct hostent *hostp;

     
    port_number = atoi(argv[4]);
      
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
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
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
        printf("INFO: Ready.\n");
    	/* prijeti odpovedi a jeji vypsani */
        clientlen = sizeof(client_address);
        bytesrx = recvfrom(server_socket, buf, BUFSIZE, 0, (struct sockaddr *) &client_address, &clientlen);
        if (bytesrx < 0) 
            perror("ERROR: recvfrom:");
    
        hostp = gethostbyaddr((const char *)&client_address.sin_addr.s_addr, 
			  sizeof(client_address.sin_addr.s_addr), AF_INET);
              
        hostaddrp = inet_ntoa(client_address.sin_addr);
        printf("Message (%lu) from %s:  %s..\n", strlen(buf+2)+2, hostaddrp, buf+2);
        printf("Message (%lu) from %s:  %s..\n", strlen(buf), hostaddrp, buf+2);
        printf("Message (%d) from %s:  %s..\n", buf[1], hostaddrp, buf+2);
        //print buffer in hexademical format
        for(int i = 0; i < buf[1]; i++){
            printf("%02x, %c.\n", buf[i], buf[i]);
        }

        strcpy(hepl_buf, buf+2);
        buf[0] = 0x01;

        buf[2] = strlen(hepl_buf)+3;
        strcpy(buf+3, hepl_buf);

        if(checkFormat(hepl_buf)){
            buf[1] = 0x00;
        }
        else{
            buf[1] = 0x01;
        }
        
        printf("out data: %s\n", buf+2);
        for(int i = 0; i < buf[2]; i++){
            printf("%02x, %c.\n", buf[i], buf[i]);
        }
    
    
        /* odeslani zpravy zpet klientovi  */        
        bytestx = sendto(server_socket, buf, strlen(hepl_buf)+3, 0, (struct sockaddr *) &client_address, clientlen);
        if (bytestx < 0) 
            perror("ERROR: sendto:");
    }

}

int main(int argc, char *argv[]){
    if(!checkArgs(argc, argv)){
        return 1;
    }

    UDP_server(argv);
    return 0;

    
}