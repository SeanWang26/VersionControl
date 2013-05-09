#ifndef UNIX_DOMAIN_SOCKETS_H
#define UNIX_DOMAIN_SOCKETS_H

#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stddef.h>


#define QLEN 5

/*
 * Create a server endpoint of a connection.
 * Return fd if all OK, <0 on error.
 */
int
serv_listen(const char *name);

#define STALE 30 /* client's name can't be older than this (sec) */
	
/*
 * Wait for a client connection to arrive, and accept it.
 * We also obtain the client's user ID from the pathname
 * that it must bind before calling us.
 * Returns new fd if all OK, <0 on error
 */
int
serv_accept(int listenfd, uid_t *uidptr);


#define CLI_PATH "/var/tmp" /* +5 for pid = 14 chars */
#define CLI_PERM S_IRWXU /* rwx for user only */

/*
  * Create a client endpoint and connect to a server.
  * Returns fd if all OK, <0 on error.
  */
int
cli_conn(const char *name);

#endif
