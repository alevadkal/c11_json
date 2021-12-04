/// Copyright © Alexander Kaluzhnyy

#include "json.h"
#include <stdio.h>

#ifndef JSON_PRINTER_INCLUDED
#define JSON_PRINTER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*json_putchar_t)(char c, void* data);

ssize_t json_uniprint(json_t* self, size_t indent, json_putchar_t putchar, void* data);
ssize_t json_fprint(json_t* self, size_t indent, FILE* file);
char* json_sprint(json_t* self, size_t indent);

#ifdef __cplusplus
}
#endif

#endif // JSON_PRINTER_INCLUDED