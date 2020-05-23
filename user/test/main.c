#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mclang.h"

func (i32, main, i32 argc, i8 *argv[])
    call (printf, "hello, this is test!\n")

    ret (0)
end