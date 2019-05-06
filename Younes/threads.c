#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <netinet/in.h> 
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct client_data_s{
    int used;
    int sock;
    pthread_t thread;
    }
client_data;

#define NCLIENTS 3
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

void *client_main(void *arg){
    
    char buffer[100];
    int size =0;
    int client_id  = *((int*) arg);
    bool flag = true;
    do{
        //read
        size = read(client[client_id].sock,buffer,sizeof(buffer));
        if(  size == -1 ){
            perror("client_main : read \n");
            break;
        }
        buffer[size]='\0';
        printf("Server : client send => %s " ,buffer);
        if( write(client[client_id].sock,"Server received your message\n",100)  == -1 ){
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

    close(client[client_id].sock);
    free_client(client_id);
    
    //return EXIT_SUCCESS;
}

void client_recu(int client_socket){
    
    int index = alloc_client();
    void *arg = &index;
    if( index != -1){
        pthread_mutex_lock(&mutex);
        client[index].sock = client_socket;
        pthread_t thread = client[index].thread;
        if( pthread_create(&thread,NULL,&client_main,arg) != 0 ) {
            printf("Failed to create thread %d\n,",index);
        }else {
            printf("thread %d created \n",index);
        }
        pthread_join(thread, NULL);
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc ,char **argv){
    if ( argc != 1){
        printf("respect this format:  ./server \n");
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
    while(1){
        client_recu(accept(srv_sock,NULL,NULL));
    }
    return 0;
}