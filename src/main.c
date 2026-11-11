#include <stdio.h>
#include <string.h>
#include "../include/http.h"

int main() {
    char method[16], uri[256], version[16];
    int status;

    printf("=== PRUEBAS DEL PARSER HTTP ===\n");

    // Prueba 1: Petición correcta (debe devolver 200)
    status = parse_http_request("GET /index.html HTTP/1.1", method, uri, version);
    printf("Peticion 1: 'GET /index.html HTTP/1.1'\n");
    printf("-> Resultado: Estado %d | Metodo: %s | URI: %s\n\n", status, method, uri);

    // Prueba 2: Método no permitido (debe devolver 405)
    status = parse_http_request("POST /login HTTP/1.1", method, uri, version);
    printf("Peticion 2: 'POST /login HTTP/1.1'\n");
    printf("-> Resultado: Estado %d (Esperado: 405 Method Not Allowed)\n\n", status);

    // Prueba 3: Petición corrupta o incompleta (debe devolver 400)
    status = parse_http_request("GET /style.css", method, uri, version);
    printf("Peticion 3: 'GET /style.css' (incompleta)\n");
    printf("-> Resultado: Estado %d (Esperado: 400 Bad Request)\n\n", status);


    printf("=== PRUEBAS DE RESPUESTA HTTP ===\n");
    char buffer_respuesta[512];
    
    // Simular que vamos a enviar un archivo HTML de 1024 bytes exitosamente
    build_http_response(buffer_respuesta, sizeof(buffer_respuesta), 200, "text/html", 1024);
    printf("Respuesta generada (200 OK):\n\n%s", buffer_respuesta);

    return 0;
}
