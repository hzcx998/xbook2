#include <termios.h>
#include <string.h>

int tcgetattr(int fildes,struct termios *termios_p)
{
    // TODO: get attr
    memset(termios_p, 0 , sizeof(*termios_p));
    return 0;
}
