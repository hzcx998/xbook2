#include  "learninggui.h"

#include  "mono_discrete_font_ascii.h"
#include  "mono_discrete_font_Chinese.h"


static  GUI_VAR_CONST  GUI_FONT  app_ascltn = 
{
    /* .type = */ MONO_DISCRETE_FONT_TYPE,
    /* .font = */ (void *)(&my_mono_discrete_ascii),
    /* .next = */ 0
};

GUI_VAR_CONST  GUI_FONT  app_font = 
{
    /* .type = */ MONO_DISCRETE_FONT_TYPE,
    /* .font = */ (void *)(&my_mono_discrete_chinese),
    /* .next = */ (GUI_FONT *)(&app_ascltn)
};
