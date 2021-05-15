/*
 * movement.h
 *
 *  Created on: 29 avr. 2021
 *      Author: xavierdulex
 */


#define SPEED_MOVE 500			//moving speed (used also in motors.c)


#ifndef MOVEMENT_H_
#define MOVEMENT_H_


//creating the movement thread
void movement_start(void);

uint8_t get_no_more_color_needed(void);

void proximity(void);

#endif /* MOVEMENT_H_ */
