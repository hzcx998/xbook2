#include <stddef.h>

static void static_init_tls(size_t *aux)
{

}

weak_alias(static_init_tls, __init_tls);