#include <stdio.h>
#include <string.h>
#include "../include/http.h"

int parse_http_request(const char *request, char *method, char *uri, char *version, int *keep_alive) {
    *keep_alive = 0; 

    if (strstr(request, "\r\n\r\n") == NULL) {
        return 400;
    }

    int parsed = sscanf(request, "%15s %255s %15s", method, uri, version);
    if (parsed != 3) return 400;
    
    if (strlen(uri) >= 255) {
        return 400;
    }


    if (strcmp(method, "GET") != 0) return 405;

    //Analiza encabezados
    char host[256] = "Desconocido";
    const char *host_ptr = strstr(request, "Host: ");
    if (host_ptr) sscanf(host_ptr, "Host: %255s", host);

    char user_agent[256] = "Desconocido";
    const char *ua_ptr = strstr(request, "User-Agent: ");
    if (ua_ptr) sscanf(ua_ptr, "User-Agent: %255[^\r\n]", user_agent);

    //Conexión persistente
    if (strstr(request, "Connection: keep-alive") || strstr(request, "Connection: Keep-Alive")) {
        *keep_alive = 1;
    }

    printf("[HTTP] Petición a %s | Cliente: %s | Persistente: %s\n", 
           host, user_agent, *keep_alive ? "SI" : "NO");

    return 200;
}

void build_http_response(char *buffer, size_t max_size, int status_code, const char *content_type, long content_length, int keep_alive) {
    const char *status_text = "OK";
    
    if (status_code == 400) status_text = "Bad Request";
    else if (status_code == 403) status_text = "Forbidden";
    else if (status_code == 404) status_text = "Not Found";
    else if (status_code == 405) status_text = "Method Not Allowed";
    else if (status_code == 500) status_text = "Internal Server Error";

    snprintf(buffer, max_size,
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: %s\r\n"
             "\r\n",
             status_code, status_text, content_type, content_length,
             keep_alive ? "keep-alive" : "close");
}
