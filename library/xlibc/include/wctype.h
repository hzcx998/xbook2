
#ifndef _LIB_WCTYPE_H
#define _LIB_WCTYPE_H
#ifdef __cplusplus
extern "C" {
#endif

typedef short wint_t;
typedef int wctype_t;
typedef int wctrans_t;

int iswalnum(wint_t c);
int iswalpha(wint_t c);
int iswcntrl(wint_t c);
int iswctype(wint_t c, wctype_t category);
int iswdigit(wint_t c);
int iswgraph(wint_t c);
int iswlower(wint_t c);
int iswprint(wint_t c);
int iswpunct(wint_t c);
int iswspace(wint_t c);
int iswupper(wint_t c);
int iswxdigit(wint_t c);
wint_t towctrans(wint_t c, wctrans_t category);
wint_t towlower(wint_t c);
wint_t towupper(wint_t c);
wctrans_t wctrans(const char *property);
wctype_t wctype(const char *property);

#ifdef __cplusplus
}
#endif

#endif  /* _LIB_WCTYPE_H */
