# MiniHTTPd - Servidor HTTP/1.1

## Instrucciones de Compilación y Ejecución

**Compilacion del proyecto:**

```bash
make
```

(Para compilación manual: `gcc -Wall -Wextra -pedantic -Iinclude src/main.c src/server.c src/http.c src/files.c src/mime.c -o minihttpd`)

**Ejecucion del servidor:**

```bash
./minihttpd
```

El servidor se pondrá a la escucha en `http://localhost:8080/`.

## Pruebas de Funcionamiento

**1. Acceso estándar**

```bash
curl -i http://localhost:8080/index.html
```

**2. Escalado de directorios / Directory Traversal (403 Forbidden):** 

```bash
curl -i --path-as-is "http://localhost:8080/../../etc/passwd"
```

**3. Método no permitido (POST - 405 Method Not Allowed):** 

```bash
curl -i -X POST http://localhost:8080/index.html
```

**4. Recurso inexistente (404 Not Found):** 

```bash
curl -i http://localhost:8080/archivo_fantasma.html
```

**5. Tamaño de solicitud excedido / Buffer Overflow (400 Bad Request):** 

```bash
curl -i "http://localhost:8080/$(printf 'A%.0s' {1..500})"
```

**6. Error interno del servidor (500 Internal Server Error):** 

```bash
mv www www_oculta
curl -i http://localhost:8080/index.html
mv www_oculta www
```
