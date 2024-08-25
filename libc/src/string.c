// Uses musl for most implementations here.

#include <ctype.h>
#include <string.h>

int memcmp(const void *_l, const void *_r, size_t n) {
    const unsigned char *l = _l;
    const unsigned char *r = _r;
    for (; n && *l == *r; l++, r++, n--);
    return *l - *r;
}

void *memset(void *dest, int c, size_t n) {
    unsigned char *s = dest;
    size_t k;

    /* Fill head and tail with minimal branching. Each
    * conditional ensures that all the subsequently used
    * offsets are well-defined and in the dest region. */

    if (!n) return dest;
    s[0] = c;
    s[n-1] = c;
    if (n <= 2) return dest;
    s[1] = c;
    s[2] = c;
    s[n-2] = c;
    s[n-3] = c;
    if (n <= 6) return dest;
    s[3] = c;
    s[n-4] = c;
    if (n <= 8) return dest;

    /* Advance pointer to align it at a 4-byte boundary,
    * and truncate n to a multiple of 4. The previous code
    * already took care of any head/tail that get cut off
    * by the alignment. */

    k = -(uintptr_t)s & 3;
    s += k;
    n -= k;
    n &= -4;

    /* Pure C fallback with no aliasing violations. */
    for (; n; n--, s++) *s = c;

    return dest;
}

void *memcpy(void *__restrict dest, const void *__restrict src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    for (; n; n--) *d++ = *s++;
    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;

    if (d == s) return d;

    if (d < s) {
        for (; n; n--) *d++ = *s++;
    } else {
        while (n) n--, d[n] = s[n];
    }

    return dest;
}

size_t strlen(const char *s) {
    const char *a = s;
    for (; *s; s++);
    return s - a;
}

int strcmp(const char *l, const char *r) {
    for (; *l==*r && *l; l++, r++);
    return *(unsigned char *)l - *(unsigned char *)r;
}

int strncmp(const char *_l, const char *_r, size_t n) {
    const unsigned char *l=(void *)_l, *r=(void *)_r;
    if (!n--) return 0;
    for (; *l && *r && n && *l == *r ; l++, r++, n--);
    return *l - *r;
}

int strcasecmp(const char *_l, const char *_r) {
    const unsigned char *l=(void *)_l, *r=(void *)_r;
    for (; *l && *r && (*l == *r || tolower(*l) == tolower(*r)); l++, r++);
    return tolower(*l) - tolower(*r);
}

int strncasecmp(const char *_l, const char *_r, size_t n) {
    const unsigned char *l=(void *)_l, *r=(void *)_r;
    if (!n--) return 0;
    for (; *l && *r && n && (*l == *r || tolower(*l) == tolower(*r)); l++, r++, n--);
    return tolower(*l) - tolower(*r);
}

char *strncpy(char *__restrict d, const char *__restrict s, size_t n) {
	char *dest = d;
    for (; n && (*d=*s); n--, s++, d++);
    memset(d, 0, n);
    return dest;
}

char *strstr(const char *haystack, const char *needle) {
	while (*haystack) {
		if (!strcmp(haystack, needle)) {
			return (char *) haystack;
		}
		++haystack;
	}
	return NULL;
}

char *strchr(const char *s, int c) {
	char *r;

	c = (unsigned char)c;
	if (!c) {
		r = (char *) s + strlen(s);
	} else {
		for (; *s && *(unsigned char *)s != c; s++);
		r = (char *) s;
	}

    return *(unsigned char *)r == (unsigned char)c ? r : 0;
}
