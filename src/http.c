#include <stdio.h>
#include <string.h>
#include "../include/http.h"

int parse_http_request(const char *request, char *method, char *uri, char *version) {
    // sscanf extrae las 3 partes separadas por espacios. 
    // Los números (%15s, %255s) evitan desbordamientos de memoria si envían textos gigantes.
    int parsed = sscanf(request, "%15s %255s %15s", method, uri, version);
    
    // Si no se extrae exactamente 3 partes, la petición está corrupta
    if (parsed != 3) {
        return 400; 
    }

    // El servidor solo debe aceptar el método GET
    if (strcmp(method, "GET") != 0) {
        return 405;
    }

    return 200;
}

void build_http_response(char *buffer, size_t max_size, int status_code, const char *content_type, long content_length) {
    const char *status_text = "OK";
    
    // Mapear el número de error al texto oficial HTTP
    if (status_code == 400) status_text = "Bad Request";
    else if (status_code == 403) status_text = "Forbidden";
    else if (status_code == 404) status_text = "Not Found";
    else if (status_code == 405) status_text = "Method Not Allowed";
    else if (status_code == 500) status_text = "Internal Server Error";

    // snprintf arma el texto de respuesta sin pasarse del límite max_size
    snprintf(buffer, max_size,
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n"
             "\r\n",
             status_code, status_text, content_type, content_length);
}
