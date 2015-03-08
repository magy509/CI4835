#ifndef CLIENT_H
#define CLIENT_H

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

// Estructura sala
struct usuario {
  char *nombre;
  int is_admin;
};

#endif
