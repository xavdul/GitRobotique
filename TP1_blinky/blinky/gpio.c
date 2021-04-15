#include <stm32f407xx.h>
#include <gpio.h>
#include <main.h>

void gpio_config_output_opendrain(GPIO_TypeDef *port, unsigned int pin, int on)
{
	if(on){
		// Output type open-drain : OTy = 1
		    port->OTYPER |= (1 << pin);
	}
	else {
		// Output type push-pull : OTy = 1
		    port->OTYPER |= ~(1 << pin);
	}


    // Output data low : ODRy = 0
    port->ODR &= ~(1 << pin);

    // Floating, no pull-up/down : PUPDRy = 00
    port->PUPDR &= ~(3 << (pin * 2));

    // Output speed highest : OSPEEDRy = 11
    port->OSPEEDR |= (3 << (pin * 2));

    // Output mode : MODERy = 01
    port->MODER = (port->MODER & ~(3 << (pin * 2))) | (1 << (pin * 2));
}

void gpio_set(GPIO_TypeDef *port, unsigned int pin)
{
    port->BSRR = (1 << pin);
}

void gpio_clear(GPIO_TypeDef *port, unsigned int pin)
{
    port->BSRR = (1 << (pin + 16));
}

void gpio_toggle(GPIO_TypeDef *port, unsigned int pin)
{
    if (port->ODR & (1<<pin)) {
        gpio_clear(port, pin);
    } else {
        gpio_set(port, pin);
    }
}

void gpio_config_input(GPIO_TypeDef *port, unsigned int pin)
{
	// Input mode : MODERy = 00
	    port->MODER = (port->MODER & ~(3 << (pin * 2)));

	// Pull-down : PUPDRy = 10
	    port->PUPDR = (port->PUPDR & ~(3 << (pin * 2))) | (2 << (pin * 2));
}
