#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

static int isoctal(int c) {
    return c >= '0' && c <= '7';
}

static int ishex(int c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// TODO this has not been thoroughly tested, at all.
int sscanf(const char *str, const char *fmt, ...) {
    char c;
    int scanned = 0;

    va_list arg;
    va_start(arg, fmt);

    while ((c = *fmt++)) {
        if (isspace(c)) {
            if (!isspace(*str)) {
                goto done;
            }

            do {
                ++str;
            } while (*str && isspace(*str));
        } else if (c == '%') {
            int skip = 0;
            int width = -1;

            if (*fmt == '*') {
                skip = 1;
                ++fmt;
            }

            if (isdigit(*fmt)) {
                width = *fmt++ - '0';
                while (isdigit(*fmt)) {
                    width *= 10;
                    width += *fmt++ - '0';
                }
            }

            switch (*fmt++) {
                case 'd': case 'i': {
                    int result_int = 0;
                    int result_sign = 1;

                    if (*str == '+') {
                        ++str;
                    } else if (*str == '-') {
                        result_sign = -1;
                    }

                    if (!isdigit(*str)) {
                        goto done;
                    }

                    do {
                        int wide = ((int64_t) result_int * 10) + (*str++ - '0');
                        if (wide > INT_MAX) {
                            goto done;
                        }
                        result_int = wide;
                    } while (--width && *str && isdigit(*str));

                    if (!skip) {
                        result_int *= result_sign;
                        *va_arg(arg, int *) = result_int;
                        ++scanned;
                    }

                    break;
                }

                case 'o': {
                    int result_int = 0;

                    if (!isoctal(*str)) {
                        goto done;
                    }

                    do {
                        int wide = ((int64_t) result_int * 8) + (*str++ - '0');
                        if (wide > INT_MAX) {
                            goto done;
                        }
                        result_int = wide;
                    } while (--width && *str && isoctal(*str));

                    if (!skip) {
                        *va_arg(arg, int *) = result_int;
                        ++scanned;
                    }

                    break;
                }

                case 'x': {
                    int result_int = 0;

                    if (!ishex(*str)) {
                        goto done;
                    }

                    do {
                        int ch = *str++;
                        if (ch >= 'a') {
                            ch += 10 - 'a';
                        } else if (ch >= 'A') {
                            ch += 10 - 'A';
                        } else {
                            ch -= '0';
                        }

                        int wide = ((int64_t) result_int * 16) + ch;
                        if (wide > INT_MAX) {
                            goto done;
                        }
                        result_int = wide;
                    } while (--width && *str && ishex(*str));

                    if (!skip) {
                        *va_arg(arg, int *) = result_int;
                        ++scanned;
                    }

                    break;
                }

                case 's': {
                    char *result_str = NULL;
                    if (!skip) {
                        result_str = va_arg(arg, char *);
                    }

                    while (--width && !isspace((c = *str))) {
                        ++str;

                        if (!skip) {
                            *result_str++ = c;
                        }
                    }

                    if (!skip) {
                        *result_str++ = 0;
                        ++scanned;
                    }

                    break;
                }
            }
        } else {
            if (*str++ != c) {
                goto done;
            }
        }
    }

done:
    va_end(arg);
    return scanned;
}
