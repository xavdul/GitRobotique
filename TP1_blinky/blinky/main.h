#ifndef MAIN_H
#define MAIN_H

/* LEDs that can be used in this TP1
  LED1 			: GPIOD pin 5
  LED3 			: GPIOD pin 6
  LED5 			: GPIOD pin 10
  LED7 			: GPIOD pin 11
  FRONT_LED 	: GPIOD pin 14
WARNING : Not on the same port !!
  BODY_LED		: GPIOB pin 2
*/
#define LED1     	GPIOD, 5
#define LED3     	GPIOD, 6
#define LED5     	GPIOD, 10
#define LED7     	GPIOD, 11
#define FRONT_LED	GPIOD, 14
#define BODY_LED	GPIOB, 2
#define SEL0		GPIOC, 13
#define SEL1		GPIOC, 14
#define SEL2		GPIOC, 15
#define SEL3		GPIOD, 4

#define LED_USED1	LED1
#define LED_USED2	LED3
#define LED_USED3	LED5
#define LED_USED4	LED7
#define LED_USED5	BODY_LED
#define LED_USED6	FRONT_LED




void waitzeub(void);
void read_port(void);

#endif /* MAIN_H_ */
