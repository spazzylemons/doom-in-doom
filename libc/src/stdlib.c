#include <ctype.h>
#include <stdlib.h>

int atoi(const char *s)
{
	char c;
	int n=0, neg=0;
	while (isspace(*s)) s++;
	switch (*s) {
	case '-': neg=1;
	case '+': s++;
	}
	/* Compute n as a negative number to avoid overflow on INT_MIN */
	while ((c = *s, c >= '0' && c <= '9'))
		n = 10*n - (*s++ - '0');
	return neg ? n : -n;
}
