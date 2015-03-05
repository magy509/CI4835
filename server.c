#include"stdio.h"
#include"stdlib.h"
#include"sys/types.h"
#include"sys/socket.h"
#include"string.h"
#include"netinet/in.h"
#include"pthread.h"
#include <errno.h>
#include <sysexits.h>

#define PORT 4444
#define BUF_SIZE 2000
#define CLADDR_LEN 100

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

  if (strcmp("conectar", command) == 0 && conv == 2){
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
  } else if (strcmp("habilitar", command) == 0 && conv == 2){
      return(1);
  } else if (strcmp("deshabilitar", command) == 0 && conv == 2){
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
  char * command;
  char message[100];
  char * arg1;
  char * arg2;
  char * buf;
  int valid;
  int conv;
  sockfd = (int) socket;
  memset(buffer, 0, BUF_SIZE);  
  for (;;) {
    ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL); 

    if (ret < 0) {  
      printf("Error recibiendo data!\n");    
    } else {

    // Validar el formato del comando del cliente
    conv = sscanf(buffer,"%ms %[^\t\n]",&command, message);
      
    if (conv != -1) {

      sscanf(message,"%ms %ms",&arg1, &arg2);
      valid = validateMsg(command, message, arg1, arg2, conv);

      if (valid == 0) {
        // Comando sin argumentos.
	      free(command);

      } else if (valid == 1){
        // Comando de un argumento message.
          free(command);

      } else if (valid == 2){
        // Comando de dos argumentos arg1 y arg2.
          free(arg1);
          free(arg2);
          free(command);
  
      } else {
         printf("\nFormato de Mensaje incorrecto.\n");
      }
    } else {
        conv = 0;
         printf("\nError de datos???\n");
    }

      if (valid != -1){
        printf("client: ");
        fputs(buffer, stdout);
      }
    }  
 }
}

void main() {
  struct sockaddr_in addr, cl_addr;
  int sockfd, len, ret, newsockfd;
  char buffer[BUF_SIZE];
  pid_t childpid;
  char clientAddr[CLADDR_LEN];
  pthread_t rThread;
 
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    printf("Error creando socket!\n");
    exit(1);
  }
 
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = PORT;
 
  ret = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
  if (ret < 0) {
    printf("Error binding!\n");
    exit(1);
 }
  
  printf("Esperando conexion...\n");
  listen(sockfd, 5);
  
  len = sizeof(cl_addr);
  newsockfd = accept(sockfd, (struct sockaddr *) &cl_addr, &len);
  if (newsockfd < 0) {
    printf("Error aceptando conexion!\n");
    exit(1);
  } 
  
  inet_ntop(AF_INET, &(cl_addr.sin_addr), clientAddr, CLADDR_LEN);
  printf("Coneccion establecida con %s...\n", clientAddr); 
  
  memset(buffer, 0, BUF_SIZE);
  
  ret = pthread_create(&rThread, NULL, receiveMessage, (void *) newsockfd);
  if (ret) {
    printf("ERROR: Return Code from pthread_create() is %d\n", ret);
    exit(1);
  }
  
  while (fgets(buffer, BUF_SIZE, stdin) != NULL) {
    ret = sendto(newsockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &cl_addr, len);  
    if (ret < 0) {  
      printf("Error enviando data!\n");  
      exit(1);
    }
  }   
  
  close(newsockfd);
  close(sockfd);
  
  pthread_exit(NULL);
  return;
}
