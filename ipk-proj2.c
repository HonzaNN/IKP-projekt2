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

int main(int argc, char *argv[]){
    if(!checkArgs(argc, argv)){
        return 1;
    }

    
}