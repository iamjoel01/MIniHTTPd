#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/stat.h>

#include "../include/server.h"
#include "../include/http.h"
#include "../include/files.h"
#include "../include/mime.h"

int main() {
    //Inicializar el socket 
    int server_fd = passiveTCP(DEFAULT_PORT, QLEN);
    if (server_fd < 0) {
        errexit("Fallo al inicializar el socket del servidor\n");
    }

    //Arrancar el motor de concurrencia epoll
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) errexit("Error al crear epoll\n");

    struct epoll_event event, events[MAX_EVENTS];
    
    set_nonblocking(server_fd);
    event.events = EPOLLIN; 
    event.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);

    printf("MiniHTTPd iniciado en http://localhost:%s\n", DEFAULT_PORT);
    printf("Esperando conexiones concurrentes (epoll + keep-alive)...\n\n");

    //Bucle de eventos epoll
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
                    errno=0;
		    filepath = get_secure_path(uri);
                    if (filepath == NULL) {
			if (errno==EACCES)status=403;
			else if (errno==EIO) status=500;
			else status = 404;
                    } else {
			struct stat path_stat;
                        if (stat(filepath, &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
                            status = 403;
                        } else {
                            f = fopen(filepath, "rb");
                            if (f) {
                                fseek(f, 0, SEEK_END);
                                file_size = ftell(f);
                                fseek(f, 0, SEEK_SET);
                            } else {
                                if (errno == EACCES) status = 403;
				else status = 500;
                            }
                        }
                    }
                }

                char response_header[512];
                const char *mime_type = "text/html";
                if (filepath && status == 200) mime_type = get_mime_type(filepath);

                char error_body[128] = "";
                if (status != 200) {
                    snprintf(error_body, sizeof(error_body), "<h1>Error %d</h1>\n", status);
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
    
    close(server_fd);
    return EXIT_SUCCESS;
}
