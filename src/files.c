#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "../include/files.h"

char *get_secure_path(const char *uri) {
    if (uri == NULL || strlen(uri) == 0) return NULL;

    char target_uri[256];
    if (strcmp(uri, "/") == 0) {
        snprintf(target_uri, sizeof(target_uri), "/index.html");
    } else {
        snprintf(target_uri, sizeof(target_uri), "%s", uri);
    }

    char relative_path[512];
    snprintf(relative_path, sizeof(relative_path), "www%s", target_uri);

    char *base_path = realpath("www", NULL);
    if (base_path == NULL) {
        errno = EIO;
        return NULL;
    }

    char *resolved_path = realpath(relative_path, NULL);
    if (resolved_path == NULL) {
        if (strstr(target_uri, "..") != NULL) {
            errno = EACCES;
        }
        free(base_path);
        return NULL;
    }

    // Si el archivo existe en el sistema pero está fuera de www
    if (strncmp(resolved_path, base_path, strlen(base_path)) != 0) {
        free(resolved_path);
        free(base_path);
        errno = EACCES;
        return NULL;
    }

    free(base_path);
    return resolved_path;
}
