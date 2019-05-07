#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <string.h>

int client_arrived(int client_sock) {
  int rc;
  char buf[512];
  printf("Le client est arrive\n");
  while(1) {
    rc = read(client_sock, buf, sizeof(buf));
    if(rc==0) {
      printf("Le client s'est deconnecte\n");
      break;
    }
    if(rc<0) {
      perror("Erreur read\n");
      break;
    }
    buf[rc]='\0';
    printf("%s", buf);
    if(write(client_sock, buf, rc)<0) {
      perror("Erreur write\n");
      break;
    }
    memset(buf ,'\0', 512); //supprimer le contenue du buffer
  }
  if(close(client_sock)<0) {
      perror("Erreur close");
      return -1;
  }
  return 0;
}

int main(int argc, char* argv[]) {
  if(argc!=2) {
    perror("Il faut respecter le format ./server port\n");
    exit(EXIT_FAILURE);
  }
  int port = htons(atoi(argv[1]));
  if(port<1024 || port>5535) {
    perror("Il faut que le port soit dans l'intervale [1024, 65535]\n");
    exit(EXIT_FAILURE);
  }
  struct sockaddr_in6 sin6;
  memset(&sin6, 0, sizeof(sin6));
  sin6.sin6_family = AF_INET6;
  sin6.sin6_port = port;
  int srv_sock=socket(AF_INET6, SOCK_STREAM, 0);
  if(srv_sock<0) {
    perror("Erreur socket\n");
    exit(EXIT_FAILURE);
  }
  int optval=1;
  if(setsockopt(src_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))<0) {
    perror("Erreur socket\n");
    exit(EXIT_FAILURE);
  }
  if(bind(srv_sock, (struct sockaddr *)&sin6, sizeof(sin6))<0) {
    perror("Erreur bind\n");
    exit(EXIT_FAILURE);
  }
  if(listen(srv_sock, 10)<0) {
    perror("Erreur listen\n");
    exit(EXIT_FAILURE);
  }
  while(1) {
    int client_sock = accept(srv_sock, NULL, NULL);
    if(client_sock<0) {
      perror("Erreur accept\n");
    }
    if(client_arrived(client_sock)<0) {
      perror("Erreur de communication avec le client\n");
    }
  }
  return 0;
}
