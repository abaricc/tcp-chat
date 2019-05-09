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

char buffer[1024];
int size =0;

int NCLIENT_CONNECTED = 0;

#define MESSAGE_MAXLEN 1024

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
    }
    return -1;
}

int alloc_client(){
    pthread_mutex_lock(&mutex);
    int indice = unused_id();
    if ( indice == -1){
         pthread_mutex_unlock(&mutex);
        return -1;
    }
    client[indice].used = 1;
    NCLIENT_CONNECTED = NCLIENT_CONNECTED + 1;
    pthread_mutex_unlock(&mutex);
    return indice;
}

void free_client(int client_id){
    pthread_mutex_lock(&mutex);
    client[client_id].used = 0;
    if ( NCLIENT_CONNECTED > 0 ){
        NCLIENT_CONNECTED = NCLIENT_CONNECTED  -1;
    }
    pthread_mutex_unlock(&mutex);
}

void *handl_client(void * arg){
    //int client_socket = *((int*) arg);
    int client_index = *((int*) arg);
    int client_socket = client[client_index].sock;
    bool flag = true;
    char response[MESSAGE_MAXLEN];
    do{
        //read client message
        size = read(client_socket,buffer,sizeof(buffer));
        if(  size == -1 ){
            perror("read \n");
            break;
        }
        buffer[size]='\0';
        /*
        * handls user messages
        * handls input : echo , rand , quit 
        * returns warning message if input is empty or unknown 
        */ 
        char *str = buffer;
        char *first_word = strsep(&str," ");
        char *everything_else = str;
        if( everything_else == NULL ){
            everything_else = "";
        }
        //printf("first word : %s\n",first_word);
        if(strcmp(first_word,"echo") == 0){
            snprintf(response,MESSAGE_MAXLEN,"ok %s\n",everything_else);
            // write response 
            if( write(client_socket,response,1024)  == -1 ){
                perror("write \n");
                break;
            }
        }else if( strcmp(first_word,"rand") == 0 ){
            int n = atoi(everything_else);
            snprintf(response,MESSAGE_MAXLEN,"ok  %d\n",(rand()%(n +1)));
            // write response 
            if( write(client_socket,response,1024)  == -1 ){
                perror("write \n");
                break;
            }
        }else if( strncmp(first_word,"quit",4) == 0 || strncmp(first_word,"q",1) == 0){
            flag=false;
            break;
        }else if( strlen(first_word) == 1) {
            // write response 
            if( write(client_socket,"fail empty command\n",1024)  == -1 ){
                perror("write \n");
                break;
            }
        }
        else{
            // write response 
            if( write(client_socket,"fail unkown command",1024)  == -1 ){
                perror("write \n");
                break;
            }
        }
        // clear the buffer
        memset(buffer ,'\0', 1024);
        // clear the response
        memset(response ,'\0', 1024);
    }while(flag == true);
    
    if (close(client_socket) == -1){
        perror("closing socket\n");
    }
    free_client(client_index);
    return EXIT_SUCCESS;
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
        //printf("client socket : %d\n",client_sock);
        int index = alloc_client();
        //void *cl_socket = &client_sock;
        void *cl_index = &index;
        if ( index != 1){
            client[index].sock = client_sock;
            //printf("client[index].sock = %d\n",client[index].sock);
        }
        pthread_t thread = client[index].thread;
        if( pthread_create(&thread,NULL,&handl_client,cl_index) != 0 ) {
            // if the thread is not created , we must free de client 
            printf("failed to create thread with index : %d\n",index);
            free_client(index);
        }else {
            printf("thread %d created || clients connected %d \n",index,NCLIENT_CONNECTED);
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