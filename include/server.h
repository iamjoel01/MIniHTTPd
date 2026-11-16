#ifndef SERVER_H
#define SERVER_H

// Configuraciones del servidor
#define DEFAULT_PORT "8080"
#define QLEN 32           // Cola de conexiones en espera
#define MAX_EVENTS 64     // Máximo de eventos simultáneos para epoll
#define BUFFER_SIZE 4096

// Funciones proporcionadas por tu profesor
int errexit(const char *format, ...);
int passivesock(const char *service, const char *transport, int qlen);
int passiveTCP(const char *service, int qlen);

// Función principal que manejará los clientes usando epoll
void run_server(int server_fd);

#endif
