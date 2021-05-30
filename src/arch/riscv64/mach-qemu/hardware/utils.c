#include <stdint.h>
#include <driver/utils.h>

void set_bit(volatile uint32 *bits, uint32 mask, uint32 value)
{
    uint32 org = (*bits) & ~mask;
    *bits = org | (value & mask);
}

void set_bit_offset(volatile uint32 *bits, uint32 mask, uint64 offset, uint32 value)
{
    set_bit(bits, mask << offset, value << offset);
}

void set_gpio_bit(volatile uint32 *bits, uint64 offset, uint32 value)
{
    set_bit_offset(bits, 1, offset, value);
}

uint32 get_bit(volatile uint32 *bits, uint32 mask, uint64 offset)
{
    return ((*bits) & (mask << offset)) >> offset;
}

uint32 get_gpio_bit(volatile uint32 *bits, uint64 offset)
{
    return get_bit(bits, 1, offset);
}
