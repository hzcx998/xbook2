/*
 * xlibc/stdio/vsscanf.c
 */

#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

enum flags {
	FL_SPLAT			= 0x01,			/* Drop the value, do not assign */
	FL_INV				= 0x02,			/* Character-set with inverse */
	FL_WIDTH			= 0x04,			/* Field width specified */
	FL_MINUS			= 0x08,			/* Negative number */
};

enum ranks {
	rank_char			= -2,
	rank_short			= -1,
	rank_int			= 0,
	rank_long			= 1,
	rank_longlong		= 2,
	rank_ptr			= INT_MAX,		/* Special value used for pointers */
};

#define MIN_RANK		rank_char
#define MAX_RANK		rank_longlong
#define INTMAX_RANK		rank_longlong
#define SIZE_T_RANK		rank_long
#define PTRDIFF_T_RANK	rank_long

enum bail {
	bail_none = 0,						/* No error condition */
	bail_eof,							/* Hit EOF */
	bail_err,							/* Conversion mismatch */
};

static inline const char * skipspace(const char * p)
{
	while (isspace((unsigned char)*p))
		p++;
	return p;
}

static inline void set_bit(unsigned long *bitmap, unsigned int bit)
{
	bitmap[bit / (8 * sizeof(long))] |= 1UL << (bit % (8 * sizeof(long)));
}

static inline int test_bit(unsigned long *bitmap, unsigned int bit)
{
	return (int)(bitmap[bit / (8 * sizeof(long))] >> (bit % (8 * sizeof(long)))) & 1;
}

int vsscanf(const char * buf, const char * fmt, va_list ap)
{
	const char * p = fmt;
	char ch;
	unsigned char uc;
	const char *q = buf;
	const char *qq;
	uintmax_t val = 0;
	int rank = rank_int;
	unsigned int width = INT_MAX;
	int base;
	enum flags flags = 0;
	enum {
		st_normal,				/* Ground state */
		st_flags,				/* Special flags */
		st_width,				/* Field width */
		st_modifiers,			/* Length or conversion modifiers */
		st_match_init,			/* Initial state of %[ sequence */
		st_match,				/* Main state of %[ sequence */
		st_match_range,			/* After - in a %[ sequence */
	} state = st_normal;
	char *sarg = NULL;			/* %s %c or %[ string argument */
	enum bail bail = bail_none;
	int converted = 0;			/* Successful conversions */
	unsigned long matchmap[((1 << 8) + ((8 * sizeof(long)) - 1)) / (8 * sizeof(long))];
	int matchinv = 0;			/* Is match map inverted? */
	unsigned char range_start = 0;

	while ((ch = *p++) && !bail)
	{
		switch (state)
		{
		case st_normal:
			if (ch == '%')
			{
				state = st_flags;
				flags = 0;
				rank = rank_int;
				width = INT_MAX;
			}
			else if (isspace((unsigned char) ch))
			{
				q = skipspace(q);
			}
			else
			{
				if (*q == ch)
					q++;
				else
					bail = bail_err;
			}
			break;

		case st_flags:
			switch (ch)
			{
			case '*':
				flags |= FL_SPLAT;
				break;
			case '0' ... '9':
				width = (ch - '0');
				state = st_width;
				flags |= FL_WIDTH;
				break;
			default:
				state = st_modifiers;
				p--;	/* Process this character again */
				break;
			}
			break;

		case st_width:
			if (ch >= '0' && ch <= '9')
			{
				width = width * 10 + (ch - '0');
			}
			else
			{
				state = st_modifiers;
				p--;	/* Process this character again */
			}
			break;

		case st_modifiers:
			switch (ch)
			{
			/* Length modifiers - nonterminal sequences */
			case 'h':
				rank--;	/* Shorter rank */
				break;
			case 'l':
				rank++;	/* Longer rank */
				break;
			case 'j':
				rank = INTMAX_RANK;
				break;
			case 'z':
				rank = SIZE_T_RANK;
				break;
			case 't':
				rank = PTRDIFF_T_RANK;
				break;
			case 'L':
			case 'q':
				rank = rank_longlong;	/* long double/long long */
				break;

			default:
				/* Output modifiers - terminal sequences */
				/* Next state will be normal */
				state = st_normal;

				/* Canonicalize rank */
				if (rank < MIN_RANK)
					rank = MIN_RANK;
				else if (rank > MAX_RANK)
					rank = MAX_RANK;

				switch (ch)
				{
				case 'P':	/* Upper case pointer */
				case 'p':	/* Pointer */
					rank = rank_ptr;
					base = 0;
					goto scan_int;

				case 'i':	/* Base-independent integer */
					base = 0;
					goto scan_int;

				case 'd':	/* Decimal integer */
					base = 10;
					goto scan_int;

				case 'o':	/* Octal integer */
					base = 8;
					goto scan_int;

				case 'u':	/* Unsigned decimal integer */
					base = 10;
					goto scan_int;

				case 'x':	/* Hexadecimal integer */
				case 'X':
					base = 16;
					goto scan_int;

				case 'n':	/* # of characters consumed */
					val = (q - buf);
					goto set_integer;

					scan_int: q = skipspace(q);
					if (!*q)
					{
						bail = bail_eof;
						break;
					}
					val = strntoumax(q, (char **)&qq, base, width);
					if (qq == q)
					{
						bail = bail_err;
						break;
					}
					q = qq;
					if (!(flags & FL_SPLAT))
						converted++;

					set_integer: if (!(flags & FL_SPLAT))
					{
						switch (rank)
						{
						case rank_char:
							*va_arg(ap, unsigned char *) = val;
							break;
						case rank_short:
							*va_arg(ap, unsigned short *) = val;
							break;
						case rank_int:
							*va_arg(ap, unsigned int *) = val;
							break;
						case rank_long:
							*va_arg(ap, unsigned long *) = val;
							break;
						case rank_longlong:
							*va_arg(ap, unsigned long long *) = val;
							break;
						case rank_ptr:
							*va_arg(ap, void **) = (void *) (uintptr_t) val;
							break;
						}
					}
					break;

				case 'c':	/* Character */
					width = (flags & FL_WIDTH) ? width : 1;
					if (flags & FL_SPLAT)
					{
						while (width--)
						{
							if (!*q)
							{
								bail = bail_eof;
								break;
							}
						}
					}
					else
					{
						sarg = va_arg(ap, char *);
						while (width--)
						{
							if (!*q)
							{
								bail = bail_eof;
								break;
							}
							*sarg++ = *q++;
						}
						if (!bail)
							converted++;
					}
					break;

				case 's':	/* String */
					uc = 1;	/* Anything nonzero */
					if (flags & FL_SPLAT)
					{
						while (width-- && (uc = *q) && !isspace(uc))
						{
							q++;
						}
					}
					else
					{
						char *sp;
						sp = sarg = va_arg(ap, char *);
						while (width-- && (uc = *q) && !isspace(uc))
						{
							*sp++ = uc;
							q++;
						}
						if (sarg != sp)
						{
							/*
							 * Terminate output
							 */
							*sp = '\0';
							converted++;
						}
					}
					if (!uc)
						bail = bail_eof;
					break;

				case '[':	/* Character range */
					sarg = (flags & FL_SPLAT) ? NULL : va_arg(ap, char *);
					state = st_match_init;
					matchinv = 0;
					memset(matchmap, 0, sizeof matchmap);
					break;

				case '%':	/* %% sequence */
					if (*q == '%')
						q++;
					else
						bail = bail_err;
					break;

				default:	/* Anything else */
					/* Unknown sequence */
					bail = bail_err;
					break;
				}
				break;
			}
			break;

		case st_match_init:	/* Initial state for %[ match */
			if (ch == '^' && !(flags & FL_INV))
			{
				matchinv = 1;
			}
			else
			{
				set_bit(matchmap, (unsigned char) ch);
				state = st_match;
			}
			break;

		case st_match:		/* Main state for %[ match */
			if (ch == ']')
			{
				goto match_run;
			}
			else if (ch == '-')
			{
				range_start = (unsigned char) ch;
				state = st_match_range;
			}
			else
			{
				set_bit(matchmap, (unsigned char) ch);
			}
			break;

		case st_match_range:	/* %[ match after - */
			if (ch == ']')
			{
				/* - was last character */
				set_bit(matchmap, (unsigned char) '-');
				goto match_run;
			}
			else
			{
				int i;
				for (i = range_start; i < (unsigned char) ch; i++)
					set_bit(matchmap, i);
				state = st_match;
			}
			break;

			match_run:	/* Match expression finished */
			qq = q;
			uc = 1;		/* Anything nonzero */
			while (width && (uc = *q) && (test_bit(matchmap, uc) ^ matchinv))
			{
				if (sarg)
					*sarg++ = uc;
				q++;
			}
			if (q != qq && sarg)
			{
				*sarg = '\0';
				converted++;
			}
			else
			{
				bail = bail_err;
			}
			if (!uc)
				bail = bail_eof;
			break;
		}
	}

	if (bail == bail_eof && !converted)
		converted = -1;

	return converted;
}
