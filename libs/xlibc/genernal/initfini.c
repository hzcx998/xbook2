extern void _init(void);
extern void _fini(void);

void __libc_csu_init(void)
{
    /* TODO: call _init */
    // _init();
}

void __libc_csu_fini(void)
{
    /* TODO: call _fini */
    // _fini();
}