/**
 * Function for creation and binding of a socket 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

int createSocket(int domain, int type, int port, struct sockaddr *p_addr);
