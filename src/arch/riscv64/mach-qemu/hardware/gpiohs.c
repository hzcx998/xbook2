// GPIOHS Protocol Implementation

#include <arch/memory.h>
#include <gpiohs.h>
#include <fpioa.h>
#include <utils.h>
#include <k210_phymem.h>

#define GPIOHS_MAX_PINNO 32

volatile gpiohs_t *const gpiohs = (volatile gpiohs_t *)GPIOHS_V;

// typedef struct _gpiohs_pin_instance
// {
//     uint64 pin;
//     gpio_pin_edge_t edge;
//     void (*callback)();
//     plic_irq_callback_t gpiohs_callback;
//     void *context;
// } gpiohs_pin_instance_t;

// static gpiohs_pin_instance_t pin_instance[32];

void gpiohs_set_drive_mode(uint8 pin, gpio_drive_mode_t mode)
{
    // configASSERT(pin < GPIOHS_MAX_PINNO);
    int io_number = fpioa_get_io_by_function(FUNC_GPIOHS0 + pin);
    // configASSERT(io_number >= 0);

    fpioa_pull_t pull = FPIOA_PULL_NONE;
    uint32 dir = 0;

    switch(mode)
    {
        case GPIO_DM_INPUT:
            pull = FPIOA_PULL_NONE;
            dir = 0;
            break;
        case GPIO_DM_INPUT_PULL_DOWN:
            pull = FPIOA_PULL_DOWN;
            dir = 0;
            break;
        case GPIO_DM_INPUT_PULL_UP:
            pull = FPIOA_PULL_UP;
            dir = 0;
            break;
        case GPIO_DM_OUTPUT:
            pull = FPIOA_PULL_DOWN;
            dir = 1;
            break;
        default:
            // configASSERT(!"GPIO drive mode is not supported.") 
            break;
    }

    fpioa_set_io_pull(io_number, pull);
    volatile uint32 *reg = dir ? gpiohs->output_en.u32 : gpiohs->input_en.u32;
    volatile uint32 *reg_d = !dir ? gpiohs->output_en.u32 : gpiohs->input_en.u32;
    set_gpio_bit(reg_d, pin, 0);
    set_gpio_bit(reg, pin, 1);
}

// gpio_pin_value_t gpiohs_get_pin(uint8 pin)
// {
//     // configASSERT(pin < GPIOHS_MAX_PINNO);
//     return get_gpio_bit(gpiohs->input_val.u32, pin);
// }

void gpiohs_set_pin(uint8 pin, gpio_pin_value_t value)
{
    // configASSERT(pin < GPIOHS_MAX_PINNO);
    set_gpio_bit(gpiohs->output_val.u32, pin, value);
}

// void gpiohs_set_pin_edge(uint8 pin, gpio_pin_edge_t edge)
// {
//     set_gpio_bit(gpiohs->rise_ie.u32, pin, 0);
//     set_gpio_bit(gpiohs->rise_ip.u32, pin, 1);

//     set_gpio_bit(gpiohs->fall_ie.u32, pin, 0);
//     set_gpio_bit(gpiohs->fall_ip.u32, pin, 1);

//     set_gpio_bit(gpiohs->low_ie.u32, pin, 0);
//     set_gpio_bit(gpiohs->low_ip.u32, pin, 1);

//     set_gpio_bit(gpiohs->high_ie.u32, pin, 0);
//     set_gpio_bit(gpiohs->high_ip.u32, pin, 1);

//     if(edge & GPIO_PE_FALLING)
//     {
//         set_gpio_bit(gpiohs->fall_ie.u32, pin, 1);
//     } else
//     {
//         set_gpio_bit(gpiohs->fall_ie.u32, pin, 0);
//     }

//     if(edge & GPIO_PE_RISING)
//     {
//         set_gpio_bit(gpiohs->rise_ie.u32, pin, 1);
//     } else
//     {
//         set_gpio_bit(gpiohs->rise_ie.u32, pin, 0);
//     }

//     if(edge & GPIO_PE_LOW)
//     {
//         set_gpio_bit(gpiohs->low_ie.u32, pin, 1);
//     } else
//     {
//         set_gpio_bit(gpiohs->low_ie.u32, pin, 0);
//     }

//     if(edge & GPIO_PE_HIGH)
//     {
//         set_gpio_bit(gpiohs->high_ie.u32, pin, 1);
//     } else
//     {
//         set_gpio_bit(gpiohs->high_ie.u32, pin, 0);
//     }

//     pin_instance[pin].edge = edge;
// }

// int gpiohs_pin_onchange_isr(void *userdata)
// {
//     gpiohs_pin_instance_t *ctx = (gpiohs_pin_instance_t *)userdata;
//     uint64 pin = ctx->pin;

//     if(ctx->edge & GPIO_PE_FALLING)
//     {
//         set_gpio_bit(gpiohs->fall_ie.u32, pin, 0);
//         set_gpio_bit(gpiohs->fall_ip.u32, pin, 1);
//         set_gpio_bit(gpiohs->fall_ie.u32, pin, 1);
//     }

//     if(ctx->edge & GPIO_PE_RISING)
//     {
//         set_gpio_bit(gpiohs->rise_ie.u32, pin, 0);
//         set_gpio_bit(gpiohs->rise_ip.u32, pin, 1);
//         set_gpio_bit(gpiohs->rise_ie.u32, pin, 1);
//     }

//     if(ctx->edge & GPIO_PE_LOW)
//     {
//         set_gpio_bit(gpiohs->low_ie.u32, pin, 0);
//         set_gpio_bit(gpiohs->low_ip.u32, pin, 1);
//         set_gpio_bit(gpiohs->low_ie.u32, pin, 1);
//     }

//     if(ctx->edge & GPIO_PE_HIGH)
//     {
//         set_gpio_bit(gpiohs->high_ie.u32, pin, 0);
//         set_gpio_bit(gpiohs->high_ip.u32, pin, 1);
//         set_gpio_bit(gpiohs->high_ie.u32, pin, 1);
//     }

//     if(ctx->callback)
//         ctx->callback();
//     if(ctx->gpiohs_callback)
//         ctx->gpiohs_callback(ctx->context);

//     return 0;
// }

// void gpiohs_set_irq(uint8 pin, uint32 priority, void (*func)())
// {

//     pin_instance[pin].pin = pin;
//     pin_instance[pin].callback = func;

//     // plic_set_priority(IRQN_GPIOHS0_INTERRUPT + pin, priority);
//     // plic_irq_register(IRQN_GPIOHS0_INTERRUPT + pin, gpiohs_pin_onchange_isr, &(pin_instance[pin]));
//     // plic_irq_enable(IRQN_GPIOHS0_INTERRUPT + pin);
// }

// void gpiohs_irq_register(uint8 pin, uint32 priority, plic_irq_callback_t callback, void *ctx)
// {
//     pin_instance[pin].pin = pin;
//     pin_instance[pin].gpiohs_callback = callback;
//     pin_instance[pin].context = ctx;

//     // plic_set_priority(IRQN_GPIOHS0_INTERRUPT + pin, priority);
//     // plic_irq_register(IRQN_GPIOHS0_INTERRUPT + pin, gpiohs_pin_onchange_isr, &(pin_instance[pin]));
//     // plic_irq_enable(IRQN_GPIOHS0_INTERRUPT + pin);
// }

// void gpiohs_irq_unregister(uint8 pin)
// {
//     pin_instance[pin] = (gpiohs_pin_instance_t){
//         .callback = NULL,
//         .gpiohs_callback = NULL,
//         .context = NULL,
//     };
//     set_gpio_bit(gpiohs->rise_ie.u32, pin, 0);
//     set_gpio_bit(gpiohs->fall_ie.u32, pin, 0);
//     set_gpio_bit(gpiohs->low_ie.u32, pin, 0);
//     set_gpio_bit(gpiohs->high_ie.u32, pin, 0);
//     // plic_irq_unregister(IRQN_GPIOHS0_INTERRUPT + pin);
// }

// void gpiohs_irq_disable(uint64 pin)
// {
//     // plic_irq_disable(IRQN_GPIOHS0_INTERRUPT + pin);
// }