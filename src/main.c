#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/server.h"

int main() {
    // 1. Inicializar el socket 
    int sd = passiveTCP(DEFAULT_PORT, QLEN);
    if (sd < 0) {
        errexit("Fallo al inicializar el socket del servidor\n");
    }

    // 2. Arrancar el motor de concurrencia epoll (un solo hilo, múltiples clientes)
    run_server(sd);

    // 3. Limpieza 
    close(sd);
    return EXIT_SUCCESS;
}
