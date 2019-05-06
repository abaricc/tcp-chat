#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

char buffer[100];
int size =0;

#define NCLIENTS 3

typedef struct client_data_s{
    int used;
    int sock;
    pthread_t thread;
    }
client_data;

client_data client[NCLIENTS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int unused_id(){
    for(int i = 0; i < NCLIENTS ; i++){
        if( client[i].used == 0){
            return i;
        }
        printf("used : %d \n",client[i].used);
    }
    return -1;
}

int alloc_client(){
    pthread_mutex_lock(&mutex);
    int indice = unused_id();
    if ( indice == -1){
        return -1;
    }
    client[indice].used = 1;
    pthread_mutex_unlock(&mutex);
    return indice;
}

void free_client(int client_id){
    pthread_mutex_lock(&mutex);
    client[client_id].used = 0;
    pthread_mutex_unlock(&mutex);
}

void *handl_client(void * arg){
    int client_socket = *((int*) arg);
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
    //free_client(client_socket);

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
    if ( listen(srv_sock,NCLIENTS) !=0 ){
        perror("listen\n");
    }else{
        printf("Succes : listening to socket %d\n",srv_sock);
    }
    int i = 0;
    while(1)
    {
        int client_sock =accept(srv_sock,NULL,NULL);
        int index = alloc_client();
        void *cl_socket = &client_sock;
        if ( index != 1){
            client[index].sock = client_sock;
        }
        pthread_t thread = client[index].thread;
        if( pthread_create(&thread,NULL,&handl_client,cl_socket) != 0 ) {
            printf("Failed to create thread %d\n,",index);
        }else {
            printf("thread %d created \n",index);
        }
        if( i >= NCLIENTS)
        {
          i = 0;
          while(i < NCLIENTS)
          {
            pthread_join(client[i++].thread,NULL);
          }
          i = 0;
        }
    }
    return 0;
}
