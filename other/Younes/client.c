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


#define SIZE_OF_BUFFER 100

int connect_server(const char *host,const char *port){

    int PORT = atoi(port); 
    struct sockaddr_in serv_addr; 
    int sock = 0; 
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    }else{
        printf("client socket created %d\n",sock);
    }
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0)  
    { 
        printf("Invalid address\n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("Connection Failed \n"); 
        return -1; 
    }else{
        printf("Connected \n");
    }
    return sock; 
}

bool receive_message(int sock , char *buffer , int size){
    bool flag = true;
    char receive[SIZE_OF_BUFFER];
    if( write(sock,buffer,size) == -1){
        perror("write client\n");
    }
    if (read (sock,receive,SIZE_OF_BUFFER) == -1){
        perror("read client \n");
    }
    printf("Server response : %s\n",receive);
    if( strncmp(buffer,"exit",4) == 0){
        flag=false;
        printf("Client exit \n");
    }
    // clear the buffer 
    memset(receive ,'\0', SIZE_OF_BUFFER);
    return flag;
}

void speak_to_server(int sock){
    bool flag = true;
    while(flag){
        printf(">>>");
        char buf[SIZE_OF_BUFFER];  
        fgets(buf, 100, stdin);
        flag = receive_message(sock,buf,SIZE_OF_BUFFER);
    } 
}

int main(int argc ,char **argv){
    if (argc != 3){
        printf("respect this format: ./client [host] [port]\n");
        exit(0);
    }
    int client_socket = connect_server(argv[1],argv[2]);
    if( client_socket != -1){
        speak_to_server(client_socket);
        if (close(client_socket) == -1){
            perror("closing socket\n");
        }   
    }
    return 0;
}

