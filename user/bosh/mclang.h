/**
 * Copyright (C), 2020, Jason Hu, www.book-os.org
 * File name: mclang.h
 * Description:
 *      This is a programming language based on the C language macro.
 *  Based on the C language, the macro function is used to redefine
 *  some keywords and syntax. This is not like the interpreter, 
 *  it will be expanded into C language code during the C macro 
 *  expansion stage, and then compiled into assembly and machine code,
 *  so its execution efficiency is the same as C language. In other words,
 *  just changed the C language, it still looks very cool!
 * 
 * Change Logs:
 *  | Date          | Author        | Notes             | Version
 *  | 2020-05-23    | Jason Hu      | First Release     | 0.0.1
 */
#ifndef __MARCO_C_LANG_H__
#define __MARCO_C_LANG_H__

#define __MARCO_C_LANG_VERSION__    "0.0.1"

/* keywords */
#define test                if(                 /* test condition */
#define again               }else if(           /* test again */
#define last                }else{              /* last to do */
#define then                ){                  /* then to do */
#define start               {                   /* start block */
#define end                 }                   /* end block */
#define loop                while(              /* loop while true */
#define ret(_value)         return (_value);    /* return a value */
#define retnil              return;             /* return nil */
#define and                 &&                  /* logical and */
#define or                  ||                  /* logical or */
#define not                 !                   /* logical not */
#define as                  =                   /* assinment */
#define coda                ;                   /* end of grammar */
#define term                break;              /* break out loop */
#define advance             continue;           /* advance processing */
#define go                  do{                 /* go to do */
#define unitl               );                  /* until with loop */
#define const               const               
#define auto                auto
#define branch              switch(             /* conditional branch */
#define jump(_label)        goto _label;        /* jump to label */
#define final               default:            /* final branch */
#define node(_case)         case (_case):       /* branch node */
#define enum                enum
#define struct              struct
#define import              extern
#define static              static
#define register            register
#define sizeof              sizeof
#define typedef             typedef
#define union               union
#define volatile            volatile
#define restrict            restrict
#define inline              inline
#ifdef _Bool
#define bool                _Bool
#define true                true
#define false               false
#else
#define bool                char
#define true                1
#define false               0
#endif
#define bitf                :                   /* bit field */
#define out                                     /* outgoing parameter */

/* integer [unsigned] integer [8, 16, 32, 64] */
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long int   u64;
typedef char                i8;
typedef short               i16;
typedef int                 i32;
typedef long int            i64;
typedef void                nil;
typedef float               f32;
typedef double              f64;
typedef long double         f128;

/* call a function */
#define call(_func, ...) \
        (_func)(__VA_ARGS__);

/* declare a function */
#define def(_ret, _func, ...) \
        _ret _func (__VA_ARGS__);
/* define a function */
#define func(_ret, _func, ...) \
        _ret _func (__VA_ARGS__){

/* get a function return value */
#define get(_func, ...) \
        (_func)(__VA_ARGS__)

/* get each item in array */
#define foreach(_item, _array) \
        int _item##_idx = 0;\
        for(_item = _array[0];\
        _item##_idx < (sizeof(_array) / sizeof(_array[0]));\
        ++_item##_idx, _item = _array[_item##_idx]

/* get each index in a number */
#define foreachi(_item, _num, _step) \
        for(_item = 0;\
        _item < _num;\
        _item += _step

/* loop in condition */
#define forcond(_init, _cond, _next) \
        for(_init; _cond; _next

/* ternary operation */
#define ternary(_cond, _x, _y) \
        _cond ? _x : _y

#endif  /* __MARCO_C_LANG_H__ */