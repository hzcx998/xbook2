#include <uview.h>

int uview_keypad2ascii(int code)
{
    if (code >= UVIEW_KEY_KP0 && code <= UVIEW_KEY_KP_PLUS) {
        switch (code) {
        case UVIEW_KEY_KP0:
            return '0';
        case UVIEW_KEY_KP1:
            return '1';
        case UVIEW_KEY_KP2:
            return '2';            
        case UVIEW_KEY_KP3:
            return '3';
        case UVIEW_KEY_KP4:
            return '4';
        case UVIEW_KEY_KP5:
            return '5';            
        case UVIEW_KEY_KP6:
            return '6';
        case UVIEW_KEY_KP7:
            return '7';
        case UVIEW_KEY_KP8:
            return '8';            
        case UVIEW_KEY_KP9:
            return '9';
        case UVIEW_KEY_KP_PERIOD:
            return '.';
        case UVIEW_KEY_KP_DIVIDE:
            return '/';            
        case UVIEW_KEY_KP_MULTIPLY:
            return '*';
        case UVIEW_KEY_KP_MINUS:
            return '-';
        case UVIEW_KEY_KP_PLUS:
            return '+';
        default:
            break;
        }
    }
    return code;
}