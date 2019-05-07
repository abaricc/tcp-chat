#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <string.h>
#include <arpa/inet.h>
#define BUFSIZE 100

void client_arrived(int client_sock) {
  int rc;
  char buf[BUFSIZE];
  printf("Le client %d est arrive\n", client_sock);
  while(1) {
    rc = read(client_sock, buf, sizeof(buf));
    if(rc==0) {
      printf("Le client %d est partie\n", client_sock);
      break;
    }
    if(rc<0) {
      perror("Erreur de lecture\n");
      break;
    }
    buf[rc]='\0';
    printf("%s\n", buf);
    if(write(client_sock, buf, rc)<0) {
      perror("Erreur d'ecriture\n");
      break;
    }
    memset(buf ,'\0', 100); //supprimer le contenue du buffer
  }
  if(close(client_sock)<0) {
      perror("Erreur de fermeture");
      exit(-1);
  }
}

int main(int argc, char* argv[]) {
  if(argc!=2) {
    perror("Il faut respecter le format ./server port\n");
    return -1;
  }
  int port = htons(atoi(argv[1]));
  if(port<1024 || port>5535) {
    perror("Il faut que le port soit dans l'intervale [1024, 65535]\n");
    return -1;
  }
  struct sockaddr_in6 sin6;
  memset(&sin6, 0, sizeof(sin6));
  sin6.sin6_family = AF_INET6;
  sin6.sin6_port = port;
  int src_sock=socket(AF_INET6, SOCK_STREAM, 0);
  if ( src_sock < 0 ){
    perror("Erreur socket\n");
    return -1;
  }
  if(bind(src_sock, (struct sockaddr *)&sin6, sizeof(sin6))<0) {
    perror("Erreur bind\n");
    return -1;
  }
  if(listen(src_sock, 10)<0) {
    perror("Erreur listen\n");
    return -1;
  }
  while(1) {
    int client_sock = accept(src_sock, NULL, NULL);
    if(client_sock<0) {
      perror("Erreur accept\n");
    }
    client_arrived(client_sock);
  }
  return 0;
}
