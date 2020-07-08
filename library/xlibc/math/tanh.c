/*
    File:       tanh.c

    Contains:   For tanh(x)

    Written by: GUI

    Copyright:  (C) 2017-2020 by GuEe Studio for Book OS. All rights reserved.
*/

#include <math.h>

M_FLOAT tanh(M_FLOAT x) {
    double pExponent, nExponent;
    pExponent = exp(x);
    nExponent = 1.0 / pExponent;
    return ((pExponent - nExponent) / (pExponent + nExponent));
}
