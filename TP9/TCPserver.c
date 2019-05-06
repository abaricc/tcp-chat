/**
 * usage : TCPserv port ...
 *
 */

#include "createSocket.h"
#include <pthread.h>
#include <string.h>

/// type of the service done
typedef void *(*service_t)(void*);

/// service of the server : echo
void* service(void* p) {
  int cdescr = (int)p;
  char buf[1024];

  while(1) {
    memset(buf, 0, 1024);
    ssize_t nb_read = read(cdescr, buf, 1024);
    printf("... server read %zu char: %s\n", nb_read, buf);

    if (nb_read > 0)
      write(cdescr, buf, nb_read);
    else
      break;
  }

  return NULL;
}

int main(int argc, char* argv[]) {

  if (argc < 2) {
    perror("Usage TCPserv port\nquit\n");
    exit(2);
  }

  struct sockaddr_in saddr; /// address IPv4 of connexion socket
  socklen_t l_saddr = sizeof(saddr);
  pthread_t tid; /// thread of server doing the service
  pthread_attr_t tattr;
  int port = atoi(argv[1]); /// port for service
  int ldescr, cdescr; /// listen and connexion descriptors
  char c;


  /// create listening socket IPv4
  ldescr = createSocket(AF_INET, SOCK_STREAM, port, (struct sockaddr*) &saddr);
  if (ldescr == -1) {
    perror("Creation socket d'ecoute impossible.\nquit\n");
    exit(2);
  }
  printf("* Server TCP port %d (%d)\n", ntohs(saddr.sin_port), saddr.sin_port);
  getchar();

  /// open the service
  if (listen(ldescr, 10)) {
    perror("Ouverture service impossible.\nquit\n");
    exit(2);
  }
  /// loop for waiting connexions
  do {
    /// accept a new connexion
    cdescr = accept(ldescr, (struct sockaddr*) &saddr, &l_saddr);
    if (cdescr == -1) {
      perror("... erreur accept\nquit\n");
      exit(2);
    }

    /// prepare thread for dealing with the connexion
    pthread_attr_init(&tattr);
    /// set thread as being detached from the father, father not waiting for its end
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    /// set the scope of the thread as a system thread
    pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM);
    if (pthread_create(&tid, &tattr, (service_t)service, (void*) cdescr) == 0)
      continue;
    else {
      /// problem at thread creation, close the connexion
      close(cdescr);
    }
    c = getchar();
  } while (c != 'q');

}
