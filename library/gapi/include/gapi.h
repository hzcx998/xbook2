
#ifndef _GAPI_GAPI_H
#define _GAPI_GAPI_H

#include "gcolor.h"
#include "gshape.h"
#include "glayer.h"
#include "gmessage.h"
#include "gscreen.h"
#include "gwindow.h"
#include "gfont.h"
#include "gtouch.h"
#include "gkeycode.h"
#include "gtimer.h"
#include "gbitmap.h"
#include "gimage.h"

/**
 * g_init - init gui
 * 
 * must be called at first.
 * 
 * @return: 0 is success, -1 is failed.
 */
int g_init(void);

/**
 * g_init - quit gui
 * 
 * must be called at finaly, before exit.
 * 
 * @return: 0 is success, -1 is failed.
 */
int g_quit(void);

#endif  /* _GAPI_GAPI_H */
