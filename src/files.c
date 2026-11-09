#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../include/files.h"

char *get_secure_path(const char *uri) {
	if(uri == NULL || strlen(uri) == 0) return NULL;

	char target_uri[256];
	if (strcmp(uri, "/") == 0) {
		strncpy(target_uri, "/index.html", sizeof(target_uri));

	} else {
		strncpy(target_uri, uri, sizeof(target_uri));
	}

	char relative_path[512];
	snprintf(relative_path, sizeof(relative_path), "www%s", target_uri);

	char *resolved_path = realpath(relative_path, NULL);
	if (resolved_path == NULL) {
		return NULL;
	}

	char *base_path = realpath("www", NULL);
	if (base_path == NULL) {
		free(resolved_path);
		return NULL;
	}

	if (strncmp(resolved_path, base_path, strlen(base_path)) !=0) {
		free(resolved_path);
		free(base_path);
		return NULL;
	}

	free(base_path);
	return resolved_path;
}

