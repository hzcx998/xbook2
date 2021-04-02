#include <ctype.h>

int isspace(char c)
{
	char comp[] = {' ', '\t', '\n', '\r', '\v', '\f'};
	int i;
	const int len = 6;
	for (i = 0; i < len; i++) {
		if (c == comp[i])
			return 1;
	}
	return 0;
}
