#include"stdio.h"  
#include"stdlib.h"  
#include"sys/types.h"  
#include"sys/socket.h"  
#include"string.h"  
#include"netinet/in.h"  
#include"netdb.h"
#include"pthread.h"
#include <errno.h>
#include <sysexits.h>
  
#define PORT 4444 
#define BUF_SIZE 2000 
  
int conectado;
char * usuario;

/**
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
int validateMsg(char * command, char * message, char * arg1, char * arg2, int conv){

  if ((conectado == 0) && strcmp("conectarse", command) != 0) {
    return(-2);
  }

  if (strcmp("conectarse", command) == 0 && conv == 2){
    if (sscanf(message,"%ms %ms",&arg1, &arg2) == 2) {
      return(1);
    } else {
      return(-1);
    }
  } else if (strcmp("salir", command) == 0){
      return(0);
  } else if (strcmp("entrar", command) == 0 && conv == 2){
      return(1);
  } else if (strcmp("dejar", command) == 0 && conv == 2){
      return(1);
  } else if (strcmp("ver_salas", command) == 0){
      return(0);
  } else if (strcmp("ver_usuarios", command) == 0){
      return(0);
  } else if (strcmp("ver_usu_salas", command) == 0 && conv == 2){
      return(1);
  } else if (strcmp("env_mensaje", command) == 0  && conv == 2){
	  if(strlen(message) <= 70){
        return(1);
  	  } else {
        return(-1);
      }
  } else if (strcmp("crear_usu", command) == 0 && conv == 2){
    if (sscanf(message,"%ms %ms",&arg1, &arg2) == 2) {
      return(1);
    } else {
      return(-1);
    }
  } else if (strcmp("elim_usu", command) == 0 && conv == 2){
    if (sscanf(message,"%ms %ms",&arg1, &arg2) == 2) {
      return(1);
    } else {
      return(-1);
    }
  } else if (strcmp("crear_sala", command) == 0 && conv == 2){
      return(1);
  } else if (strcmp("elim_sala", command) == 0 && conv == 2){
      return(1);
  } else if (strcmp("hab_sala", command) == 0 && conv == 2){
      return(1);
  } else if (strcmp("deshab_sala", command) == 0 && conv == 2){
      return(1);
  } else if (strcmp("ver_log", command) == 0){
      return(0);
  } else {
      return(-1);
  }
}

void * receiveMessage(void * socket) {
  int sockfd, ret;
  char buffer[BUF_SIZE]; 
  sockfd = (int) socket;
  memset(buffer, 0, BUF_SIZE);

  for (;;) {
    ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);  
    if (ret < 0) {  
      printf("Error recibiendo data!\n");    
    } else {

      printf("server: ");
      fputs(buffer, stdout);
    }
  }
}

int main(int argc, char**argv) {  
  struct sockaddr_in addr, cl_addr;  
  int sockfd, ret;  
  char buffer[BUF_SIZE]; 
  char * serverAddr;
  char * command;
  char * message;
  char * arg1;
  char * arg2;
  char * buf;
  int valid;
  int conv;
  pthread_t rThread;
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
  addr.sin_port = PORT;     
  
  ret = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));  
  if (ret < 0) {  
    printf("Error conectando con el servidor!\n");  
    exit(1);  
  }  
  printf("Conexion con el servidor establecida\n");  
  
  memset(buffer, 0, BUF_SIZE);
  
  ret = pthread_create(&rThread, NULL, receiveMessage, (void *) sockfd);
  if (ret) {
    printf("ERROR: Return Code from pthread_create() is %d\n", ret);
    exit(1);
  }
  
  while (fgets(buffer, BUF_SIZE, stdin) != NULL) {


    // Validar el formato del comando del usuario
    conv = sscanf(buffer,"%ms %m[^\t\n]",&command, &message);
    //printf("\nComando %s. Argumento %s. Conversión %d\n", command, message, conv);
      
    if (conv != -1) {

      valid = validateMsg(command, message, arg1, arg2, conv);

      if (valid == 0) {
	      free(command);
          free(message);
      } else if (valid == 1){
          free(command);
          free(message);
      } else if (valid == 2){
          free(arg1);
          free(arg2);
          free(command);
          free(message);
      } else if (valid == -2){
        printf("\nDebe autenticarse para llevar a cabo esa operación.\n");
      } else {
         printf("\nFormato de Mensaje incorrecto.\n");
      }
        if (valid >= 0){
          ret = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &addr, sizeof(addr));
          if (ret < 0) {  
            printf("Error enviando data!\n\t-%s", buffer);
      	  }
        }
    } else {
        conv = 0;
    }
  }
  
  close(sockfd);
  pthread_exit(NULL);
 
 return 0;    
}  
