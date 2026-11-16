#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/epoll.h>

#include "../include/server.h"
#include "../include/http.h"
#include "../include/files.h"
#include "../include/mime.h"

unsigned short portbase = 0;

/* Función de manejo de errores */
int errexit(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}

/* Función vital para epoll: Configura un socket como "No Bloqueante" */
static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int passivesock(const char *service, const char *transport, int qlen) {
    struct servent *pse;
    struct protoent *ppe;
    struct sockaddr_in sin;
    int s, type;
    int opt = 1;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;

    if ((pse = getservbyname(service, transport)))
        sin.sin_port = htons(ntohs((unsigned short)pse->s_port) + portbase);
    else if ((sin.sin_port = htons((unsigned short)atoi(service))) == 0)
        errexit("can't get \"%s\" service entry\n", service);

    if ((ppe = getprotobyname(transport)) == 0)
        errexit("can't get \"%s\" protocol entry\n", transport);

    if (strcmp(transport, "udp") == 0) type = SOCK_DGRAM;
    else type = SOCK_STREAM;

    s = socket(PF_INET, type, ppe->p_proto);
    if (s < 0) errexit("can't create socket: %s\n", strerror(errno));

    // Permitir reutilizar el puerto inmediatamente
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("can't bind to %s port: %s\n", service, strerror(errno));

    if (type == SOCK_STREAM && listen(s, qlen) < 0)
        errexit("can't listen on %s port: %s\n", service, strerror(errno));

    return s;
}

int passiveTCP(const char *service, int qlen) {
    return passivesock(service, "tcp", qlen);
}

// Motor concurrente con epoll
void run_server(int server_fd) {
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) errexit("Error al crear epoll\n");

    struct epoll_event event, events[MAX_EVENTS];
    
    set_nonblocking(server_fd);
    event.events = EPOLLIN; 
    event.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);

    printf("MiniHTTPd iniciado en http://localhost:%s\n", DEFAULT_PORT);
    printf("Esperando conexiones concurrentes (epoll + keep-alive)...\n\n");

    while (1) {
        int n_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        
        for (int i = 0; i < n_events; i++) {
            if (events[i].data.fd == server_fd) {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                if (client_fd == -1) continue;

                set_nonblocking(client_fd);
                event.events = EPOLLIN; 
                event.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
                
            } else {
                int client_fd = events[i].data.fd;
                char buffer[BUFFER_SIZE];
                memset(buffer, 0, BUFFER_SIZE);

                int bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_read <= 0) {
                    close(client_fd); // El navegador se desconectó voluntariamente
                    continue;
                }

                int keep_alive = 0;
                char method[16], uri[256], version[16];
                
                int status = parse_http_request(buffer, method, uri, version, &keep_alive);

                char *filepath = NULL;
                FILE *f = NULL;
                long file_size = 0;

                if (status == 200) {
                    filepath = get_secure_path(uri);
                    if (filepath == NULL) {
                        status = 404; 
                    } else {
                        f = fopen(filepath, "r");
                        if (f) {
                            fseek(f, 0, SEEK_END);
                            file_size = ftell(f);
                            fseek(f, 0, SEEK_SET);
                        } else {
                            status = 404;
                        }
                    }
                }

                char response_header[512];
                const char *mime_type = "text/html";
                if (filepath && status == 200) mime_type = get_mime_type(filepath);

                char error_body[128] = "";
                if (status != 200) {
                    snprintf(error_body, sizeof(error_body), "<h1>Error %d</h1>", status);
                    file_size = strlen(error_body);
                }

                build_http_response(response_header, sizeof(response_header), status, mime_type, file_size, keep_alive);
                
                send(client_fd, response_header, strlen(response_header), 0);
                if (status == 200 && f) {
                    char file_buf[4096];
                    size_t read_bytes;
                    while ((read_bytes = fread(file_buf, 1, sizeof(file_buf), f)) > 0) {
                        send(client_fd, file_buf, read_bytes, 0);
                    }
                    fclose(f);
                } else if (status != 200) {
                    send(client_fd, error_body, strlen(error_body), 0);
                }

                if (filepath) free(filepath);
                
                if (!keep_alive) {
                    close(client_fd);
                }
            }
        }
    }
}

