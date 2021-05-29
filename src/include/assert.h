#ifndef _XBOOK_ASEERT_H
#define _XBOOK_ASEERT_H

//断言
#define CONFIG_ASSERT 0

#if CONFIG_ASSERT == 1
void assertion_failure(char *exp, char *file, char *baseFile, int line);
#define assert(exp)  if (exp) ; \
        else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else
#define assert(exp)
#endif

#endif   /* _XBOOK_ASEERT_H */
