#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s file.hdm\n", argv[0]);
        return 1;
    }

    char* result = parse_hdm_file(argv[1]);
    if (result) {
        printf("%s\n", result);
        free(result);
    } else {
        fprintf(stderr, "Failed to parse .hdm file.\n");
    }

    return 0;
}

