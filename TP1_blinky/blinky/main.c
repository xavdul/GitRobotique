#include <stm32f4xx.h>
#include <system_clock_config.h>
#include <gpio.h>
#include <main.h>
#include <timer.h>


// Init function required by __libc_init_array
void _init(void) {}



int main(void)
{
    SystemClock_Config();

    // Enable GPIOD peripheral clock
    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIODEN;

    RCC->AHB1ENR	|= RCC_AHB1ENR_GPIOBEN;

    RCC->AHB1ENR	|= RCC_AHB1ENR_GPIOCEN;

    // LED used init

    gpio_config_output_opendrain(LED_USED1, 1);
    gpio_config_output_opendrain(LED_USED2, 1);
    gpio_config_output_opendrain(LED_USED3, 1);
    gpio_config_output_opendrain(LED_USED4, 1);
    gpio_config_output_opendrain(LED_USED5, 0);
    gpio_config_output_opendrain(LED_USED6, 0);

    gpio_set(LED_USED1);
    gpio_set(LED_USED2);
    gpio_set(LED_USED3);
    gpio_set(LED_USED4);
    gpio_clear(LED_USED5);
    gpio_set(LED_USED6);

    timer7_start();




    while (1) {
//    	waitzeub();
//    	gpio_toggle(LED_USED1);
//    	waitzeub();
//    	gpio_toggle(LED_USED2);
//    	waitzeub();
//    	gpio_toggle(LED_USED3);
//    	waitzeub();
//    	gpio_toggle(LED_USED4);
//    	waitzeub();
//    	gpio_toggle(LED_USED5);
//    	waitzeub();
//    	gpio_toggle(LED_USED6);
//    	read_port();
    }
}


void waitzeub(){
	for(int i = 0; i < 1680000; i++) {
		asm("nop");
	}
}

void read_port(){
	// Configuring ports as inputs for reading
    gpio_config_input(SEL0);
    gpio_config_input(SEL1);
    gpio_config_input(SEL2);
    gpio_config_input(SEL3);

    // Extract values from IDR register
	int val0 = (GPIOC->IDR & (1 << 13))>>13;
	int val1 = (GPIOC->IDR & (1 << 14))>>13;
	int val2 = (GPIOC->IDR & (1 << 15))>>13;
	int val3 = (GPIOD->IDR & (1 << 4)) >> 1;

	int val = val0+val1+val2+val3;

	if(val == 1){
		gpio_toggle(LED_USED1);
	}
	else if(val == 2){
		gpio_toggle(LED_USED2);
	}
	else if(val == 3){
		gpio_toggle(LED_USED3);
	}
	else if(val == 4){
		gpio_toggle(LED_USED4);
	}
	else if(val == 5){
		gpio_toggle(LED_USED5);
	}
	else if (val == 6){
		gpio_toggle(LED_USED6);
		}


	// Configuring ports as outputs for led use
	gpio_config_output_opendrain(LED_USED1, 1);
	gpio_config_output_opendrain(LED_USED2, 1);
	gpio_config_output_opendrain(LED_USED3, 1);
	gpio_config_output_opendrain(LED_USED4, 1);
	gpio_config_output_opendrain(LED_USED5, 0);
	gpio_config_output_opendrain(LED_USED6, 0);



}
