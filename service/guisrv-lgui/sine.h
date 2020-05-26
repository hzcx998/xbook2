#ifndef __TOOLS_SINE_HEADER__
#define __TOOLS_SINE_HEADER__

#ifdef  __cplusplus
extern "C"
{
#endif

    long  sine(unsigned long angle);
    #define cosine(angle)    sine((angle + 0x40000000))

#ifdef  __cplusplus
}
#endif

#endif // __TOOLS_SINE_HEADER__
