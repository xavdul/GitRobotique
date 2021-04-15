#include <stdlib.h>
#include <stdint.h>
#include <stm32f4xx.h>
#include <gpio.h>
#include <motor.h>

#define TIMER_CLOCK         84000000
#define TIMER_FREQ          100000 // [Hz]
#define PRESCALER_MOT		10
#define COUNTER_MAX_MOT     84       // timer max counter -> 100kHz

#define MOTOR_SPEED_LIMIT   13 // [cm/s]
#define NSTEP_ONE_TURN      1000 // number of step for 1 turn of the motor
#define NSTEP_ONE_EL_TURN   4  //number of steps to do 1 electrical turn
#define NB_OF_PHASES        4  //number of phases of the motors
#define WHEEL_PERIMETER     13 // [cm]

//timers to use for the motors
#define MOTOR_RIGHT_TIMER       TIM6
#define MOTOR_RIGHT_TIMER_EN    RCC_APB1ENR_TIM6EN
#define MOTOR_RIGHT_IRQHandler  TIM6_DAC_IRQHandler
#define MOTOR_RIGHT_IRQ         TIM6_DAC_IRQn

#define MOTOR_LEFT_TIMER        TIM7
#define MOTOR_LEFT_TIMER_EN     RCC_APB1ENR_TIM7EN
#define MOTOR_LEFT_IRQ          TIM7_IRQn
#define MOTOR_LEFT_IRQHandler   TIM7_IRQHandler

/*
*
*   TO COMPLETE
*   Complete the right GPIO port and pin to be able to control the motors
*/
#define MOTOR_RIGHT_A	GPIOE, 13
#define MOTOR_RIGHT_B	GPIOE, 12
#define MOTOR_RIGHT_C	GPIOE, 14
#define MOTOR_RIGHT_D	GPIOE, 15

#define MOTOR_LEFT_A	GPIOE, 9
#define MOTOR_LEFT_B	GPIOE, 8
#define MOTOR_LEFT_C	GPIOE, 11
#define MOTOR_LEFT_D	GPIOE, 10

int i, j = 0;


/*
*
*   TO COMPLETE
*   step_halt is an array contaning 4 elements describing the state when the motors are off.
*   step_table is an array of 4 lines of 4 elements. Each line describes a step.
*/
static const uint8_t step_halt[NB_OF_PHASES] = {0, 0, 0, 0};
static const uint8_t step_table[NSTEP_ONE_EL_TURN][NB_OF_PHASES] = {
    {1, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 1, 0, 1},
    {1, 0, 0, 1},
};

/*
*
*   Hint :
*   You can declare here static variables which can be used to store the steps counter of the motors
*   for example. They will be available only for the code of this file.
*/

/*
*
*   TO COMPLETE
*
*   Performs the init of the timers and of the gpios used to control the motors
*/
void motor_init(void)
{
	RCC-> AHB1ENR	|= RCC_AHB1ENR_GPIOEEN;
	RCC-> APB1ENR 	|= MOTOR_LEFT_TIMER_EN;
	RCC-> APB1ENR   |= MOTOR_RIGHT_TIMER_EN;


	gpio_config_output_pushpull(MOTOR_RIGHT_A);
	gpio_config_output_pushpull(MOTOR_RIGHT_B);
	gpio_config_output_pushpull(MOTOR_RIGHT_C);
	gpio_config_output_pushpull(MOTOR_RIGHT_D);

	gpio_config_output_pushpull(MOTOR_LEFT_A);
	gpio_config_output_pushpull(MOTOR_LEFT_B);
	gpio_config_output_pushpull(MOTOR_LEFT_C);
	gpio_config_output_pushpull(MOTOR_LEFT_D);

	//initialize motors to 0 0 0 0
	gpio_clear(MOTOR_LEFT_A);
	gpio_clear(MOTOR_LEFT_B);
	gpio_clear(MOTOR_LEFT_C);
	gpio_clear(MOTOR_LEFT_D);

	gpio_clear(MOTOR_RIGHT_A);
	gpio_clear(MOTOR_RIGHT_B);
	gpio_clear(MOTOR_RIGHT_C);
	gpio_clear(MOTOR_RIGHT_D);

    // Enable TIM6 interrupt vector
    NVIC_EnableIRQ(TIM6_DAC_IRQn);

    // Configure TIM6
    TIM6->PSC = PRESCALER_MOT - 1;      // Note: final timer clock  = timer clock / (prescaler + 1)
    TIM6->ARR = COUNTER_MAX_MOT;    // Note: timer reload takes 1 cycle, thus -1
    TIM6->DIER |= TIM_DIER_UIE;          // Enable update interrupt
    TIM6->CR1 |= TIM_CR1_CEN;            // Enable timer

    // Enable TIM7 interrupt vector
    NVIC_EnableIRQ(TIM7_IRQn);

    // Configure TIM7
    TIM7->PSC = PRESCALER_MOT - 1;      // Note: final timer clock  = timer clock / (prescaler + 1)
    TIM7->ARR = COUNTER_MAX_MOT;    // Note: timer reload takes 1 cycle, thus -1
    TIM7->DIER |= TIM_DIER_UIE;          // Enable update interrupt
    TIM7->CR1 |= TIM_CR1_CEN;            // Enable timer

}

/*
*
*   TO COMPLETE
*
*   Updates the state of the gpios of the right motor given an array of 4 elements
*   describing the state. For example step_table[0] which gives the first step.
*/
static void right_motor_update(const uint8_t *out)
{
	if(out[0])
		gpio_set(MOTOR_RIGHT_A);
	else
		gpio_clear(MOTOR_RIGHT_A);
	if(out[1])
		gpio_set(MOTOR_RIGHT_B);
	else
		gpio_clear(MOTOR_RIGHT_B);
	if(out[2])
		gpio_set(MOTOR_RIGHT_C);
	else
		gpio_clear(MOTOR_RIGHT_C);
	if(out[3])
		gpio_set(MOTOR_RIGHT_D);
	else
		gpio_clear(MOTOR_RIGHT_D);
}


/*
*
*   TO COMPLETE
*
*   Updates the state of the gpios of the left motor given an array of 4 elements
*   describing the state. For exeample step_table[0] which gives the first step.
*/
static void left_motor_update(const uint8_t *out)
{
	if(out[0])
		gpio_set(MOTOR_LEFT_A);
	else
		gpio_clear(MOTOR_LEFT_A);
	if(out[1])
		gpio_set(MOTOR_LEFT_B);
	else
		gpio_clear(MOTOR_LEFT_B);
	if(out[2])
		gpio_set(MOTOR_LEFT_C);
	else
		gpio_clear(MOTOR_LEFT_C);
	if(out[3])
		gpio_set(MOTOR_LEFT_D);
	else
		gpio_clear(MOTOR_LEFT_D);
}

/*
*
*   TO COMPLETE
*
*   Stops the motors (all the gpio must be clear to 0) and set 0 to the ARR register of the timers to prevent
*   the interrupts of the timers (because it never reaches 0 after an increment)
*/
void motor_stop(void)
{

}

/*
*
*   TO COMPLETE
*
*   Sets the position to reach for each motor.
*   The parameters are in cm for the positions and in cm/s for the speeds.
*/
void motor_set_position(float position_r, float position_l, float speed_r, float speed_l)
{

}

/*
*
*   TO COMPLETE
*
*   Sets the speed of the motors.
*   The parameters are in cm/s for the speed.
*   To set the speed, you need to change the ARR value of the timers.
*   Remember : the timers generate an interrupt when they reach the value of ARR.
*   Don't forget to convert properly the units in order to have the correct ARR value
*   depending on the TIMER_FREQ and the speed chosen.
*/
void motor_set_speed(float speed_r, float speed_l)
{

}

/*
*
*   TO COMPLETE
*
*   Interrupt of the timer of the right motor.
*   Performs a step of the motor and stops it if it reaches the position given in motor_set_position().
*/
void MOTOR_RIGHT_IRQHandler(void)
{
	/*
    *
    *   BEWARE !!
    *   Based on STM32F40x and STM32F41x Errata sheet - 2.1.13 Delay after an RCC peripheral clock enabling
    *
    *   As there can be a delay between the instruction of clearing of the IF (Interrupt Flag) of corresponding register (named here CR) and
    *   the effective peripheral IF clearing bit there is a risk to enter again in the interrupt if the clearing is done at the end of ISR.
    *
    *   As tested, only the workaround 3 is working well, then read back of CR must be done before leaving the ISR
    *
    */

	/* do something ... */

	right_motor_update(step_table[i]);
	i++;
	if(i==4)
		i = 0;

	// Clear interrupt flag
	MOTOR_RIGHT_TIMER->SR &= ~TIM_SR_UIF;
	MOTOR_RIGHT_TIMER->SR;	// Read back in order to ensure the effective IF clearing
}

/*
*
*   TO COMPLETE
*
*   Interrupt of the timer of the left motor.
*   Performs a step of the motor and stops it if it reaches the position given in motor_set_position().
*/
void MOTOR_LEFT_IRQHandler(void)
{
	/*
    *
    *   BEWARE !!
    *   Based on STM32F40x and STM32F41x Errata sheet - 2.1.13 Delay after an RCC peripheral clock enabling
    *
    *   As there can be a delay between the instruction of clearing of the IF (Interrupt Flag) of corresponding register (named here CR) and
    *   the effective peripheral IF clearing bit there is a risk to enter again in the interrupt if the clearing is done at the end of ISR.
    *
    *   As tested, only the workaround 3 is working well, then read back of CR must be done before leaving the ISR
    *
    */

	/* do something ... */

	left_motor_update(step_table[j]);
	j++;
	if(j==4)
		j = 0;
	// Clear interrupt flag
    MOTOR_LEFT_TIMER->SR &= ~TIM_SR_UIF;
    MOTOR_LEFT_TIMER->SR;	// Read back in order to ensure the effective IF clearing
}

