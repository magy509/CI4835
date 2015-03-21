#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <sysexits.h>
#include <glib.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include "server.h"
#include "comando.h"

#define PORT "4444"
#define BUF_SIZE 2000
#define CLADDR_LEN 100

/**
 * Declaración de variables globales
**/
GList * user = NULL;
GList * room = NULL;

char * lista;

void imprimirSala(gconstpointer sala_, gconstpointer is_admin_) {
  struct sala * sala = (struct sala *)sala_;
  int is_admin = *((int *)is_admin_);
  char * str_arg1;
  char * str_arg2;

  str_arg1 = malloc(strlen(sala->nombre)+1);
  str_arg1 = sala->nombre; 
  if (is_admin == 1){
    if (0 == strlen(lista)){
      if (sala->status != 0){
        asprintf(&lista, "%s - Habilitada\n", str_arg1);
      } else {
        asprintf(&lista, "%s - Deshabilitada\n", str_arg1);
      } 
    } else {
      if (sala->status != 0){
        asprintf(&lista, "%s%s - Habilitada\n", lista, str_arg1);
      } else {
        asprintf(&lista, "%s%s - Deshabilitada\n", lista, str_arg1);
      } 
    }
  } else {
      if (sala->status == 1) {
        if (0 == strlen(lista)){
          asprintf(&lista, "%s\n", str_arg1);
      } else {
        asprintf(&lista, "%s%s\n", lista, str_arg1);
      }
    }
  }
}

void imprimirUsuario(gconstpointer usuario_, gconstpointer is_admin_) {
  struct usuario * usuario = (struct usuario *)usuario_;
  int is_admin = *((int *)is_admin_);
  char * str_arg1;
  char * str_arg2;

  str_arg1 = malloc(strlen(usuario->nombre)+1);
  str_arg1 = usuario->nombre; 
  str_arg2 = malloc(strlen(usuario->clave)+1);
  str_arg2 = usuario->clave; 


  if (is_admin == 1) {
      if (usuario->status == 0){
        if(strlen(lista) == 0){
          asprintf(&lista, "Nombre: %s Clave: %s - Desconectado\n", str_arg1, str_arg2);
        } else {
          asprintf(&lista, "%sNombre: %s Clave: %s - Desconectado\n", lista, str_arg1, str_arg2);
        }
      } else {
        if(strlen(lista) == 0){
          asprintf(&lista, "Nombre: %s Clave: %s - Conectado\n", str_arg1, str_arg2);
        } else {
          asprintf(&lista, "%sNombre: %s Clave: %s - Conectado\n", lista, str_arg1, str_arg2);
        }
      }
  } else {
    if (usuario -> status != 0){
      if(strlen(lista) == 0){
        asprintf(&lista, "Nombre: %s\n", str_arg1);
      } else {
        asprintf(&lista, "%sNombre: %s\n", lista, str_arg1);
      }
    }
  }
}

gint buscarUsuarioPorNombre(gconstpointer usuario_, gconstpointer nombre_) {
  struct usuario * usuario = (struct usuario *)usuario_;
  char * nombre = (char *)nombre_;
  return(strcmp(usuario->nombre, nombre));
}

gint buscarSalaPorNombre(gconstpointer sala_, gconstpointer nombre_) {
  struct sala * sala = (struct sala *)sala_;
  char * nombre = (char *)nombre_;
  return(strcmp(sala->nombre, nombre));
}

gint buscarUsuarioPorSocket(gconstpointer usuario_, gconstpointer socket_) {
  struct usuario * usuario = (struct usuario *)usuario_;
  int socket = *((int *)socket_);
  if (usuario -> socket == socket){
    return 0;
  }
  return -1;
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
    return 1;
  }
}

int verUsuarios(int is_admin){
  if (g_list_length(user) == 0) {
    return 0;
  } else {
    g_list_foreach (user, (GFunc)imprimirUsuario, &is_admin);
    return 1;
  }
}

int verSalas(int is_admin){
  if (g_list_length (room) == 0){
    return 0;
  } else {
    g_list_foreach (room, (GFunc)imprimirSala, &is_admin);
    return 1;
  }
}

void mandar_todos_los_usuarios(gpointer usuario_, gpointer conn_) {
  char * usuario = (char *)usuario_;
  int conn = *((int *)conn_);

  char * texto = NULL;
  if (-1 == asprintf(&texto, "Nombre de usuario conectado: %s\n", usuario)) {
    perror("asprintf");
    exit(EX_IOERR);
  }
  escribir(conn, texto, strlen(texto));
  free(texto);
}


/**
 * Listar los usuarios de una sala
 * @param nombre de la sala
 * @return Devuelve -1 si la sala no existe, -2 si la sala está inactiva,
 * 0 si no hay usuarios conectados a la sala,
 * 1 si se muestra el listado exitosamente.
**/
int verUsuSalas(char * s, int conn){
  
  GList * resultado = g_list_find_custom(room, s, buscarSalaPorNombre);
  if (resultado == NULL) {
    return(-1); //La sala no existe.
  } else {
    if (((struct sala *)resultado->data)->status == 0) {
      return(-2); //La sala está inactiva
    } else if (((struct sala *)resultado->data)->n_usuarios == 0) {
      return(0); // No hay usuarios conectados
    } else {
      printf("Antes de la función mandar\n");
      g_list_foreach(((struct sala *)resultado->data)->usuarios, mandar_todos_los_usuarios, &conn);
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
    u->socket = -1;
    u->sala= NULL;
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
    printf("Sala\n",((struct usuario *)resultado->data)->sala);
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
      printf("El usuario está en una sala\n");
      dejarSala(((struct usuario *)resultado->data)->sala, ((struct usuario *)resultado->data)->nombre);
      ((struct usuario *)resultado->data)->status = 0;
      ((struct usuario *)resultado->data)->socket = -1;
      return(1); //El usuario está conectado a una sala. Aquí tengo que poder desconectar al usuario de la sala y luego del sistema
    } else {
      ((struct usuario *)resultado->data)->status = 0;
      ((struct usuario *)resultado->data)->socket = -1;
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
int conectar(char * nombre, char * clave, int conn){
  GList * resultado = g_list_find_custom(user, nombre, buscarUsuarioPorNombre);
  if (resultado == NULL){
    return(-1); //Usuario no existe.
  } else {
    if (strcmp(((struct usuario *)resultado->data)->clave, clave) == 0) {
      if (((struct usuario *)resultado->data)->status > 0){
        return(0); //Usuario ya está conectado.
      } else {
        ((struct usuario *)resultado->data)->status = 1;
        ((struct usuario *)resultado->data)->socket = conn;
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
          ((struct sala *)resultSala->data)->n_usuarios +=1;
          ((struct usuario *)resultUsuario->data)->status = 2;
          ((struct usuario *)resultUsuario->data)->sala = room_name;
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

void enviarTodos(gpointer usuario_, gpointer mensaje_) {
  char * usuario = (char *)usuario_;
  char * mensaje = (char *)mensaje_;

  GList * resultado = g_list_find_custom(user, usuario, buscarUsuarioPorNombre);

  int res = asprintf(&lista, "%s: %s\n", usuario, mensaje);
//  printf("Resultado del asprintf %d\n", res);
//  char * texto = NULL;
//  if (-1 == asprintf(&texto,"%s: %s\n", usuario, mensaje)) {
//    perror("asprintf");
//    exit(EX_IOERR);
//  }


  int conn = ((struct usuario *)resultado->data)->socket;

  escribir(conn, lista, strlen(lista));
  free(lista);
}


/**
 * Listar los usuarios de una sala
 * @param nombre de la sala
 * @return Devuelve -1 si la sala no existe, -2 si la sala está inactiva,
 * 0 si no hay usuarios conectados a la sala,
 * 1 si se muestra el listado exitosamente.
**/
int enviarMensaje(char * sala, char * mensaje){
  
  GList * resultado = g_list_find_custom(room, sala, buscarSalaPorNombre);
  g_list_foreach(((struct sala *)resultado->data)->usuarios, enviarTodos, mensaje);
  return(1);
}

/**
 * Se encarga de escribir en el file descriptor del socket el contenido
 * de buf.
 * @param fd file descriptor del socket donde se va a escribir.
 * @param buf contenido que se va a escribir en fd.
 * @param count tamaño del contenido que se va a escribir.
 */
escribir(int fd, void * buf, size_t count) {
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
      if (EINTR == errno || EAGAIN == errno) continue;
      perror("read");
      exit(EX_IOERR);
    }

    count -= leido;
    buf += leido;
  }
}

int main() {
        int i;
        int j;
        int verbose = 1;
        GList * open_conn = NULL;
        int ipv = IPV_UNSET; // Opción para protocolo de red (versión de IP)
        int backlog = DEFAULT_BACKLOG; // Opción para máximo número de conexiones en espera

        struct usuario * u = malloc(sizeof(struct usuario));
        u->nombre = "admin";
        u->clave = "claveadmin";
        u->status = 0;
        u->is_admin = 1;
        u->socket = -1;
        user = g_list_append(user, u);

        struct addrinfo hints; // Opciones para getaddrinfo()
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_flags = AI_PASSIVE;
        hints.ai_socktype = SOCK_STREAM;

        struct protoent * proto = getprotobyname("TCP"); // Protocolo de transporte
        hints.ai_protocol = proto->p_proto;

        struct addrinfo *results; // Retorno de getaddrinfo()
        i = getaddrinfo(NULL, PORT, &hints, &results);
        if (0 != i) {
          fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(i));
          exit(EX_OSERR);
        }

        int addrc; // Número de sockets retornado por getaddrinfo()
        struct addrinfo *rp; // Iterador para retorno de getaddrinfo()
        for (addrc = 0, rp = results; NULL != rp; rp = rp->ai_next, ++addrc);
        if (0 == addrc) {
          fprintf(stderr, "No se encontró ninguna manera de crear el servicio.\n");
          exit(EX_UNAVAILABLE);
        }

        int * sockfds = (int *)calloc(addrc, sizeof(int)); // Arreglo de file descriptors para atender clientes
        if (NULL == sockfds) {
          perror("calloc");
          exit(EX_OSERR);
        }

        int socks = 0;
        for (i = 0, rp = results; NULL != rp; ++i, rp = rp->ai_next) {
          sockfds[i] = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
          if (-1 == sockfds[i]) {
            if (verbose) perror("socket");
            sockfds[i] = -1;
            continue;
          }

          if (IPV_UNSET == ipv && PF_INET6 == rp->ai_family) {
#if defined(IPV6_V6ONLY)
            j = 1;
            if (-1 == setsockopt(sockfds[i], IPPROTO_IPV6, IPV6_V6ONLY, &j, sizeof(j))) {
              if (verbose) perror("setsockopt");
              close(sockfds[i]);
              sockfds[i] = -1;
              continue;
            }
#else
            fprintf(stderr, "Imposible usar la opción IPV6_V6ONLY para sockets IPv6; no se utilizará el socket.\n");
            close(sockfds[i]);
            sockfds[i] = -1;
            continue;
#endif
          }

          j = 1;
          if (-1 == setsockopt(sockfds[i], SOL_SOCKET, SO_REUSEADDR, &j, sizeof(j))) {
            if (verbose) perror("setsockopt");
            close(sockfds[i]);
            sockfds[i] = -1;
            continue;
          }

          if (-1 == bind(sockfds[i], rp->ai_addr, rp->ai_addrlen)) {
            if (verbose) perror("bind");
            close(sockfds[i]);
            sockfds[i] = -1;
            continue;
          }

          if (-1 == listen(sockfds[i], backlog)) {
            if (verbose) perror("listen");
            close(sockfds[i]);
            sockfds[i] = -1;
            continue;
          }

          ++socks;

          if (verbose) {
            fprintf(
              stderr,
              "Recibiendo conexiones en:\n"
              "ai_family = %d\n"
              "ai_socktype = %d\n"
              "ai_protocol = %d\n"
              "\n",
              rp->ai_family,
              rp->ai_socktype,
              rp->ai_protocol
            );
            proto = getprotobynumber(rp->ai_protocol);
            if (NULL == proto) {
              fprintf(stderr, "protocolo desconocido!\n");
            } else {
              fprintf(stderr, "protocolo: %s\n", proto->p_name);
            }
          }
        }

        freeaddrinfo(results);

        if (socks <= 0) {
          fprintf(stderr, "No se encontró ninguna manera de crear el servicio.\n");
          exit(EX_UNAVAILABLE);
        }

        for (i = 0, j = 0; i < socks; ++i) {
          if (sockfds[i] == -1) {
            if (j == 0) j = i+1;
            for (; j < addrc; ++j) if (sockfds[j] != -1) break;
            sockfds[i] = sockfds[j];
            ++j;
          }
        }
        sockfds = (int *)(realloc(sockfds, socks*sizeof(int)));
        if (NULL == sockfds) {
          perror("realloc");
          exit(EX_OSERR);
        }

        for (i = 0; i < socks; ++i) {
          if (-1 == fcntl(sockfds[i], F_SETFL, O_NONBLOCK)) {
            perror("fcntl");
            exit(EX_OSERR);
          }
        }

        for (;;) { // main loop
          int nfds = -1; // Número de sockets con actividad para select()
          fd_set fds; // Conjunto de file descriptors para select()
          fd_set listen_fds; // Conjunto de file descriptors que esperan por conexiones
          FD_ZERO(&fds);

          for (i = 0; i < socks; ++i) {
            nfds = MAX(nfds, sockfds[i]);
            FD_SET(sockfds[i], &fds);
          }
          for (i = 0; i < g_list_length(open_conn); i++){
            
            nfds = MAX(nfds, *((int *)g_list_nth(open_conn, i)->data));
            FD_SET(*((int *)g_list_nth(open_conn, i)->data), &fds);
          }
          if (nfds < 0) {
            fprintf(stderr, "Error calculando valor máximo de sockets: se obtuvo %d.\n", nfds);
            exit(EX_SOFTWARE);
          }

          if ((nfds = select(nfds + 1, &fds, NULL, NULL, NULL)) == -1) {
            if (errno == EINTR) {
              if (verbose) perror("select");
              continue;
            } else {
              perror("select");
              exit(EX_OSERR);
            }
          }

          for (i = 0; i < socks; ++i) {
            if (FD_ISSET(sockfds[i], &fds)) {
              int conn = accept(sockfds[i], NULL, NULL); 
              if (-1 == conn) {
                if (verbose) {
                  perror("Conexión perdida; error haciendo accept");
                }
                continue;
              }
              if (-1 == fcntl(conn, F_SETFL, O_NONBLOCK)) {
                if (verbose) {
                  perror("fcntl");
                }
                if (shutdown(conn, SHUT_RDWR) == -1) {
                  if (verbose) perror("shutdown");
                }
                if (close(conn) == -1) {
                  if (verbose) perror("close");
                }
                continue;
              }
              int * conn_p = malloc(sizeof(int));
              *conn_p = conn;
              open_conn = g_list_append(open_conn, conn_p);
              // TODO: agregar conn a lista de conexiones abiertas
            } /* if (FD_ISSET(fds)) */
          } /* for (0 <= i < socks) */

          for (i = 0; i < g_list_length(open_conn); i++) {
            int conn = *((int *)g_list_nth(open_conn, i)->data);
            if (FD_ISSET(conn, &fds)) {
              int c;
              switch (read(conn, &c, sizeof(int))) {
                case -1:
                  if (EINTR == errno || EAGAIN == errno) continue;
                  if (verbose) perror("Error leyendo comando de cliente: recv");
                  exit(EX_IOERR);
                break;
	        case 0:
                  if (verbose) fprintf(stderr, "Conexión terminada por el cliente.\n");
                break;
                default:
                  puts("miaaaaau"); ((void (*)())0)();
	        case 4: {
                  int salida;
                  char * result = NULL;
                  lista = (char *)calloc(BUF_SIZE + 1, sizeof(char));
                  switch (c) {
                    case COMANDO_CONECTARSE: {
                      int longitud_nombre;
                      leer(conn, &longitud_nombre, sizeof(int));
                      char * nombre = (char *)calloc(longitud_nombre + 1, sizeof(char));
                      leer(conn, nombre, longitud_nombre);
                      int longitud_clave;
                      leer(conn, &longitud_clave, sizeof(int));
                      char * clave = (char *)calloc(longitud_nombre + 1, sizeof(char));
                      leer(conn, clave, longitud_clave);
                      salida = conectar(nombre, clave, conn);
                      if (salida == 1) {
                        result = "Conexion exitosa.\n";
                      } else if (salida == 0) {
                        result = "El usuario ya está conectado.\n";      
                      } else if (salida == -2) {
                        result = "Contraseña Invalida.";
                      } else {
                        result = "Nombre de usuario no existe.\n";      
                      }
                    } break;
                    case COMANDO_SALIR: {
                      GList * resultado = g_list_find_custom(user, &conn, buscarUsuarioPorSocket);
                      char * nombre = ((struct usuario *)resultado->data)->nombre;
                      salida = salir(nombre);
		      if (salida == 1) {
		        result = "El usuario ha salido del chat exitosamente.\n";
		      } else if (salida == 0) {
		        result = "El usuario no está conectado.\n";
		      } else if (salida == 2) {
		        result = "El usuario está conectado a una sala.\n";
		      } else {
		        result = "El usuario no existe.\n";
		      }
                    } break;
                    case COMANDO_ENTRAR: {
                      int longitud_sala;
                      leer(conn, &longitud_sala, sizeof(int));
		      char * sala = (char *)calloc(longitud_sala + 1, sizeof(char));
                      leer(conn, sala, longitud_sala);
                      GList * resultado = g_list_find_custom(user, &conn, buscarUsuarioPorSocket);
                      char * nombre = ((struct usuario *)resultado->data)->nombre;
		      salida = entrarSala(sala, nombre);
		      if (salida == -4) {
			result = "El usuario no está conectado al chat.\n";
		      } else if (salida == -3) {
		        result = "El usuario no existe.\n";
		      } else if (salida == -2) {
		        result = "El usuario ya está conectado a una sala.\n";
		      } else if (salida == 0) {
		        result = "La sala está inactiva.\n";
		      } else if (salida == 1) {
		        result = "Conexión exitosa a la sala.\n";
		      } else {
		        result = "La sala no existe.\n";
		      }
                    } break;
                    case COMANDO_DEJAR: {
                      int longitud_sala;
                      leer(conn, &longitud_sala, sizeof(int));
		      char * sala = (char *)calloc(longitud_sala + 1, sizeof(char));
                      leer(conn, sala, longitud_sala);
                      GList * resultado = g_list_find_custom(user, &conn, buscarUsuarioPorSocket);
                      char * nombre = ((struct usuario *)resultado->data)->nombre;
                      salida = dejarSala(sala, nombre);
                      if (salida == 0) {
                        result = "El usuario no está conectado a la sala.\n";
                      } else if (salida == 1){
                        result = "El usuario dejó la sala exitosamente.\n";
                      } else {
                        result = "La sala no existe.\n";
                      }
                    } break;
                    case COMANDO_VER_SALAS: {
                      GList * resultado = g_list_find_custom(user, &conn, buscarUsuarioPorSocket);
                      char * nombre = ((struct usuario *)resultado->data)->nombre;
                      resultado = g_list_find_custom(user, nombre, buscarUsuarioPorNombre);
                      int is_admin = ((struct usuario *)resultado->data)->is_admin;
                      salida = verSalas(is_admin);
                      if (salida == 1) {
		        result = (char *)calloc(strlen(lista) + 1, sizeof(char));
                        result = lista;
                      } else {
                        result = "No hay salas en el servidor.\n";
                      }
                    } break;
                    case COMANDO_VER_USUARIOS: {
                      GList * resultado = g_list_find_custom(user, &conn, buscarUsuarioPorSocket);
                      char * nombre = ((struct usuario *)resultado->data)->nombre;
                      int is_admin = ((struct usuario *)resultado->data)->is_admin;
                      salida = verUsuarios(is_admin);
                      if (salida == 1) {
		        result = (char *)calloc(strlen(lista) + 1, sizeof(char));
                        result = lista;
                      } else {
                        result = "No hay usuarios conectados en el servidor.\n";
                      }
                    } break;
                    case COMANDO_VER_USUARIOS_SALAS: {
                      int longitud_sala;
                      leer(conn, &longitud_sala, sizeof(int));
		      char * sala = (char *)calloc(longitud_sala + 1, sizeof(char));
                      leer(conn, sala, longitud_sala);
                      salida = verUsuSalas(sala, conn);
                      if (salida == -2) {
                        result = "La sala está inactiva\n";
                      } else if (salida == 0) {
                        result = "La sala no tiene usuarios conectados\n";
                      } else if (salida == 1) {
                        result = "";
                      } else {
                        result = "La sala no existe.\n";
                      }
                    } break;
                    case COMANDO_ENVIAR_MENSAJE: {
                      int longitud_mensaje;
                      leer(conn, &longitud_mensaje, sizeof(int));
		      char * mensaje = (char *)calloc(longitud_mensaje + 1, sizeof(char));
                      leer(conn, mensaje, longitud_mensaje);

                      GList * resultado = g_list_find_custom(user, &conn, buscarUsuarioPorSocket);
                      char * nombre = ((struct usuario *)resultado->data)->nombre;
                      char * sala = ((struct usuario *)resultado->data)->sala;
                      salida = enviarMensaje(sala, mensaje);
                      result = "";
                    } break;
                    case COMANDO_CREAR_USUARIO: {
                      int longitud_nombre;
                      leer(conn, &longitud_nombre, sizeof(int));
                      char * nombre = (char *)calloc(longitud_nombre + 1, sizeof(char));
                      leer(conn, nombre, longitud_nombre);
                      int longitud_clave;
                      leer(conn, &longitud_clave, sizeof(int));
                      char * clave = (char *)calloc(longitud_nombre + 1, sizeof(char));
                      leer(conn, clave, longitud_clave);
                      GList * resultado = g_list_find_custom(user, &conn, buscarUsuarioPorSocket);
                      int is_admin = ((struct usuario *)resultado->data)->is_admin;
                      if (is_admin == 0) {
                        result = "Comando no autorizado\n";
                        break;
                      }
                      salida = crearUsuario(nombre, clave);
                      if (salida == 1){
                        result = "Usuario creado exitosamente.\n";
                      } else {
                        result = "Nombre de usuario ya existe.\n";
                      }
                    } break;
                    case COMANDO_ELIMINAR_USUARIO: {
                      int longitud_nombre;
                      leer(conn, &longitud_nombre, sizeof(int));
		      char * nombre = (char *)calloc(longitud_nombre + 1, sizeof(char));
                      leer(conn, nombre, longitud_nombre);
                      int longitud_clave;
                      leer(conn, &longitud_clave, sizeof(int));
                      char * clave = (char *)calloc(longitud_nombre + 1, sizeof(char));
                      leer(conn, clave, longitud_clave);
                      GList * resultado = g_list_find_custom(user, &conn, buscarUsuarioPorSocket);
                      int is_admin = ((struct usuario *)resultado->data)->is_admin;
                      if (is_admin == 0) {
                        result = "Comando no autorizado\n";
                        break;
                      }
                      salida = eliminarUsuario(nombre, clave);
                      if (salida == -2) {
                        result = "Contraseña incorrecta.\n";
                      } else if (salida == 0) {
                        result = "El usuario está conectado a una sala. No puede ser eliminado.\n";
                      } else if (salida == 1){
                        result = "El usuario fue eliminado exitosamente.\n";
                      } else {
                        result = "El usuario no existe.\n";
                      }
                    } break;
                    case COMANDO_CREAR_SALA: {
                      int longitud_sala;
                      leer(conn, &longitud_sala, sizeof(int));
		      char * sala = (char *)calloc(longitud_sala + 1, sizeof(char));
                      leer(conn, sala, longitud_sala);
                      GList * resultado = g_list_find_custom(user, &conn, buscarUsuarioPorSocket);
                      int is_admin = ((struct usuario *)resultado->data)->is_admin;
                      if (is_admin == 0) {
                        result = "Comando no autorizado\n";
                      } else {
                        salida = crearSala(sala);
                        if (salida == 1){
                          result = "Sala creada exitosamente.\n";
                        } else {
                          result = "La sala ya existe.\n";
                        }
                      }
                    } break;
                    case COMANDO_ELIMINAR_SALA: {
                      int longitud_sala;
                      leer(conn, &longitud_sala, sizeof(int));
		      char * sala = (char *)calloc(longitud_sala + 1, sizeof(char));
                      leer(conn, sala, longitud_sala);
                      GList * resultado = g_list_find_custom(user, &conn, buscarUsuarioPorSocket);
                      int is_admin = ((struct usuario *)resultado->data)->is_admin;
                      if (is_admin == 0) {
                        result = "Comando no autorizado\n";
                        break;
                      }
		      salida = eliminarSala(sala);
		      if(salida == 0) {
		        result = "Hay usuarios conectados en la sala. No puede ser eliminada.\n";
		      } else if (salida == 1) {
		        result = "Sala eliminada exitosamente.\n";
		      } else {
		        result = "La sala no existe.\n";
		      }
                    } break;
                    case COMANDO_HABILITAR_SALA: {
                      int longitud_sala;
                      leer(conn, &longitud_sala, sizeof(int));
		      char * sala = (char *)calloc(longitud_sala + 1, sizeof(char));
                      leer(conn, sala, longitud_sala);
                      GList * resultado = g_list_find_custom(user, &conn, buscarUsuarioPorSocket);
                      int is_admin = ((struct usuario *)resultado->data)->is_admin;
                      if (is_admin == 0) {
                        result = "Comando no autorizado\n";
                        break;
                      }
                      salida = habilitarSala(sala);
                      if (salida == -1){
                        result = "La sala no existe.\n";
                      } else {
                        result = "Sala habilitada.\n";
                      }
                    } break;
                    case COMANDO_DESHABILITAR_SALA: {
                      int longitud_sala;
                      leer(conn, &longitud_sala, sizeof(int));
		      char * sala = (char *)calloc(longitud_sala + 1, sizeof(char));
                      leer(conn, sala, longitud_sala);
                      GList * resultado = g_list_find_custom(user, &conn, buscarUsuarioPorSocket);
                      int is_admin = ((struct usuario *)resultado->data)->is_admin;
                      if (is_admin == 0) {
                        result = "Comando no autorizado\n";
                        break;
                      }
                      salida = deshabilitarSala(sala);
                      if (salida == -1){
                        result = "La sala no existe.\n";
                      } else {
                        result = "Sala deshabilitada exitosamente.\n";
                      }
                    } break;
                    case COMANDO_VER_BITACORA: {
                    } break;
                  }
                  escribir(conn, result, strlen(result));
                } break;
              } /* switch(read(enum comando)) */
            } /* if (FD_ISSET(fds)) */
          } /* for (sockets a clientes) */
        } /* main loop */

        if (verbose) fprintf(stderr, "Error imposible.\n");
        exit(EX_SOFTWARE);
}
