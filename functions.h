#ifndef FUNCTIONS_H
#define FUNCTIONS_H

char* parse_hdm_file(const char* filename);
char* process_blocks(const char* input);
char* replace_custom_tags(const char* input);
char* interpolate_vars(const char* input);
char* curl_get(const char* url);

#endif

