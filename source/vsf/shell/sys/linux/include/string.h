#ifndef __STRING_H__
#define __STRING_H__

#include "sys/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void * memset(void *s, int ch, size_t n);
size_t strlen(const char *str);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t n);
char * strdup(const char *str);
char * strcpy(char *dest, const char *src);
char * strncpy(char *dest, const char *src, size_t n);
char * strcat(char *dest, const char *src);
char * strncat(char *dest, const char *str, size_t n);
char * strstr(const char *str1, const char *str2);
char * strchr(const char *str, int c);
char * strrchr(const char *str, int c);
int strcoll(const char *str1, const char *str2);
char * strtok(char *str, const char *delim);
size_t strxfrm(char *dest, const char *src, size_t n);
size_t strspn(const char *str1, const char *str2);
size_t strcspn(const char *str1, const char *str2);
char * strpbrk(const char *str1, const char *str2);
int strcasecmp(const char *s1, const char *s2);
char * strerror(int errnum);

#if defined(__WIN__)
int stricmp(const char *s1, const char *s2);
#   if defined(__CPU_X64__)
void * memcpy(void *dest, const void *src, unsigned long long n);
#   else
void * memcpy(void *dest, const void *src, unsigned int n);
#   endif
#else
void * memcpy(void *dest, const void *src, size_t n);
#endif
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *str1, const void *str2, size_t n);
void *memchr(const void *buf, int ch, size_t count);

#ifdef __cplusplus
}
#endif

// TODO: add cpp related code outside extern "C"
#ifdef __cplusplus
#   ifdef __WIN__
#       include <xstring>
#   endif
#endif

#endif
