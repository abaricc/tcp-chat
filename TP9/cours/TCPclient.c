/**
 * usage : TCPcli host port
 *
 */

#include "createSocket.h"
#include <string.h>     /* for memset */
#include <arpa/inet.h>  /* for inet_pton */

int	get_server_address_1(char *hostname, char *port, int af, struct sockaddr *saddr);
int	get_server_address_2(char *hostname, char *port, int af, struct sockaddr *saddr);
int	get_server_address_3(char *hostname, char *port, int af, struct sockaddr *saddr);

int
main(int argc, char *argv[])
{
  struct sockaddr_in saddr, caddr;	/* server and client IPv4 address for
					 * the socket */
  socklen_t l_saddr = sizeof(saddr);	/* size of socket address */
  int	  port, cdescr;

  if (argc < 3) {
    perror("Usage TCPclient serverhostname port\nquit\n");
    exit(2);
  }
  /* create the client socket on any port */
  cdescr = createSocket(AF_INET, SOCK_STREAM, 0, (struct sockaddr *)&caddr);
  if (cdescr == -1) {
    perror("Creation socket client impossible\n");
    exit(2);
  }
  printf("* Client TCP sur port %d\n", ntohs(caddr.sin_port));

  /* find server address */
  /* - with gethostbyname */
  if (-1 == get_server_address_1(argv[1], argv[2], AF_INET, (struct sockaddr *)&saddr))
    exit(2);
  /* - with inet_pton */
  /* if (-1 == get_server_address_2(argv[1], argv[2], AF_INET, (struct sockaddr *)&saddr))
    exit(2); */
  /* - with getaddrinfo */
  /* if (-1 == get_server_address_3(argv[1], argv[2], AF_INET, (struct sockaddr *)&saddr))
    exit(2); */

  /* connect to the server */
  if (connect(cdescr, (struct sockaddr *)&saddr, sizeof(saddr)) == -1) {
    perror("... connexion impossible\nquit\n");
    exit(2);
  }
  printf("... connexion acceptee\n");

  int	  lread = 0;
  char	  buf[512];
  char	  c;

  do {
    memset(buf, 0, 512);
    printf("Message pour le serveur: ");
    fflush(stdout);
    scanf("%s", buf);
    printf("... envoye %s\n", buf);
    write(cdescr, buf, strlen(buf) + 1);
    lread = read(cdescr, buf, 512);
    printf("... recu %d caracteres, %s\n", lread, buf);
  } while (strncmp(buf, "quit", 4) != 0);

  close(cdescr);

  return 0;
}

/*
 * Use gethostbyname2
 */
int
get_server_address_1(char *hostname, char *port, int af, struct sockaddr *saddr)
{
  struct hostent *hs;		/* for server host */
  /* search the server hostname */
  if ((hs = gethostbyname2(hostname, af)) == NULL) {
    perror("Machine serveur inconnue\n");
    return -1;
  }
  if (af == AF_INET) {
    struct sockaddr_in* saddr4 = (struct sockaddr_in*) saddr;
    memset(saddr4, 0, sizeof(struct sockaddr_in));
    saddr4->sin_family = AF_INET;
    saddr4->sin_port = htons(atoi(port));
    memcpy(&(saddr4->sin_addr.s_addr), hs->h_addr, hs->h_length);
  } else {
    struct sockaddr_in6* saddr6 = (struct sockaddr_in6*) saddr;
    memset(saddr6, 0, sizeof(struct sockaddr_in6));
    saddr6->sin6_family = AF_INET6;
    saddr6->sin6_port = htons(atoi(port));
    memcpy(&(saddr6->sin6_addr.s6_addr), hs->h_addr, hs->h_length);
  }
  return 0;
}

/*
 * Use inet_pton
 */
int
get_server_address_2(char *hostname, char *port, int af, struct sockaddr *saddr)
{
  if (af == AF_INET) {
    struct sockaddr_in* saddr4 = (struct sockaddr_in*) saddr;
    memset(saddr4, 0, sizeof(struct sockaddr_in));
    saddr4->sin_family = AF_INET;
    saddr4->sin_port = htons(atoi(port));
    int	    rc = inet_pton(AF_INET, hostname, &(saddr4->sin_addr.s_addr));
    if (0 == rc) {
      printf("Adresse hote mal formee\n");
      return -1;
    } else if (0 > rc) {
      perror("Machine serveur inconnue\n");
      return -1;
    }
  } else {
    struct sockaddr_in6* saddr6 = (struct sockaddr_in6*) saddr;
    memset(saddr6, 0, sizeof(struct sockaddr_in6));
    saddr6->sin6_family = AF_INET6;
    saddr6->sin6_port = htons(atoi(port));
    int	    rc = inet_pton(AF_INET6, hostname, &(saddr6->sin6_addr.s6_addr));
    if (0 == rc) {
      printf("Adresse hote mal formee\n");
      return -1;
    } else if (0 > rc) {
      perror("Machine serveur inconnue\n");
      return -1;
    }
  }
  return 0;
}

/*
 * Use getaddrinfo Warning : port is the name of the service
 */
int
get_server_address_3(char *hostname, char *port, int af, struct sockaddr *saddr)
{
  struct addrinfo hints;
  struct addrinfo *r, *p;
  int	  s, rc;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = af;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  rc = getaddrinfo(hostname, port, &hints, &r);
  if (rc != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
    exit(1);
  }
  for (p = r; p != NULL; p = p->ai_next) {
    s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (s < 0) {
      close(s);
      continue;
    }
    rc = connect(s, p->ai_addr, p->ai_addrlen);
    if (rc >= 0)
      break;
    close(s);
  }

  if (p == NULL) {
    fprintf(stderr, "La connection a échoué.\n");
    exit(1);
  }
  /* connexion success */
  close(s);
  if (af == AF_INET) {
    struct sockaddr_in* saddr4 = (struct sockaddr_in*) saddr;
    memset(saddr4, 0, sizeof(struct sockaddr_in));
    saddr4->sin_family = AF_INET;
    saddr4->sin_port = htons(atoi(port));
    memcpy(&(saddr4->sin_addr.s_addr), p->ai_addr, p->ai_addrlen);
  } else {
    struct sockaddr_in6* saddr6 = (struct sockaddr_in6*) saddr;
    memset(saddr6, 0, sizeof(struct sockaddr_in6));
    saddr6->sin6_family = AF_INET6;
    saddr6->sin6_port = htons(atoi(port));
    memcpy(&(saddr6->sin6_addr.s6_addr), p->ai_addr, p->ai_addrlen);
  }
  freeaddrinfo(r);
  return 0;
}
