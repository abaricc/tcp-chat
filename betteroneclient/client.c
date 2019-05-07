#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

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

int received_message(int sock, char *buffer, int size) {
    char received[size];
    if(write(sock, buffer, size)<0) {
        perror("Erreur write\n");
    }
    if(read(sock, received, size)<0) {
        perror("Erreur read\n");
    }
    printf("%s", received);
    if((strncmp(buffer, "quit", 4)==0)||(strncmp(buffer, "q", 1)==0)){
        printf("Vous vous etes deconecte\n");
        return 0;
    }
    memset(received, '\0', size); //supprimer le contenue du buffer
    return 1;
}

void speak_to_server(int sock) {
    int rc;
    while(1){
        printf(">>> ");
        char buf[512];
        fgets(buf, 512, stdin);
        rc = received_message(sock, buf, 512);
        if(rc<0) {
          perror("Le message n'a pas ete recu\n");
        }
        if(rc==0) {
          //Le message de deconnection a deja ete affiche dans received_message
          break;
        }
    }
}

int main(int argc ,char **argv){
    if (argc != 3){
      perror("Il faut respecter le format ./client ::1 port\n");
      exit(EXIT_FAILURE);
    }
    int port = htons(atoi(argv[2]));
    if(port<1024 || port>5535) {
      perror("Il faut que le port soit dans l'intervale [1024, 65535]\n");
      exit(EXIT_FAILURE);
    }
    int client_socket = connect_server(argv[1],argv[2]);
    if(client_socket<0) {
      perror("Erreur de conection avec le serveur\n");
      exit(EXIT_FAILURE);
    }
    speak_to_server(client_socket);
    if(close(client_socket)<0){
      perror("Erreur close\n");
      exit(EXIT_FAILURE);
    }
    return 0;
}