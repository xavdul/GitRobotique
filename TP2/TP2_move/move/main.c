#include <stm32f4xx.h>
#include <system_clock_config.h>
#include <gpio.h>
#include <main.h>
#include <timer.h>
#include <motor.h>
#include <selector.h>

#define PI                  3.1415926536f
//TO ADJUST IF NECESSARY. NOT ALL THE E-PUCK2 HAVE EXACTLY THE SAME WHEEL DISTANCE
#define WHEEL_DISTANCE      5.35f    //cm
#define PERIMETER_EPUCK     (PI * WHEEL_DISTANCE)

// Init function required by __libc_init_array
void _init(void) {}

// Simple delay function
void delay(unsigned int n)
{
    while (n--) {
        __asm__ volatile ("nop");
    }
}


int main(void)
{
    SystemClock_Config();
    // Enable GPIOD and GPIOB peripheral clock
//    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOBEN;
//    RCC-> AHB1ENR	|= RCC_AHB1ENR_GPIODEN;
    RCC-> AHB1ENR	|= RCC_AHB1ENR_GPIOEEN;


    //gpio_config_output_pushpull(FRONT_LED);
   // init_selector();

  //  timer4_start();

    //gpio_set(FRONT_LED);

  //  GPIOD->MODER = 2 << 28;

  //  GPIOD->AFR[1] = (GPIOD->AFR[1] & (15 << 24)) | (2 << 24);

   // TIM4->CCMR2		|= (6 << 4);
    //Enable OC3 output
  //  TIM4->CCER 		|= (1 << 8);
    // Output active high
  //  TIM4->CCER		|= (0 << 9);

    motor_init();



    while (1) {
    	//TIM4->CCR3 = get_selector()*6;
    }
}

