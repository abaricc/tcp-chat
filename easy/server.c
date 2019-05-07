#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <netinet/in.h> 
#include <netdb.h>
#include <string.h>
#include <stdbool.h>

char buffer[100];
int size =0;

void client_recu(int client_socket){
    bool flag = true;
    do{
        //read
        size = read(client_socket,buffer,sizeof(buffer));
        if(  size == -1 ){
            perror("read \n");
            break;
        }
        buffer[size]='\0';
        printf("Server : client send => %s \n" ,buffer);
        // write response 
        if( write(client_socket,"Server received your message\n",100)  == -1 ){
            perror("write \n");
            break;
        }
        if( strncmp(buffer,"exit",4) == 0){
            flag=false;
            printf("Client exit \n");
            printf("waiting for other clients ...\n");
        }
        // clear the buffer
        memset(buffer ,'\0', 100);
    }while(flag == true);

    if (close(client_socket) == -1){
        perror("closing socket\n");
    }

}

int main(int argc ,char **argv){
    if ( argc != 1){
        printf("respect this format: ./server \n");
        exit(0);
    }
    //config pour liaison numero de port 
    struct sockaddr_in6 sin6;
    memset(&sin6, 0, sizeof(sin6));
    int port = 42000;
    sin6.sin6_family = PF_INET6;
    sin6.sin6_port = htons(port);

     //creation du socket serveur
    int domain = PF_INET6;
    int type = SOCK_STREAM;
    int protocol = 0;
    int srv_sock = socket(domain,type,protocol);
    if ( srv_sock < 0 ){
        perror("socket error\n");
    }else{
        printf("Succes : socket => %d \n",srv_sock);
    }
    //bind 
    if ( bind(srv_sock,(struct sockaddr*)&sin6, sizeof(sin6) ) != 0 ){
        perror("bind\n");
    }else{
        printf("Succes bind \n");
    }
    //listen
    if ( listen(srv_sock,1) !=0 ){
        perror("listen\n");
    }else{
        printf("Succes : listening to socket %d\n",srv_sock);
    }
    while(1){
        int client = accept(srv_sock,NULL,NULL);
        client_recu(client);
    }
    return 0;
}