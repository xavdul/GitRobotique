/*
 * movement.c
 *
 *  Created on: 29 avr. 2021
 *      Author: xavierdulex
 *
 *  Description :
 *
 */

#include <sensors/proximity.h>
#include <sensors/VL53L0X/VL53L0X.h>

#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <selector.h>
#include <process_image.h>
#include <movement.h>


#include <main.h>
#include <motors.h>

#include <leds.h>
#include <spi_comm.h>


//proximity sensors
#define LEFT_IR 5
#define RIGHT_IR 2
#define DIAG_LEFT_IR 6
#define DIAG_RIGHT_IR 1
#define FRONT_LEFT_IR 7
#define FRONT_RIGHT_IR 0

//thresholds and distances for the obstacle detection
#define TH_PROX 500				//proximity threshold for the lateral obstacles
#define MIN_DIST_FRONT 50		//TOF detection distance [mm] for the frontal obstacles
#define DIST_DETECTION 30		//TOF threshold distance [mm] to prepare for passing trough the "color wall"
#define MIN_MAUVAISE_COULEUR 60 //TOF detection distance [mm] to turn if the detected color is different as the selected one

//steps number for turns
#define STEPS_NINETY 270		//number of steps to turn for a 90° turn
#define STEPS_RED 480			//number of steps for the red start turn
#define STEPS_BLUE 400			//number of steps for the blue start turn
#define STEPS_GREEN 550			//number of steps for the green start turn

//speeds
#define SPEED_TURN 700			//turning speed for 90° turns
#define SPEED_CORR 300			//speed correction for lateral obstacle avoidance
#define SPEED_END  210			//rotating speed for the color detection in FINISH mode
#define STOP       0			//stopped speed

//distances
#define DIST_OBSTACLE 10000000  //distance needed to pass through an obstacle

//condition variables
static uint8_t no_more_color_needed = 0; //tells if a color detection is needed or not
static uint8_t find_the_end = 0;		 //tells if we are in the end of the labyrinth or not

// movement thread

static THD_WORKING_AREA(wamovement, 512);
static THD_FUNCTION(movement, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    volatile long i;


    while(1){

    	time = chVTGetSystemTime();

    	if(get_mode() == MOVEMENT){									//trajectory control for movement mode
    		if(get_color_detected() == WHITE){						//the obstacle in front is a wall (WHITE)
    			if(VL53L0X_get_dist_mm() < MIN_DIST_FRONT){			//front obstacle avoidance
    				left_turn();
    				while(right_motor_get_pos() != STEPS_NINETY){}
    			}
    			proximity();										//side obstacles avoidance
    		}
    		else{													//the obstacle in front is of a color :
    			if(get_color_detected() == get_color_selected()){	//color of the obstacle is the one selected -> pass trough the obstacle
    				if(VL53L0X_get_dist_mm() > DIST_DETECTION){		//while at distance from the obstacle, avoid side obstacles
    					proximity();
    				}
    				else{											//when close to the obstacle :
    					no_more_color_needed = 1;					//deactivate color detection to avoid errors while passing through
    					left_motor_set_speed(SPEED_MOVE);
    					right_motor_set_speed(SPEED_MOVE);
    					for(i =0; i < DIST_OBSTACLE; i++){}			//force forward move during DIST_OBSTACLE while passing through
    					no_more_color_needed = 0;					//reactivate color detection
    				}
    			}
    			else{
    				if(VL53L0X_get_dist_mm() > MIN_MAUVAISE_COULEUR){ //color of the obstacle in front is not the one selected :
    					proximity();								  //avoid side obstacle while far from the object
    				}
    				else{											  //when close enough to the object, take the other route by turning left
    					left_turn();
    					while(right_motor_get_pos() != STEPS_NINETY){}
    				}
    			}
    		}
    	}
    	else if(get_mode() == START){								//trajectory control for START mode
    		if(get_color_selected() == RED){
    			left_turn();
    			while(right_motor_get_pos() != STEPS_RED){}			//turn away from RED start position

    		}
    		else if(get_color_selected() == BLUE){
    			left_turn();
    			while(right_motor_get_pos() != STEPS_BLUE){}		//turn away from BLUE start position
    		}
    		else if(get_color_selected() == GREEN){
    			left_turn();
    			while(right_motor_get_pos() != STEPS_GREEN){}		//turn away from GREEN start position
    		}
    		next_mode();											//pass to MOVEMENT mode to move and avoid obstacles
    	}
    	else if(get_mode() == FINISH){								//trajectory control for FINISH mode
    		if(VL53L0X_get_dist_mm() > MIN_DIST_FRONT && find_the_end == 0){
    			proximity();										//avoid side obstacle if not in the end zone
    		}
    		else{
    			left_motor_set_speed(STOP);							//if in the end zone
    			right_motor_set_speed(STOP);
    			find_the_end = 1;
    			if(get_color_detected() == get_color_selected()){	//illuminate body led if end color is found in end zone
    				find_the_end = 0;
    				set_body_led(1);
    				next_mode();
    			}
    			else{												//turn to find the end color matching to the selected color
    				right_motor_set_pos(0);
    				left_motor_set_pos(0);
    				left_motor_set_speed(-SPEED_END);
    				right_motor_set_speed(SPEED_END);
    				while(right_motor_get_pos() != STEPS_NINETY){}
    			}
    		}
    	}
    	else{														//stop if in other modes
    		left_motor_set_speed(STOP);
    		right_motor_set_speed(STOP);
    	}

    	//check if we can pass to FINISH mode
    	if(get_color_detected() == GREEN && (get_mode() == MOVEMENT)){
    		next_mode();
		}
    	chThdSleepUntilWindowed(time, time + MS2ST(10)); // 100 Hz
    }

}

uint8_t get_no_more_color_needed(void){
	return no_more_color_needed;
}

void proximity(void){
	if(get_prox(DIAG_LEFT_IR) > TH_PROX					//if there is an obstacle on the left
			|| get_prox(FRONT_LEFT_IR) > TH_PROX){

		left_motor_set_speed(SPEED_MOVE + SPEED_CORR);	//trajectory correction to the right
		right_motor_set_speed(SPEED_MOVE - SPEED_CORR);
	}
	else if(get_prox(DIAG_RIGHT_IR) > TH_PROX			//if there is an obstacle on the right
			|| get_prox(FRONT_RIGHT_IR) > TH_PROX){

		left_motor_set_speed(SPEED_MOVE - SPEED_CORR);	//trajectory correction to the left
		right_motor_set_speed(SPEED_MOVE + SPEED_CORR);
	}
	else{
		left_motor_set_speed(SPEED_MOVE);				//if no obstacle, no correction
		right_motor_set_speed(SPEED_MOVE);
	}
}

void movement_start(void){
	chThdCreateStatic(wamovement, sizeof(wamovement), NORMALPRIO, movement, NULL);
}



