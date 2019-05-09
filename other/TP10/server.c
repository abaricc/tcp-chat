#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <string.h>
#include <arpa/inet.h>

/*
int random() {
  return rannd();
}
*/
void client_arrived(int client_sock) {
  int rc;
  char buf[100];
  char *first_word, *the_rest;
  while(1) {
    rc = read(client_sock, buf, sizeof(buf));
    if(rc==0) {
      printf("Le client est partie\n");
      break;
    }
    if(rc<0) {
      perror("Erreur de lecture\n");
      break;
    }
    buf[rc]='\0';
    first_word = strsep(buf," ");
    if(write(client_sock, buf, rc)<0) {
      perror("Erreur d'ecriture\n");
      break;
    }
  }
  close(client_sock);
}

int main(int argc, char* argv[]) {
  //signal();
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
  listen(src_sock, 10);
  while(1) {
    int client_sock = accept(src_sock, NULL, NULL);
    client_arrived(client_sock);
  }
}
