#include <stdio.h>
#include <stdlib.h>
#include "../include/mime.h"
#include "../include/files.h"

int main() {
    printf("=== PRUEBAS DEL MODULO MIME ===\n");
    printf("Archivo 'index.html' -> Tipo: %s\n", get_mime_type("index.html"));
    printf("Archivo 'style.css'  -> Tipo: %s\n", get_mime_type("style.css"));
    printf("Archivo 'image.png'  -> Tipo: %s\n", get_mime_type("image.png"));
    printf("Archivo 'desconocido.rar' -> Tipo: %s\n\n", get_mime_type("desconocido.rar"));

    printf("=== PRUEBAS DEL MODULO FILES ===\n");
    
    // Prueba 1: Ruta normal
    char *ruta1 = get_secure_path("/");
    printf("Peticion URI '/' -> Ruta segura: %s\n", ruta1 ? ruta1 : "NULL (Archivo no existe o peligroso)");
    if (ruta1) free(ruta1); // Liberar memoria

    // Prueba 2: Ruta a un archivo específico
    char *ruta2 = get_secure_path("/style.css");
    printf("Peticion URI '/style.css' -> Ruta segura: %s\n", ruta2 ? ruta2 : "NULL (Archivo no existe o peligroso)");
    if (ruta2) free(ruta2);

    // Prueba 3: Intento de hackeo (Directory Traversal)
    char *ruta3 = get_secure_path("/../../etc/passwd");
    printf("Peticion URI '/../../etc/passwd' -> Ruta segura: %s\n", ruta3 ? ruta3 : "NULL (BLOQUEADO CORRECTAMENTE)");
    if (ruta3) free(ruta3);

    return 0;
}
