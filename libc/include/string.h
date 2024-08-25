#ifndef _STRING_H_
#define _STRING_H_

#include <stdint.h>

int memcmp(const void *, const void *, size_t);
void *memset(void *, int, size_t);
void *memcpy(void *__restrict, const void *__restrict, size_t);
void *memmove(void *, const void *, size_t);

size_t strlen(const char *);

int strcmp(const char *, const char *);
int strncmp(const char *, const char *, size_t);

int strcasecmp(const char *, const char *);
int strncasecmp(const char *, const char *, size_t);

char *strncpy(char *__restrict dst, const char *__restrict src, size_t);
char *strstr(const char *haystack, const char *needle);
char *strchr(const char *s, int c);


#endif
