#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#define MESSAGE_MAXLEN 1024

int connect_server(const char *host, const char *port) {
  struct sockaddr_in6 sin6;
  int sock = socket(AF_INET6, SOCK_STREAM, 0);
  if(sock<0)
  {
    perror("Erreur socket\n");
    return -1;
  }
  memset(&sin6, '0', sizeof(sin6));
  sin6.sin6_family = AF_INET6;
  sin6.sin6_port = htons(atoi(port));
  if(inet_pton(AF_INET6, host, &sin6.sin6_addr)<0)
  {
    printf("L'adresse n'est pas valide\n");
    return -1;
  }
  if(connect(sock, (struct sockaddr *)&sin6, sizeof(sin6))<0)
  {
    printf("Erreur connect\n");
    return -1;
  }
  return sock;
}

int receive_message(int sock, char *buffer, int size) {
  char received[MESSAGE_MAXLEN];
  if(write(sock, buffer, size)<0) {
    perror("Erreur write\n");
  }
  if(read(sock, received, size)<0) {
    perror("Erreur read\n");
  }
  if((strncmp(buffer, "quit", 4)==0)||(strncmp(buffer, "q", 1)==0)) {
    printf("Vous vous etes deconecte\n"); //Le message de deconnection
    return 0;
  }
  printf("%s", received);
  memset(received, '\0', size); //supprimer le contenue du buffer
  return 1;
}

void speak_to_server(int sock) {
  int rc;
  while(1){
    printf(">>> ");
    char buf[MESSAGE_MAXLEN];
    fgets(buf, sizeof(buf), stdin);
    rc = receive_message(sock, buf, sizeof(buf));
    if(rc<0) {
      perror("Le message n'a pas ete recu\n");
    }
    if(rc==0) {
      break; //le client se deconnecte
    }
  }
}

int main(int argc ,char **argv){
  if (argc != 3){
    perror("Il faut respecter le format ./client ::1 port\n");
    exit(2);
  }
  int port = htons(atoi(argv[2]));
  if(port<1024 || port>65535) {
    perror("Il faut que le port soit dans l'intervale [1024, 65535]\n");
    exit(2);
  }
  int client_socket = connect_server(argv[1],argv[2]);
  if(client_socket<0) {
    perror("Erreur de conection avec le serveur\n");
    exit(2);
  }
  speak_to_server(client_socket);
  if(close(client_socket)<0){
    perror("Erreur close\n");
    exit(2);
  }
  return 0;
}
