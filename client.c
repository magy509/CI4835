#include"stdio.h"  
#include"stdlib.h"  
#include"sys/types.h"  
#include"sys/socket.h"  
#include"string.h"  
#include"netinet/in.h"  
#include"netdb.h"
#include"pthread.h"
  
#define PORT 4444 
#define BUF_SIZE 2000 
  
int validateMsg(char * command, char * message){
  char arg1[10];
  char arg2[10];

  if (strcmp("conectar", command) == 0){
    sscanf(message,"%s %s",arg1,arg2);
    printf("mensaje:%s \n", message);
    printf("usuario:%s\n", arg1);
    printf("contraseña:%s\n", arg2);

      // Valido que no sean nulos //TODO
      if (arg1 != NULL && arg2 != NULL){
        return(1);
      } else {
        return(-1);
      }
  } else if (strcmp("salir", command) == 0){
      return(1);
  } else if (strcmp("entrar", command) == 0){
      // Valido que la sala no sea nula //TODO
      if (message != NULL){
        return(1);
      } else {
        return(-1);
      }
  } else if (strcmp("dejar", command) == 0){
      // Valido que la sala no sea nula //TODO
      if (message != NULL){
        return(1);
      } else {
        return(-1);
      }
  } else if (strcmp("ver_salas", command) == 0){
      return(1);
  } else if (strcmp("ver_usuarios", command) == 0){
    return(1);
  } else if (strcmp("ver_usu_salas", command) == 0){
      // Valido que la sala no sea nula //TODO
      if (message != NULL){
        return(1);
      } else {
        return(-1);
      }
  } else if (strcmp("env_mensaje", command) == 0){

      printf("palabra:%d \n", strlen(message));

	  if(strlen(message) <= 70 && strcmp(message,"") != 0){ //TODO
        return(1);
  	  } else {
        return(-1);
      }
  } else if (strcmp("crear_usu", command) == 0){
      // Valido que el nombre y la contraseña no sean nulos //TODO
      if (arg1 != NULL && arg2 != NULL){
        return(1);
      } else {
        return(-1);
      }
  } else if (strcmp("elim_usu", command) == 0){
      // Valido que el nombre y la contraseña no sean nulos //TODO
      if (arg1 != NULL && arg2 != NULL){
        return(1);
      } else {
        return(-1);
      }
  } else if (strcmp("crear_sala", command) == 0){
      // Valido que el nombre de la sala no sea nulo //TODO
      if (message != NULL){
        return(1);
      } else {
        return(-1);
      }
  } else if (strcmp("elim_sala", command) == 0){
      // Valido que el nombre de la sala no sea nulo //TODO
      if (message != NULL){
        return(1);
      } else {
        return(-1);
      }
  } else if (strcmp("habilitar", command) == 0){
      // Valido que el nombre de la sala no sea nulo //TODO
      if (message != NULL){
        return(1);
      } else {
        return(-1);
      }
  } else if (strcmp("deshabilitar", command) == 0){
      // Valido que el nombre de la sala no sea nulo //TODO
      if (message != NULL){
        return(1);
      } else {
        return(-1);
      }
  } else if (strcmp("ver_log", command) == 0){
    return(1);
  } else {
    return(-1);
  }
}

void * receiveMessage(void * socket) {
  int sockfd, ret;
  char buffer[BUF_SIZE]; 
  char command[13];
  char message[100];
  int validate;
  int len;
  len = 0;
  sockfd = (int) socket;
  memset(buffer, 0, BUF_SIZE);  

  for (;;) {
    ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);  
    if (ret < 0) {  
      printf("Error recibiendo data!\n");    
    } else {
 
      // Validar el formato del comando del usuario
      sscanf(buffer,"%s %[^\t\n]",command, message);
      validate = validateMsg(command, message);

      printf("palabra:%d ", validate);


      // Aquí es donde recibo la información del servidor y hago cosas chéveres!!
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
  pthread_t rThread;
  
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

    ret = sendto(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &addr, sizeof(addr));

    if (ret < 0) {  
      printf("Error enviando data!\n\t-%s", buffer);
	}
  }
  
  close(sockfd);
  pthread_exit(NULL);
 
 return 0;    
}  
