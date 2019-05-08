#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <string.h>
#include <pthread.h>

#define NCLIENTS 3 //100

#include <stdbool.h>
char buffer[100];
int size=0;

typedef struct client_data {
  int used;
  int sock;
  pthread_t thread;
} client_data;

client_data client[NCLIENTS];
int nr_clients;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int unused_id() {
  for(int i=0; i<NCLIENTS; i++) {
    if(client[i].used == 0) {
      return i;
    }
  }
  return -1; //aucun indice libre
}

int alloc_client() {
  pthread_mutex_lock(&mutex);
  int id = unused_id();
  if(id == -1) {
    pthread_mutex_unlock(&mutex);
    return -1; //aucun indice libre
  }
  else {
    client[id].used=1;
    nr_clients++;
    pthread_mutex_unlock(&mutex); //liberer le mutex
    return id;
  }
}

void free_client(int client_id) {
  if(client_id != -1) {
    pthread_mutex_lock(&mutex);
    client[client_id].used=0;
    nr_clients--;
    pthread_mutex_unlock(&mutex);
  }
}

void *handl_client(void * arg){
int client_socket = 4;
    //int client_socket = *((int*) arg);

    bool flag = true;
    do{
        //read
printf("in handl_client = client_socket = %d\n", client_socket);
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


void* client_main(void* arg) {
  int client_sock = *((int*) arg);
  char buf[100];
  while(1) {
    memset(buf, 0, 100);
    ssize_t rc = read(client_sock, buf, 100);
    if(rc==0) {
      printf("Le client s'est deconnecte\n");
      free_client(client_sock);
      break;
    }
    if(rc<0) {
      perror("Erreur read");
      break;
    }
    buf[rc]='\0';
    printf("%s", buf);
    if(write(client_sock, buf, rc)<0) {
      perror("Erreur write");
      break;
    }
    memset(buf ,'\0', 100); //supprimer le contenue du buffer
  }
  if(close(client_sock)<0) {
      perror("Erreur close");
  }
  free_client(client_sock);
}

int client_arrived(int client_sock) {
  int index = alloc_client();
  if(index<0) {
    printf("Il y n'a aucun indice libre\n");
    return -1;
  }
  client[index].sock = client_sock;
//printf("in client_arrived = client[index].sock = %d\n", client[index].sock);
  pthread_t thread = client[index].thread;
  void *client_sock_p = &client_sock;
printf("in client_arrived = client_socket = %d\n", client_sock);
  if(pthread_create(&thread, NULL, &handl_client, (void *)client_sock)<0) {
    perror("Le thread n'a pas pu etre cree");
    return -1;
  }
  printf("Un thread a ete cree\n");
  return 0;
}

int main(int argc, char* argv[]) {
  if(argc!=2) {
    perror("Il faut respecter le format ./server port");
    exit(EXIT_FAILURE);
  }
  int port = htons(atoi(argv[1]));
  if(port<1024 || port>65535) {
    perror("Il faut que le port soit dans l'intervale [1024, 65535]");
    exit(EXIT_FAILURE);
  }
  struct sockaddr_in6 sin6;
  memset(&sin6, 0, sizeof(sin6));
  sin6.sin6_family = AF_INET6;
  sin6.sin6_port = port;
  int srv_sock=socket(AF_INET6, SOCK_STREAM, 0);
  if(srv_sock<0) {
    perror("Erreur socket");
    exit(EXIT_FAILURE);
  }
  int optval=1;
  if(setsockopt(srv_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))<0) {
    perror("Erreur socket");
    exit(EXIT_FAILURE);
  }
  if(bind(srv_sock, (struct sockaddr *)&sin6, sizeof(sin6))<0) {
    perror("Erreur bind");
    exit(EXIT_FAILURE);
  }
  if(listen(srv_sock, 10)<0) {
    perror("Erreur listen");
    exit(EXIT_FAILURE);
  }
  nr_clients = 0;
  while(1) {
    int client_sock = accept(srv_sock, NULL, NULL);
printf("in main = client_socket = %d\n", client_sock);
    if(client_sock<0) {
      perror("Erreur accept");
    }
    if(client_arrived(client_sock)<0) {
      perror("Erreur de communication avec le client");
    }
    if(nr_clients>=NCLIENTS) {
      nr_clients = 0;
      while(nr_clients<NCLIENTS) {
        pthread_join(client[nr_clients++].thread, NULL);
      }
      nr_clients = 0;
    }
  }
  return 0;
}
