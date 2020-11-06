#include <xbook/trigger.h>

int trigaddset(trigset_t *set,int trig)
{
    if (TRIGGER_IS_BAD(trig))
        return -1;
    *set |= (1 << trig);
    return 0;
}

int trigdelset(trigset_t *set,int trig)
{
    if (TRIGGER_IS_BAD(trig))
        return -1;
    *set &= ~(1 << trig);
    return 0;
}

int trigemptyset(trigset_t *set)
{
    *set = 1; 
    return 0;
}

int trigfillset(trigset_t *set)
{
    *set = 0xffffffffUL;
    return 0;
}

int trigismember(trigset_t *set,int trig)
{
    if (TRIGGER_IS_BAD(trig))
        return 0;
    return (*set & (1 << trig));
}

int trigorset(trigset_t *set, trigset_t *setb)
{
    *set |= *setb;
    return 0;
}

int trigisempty(trigset_t *set)
{
    if (*set > 1) {
        return 0;
    } else {
        return 1;
    }
}

int trigisfull(trigset_t *set)
{
    if (*set == 0xffffffffUL) {
        return 1;
    } else {
        return 0;
    }
}

int trigmask(int trig)
{
    if (TRIGGER_IS_BAD(trig))
        return -1;
    return (1 << trig);
}
