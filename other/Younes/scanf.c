#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <netinet/in.h> 
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>

int main(){
    char buf[100];  
    fgets(buf, 100, stdin); 
    printf("string is: %s\n", buf); 
    return 0;
}