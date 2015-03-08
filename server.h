#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include <sys/prctl.h>
#include <linux/prctl.h>

#include <glib.h>

extern GList * user;
extern GList * room;

// Estructura sala
struct sala {
  char *nombre;
  int status;
  int n_usuarios;
  GList * usuarios;
};

// Estructura usuario
struct usuario {
  char *nombre;
  char *clave;
  int status;
  int is_admin;
};

#endif
