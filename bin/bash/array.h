/* array.h -- definitions for the interface exported by array.c that allows
   the rest of the shell to manipulate array variables. */

/* Copyright (C) 1997-2020 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _ARRAY_H_
#define _ARRAY_H_

#include "stdc.h"

typedef intmax_t	arrayind_t;

enum atype {array_indexed, array_assoc};	/* only array_indexed used */

typedef struct array {
	enum atype	type;
	arrayind_t	max_index;
	int		num_elements;
	struct array_element *head;
	struct array_element *lastref;
} ARRAY;

typedef struct array_element {
	arrayind_t	ind;
	char	*value;
	struct array_element *next, *prev;
} ARRAY_ELEMENT;

typedef int sh_ae_map_func_t PARAMS((ARRAY_ELEMENT *, void *));

/* Basic operations on entire arrays */
extern ARRAY	*array_create PARAMS((void));
extern void	array_flush PARAMS((ARRAY *));
extern void	array_dispose PARAMS((ARRAY *));
extern ARRAY	*array_copy PARAMS((ARRAY *));
extern ARRAY	*array_slice PARAMS((ARRAY *, ARRAY_ELEMENT *, ARRAY_ELEMENT *));
extern void	array_walk PARAMS((ARRAY   *, sh_ae_map_func_t *, void *));

extern ARRAY_ELEMENT *array_shift PARAMS((ARRAY *, int, int));
extern int	array_rshift PARAMS((ARRAY *, int, char *));
extern ARRAY_ELEMENT *array_unshift_element PARAMS((ARRAY *));
extern int	array_shift_element PARAMS((ARRAY *, char *));

extern ARRAY	*array_quote PARAMS((ARRAY *));
extern ARRAY	*array_quote_escapes PARAMS((ARRAY *));
extern ARRAY	*array_dequote PARAMS((ARRAY *));
extern ARRAY	*array_dequote_escapes PARAMS((ARRAY *));
extern ARRAY	*array_remove_quoted_nulls PARAMS((ARRAY *));

extern char	*array_subrange PARAMS((ARRAY *, arrayind_t, arrayind_t, int, int, int));
extern char	*array_patsub PARAMS((ARRAY *, char *, char *, int));
extern char	*array_modcase PARAMS((ARRAY *, char *, int, int));

/* Basic operations on array elements. */
extern ARRAY_ELEMENT *array_create_element PARAMS((arrayind_t, char *));
extern ARRAY_ELEMENT *array_copy_element PARAMS((ARRAY_ELEMENT *));
extern void	array_dispose_element PARAMS((ARRAY_ELEMENT *));

extern int	array_insert PARAMS((ARRAY *, arrayind_t, char *));
extern ARRAY_ELEMENT *array_remove PARAMS((ARRAY *, arrayind_t));
extern char	*array_reference PARAMS((ARRAY *, arrayind_t));

/* Converting to and from arrays */
extern WORD_LIST *array_to_word_list PARAMS((ARRAY *));
extern ARRAY *array_from_word_list PARAMS((WORD_LIST *));
extern WORD_LIST *array_keys_to_word_list PARAMS((ARRAY *));

extern ARRAY *array_assign_list PARAMS((ARRAY *, WORD_LIST *));

extern char **array_to_argv PARAMS((ARRAY *, int *));

extern char *array_to_kvpair PARAMS((ARRAY *, int));
extern char *array_to_assign PARAMS((ARRAY *, int));
extern char *array_to_string PARAMS((ARRAY *, char *, int));
extern ARRAY *array_from_string PARAMS((char *, char *));

/* Flags for array_shift */
#define AS_DISPOSE	0x01

#define array_num_elements(a)	((a)->num_elements)
#define array_max_index(a)	((a)->max_index)
#define array_first_index(a)	((a)->head->next->ind)
#define array_head(a)		((a)->head)
#define array_empty(a)		((a)->num_elements == 0)

#define element_value(ae)	((ae)->value)
#define element_index(ae)	((ae)->ind)
#define element_forw(ae)	((ae)->next)
#define element_back(ae)	((ae)->prev)

#define set_element_value(ae, val)	((ae)->value = (val))

/* Convenience */
#define array_push(a,v)	\
  do { array_rshift ((a), 1, (v)); } while (0)
#define array_pop(a) \
  do { array_dispose_element (array_shift ((a), 1, 0)); } while (0)

#define GET_ARRAY_FROM_VAR(n, v, a) \
  do { \
    (v) = find_variable (n); \
    (a) = ((v) && array_p ((v))) ? array_cell (v) : (ARRAY *)0; \
  } while (0)

#define ALL_ELEMENT_SUB(c)	((c) == '@' || (c) == '*')

/* In eval.c, but uses ARRAY * */
extern int execute_array_command PARAMS((ARRAY *, void *));

#endif /* _ARRAY_H_ */
