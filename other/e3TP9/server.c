#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define NCLIENTS 3 //100

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
  int id = unused_id();
  pthread_mutex_lock(&mutex);
  if (id == -1) {
    pthread_mutex_unlock(&mutex);
    return -1; //aucun indice libre
  }
  else {
    client[id].used = 1;
    pthread_mutex_unlock(&mutex); //liberer le mutex
    return id;
  }
}

void free_client(int client_id) {
  if(client_id != -1) {
    pthread_mutex_lock(&mutex);
    client[client_id].used = 0;
    pthread_mutex_unlock(&mutex);
    nr_clients--;
  }
}

void* client_main(void* arg) {
  int client_id = *((int *) arg);
  int client_sock = client[client_id].sock;
  int rc;
  char buf[100];
  while(1) {
    rc = read(client_sock, buf, sizeof(buf));
    printf("rc = %d\n", rc);
    if(rc==0) {
      printf("Le client est partie\n");
      break;
    }
    if(rc<0) {
      perror("Erreur de lecture\n");
      break;
    }
    buf[rc]='\0';
    if(write(client_sock, buf, rc)<0) {
      perror("Erreur d'ecriture\n");
      break;
    }
  }
  if(close(client_sock)<0) {
    perror("Erreur de fermeture\n");
  }
  free_client(client_id);
}

void client_arrived(int client_sock) {
  int create_thread;
  void *sock = &client_sock;
  int client_id = alloc_client();
  if(client_id != -1) {
    client[client_id].sock = client_sock;
    create_thread = pthread_create(&(client[client_id].thread), NULL, &client_main, sock);//(void*)nr_clients
    if(create_thread<0) {
      perror("le thread n'a pas pu etre cree\n");
      exit(-1);
    }
    else {
        printf("un thread est cree\n");
    }
    //client_main(client[client_id].sock, client_id);
  }
}

int main(int argc, char* argv[]) {
  if(argc!=2) {
    perror("Il faut respecter le format ./server port\n");
    exit(-1);
  }
  int src_sock=socket(AF_INET6, SOCK_STREAM, 0);
  struct sockaddr_in6 sin6;
  memset(&sin6, 0, sizeof(sin6));
  sin6.sin6_family = AF_INET6;
  sin6.sin6_port = htons(atoi(argv[1]));
  //setsockop(); port#[1024, 65535]
  int optval=1;
  setsockopt(src_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  bind(src_sock, (struct sockaddr *)&sin6, sizeof(sin6));
  listen(src_sock, NCLIENTS);
  nr_clients = 0;
  while(1) {
    int client_sock = accept(src_sock, NULL, NULL);
    printf("i = %d\n", nr_clients);
    if(client_sock<0) {
      perror("socket pas acceptee\n");
      exit(-1);
    }
    nr_clients++;
    if(nr_clients<=NCLIENTS) {
      client_arrived(client_sock);
    }
    else {
      printf("le nombre de clients a depasse la limite\n");
    }
  }
}
