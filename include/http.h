#ifndef HTTP_H
#define HTTP_H

#include <stddef.h> // Para usar size_t

// Analiza la primera línea de la petición HTTP.
int parse_http_request(const char *request, char *method, char *uri, char *version);

// Construye de forma segura el bloque de texto de la respuesta HTTP con sus encabezados.
void build_http_response(char *buffer, size_t max_size, int status_code, const char *content_type, long content_length);

#endif
