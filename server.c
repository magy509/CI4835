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

/**
 * Declaración de variables globales
**/
GList * user = NULL;
GList * room = NULL;

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
  printf("\nUsuarios de la sala: %d.", sala->n_usuarios);
}

gint imprimirUsuario(gconstpointer usuario_) {
  struct usuario * usuario = (struct usuario *)usuario_;

  // Aquí tengo que verificar el tipo de usuario que envía el mensaje.
  // Si es un usuario sin privilegios, solo podrá ver los nombres.
  // Si es un usuario con privilegios, podrá ver toda la información.

  printf("\nNombre del usuario: %s",usuario->nombre);
  printf("\nClave del usuario: %s",usuario->clave);
  printf("\nEstado del usuario: %d",usuario->status);
}

gint imprimirUsuariosSala(gconstpointer usuario_) {
//  char * usuario = (char * usuario)usuario_;

  printf("\nNombre del usuario");
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

int verUsuarios(){
  if (g_list_length(user) == 0) {
    return(0);
  } else {
    g_list_foreach (user, (GFunc)imprimirUsuario, NULL);
    return(1);
  }
}

int verSalas(){
  if (g_list_length (room) == 0){
    return(0);
  } else {
    g_list_foreach (room, (GFunc)imprimirSala, NULL);
    return(1);
  }
}

void imprimirTexto(gpointer texto_, gpointer formato_) {
 char * texto = (char *)texto_;
 char * formato = (char *)formato_;
 printf(formato, texto);
}

/**
 * Listar los usuarios de una sala
 * @param nombre de la sala
 * @return Devuelve -1 si la sala no existe, -2 si la sala está inactiva,
 * 0 si no hay usuarios conectados a la sala,
 * 1 si se muestra el listado exitosamente.
**/
int verUsuSalas(char * s){
  
  GList * resultado = g_list_find_custom(room, s, buscarSalaPorNombre);
  if (resultado == NULL) {
    printf("\nEntró a no existe con %s", s);
    return(-1); //La sala no existe.
  } else {
    printf("\nEntró a existe con %s", s);
    if (((struct sala *)resultado->data)->status == 0) {
    printf("\nSala inactiva");
      return(-2); //La sala está inactiva
    } else if (((struct sala *)resultado->data)->n_usuarios == 0) {
    printf("\nSala vacía");
      return(0); // No hay usuarios conectados
    } else {
    printf("\nSala con usuarios");
    int cant = g_list_length(((struct sala *)resultado->data)->usuarios);
    char * nombre = g_list_first(((struct sala *)resultado->data)->usuarios)->data;
    printf("\nCantidad de usuarios en la lista %d", cant);
    printf("\nPrimer usuario en la lista %s", nombre);
    
    g_list_foreach(((struct sala *)resultado->data)->usuarios, imprimirTexto, "\nNombre del usuario: %s\n");
      return(1);
    }
  }
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
    if (((struct sala *)resultado->data)->n_usuarios > 1){
      return(0); // Hay usuarios conectados a la sala
    } else {
      ((struct sala *)resultado->data)->status = 0;
      return(1);
    }
  }
}

/**
 * Habilita una sala del servidor.
 * @param nombre de la sala
 * @return Devuelve -1 si la sala no existe, 1 si fue habilitada exitosamente.
**/
int habilitarSala(char * s){

  GList * resultado = g_list_find_custom(room, s, buscarSalaPorNombre);

  if (resultado == NULL){
    return(-1);
  } else {
    ((struct sala *)resultado->data)->status = 1;
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

  GList * resultado = g_list_find_custom(user, usuario, buscarUsuarioPorNombre);

  if (resultado == NULL){
    struct usuario * u = malloc(sizeof(struct usuario));
    u->nombre = usuario;
    u->clave = clave;
    u->status = 0;
    u->is_admin = 0;
    user = g_list_append(user, u);
    return(1);
  } else {
    return(-1);
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

/**
 * Desconecta un usuario del servidor
 * @param nombre del usuario.
 * @return Devuelve 0 si el usuario no está conectado
 * y 1 si el usuario se desconecta exitosamente.
**/
int salir(char * nombre){

  GList * resultado = g_list_find_custom(user, nombre, buscarUsuarioPorNombre);

  if (resultado == NULL){
    return(-1); //El usuario no existe
  } else {
    if (((struct usuario *)resultado->data)->status == 0) {
      return(0); //El usuario no está conectado.
    } else if (((struct usuario *)resultado->data)->status == 2) {
      return(2); //El usuario está conectado a una sala. Aquí tengo que poder desconectar al usuario de la sala y luego del sistema
    } else {
      ((struct usuario *)resultado->data)->status = 0;
      return(1);
    }
  }
}

/**
 * Conecta un usuario al servidor
 * @param nombre del usuario.
 * @param clave del usuario.
 * @return Devuelve -1 si el nombre de usuario no existe, -2 si la contraseña es incorrecta,
 * 0 si el usuario ya está conectado y 1 si el usuario se conecta exitosamente.
**/
int conectar(char * nombre, char * clave){
  GList * resultado = g_list_find_custom(user, nombre, buscarUsuarioPorNombre);
  if (resultado == NULL){
    return(-1); //Usuario no existe.
  } else {
    if (strcmp(((struct usuario *)resultado->data)->clave, clave) == 0) {
      if (((struct usuario *)resultado->data)->status > 0){
        return(0); //Usuario ya está conectado.
      } else {
      ((struct usuario *)resultado->data)->status = 1;
        return(1); // Conexión exitosa.
      } 
    } else {
      return(-2); // Clave inválida.
    }
  }
}

/**
 * Conecta un usuario a una sala
 * @param nombre de la sala.
 * @param nombre del usuario.
 * @return Devuelve -3 si el usuario no existe, -2 si el usuario ya está conectado a una sala,
 * -4 si el usuario no está conectado en el SCS, -1 si la clave no existe, 0 si la sala está deshabilitada
 * y 1 si el usuario se conecta exitosamente.
**/
int entrarSala(char * room_name, char * nombre){

  GList * resultUsuario = g_list_find_custom(user, nombre, buscarUsuarioPorNombre);

  if (resultUsuario == NULL){
    return(-3);  //El usuario no existe
  } else {

    if (((struct usuario *)resultUsuario->data)->status == 2) {
      return(-2); // El usuario ya está conectado a una sala.
    } else if (((struct usuario *)resultUsuario->data)->status == 0){
      return(-4); // El usuario no está conectado en el SCS.
    } else {
      
      GList * resultSala = g_list_find_custom(room, room_name, buscarSalaPorNombre);

      if (resultSala == NULL){

        return(-1);  // La sala no existe
    
      } else {

        if (((struct sala *)resultSala->data)->status == 0) {
        return(0); // La sala está inactiva

        } else {

          ((struct sala *)resultSala->data)->usuarios = g_list_append(((struct sala *)resultSala->data)->usuarios, nombre);
          ((struct usuario *)resultUsuario->data)->status = 2;
          ((struct sala *)resultSala->data)->n_usuarios +=1;
          return(1); //Conexión exitosa
        }
      }
    }
  }
}

/**
 * Saca un usuario a una sala
 * @param nombre de la sala.
 * @param nombre del usuario.
 * @return Devuelve 0 si el usuaio no está conectado a la sala, -1 si la sala no existe
 * y 1 si el usuario sale exitosamente.
**/
int dejarSala(char * room_name, char * nombre){

  GList * resultUsuario = g_list_find_custom(user, nombre, buscarUsuarioPorNombre);

  if (resultUsuario == NULL){
    return(0);  //El usuario no existe
  } else {

    if (((struct usuario *)resultUsuario->data)->status == 1) {
      return(0); // El usuario no está conectado a una sala.
    } else if (((struct usuario *)resultUsuario->data)->status == 0){
      return(0); // El usuario no está conectado a la sala.
    } else {
      
      GList * resultSala = g_list_find_custom(room, room_name, buscarSalaPorNombre);

      if (resultSala == NULL){

        return(-1);  // La sala no existe
    
      } else {

        if (((struct sala *)resultSala->data)->status == 0) {
          return(0); // El usuario no está conectado a la sala.
        } else  if (((struct sala *)resultSala->data)->n_usuarios == 0){
          return(0);
        } else {
          ((struct sala *)resultSala->data)->usuarios = g_list_remove(((struct sala *)resultSala->data)->usuarios, nombre);
          ((struct usuario *)resultUsuario->data)->status = 1;
          ((struct sala *)resultSala->data)->n_usuarios -=1;
          return(1);
        }
      }
    }
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

  if (strcmp("conectarse", command) == 0 && conv == 2){
    if (sscanf(message,"%ms %ms",&arg1, &arg2) == 2) {

      salida = conectar(arg1, arg2);
      if (salida == 1) {
        result = "\nConexion exitosa.";
      } else if (salida == 0) {
        result = "\nEl usuario ya está conectado.";      
      } else if (salida == -2) {
        result = "\nContraseña Invalida.";
      } else {
        result = "\nNombre de usuario no existe.";      
      }
    } else {
      result = "\nNo se pudo ejecutar el comando.";
    }
    return(result);

  } else if (strcmp("salir", command) == 0){
      salida = salir("magy"); // Tengo que pasar el nombre del usuario asociado al cliente.
      if (salida == 1) {
        result = "\nEl usuario ha salido del char exitosamente.";
      } else if (salida == 0) {
        result = "\nEl usuario no está conectado.";
      } else if (salida == 2) {
        result = "\nEl usuario está conectado a una sala.";
      } else {
        result = "\nEl usuario no existe.";
      }
      return(result);

  } else if (strcmp("entrar", command) == 0 && conv == 2){

    salida = entrarSala(message, "magy");
    
    if (salida == -4) {
      result = "\nEl usuario no está conectado al chat.";
    } else if (salida == -3) {
      result = "\nEl usuario no existe.";
    } else if (salida == -2) {
      result = "\nEl usuario ya está conectado a una sala.";
    } else if (salida == 0) {
      result = "\nLa sala está inactiva.";
    } else if (salida == 1) {
      result = "\nConexión exitosa a la sala.";
    } else {
      result = "\nLa sala no existe.";
    }
    return(result);

  } else if (strcmp("dejar", command) == 0 && conv == 2){
    salida = dejarSala(message, "magy");
    if (salida == 0) {
      result = "\nEl usuario no está conectado a la sala.";
    } else if (salida == 1){
      result = "\nEl usuario dejó la sala exitosamente.";
    } else {
      result = "\nLa sala no existe.";
    }
      return(result);

  } else if (strcmp("ver_salas", command) == 0){
    salida = verSalas();
    if (salida == 1) {
      result = "\nLista de salas"; //Aquí, result debería ser el string que se imprime en la otra cosa.
    } else {
      result = "\nNo hay salas en el servidor.";
    }
	return(result);

  } else if (strcmp("ver_usuarios", command) == 0){
     salida = verUsuarios();
     if (salida == 1) {
       result = "\nLista Usuarios";
     } else {
       result = "\nNo hay usuarios en el servidor.";
     }
     return(result);

  } else if (strcmp("ver_usu_salas", command) == 0 && conv == 2){

    salida = verUsuSalas(message);

    if (salida == -2) {
      result = "\nLa sala está inactiva";
    } else if (salida == 0) {
      result = "\nLa sala no tiene usuarios conectados";
    } else if (salida == 1) {
      result = "\nUsuarios Conectados:"; //Aquí tengo que pasar un string con los usuarios conectados TODO
    } else {
      result = "\nLa sala no existe.";
    }
    return(result);

  } else if (strcmp("env_mensaje", command) == 0  && conv == 2){
	  if(strlen(message) <= 70){
        return("Holis");
  	  } else {
      result = "\nNo se pudo ejecutar el comando.";
      }
      return(result);

  } else if (strcmp("crear_usu", command) == 0 && conv == 2){
    if (sscanf(message,"%ms %ms",&arg1, &arg2) == 2) {

      salida = crearUsuario(arg1, arg2);
      if (salida == 1){
        result = "\nUsuario creado exitosamente.";
      } else {
        result = "\nNombre de usuario ya existe.";
      }
    } else {
      result = "\nNo se pudo ejecutar el comando.";
    }
    return(result);

  } else if (strcmp("elim_usu", command) == 0 && conv == 2){
    if (sscanf(message,"%ms %ms",&arg1, &arg2) == 2) {

      salida = eliminarUsuario(arg1, arg2);
      if (salida == -2) {
        result = "\nContraseña incorrecta.";
      } else if (salida == 0) {
        result = "\nEl usuario está conectado a una sala. No puede ser eliminado.";
      } else if (salida == 1){
        result = "\nEl usuario fue eliminado exitosamente.";
      } else {
        result = "\nEl usuario no existe.";
      }
    } else {
      result = "\nNo se pudo ejecutar el comando.";
    }
    return(result);

  } else if (strcmp("crear_sala", command) == 0 && conv == 2){

      salida = crearSala(message);
      if (salida == 1){
        result = "\nSala creada exitosamente.";
      } else {
        result = "\nLa sala ya existe.";
      }
      return(result);

  } else if (strcmp("elim_sala", command) == 0 && conv == 2){

      salida = eliminarSala(message);
      if(salida == 0) {
        result = "\nHay usuarios conectados en la sala. No puede ser eliminada.";
      } else if (salida == 1) {
        result = "\nSala eliminada exitosamente.";
      } else {
        result = "\nLa sala no existe.";
      }
      return(result);

  } else if (strcmp("hab_sala", command) == 0 && conv == 2){

      salida = habilitarSala(message);
      if (salida == -1){
          result = "\nLa sala no existe.";
      } else {
          result = "\nSala habilitada.";
      }
      return(result);

  } else if (strcmp("deshab_sala", command) == 0 && conv == 2){

      salida = deshabilitarSala(message);
      if (salida == 0) {
        result = "\nHay usuarios conectados en la sala. No puede ser deshabilitada.";
      } else if (salida == 1){
        result = "\nSala deshabilitada exitosamente.";
      } else {
        result = "\nLa sala no existe.";
      }
      return(result);

  } else if (strcmp("ver_log", command) == 0){
      return("Holis");

  } else {
      result = "\nNo se pudo ejecutar el comando.";
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

  struct usuario * u = malloc(sizeof(struct usuario));
  u->nombre = "admin";
  u->clave = "claveadmin";
  u->status = 0;
  u->is_admin = 1;
  user = g_list_append(user, u);

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
