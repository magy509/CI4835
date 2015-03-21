#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sysexits.h>
#include <pthread.h>
#include <unistd.h>
#include "comando.h"

#define PORT 4444
#define BUF_SIZE 2000

int conectado;
char * result;
char * usuario;

/**
 * Se encarga de escribir en el file descriptor del socket el contenido
 * de buf.
 * @param fd file descriptor del socket donde se va a escribir.
 * @param buf contenido que se va a escribir en fd.
 * @param count tamaño del contenido que se va a escribir.
 */
void escribir(int fd, void * buf, size_t count) {
  while (count > 0) {
    int escrito = write(fd, buf, count);
    if (-1 == escrito) {
      if (EINTR == errno) continue;
      perror("write");
      exit(EX_IOERR);
    }

    count -= escrito;
    buf += escrito;
  }
}

/**
 * Se encarga de leer del file descriptor del socket el contenido
 * de buf.
 * @param socket file descriptor del socket del que se va a leer.
 * @param buf direccion donde se almacenará lo leido en fd.
 * @param count tamaño del contenido que se va a leer.
 */
void leer(int socket, void * buf, size_t count) {
  while (count > 0) {
    int leido = read(socket, buf, count);

    if (-1 == leido) {
      if (EINTR == errno) continue;
      perror("read");
      exit(EX_IOERR);
    }

    count -= leido;
    buf += leido;
  }
}

/*
 * Valida que lo ingresado por el usuario tenga un formato válido.
 * Esto abarca comandos y cantidad de argumentos.
 * @param comando recibido por cónsola.
 * @param string que sigue al comando.
 * @param argumento auxiliar para separar el string message.
 * @param argumento auxiliar para separar el string message.
 * @param cantidad de elementos que fueron convertidos exitosamente en el sscanf.
 * @return Devuelve 0 si el comando no utiliza argumentos. Devuelve 1 si el comando utiliza 1 argumento.
 * Devuelve 2 si el comando utiliza dos argumentos. Devuelve -1 si es un comando inválido.
**/
int validateMsg(char * command, char * message, char * arg1, char * arg2, int conv, int sockfd){
  int salida;
  if (strcmp("conectarse", command) == 0 && conv == 2){
    if (conectado == 1) {
      return 0;
    }
    if (sscanf(message,"%ms %ms",&arg1, &arg2) == 2) {
      int comando = COMANDO_CONECTARSE;
      escribir(sockfd, &comando, sizeof(int));
      int longitud_arg1 = strlen(arg1);
      escribir(sockfd, &longitud_arg1, sizeof(int));
      escribir(sockfd, arg1, longitud_arg1);
      int longitud_arg2 = strlen(arg2);
      escribir(sockfd, &longitud_arg2, sizeof(int));
      escribir(sockfd, arg2, longitud_arg2);
      conectado = 1; 
        return 1;
      
    } else {
      return -1;
    }
  
  } else if (strcmp("salir", command) == 0 && conectado != 0){
      enum comando comando = COMANDO_SALIR;
      escribir(sockfd, &comando, sizeof(comando));
      conectado = 0;
      return 1;
  
  } else if (strcmp("entrar", command) == 0 && conv == 2 && conectado != 0){
      enum comando comando = COMANDO_ENTRAR;
      escribir(sockfd, &comando, sizeof(comando));
      int longitud_arg1 = strlen(message);
      escribir(sockfd, &longitud_arg1, sizeof(int));
      escribir(sockfd, message, longitud_arg1);
      return 1;
  
  } else if (strcmp("dejar", command) == 0 && conv == 2 && conectado != 0){
      enum comando comando = COMANDO_DEJAR;
      escribir(sockfd, &comando, sizeof(comando));
      int longitud_arg1 = strlen(message);
      escribir(sockfd, &longitud_arg1, sizeof(int));
      escribir(sockfd, message, longitud_arg1);
      return 1;
  
  } else if (strcmp("ver_salas", command) == 0 && conectado != 0){
      int comando = COMANDO_VER_SALAS;
      escribir(sockfd, &comando, sizeof(comando));
      return 1;
  
  } else if (strcmp("ver_usuarios", command) == 0 && conectado != 0){
      enum comando comando = COMANDO_VER_USUARIOS;
      escribir(sockfd, &comando, sizeof(comando));
      return 1;
  
  } else if (strcmp("ver_usu_salas", command) == 0 && conv == 2 && conectado != 0){
      enum comando comando = COMANDO_VER_USUARIOS_SALAS;
      escribir(sockfd, &comando, sizeof(comando));
      int longitud_arg1 = strlen(message);
      escribir(sockfd, &longitud_arg1, sizeof(int));
      escribir(sockfd, message, longitud_arg1);
      return 1;
  
  } else if (strcmp("env_mensaje", command) == 0  && conv == 2 && conectado != 0){
      if(strlen(message) <= 70){
        enum comando comando = COMANDO_ENVIAR_MENSAJE;
        escribir(sockfd, &comando, sizeof(comando));
        int longitud_arg1 = strlen(message);
        escribir(sockfd, &longitud_arg1, sizeof(int));
        escribir(sockfd, message, longitud_arg1);
        return 1;
  	  } else {
        return -1;
      }
  
  } else if (strcmp("crear_usu", command) == 0 && conv == 2 && conectado != 0){
    if (sscanf(message,"%ms %ms",&arg1, &arg2) == 2) {
      int comando = COMANDO_CREAR_USUARIO;
      escribir(sockfd, &comando, sizeof(int));
      int longitud_arg1 = strlen(arg1);
      escribir(sockfd, &longitud_arg1, sizeof(int));
      escribir(sockfd, arg1, longitud_arg1);
      int longitud_arg2 = strlen(arg2);
      escribir(sockfd, &longitud_arg2, sizeof(int));
      escribir(sockfd, arg2, longitud_arg2);
      return 1;
    } else {
      return -1;
    }
  
  } else if (strcmp("elim_usu", command) == 0 && conv == 2 && conectado != 0){
    if (sscanf(message,"%ms %ms",&arg1, &arg2) == 2) {
      enum comando comando = COMANDO_ELIMINAR_USUARIO;
      escribir(sockfd, &comando, sizeof(comando));
      int longitud_arg1 = strlen(arg1);
      escribir(sockfd, &longitud_arg1, sizeof(int));
      escribir(sockfd, arg1, longitud_arg1);
      int longitud_arg2 = strlen(arg2);
      escribir(sockfd, &longitud_arg2, sizeof(int));
      escribir(sockfd, arg2, longitud_arg2);
      return 1;
    } else {
      return -1;
    }
  
  } else if (strcmp("crear_sala", command) == 0 && conv == 2 && conectado != 0){
      enum comando comando = COMANDO_CREAR_SALA;
      escribir(sockfd, &comando, sizeof(comando));
      int longitud_arg1 = strlen(message);
      escribir(sockfd, &longitud_arg1, sizeof(int));
      escribir(sockfd, message, longitud_arg1);
      return 1;
  
  } else if (strcmp("elim_sala", command) == 0 && conv == 2 && conectado != 0){
      enum comando comando = COMANDO_ELIMINAR_SALA;
      escribir(sockfd, &comando, sizeof(comando));
      int longitud_arg1 = strlen(message);
      escribir(sockfd, &longitud_arg1, sizeof(int));
      escribir(sockfd, message, longitud_arg1);
      return 1;
  
  } else if (strcmp("hab_sala", command) == 0 && conv == 2 && conectado != 0){
      enum comando comando = COMANDO_HABILITAR_SALA;
      escribir(sockfd, &comando, sizeof(comando));
      int longitud_arg1 = strlen(message);
      escribir(sockfd, &longitud_arg1, sizeof(int));
      escribir(sockfd, message, longitud_arg1);
      return 1;
  
  } else if (strcmp("deshab_sala", command) == 0 && conv == 2 && conectado != 0){
      enum comando comando = COMANDO_DESHABILITAR_SALA;
      escribir(sockfd, &comando, sizeof(comando));
      int longitud_arg1 = strlen(message);
      escribir(sockfd, &longitud_arg1, sizeof(int));
      escribir(sockfd, message, longitud_arg1);
      return 1;
  
  } else if (strcmp("ver_log", command) == 0 && conectado != 0){
      return 1;
      enum comando comando = COMANDO_VER_BITACORA;
      escribir(sockfd, &comando, sizeof(comando));
  
  } else {
      return -1;
  }
}

int sockfd;

void * printer(void * data) {
  size_t len;
  char buffer[1000];
  for (;;) {
    len = read(sockfd, buffer, 1000);
    escribir(STDOUT_FILENO, buffer, len);
  }
}

int main(int argc, char**argv) {
  struct sockaddr_in addr, cl_addr;
  char buffer[BUF_SIZE];
  char * serverAddr;
  char * arg1;
  char * arg2;
  char * buf;
  int ret;
  int valid;
  conectado = 0;  // Esta variable debe cambiar cuando el usuario se conecte
  if (argc < 2) {
    printf("usage: client < ip address >\n");
    exit(1);
  }

  serverAddr = argv[1];

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    printf("Error creando socket!\n");
    exit(1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(serverAddr);
  addr.sin_port = htons(PORT);

  ret = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
  if (ret < 0) {
    printf("Error conectando con el servidor!\n");
    exit(1);
  }
  printf("Conexion con el servidor establecida\n");

  {
    pthread_t t;
    pthread_create(&t, NULL, printer, NULL);
  }

  for (;;) {
    // Validar el formato del comando del usuario
    char * command;
    char * message;
    char * line = NULL;
    size_t line_len;
    if (-1 == getline(&line, &line_len, stdin)){
      exit(0);
    };
    int conv = sscanf(line,"%ms %m[^\t\n]", &command, &message);

    if (conv != -1) {

      valid = validateMsg(command, message, arg1, arg2, conv, sockfd);

      if (valid == 0) {
        printf("El usuario ya está conectado. Debe desconectarse antes de iniciar una nueva conexión.\n");
      } else if (valid == -2){
        printf("Debe autenticarse para llevar a cabo esa operación.\n");
      } else if (valid == -1){
        printf("Formato de Mensaje incorrecto.\n");
      }
    } else {
        conv = 0;
    }
  }

  close(sockfd);

  return 0;
}
