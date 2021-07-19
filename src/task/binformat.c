#include <xbook/binformat.h>
#include <xbook/task.h>
#include <stddef.h>

/**
 * @brief 将字符串数组拷贝进用户栈(sp), 并将字符串
 * 地址按照原始顺序存放在ustack中.
 *
 * @note 通常这用于拷贝用户参数和环境变量
 *
 * @return int 字符串的数量
 */
int bin_program_copy_string2stack(bin_program_t *bin, char *strs[])
{
    int i = bin->stack_top;
    for (; strs[i]; i++) {
        if (i > MAX_TASK_STACK_ARG_NR)
            return -1;
        bin->sp -= strlen(strs[i]) + 1;
        bin->sp -= bin->sp % 16;
        if (bin->sp < bin->stackbase) {
            return -1;
        }
        if (page_copy_out(bin->pagetable, bin->sp, strs[i], strlen(strs[i]) + 1) < 0)
            return -1;
        bin->ustack[i] = bin->sp;
    }
    bin->ustack[i] = 0;
    int c = i - bin->stack_top;
    bin->stack_top = i + 1;
    return c;
}

uint64_t bin_program_copy_string(bin_program_t *bin, const char *s)
{
    bin->sp -= strlen(s) + 1;
    bin->sp -= bin->sp % 16;
    if (page_copy_out(bin->pagetable, bin->sp, (char *)s, strlen(s) + 1) < 0)
        return -1;
    return bin->sp;
}

/**
 * @brief 将一个8字节的值推入栈中
 *
 * @return uint64_t
 */
uint64_t bin_program_push_stack(uint64_t val)
{
    return 0;
}

void bin_program_init(bin_program_t *bin)
{
    bin->sp = 0;
    bin->stackbase = 0;
    bin->pagetable = NULL;
    bin->filename = NULL;
    bin->argc = 0;
    bin->envc = 0;
    bin->ustack = NULL;
    bin->stack_top = 0;
}