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

#define NCLIENTS 100
#define MESSAGE_MAXLEN 1024

typedef struct client_data {
  int index;
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
  pthread_mutex_lock(&mutex);
  client[client_id].used=0;
  nr_clients--;
  pthread_mutex_unlock(&mutex);
}

void* client_main(void* arg) {
  int index = *((int*) arg);
  int client_sock = client[index].sock;
  char buf[MESSAGE_MAXLEN];
  ssize_t rc;
  printf("Le client %d est arrive\n", index);
  while(1) {
    memset(buf, 0, MESSAGE_MAXLEN);
    rc = read(client_sock, buf, sizeof(buf));
    if(rc==0) {
      printf("Le client %d s'est deconnecte\n", index);
      free_client(index);
      break;
    }
    if(rc<0) {
      perror("Erreur read");
      break;
    }
    buf[rc]='\0';
    printf("Client %d => %s", index, buf);
    if(write(client_sock, buf, rc)<0) {
      perror("Erreur write");
      break;
    }
    memset(buf ,'\0', MESSAGE_MAXLEN); //supprimer le contenue du buffer
  }
  if(close(client_sock)<0) {
      perror("Erreur close");
  }
  free_client(index);
}

int client_arrived(int client_sock) {
  int index = alloc_client();
  if(index<0) {
    printf("Il y n'a aucun indice libre\n");
    return -1;
  }
  client[index].index = index;
  client[index].sock = client_sock;
  pthread_t thread = client[index].thread;
  if(pthread_create(&thread, NULL, &client_main, &client[index].index)<0) {
    perror("Le thread n'a pas pu etre cree");
    return -1;
  }
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
    if(client_sock<0) {
      perror("Erreur accept");
    }
    if(client_arrived(client_sock)<0) {
      printf("Socket %d n'a pas pu se connecter\n", client_sock);
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
