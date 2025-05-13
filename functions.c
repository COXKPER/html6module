#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include "functions.h"

#define MAX_VARS 256

typedef struct {
    char* name;
    char* value;
} Var;

static Var vars[MAX_VARS];
static int var_count = 0;

void set_var(const char* name, const char* value) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            free(vars[i].value);
            vars[i].value = strdup(value);
            return;
        }
    }
    vars[var_count].name = strdup(name);
    vars[var_count].value = strdup(value);
    var_count++;
}

const char* get_var(const char* name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0)
            return vars[i].value;
    }
    return "";
}

char* curl_buffer = NULL;
size_t curl_write(void* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t len = size * nmemb;
    curl_buffer = realloc(curl_buffer, (curl_buffer ? strlen(curl_buffer) : 0) + len + 1);
    strncat(curl_buffer, ptr, len);
    return len;
}

char* curl_get(const char* url) {
    CURL* curl = curl_easy_init();
    if (!curl) return strdup("");

    curl_buffer = calloc(1, 1);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return curl_buffer;
}

char* read_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char* buffer = malloc(len + 1);
    fread(buffer, 1, len, f);
    fclose(f);
    buffer[len] = '\0';
    return buffer;
}

char* interpolate_vars(const char* input) {
    char* output = malloc(strlen(input) * 2);
    output[0] = '\0';

    const char* ptr = input;
    while (*ptr) {
        const char* start = strstr(ptr, "{{");
        if (!start) {
            strcat(output, ptr);
            break;
        }

        strncat(output, ptr, start - ptr);
        const char* end = strstr(start, "}}");
        if (!end) break;

        char varname[64];
        strncpy(varname, start + 2, end - start - 2);
        varname[end - start - 2] = '\0';

        const char* value = get_var(varname);
        strcat(output, value);
        ptr = end + 2;
    }

    return output;
}

char* eval_expression(const char* expr) {
    // If it's a function call: var = func()
    if (strncmp(expr, "curl_get(", 9) == 0) {
        const char* start = strchr(expr, '"');
        const char* end = strrchr(expr, '"');
        if (start && end && end > start) {
            char url[512];
            strncpy(url, start + 1, end - start - 1);
            url[end - start - 1] = '\0';
            return curl_get(url);
        }
    }

    // If it's var(var)
    char var[64], arg[64];
    if (sscanf(expr, "%63[^ (](%63[^)])", var, arg) == 2) {
        // Treat like pass-through: return value of arg
        return strdup(get_var(arg));
    }

    // Just a value
    return strdup(expr);
}

char* handle_logic_block(const char* block) {
    char* out = malloc(2048);
    out[0] = '\0';

    char* copy = strdup(block);
    char* line = strtok(copy, "\n");

    while (line) {
        while (isspace(*line)) line++;

        if (strncmp(line, "set ", 4) == 0 || strchr(line, '=')) {
            char name[64], rest_buf[512];
if (sscanf(line, "%63[^=]=%511[^\n]", name, rest_buf) == 2) {
    char* rest = rest_buf;
    while (isspace(*rest)) rest++;


                char* value = NULL;
                if (*rest == '"') {
                    // string literal
                    rest[strlen(rest) - 1] = '\0';
                    value = strdup(rest + 1);
                } else {
                    value = eval_expression(rest);
                }
                set_var(name, value);
                free(value);
            }
        } else if (strncmp(line, "echo ", 5) == 0) {
            const char* quote = strchr(line, '"');
            if (quote) {
                const char* end = strrchr(line, '"');
                if (end && end != quote) {
                    strncat(out, quote + 1, end - quote - 1);
                }
            }
        }

        line = strtok(NULL, "\n");
    }

    free(copy);
    return out;
}

char* process_blocks(const char* input) {
    const char* start = "<%";
    const char* end = "^>";
    const char* ptr = input;

    char* output = malloc(strlen(input) * 3);
    output[0] = '\0';

    while (*ptr) {
        const char* s = strstr(ptr, start);
        if (!s) {
            strcat(output, ptr);
            break;
        }

        strncat(output, ptr, s - ptr);
        const char* e = strstr(s, end);
        if (!e) break;

        char* block = strndup(s + 2, e - s - 2);
        char* result = handle_logic_block(block);
        strcat(output, result);
        free(result);
        free(block);
        ptr = e + strlen(end);
    }

    return output;
}

char* replace_custom_tags(const char* input) {
    char* output = strdup(input);
    // Add your custom <card>, etc. replacements
    return output;
}

char* parse_hdm_file(const char* filename) {
    char* raw = read_file(filename);
    if (!raw) return NULL;

    char* processed = process_blocks(raw);
    char* interpolated = interpolate_vars(processed);
    char* final_output = replace_custom_tags(interpolated);

    free(raw);
    free(processed);
    free(interpolated);
    return final_output;
}

