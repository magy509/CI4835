#include"stdio.h"
#include"stdlib.h"
#include"sys/types.h"
#include"sys/socket.h"
#include"string.h"
#include"netinet/in.h"
#include"pthread.h"

#define PORT 4444
#define BUF_SIZE 2000
#define CLADDR_LEN 100

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
      printf("client: ");
      fputs(buffer, stdout);
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