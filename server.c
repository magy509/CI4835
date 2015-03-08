#include"stdio.h"
#include"stdlib.h"
#include"sys/types.h"
#include"sys/socket.h"
#include"string.h"
#include"netinet/in.h"
#include"pthread.h"
#include <errno.h>
#include <sysexits.h>
#include <glib.h>

#include "server.h"

#define PORT 4444
#define BUF_SIZE 2000
#define CLADDR_LEN 100

void print_list(gpointer item){
  printf("\nVer Salas");
  printf("%s\n", item);
}

gint imprimirSala(gconstpointer sala_) {
  struct sala * sala = (struct sala *)sala_;

  // Aquí tengo que verificar el tipo de usuario que envía el mensaje.
  // Si es un usuario sin privilegios, solo podrá ver las salas habilitadas
  // Si es un usuario con privilegios, podrá ver todas las salas.

  printf("\nNombre de la sala: %s",sala->nombre);
  if (sala->status == 1){
      printf(" - habilitada");  
  } else {
      printf(" - deshabilitada");
  }
}

gint imprimirUsuario(gconstpointer usuario_) {
  struct usuario * usuario = (struct usuario *)usuario_;

  // Aquí tengo que verificar el tipo de usuario que envía el mensaje.
  // Si es un usuario sin privilegios, solo podrá ver los nombres.
  // Si es un usuario con privilegios, podrá ver toda la información.

  printf("\nNombre del usuario: %s",usuario->nombre);
  printf("\nClave del usuario: %s",usuario->clave);
}

gint buscarUsuarioPorNombre(gconstpointer usuario_, gconstpointer nombre_) {
  struct usuario * usuario = (struct usuario *)usuario_;
  char * nombre = (char *)nombre_;
//  printf("\nNombre del usuario: %s\n",usuario->nombre);
  return(strcmp(usuario->nombre, nombre));
}

gint buscarSalaPorNombre(gconstpointer sala_, gconstpointer nombre_) {
  struct sala * sala = (struct sala *)sala_;
  char * nombre = (char *)nombre_;
  printf("\nNombre de la sala: %s\n",sala->nombre);
  return(strcmp(sala->nombre, nombre));
}

gint buscarEstadoSalaDes(gconstpointer sala_, gconstpointer nombre_) {
  struct sala * sala = (struct sala *)sala_;
  char * nombre = (char *)nombre_;

  if (strcmp(sala->nombre, nombre) == 0) {
      sala->status = 0;
  }
  return(strcmp(sala->nombre, nombre));
}

gint buscarEstadoSala(gconstpointer sala_, gconstpointer nombre_) {
  struct sala * sala = (struct sala *)sala_;
  char * nombre = (char *)nombre_;

  if (strcmp(sala->nombre, nombre) == 0) {
      sala->status = 1;
  }
  return(strcmp(sala->nombre, nombre));
}

/**
 * Elimina un usuario.
 * @param apuntador a un usuario.
 * @param apuntador al nombre del usuario a eliminar.
**/
gint buscarBorrarUsuario(gconstpointer usuario_, gconstpointer nombre_) {
  struct usuario * usuario = (struct usuario *)usuario_;
  char * nombre = (char *)nombre_;
  if (strcmp(usuario->nombre, nombre) == 0) {
      user = g_list_remove(user, usuario);
    return(1);
  }
}

/**
 * Elimina una sala.
 * @param apuntador a un elemento de la sala.
 * @param apuntador al nombre de la sala a eliminar.
**/
gint buscarBorrarSala(gconstpointer sala_, gconstpointer nombre_) {
  struct sala * sala = (struct sala *)sala_;
  char * nombre = (char *)nombre_;
  if (strcmp(sala->nombre, nombre) == 0) {
      room = g_list_remove(room, sala);
    return(1);
  }
}

gint buscarCantidadSala(gconstpointer sala_, gconstpointer nombre_) {
  struct sala * sala = (struct sala *)sala_;
  char * nombre = (char *)nombre_;
  if (strcmp(sala->nombre, nombre) == 0) {
    if (sala->n_usuarios == 0){
      sala->status = 0; //Deshabilito la sala antes de eliminarla
    }
  }
}

void verUsuarios(){
  g_list_foreach (user, (GFunc)imprimirUsuario, NULL);
}

void verSalas(){
  g_list_foreach (room, (GFunc)imprimirSala, NULL);
}

/**
 * Deshabilita una sala del servidor.
 * @param nombre de la sala
 * @return Devuelve -1 si la sala no existe, 0 si hay usuarios conectados a la sala,
 * 1 si fue deshabilitada exitosamente.
**/
int deshabilitarSala(char * s){

  GList * resultado = g_list_find_custom(room, s, buscarSalaPorNombre);

  if (resultado == NULL){
    return(-1); // La sala no existe
  } else {
    resultado = g_list_find_custom(room, s, buscarCantidadSala);
    if (resultado == NULL){
      return(0); // Hay usuarios conectados a la sala
    } else {
      resultado = g_list_find_custom(room, s, buscarEstadoSalaDes);
      if (resultado == NULL){
        return(-1);
      } else {
        return(1);
      }
    }
  }
}

/**
 * Habilita una sala del servidor.
 * @param nombre de la sala
 * @return Devuelve -1 si la sala no existe, 1 si fue habilitada exitosamente.
**/
int habilitarSala(char * s){

  GList * resultado = g_list_find_custom(room, s, buscarEstadoSala);

  if (resultado == NULL){
      return(-1);
  } else {
      return(1);
  }
}

/**
 * Crea una sala en el servidor.
 * @param nombre de la sala.
 * @return Devuelve -1 si la sala existe, 1 si fue creada exitosamente.
**/
int crearSala(char * room_name){

  if (g_list_length (room) == 0){

    struct sala * s = malloc(sizeof(struct sala));
    s->nombre = room_name;
    s->status = 0;
    s->n_usuarios = 0;
    s->usuarios = NULL;
    room = g_list_append(room, s);
    return(1);

  } else {

    GList * resultado = g_list_find_custom(room, room_name, buscarSalaPorNombre);

    if (resultado == NULL){
      struct sala * s = malloc(sizeof(struct sala));
      s->nombre = room_name;
      s->status = 0;
      s->n_usuarios = 0;
      s->usuarios = NULL;
      room = g_list_append(room, s);
      return(1);
    } else {
      return(-1);
    }
  }
}

/**
 * Crea un usuario en el servidor.
 * @param nombre del usuario.
 * @param clave del usuario.
 * @return Devuelve -1 si el usuario existe, 1 si fue creado exitosamente.
**/
int crearUsuario(char * usuario, char * clave){
  if (g_list_length (user) == 0){

      struct usuario * u = malloc(sizeof(struct usuario));
      u->nombre = usuario;
      u->clave = clave;
      u->status = 0;
      user = g_list_append(user, u);
      return(1);

  } else {

      GList * resultado = g_list_find_custom(user, usuario, buscarUsuarioPorNombre);

      if (resultado == NULL){
        struct usuario * u = malloc(sizeof(struct usuario));
        u->nombre = usuario;
        u->clave = clave;
        u->status = 0;
        user = g_list_append(user, u);
        return(1);

      } else {

          return(-1);
      }
  }
}

/**
 * Elimina un usuario del servidor. Valida que el usuario exista y que no esté conectado a ninguna sala.
 * @param nombre del usuario.
 * @param clave del usuario.
 * @return Devuelve -1 si el usuario no existe, -2 si la contraseña ingresada es incorrecta,
 * 0 si está conectado, 1 si el usuario es eliminado exitosamente.
**/
int eliminarUsuario(char * usuario, char * clave) {
  GList * resultado = g_list_find_custom(user, usuario, buscarUsuarioPorNombre);
  if (resultado == NULL) {
    
    return(-1);
  } else if (((struct usuario *)resultado->data)->status > 0) {
    return(0);
  } else {
    if (strcmp(((struct usuario *)resultado->data)->clave, clave) == 0) {
      resultado = g_list_find_custom(user, usuario, buscarBorrarUsuario);
      return(1);
    } else {
      return(-2);
    }
  }
}

/**
 * Elimina una sala del servidor. Valida que la sala exista y que no tenga usuarios conectados.
 * @param nombre de la sala.
 * @return Devuelve -1 si la sala no existe, 0 si la sala tiene usuarios conectados,
 * y 1 si la sala es eliminada exitosamente.
**/
int eliminarSala(char * room_name) {
  GList * resultado = g_list_find_custom(room, room_name, buscarSalaPorNombre);
  if (resultado == NULL) {
    return -1;
  } else if (((struct sala *)resultado->data)->n_usuarios > 0) {
    return 0;
  } else {
    resultado = g_list_find_custom(room, room_name, buscarBorrarSala);
    return 1;
  }
}

int conectar(char * nombre, char * clave){
  GList * resultado = g_list_find_custom(user, nombre, buscarUsuarioPorNombre);
  if (resultado != NULL){
    if (strcmp(((struct usuario *)resultado->data)->nombre, nombre) == 0) {
      if (strcmp(((struct usuario *)resultado->data)->clave, clave) == 0) {
        return(1);
      } else {
        return(-2);
      }
    return 1;
    }
  } else {
    return(-1);
  }
}

/**
 * Valida que lo enviado por el cliente, tenga un formato válido.
 * Esto abarca comandos y cantidad de argumentos.
 * @param comando recibido por cónsola.
 * @param string que sigue al comando.
 * @param argumento auxiliar para separar el string message.
 * @param argumento auxiliar para separar el string message.
 * @param cantidad de elementos que fueron convertidos exitosamente en el sscanf.
 * @return Devuelve 0 si el comando no utiliza argumentos. Devuelve 1 si el comando utiliza 1 argumento.
 * Devuelve 2 si el comando utiliza dos argumentos. Devuelve -1 si es un comando inválido.
**/
char * validateMsg(char * command, char * message, char * arg1, char * arg2, int conv){
  int salida;
  char * result;

  if (strcmp("conectar", command) == 0 && conv == 2){
    if (sscanf(message,"%ms %ms",&arg1, &arg2) == 2) {
      salida = conectar(arg1, arg2);

      printf("\nSalida %d", salida);
      if (salida == 1) {
        result = "Conexion exitosa.";
      } else if (salida == -1) {
        result = "Nombre de usuario no existe.";      
      } else if (salida == -2) {
        result = "Contraseña Invalida.";
      }
    } else {
      result = "No se pudo ejecutar el comando.";
    }
      return(result);

  } else if (strcmp("salir", command) == 0){
      return(result);

  } else if (strcmp("entrar", command) == 0 && conv == 2){
      return("Holis");

  } else if (strcmp("dejar", command) == 0 && conv == 2){
      return("Holis");

  } else if (strcmp("ver_salas", command) == 0){
    verSalas();
	return("Holis!");

  } else if (strcmp("ver_usuarios", command) == 0){
     verUsuarios();
      return("Holis!");

  } else if (strcmp("ver_usu_salas", command) == 0 && conv == 2){
      return("Holis!");

  } else if (strcmp("env_mensaje", command) == 0  && conv == 2){
	  if(strlen(message) <= 70){
        return("Holis");
  	  } else {
      result = "No se pudo ejecutar el comando.";
        return(result);
      }

  } else if (strcmp("crear_usu", command) == 0 && conv == 2){
    if (sscanf(message,"%ms %ms",&arg1, &arg2) == 2) {
        salida = crearUsuario(arg1, arg2);
      if (salida == 1){
        result = "Usuario creado exitosamente.";
      } else {
        result = "Nombre de usuario ya existe.";
      }

      return(result);

    } else {
      result = "No se pudo ejecutar el comando.";
      return(result);
    }

  } else if (strcmp("elim_usu", command) == 0 && conv == 2){
    if (sscanf(message,"%ms %ms",&arg1, &arg2) == 2) {
      printf("Validando");
      salida = eliminarUsuario(arg1, arg2);
      printf("Validando con salida = %d", salida);
      if (salida == -2) {
        result = "Contraseña incorrecta.";
      } else if (salida == 0) {
        result = "El usuario está conectado.";
      } else if (salida == 1){
        result = "El usuario fue eliminado exitosamente.";
      } else {
        result = "El usuario no existe.";
      }
    } else {
      result = "No se pudo ejecutar el comando.";
    }
      return(result);

  } else if (strcmp("crear_sala", command) == 0 && conv == 2){
      salida = crearSala(message);
      if (salida == 1){
        result = "Sala creada exitosamente.";

      } else {
        result = "La sala ya existe.";
      }
      return(result);

  } else if (strcmp("elim_sala", command) == 0 && conv == 2){
      salida = eliminarSala(message);
      if(salida == 0) {
        result = "Hay usuarios conectados en la sala. No puede ser eliminada.";
      } else if (salida == 1) {
        result = "Sala eliminada exitosamente.";
      } else {
        result = "La sala no existe.";
      }
      return(result);

  } else if (strcmp("habilitar", command) == 0 && conv == 2){

      if (habilitarSala(message) == -1){
          result = "Sala no existe.";
      } else {
          result = "Sala habilitada.";
      }
      return(result);

  } else if (strcmp("deshabilitar", command) == 0 && conv == 2){
      if (deshabilitarSala(message) == 0) {
        result = "Hay usuarios conectados en la sala. No puede ser deshabilitada.";
      } else if (deshabilitarSala(message) == 1){
        result = "Sala deshabilitada exitosamente.";
      } else {
        result = "La sala no existe.";
      }

      return(result);

  } else if (strcmp("ver_log", command) == 0){
      return("Holis");

  } else {
      result = "No se pudo ejecutar el comando.";
      return(result);
  }
}

void * receiveMessage(void * socket) {
  int sockfd, ret;
  char buffer[BUF_SIZE];
  char * command;
  char * message;
  char * arg1;
  char * arg2;
//  char * buf;
  char * valid;
  int conv;
  sockfd = (int) socket;
  memset(buffer, 0, BUF_SIZE);  
  for (;;) {
    ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL); 
    if (ret < 0) {  
      printf("Error recibiendo data!\n");    
    } else {

    // Validar el formato del comando del cliente
    conv = sscanf(buffer,"%ms %m[^\t\n]",&command, &message);
//    printf("Conv %d!\n", conv);    
    if (conv != -1) {

      if (conv > 1) {
          sscanf(message,"%ms %ms",&arg1, &arg2);
      }

      valid = validateMsg(command, message, arg1, arg2, conv);
    if (valid != NULL) {
        //printf("Resultado %s: ", valid);
    }
    } else {
        conv = 0;
         printf("\nError de datos???\n");
    }

      if (valid != NULL){
        printf("\nResultado: %s\n", valid);
        // Aqui, conseguir la manera de enviar lo que está en la variable valid al cliente.
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
