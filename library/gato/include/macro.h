#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define M_PI 3.14159265358979323846
#define PI 3.14159265359f

//https://www.jianshu.com/p/5198d8aa80c1
#define KAPPA90 (0.5522847493f)
#define fclampf(v, a, b) fminf(fmaxf(a, v), b)
/*
#define min(a, b) ({typeof(a) _amin = (a); typeof(b) _bmin = (b); (void)(&_amin == &_bmin); _amin < _bmin ? _amin : _bmin; })
#define max(a, b) ({typeof(a) _amax = (a); typeof(b) _bmax = (b); (void)(&_amax == &_bmax); _amax > _bmax ? _amax : _bmax; })
*/
#ifdef __cplusplus
}
#endif