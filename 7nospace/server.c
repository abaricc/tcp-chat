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
#define RECV_MAX 10

typedef struct client_data {
  int index;
  int used;
  int sock;
  char nick[MESSAGE_MAXLEN];
  char recv[RECV_MAX][MESSAGE_MAXLEN];
  pthread_t thread;
} client_data;

client_data client[NCLIENTS];
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
    sprintf(client[id].nick, "client%d", id);
    pthread_mutex_unlock(&mutex); //liberer le mutex
    return id;
  }
}

void free_client(int client_id) {
  if(client_id != -1) {
    pthread_mutex_lock(&mutex);
    client[client_id].used=0;
    pthread_mutex_unlock(&mutex);
  }
}

int do_echo(int client_id, char *args, char *resp, int resp_len) {
  if(args==NULL) {
    snprintf(resp, resp_len, "ok\n");
    return 0;
  }
  snprintf(resp, resp_len, "ok %s", args);
  return 0;
}

int do_rand(int client_id, char *args, char *resp, int resp_len) {
  int random;
  if(args==NULL) {
    random = rand() % 100;
    snprintf(resp, resp_len, "ok %d\n", random);
    return 0;
  }
  int max = atoi(args);
  snprintf(resp, resp_len, "ok %d\n", rand()%max);
  return 0;
}

int do_nick(int client_id, char *args, char *resp, int resp_len) {
  if(args==NULL) {
    snprintf(resp, resp_len, "Nick a besoin d'un argument\n");
    return 0;
  }
  pthread_mutex_lock(&mutex);
  sprintf(client[client_id].nick, "%s", args);
  pthread_mutex_unlock(&mutex);
  snprintf(resp, resp_len, "Nouveau pseudo : %s", args);
  return 0;
}

int do_list(int client_id, char *args, char *resp, int resp_len) {
  if(args!=NULL) {
    snprintf(resp, resp_len, "List n'a pas besoin d'arguments\n");
    return 0;
  }
  char list[1027]; //1027 est la taille max pour sprintf
  pthread_mutex_lock(&mutex);
  for(int i=0; i<NCLIENTS; i++) {
    if((client[i].used == 1)&&(i!=client_id)) {
        sprintf(list, " %s", client[i].nick);
    }
  }
  pthread_mutex_unlock(&mutex);
  snprintf(resp, resp_len, "ok%s\n", list);
  return 0;
}

int do_send(int client_id, char *args, char *resp, int resp_len) {
  if(args==NULL) {
    snprintf(resp, resp_len, "Send a besoin d'un argument\n");
    return 0;
  }
  char *nick, *message;
  nick = strsep(&args," ");
  message = args;
  pthread_mutex_lock(&mutex);
  for(int i=0; i<NCLIENTS; i++) {
    if(client[i].used==1) {
      if(strcmp(client[i].nick, nick)==0) {
        for(int r=0; r<RECV_MAX; r++) {
          if(strlen(client[i].recv[r])<2) {
            sprintf(client[i].recv[r], "%s : %s", client[i].nick, message);
            pthread_mutex_unlock(&mutex);
            snprintf(resp, resp_len, "Votre message a ete envoye a : %s\n", nick);
            return 0;
          }
        }
      }
    }
  }
  pthread_mutex_unlock(&mutex);
  snprintf(resp, resp_len, "Boite mail de %s est pleine\n", nick);
  return 0;
}

int do_recv(int client_id, char *args, char *resp, int resp_len) {
  if(args!=NULL) {
    snprintf(resp, resp_len, "Recv n'a pas besoin d'un argument\n");
    return 0;
  }
  pthread_mutex_lock(&mutex);
  for(int r; r<RECV_MAX; r++) {
    sprintf(resp, "%s", client[client_id].recv[r]);
    if(strlen(resp)>1) {
      if(write(client[client_id].sock, resp, strlen(resp))<0) {
        pthread_mutex_unlock(&mutex);
        perror("Erreur write");
        return -1; //erreur : on deconnecte le client
      }
    }
  }
  pthread_mutex_unlock(&mutex);
  snprintf(resp, resp_len, "Fin messages\n");
  return 0;
}

int do_quit(int client_id, char *resp, int resp_len) {
  snprintf(resp, resp_len, "quit");
  return 1;
}


int eval_msg(int client_id, char *msg, char *resp, int resp_len) {
  char *cmd, *args;
  cmd = strsep(&msg," ");
  args = msg;
  if(strcmp(cmd,"echo")==0||strcmp(cmd,"echo\n")==0)
    return do_echo(client_id, args, resp, resp_len);
  else if(strcmp(cmd,"rand")==0||strcmp(cmd,"rand\n")==0)
    return do_rand(client_id, args, resp, resp_len);
  else if(strcmp(cmd,"nick")==0||strcmp(cmd,"nick\n")==0)
    return do_nick(client_id, args, resp, resp_len);
  else if(strcmp(cmd,"list")==0||strcmp(cmd,"list\n")==0)
    return do_list(client_id, args, resp, resp_len);
  else if(strcmp(cmd,"send")==0||strcmp(cmd,"send\n")==0)
    return do_send(client_id, args, resp, resp_len);
  else if(strcmp(cmd,"recv")==0||strcmp(cmd,"recv\n")==0)
    return do_recv(client_id, args, resp, resp_len);
  if(strcmp(cmd,"quit")==0||strcmp(cmd,"quit\n")==0||strcmp(cmd,"q")==0||strcmp(cmd,"q\n")==0)
    return do_quit(client_id, resp, resp_len); //retourne 1
  else {
    snprintf(resp, resp_len, "Command introuvable : %s", cmd);
    return 0;
  }
}

int receive_message(int client_id, char *msg, int size) {
    int client_sock = client[client_id].sock;
    char received_msg[MESSAGE_MAXLEN];
    int rc = read(client_sock, received_msg, size);
    if(rc<0) {
        perror("Erreur read");
        return -1;
    }
    received_msg[rc] = '\n';
    printf("Client %d => %s", client_id, received_msg);
    snprintf(msg, size, "%s", received_msg);
    memset(received_msg, '\0', size); //supprimer le contenue du buffer
    return 0;
}

void* client_main(void* arg) {
  int client_id = *((int*) arg);
  int client_sock = client[client_id].sock;
  char msg[MESSAGE_MAXLEN];
  char resp[MESSAGE_MAXLEN];
  int eval;
  printf("Le client %d est arrive\n", client_id);
  while(1) {
    if(receive_message(client_id, msg, sizeof(msg))<0) {
      break; //erreur : on deconnecte le client
    }
    eval = eval_msg(client_id, msg, resp, sizeof(resp));
    if(eval<0) {
      break; //erreur : on deconnecte le client
    }
    if(write(client_sock, resp, strlen(resp))<0) {
      perror("Erreur write");
      break; //erreur : on deconnecte le client
    }
    if(eval==1) { //eval_msg retourne 1 si il faut deconnecter le client
      printf("Client %d s'est deconnecte\n", client_id);
      break; //on deconnecte le client
    }
  }
  if(close(client_sock)<0) {
      perror("Erreur close");
  }
  free_client(client_id);
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
  while(1) {
    int client_sock = accept(srv_sock, NULL, NULL);
    if(client_sock<0) {
      perror("Erreur accept");
    }
    if(client_arrived(client_sock)<0) {
      printf("Socket %d n'a pas pu se connecter\n", client_sock);
    }
  }
  return 0;
}
