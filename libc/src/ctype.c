#include <ctype.h>

int isdigit(int c) {
    return c >= '0' && c <= '9';
}

int isspace(int c) {
    return c == ' ' || (unsigned)c-'\t' < 5;
}

int tolower(int c) {
    if (c >= 'A' && c <= 'Z') c |= 32;
    return c;
}

int toupper(int c) {
    if (c >= 'a' && c <= 'z') c &= 0x5f;
    return c;
}
