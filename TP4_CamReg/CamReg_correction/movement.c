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


#include <main.h>
#include <motors.h>

#include <leds.h>
#include <spi_comm.h>

//---------------------//
//proximity sensors

#define LEFT_IR 5
#define RIGHT_IR 2
#define DIAG_LEFT_IR 6
#define DIAG_RIGHT_IR 1
#define FRONT_LEFT_IR 7
#define FRONT_RIGHT_IR 0

#define TH_PROX 450
#define TH_PROX_COLOR 550
#define MIN_DIST_FRONT 10
#define DIST_DETECTION 30
#define MIN_MAUVAISE_COULEUR 60
#define STEPS_90 270

//---------------------//

//TP2
#define PI                  3.1415926536f
//TO ADJUST IF NECESSARY. NOT ALL THE E-PUCK2 HAVE EXACTLY THE SAME WHEEL DISTANCE
#define WHEEL_DISTANCE      5.35f    //cm
#define PERIMETER_EPUCK     (PI * WHEEL_DISTANCE)
//TP2
//Convert angle to wheel angle
#define ANGLE(angle) (PERIMETER_EPUCK*angle)/360

//---------------------//


#define SPEED_MOVE_TURN 700
#define SPEED_TURN 700
#define SPEED_CORR 300
#define STOP 0

static uint8_t no_more_color_needed = 0;
static uint8_t find_the_end = 0;

// movement thread

static THD_WORKING_AREA(wamovement, 512);
static THD_FUNCTION(movement, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    volatile long i;


    while(1){

    	time = chVTGetSystemTime();

    	// uint16_t distance = VL53L0X_get_dist_mm();


    	if(get_mode() == MOVEMENT){
    		chprintf((BaseSequentialStream *)&SD3, "MOVEMENT MOVEMENT \r\n");
    		if(get_color_detected() == 0){
    			if(VL53L0X_get_dist_mm() < MIN_DIST_FRONT){ //40 ou 50 (VOIR AVEC DEBUT)
    				right_motor_set_pos(0);
    				left_motor_set_pos(0);
    				left_motor_set_speed(-SPEED_MOVE);
    				right_motor_set_speed(SPEED_MOVE);
    				while(right_motor_get_pos() != STEPS_90){}
    			}
    			else if(get_prox(DIAG_LEFT_IR) > TH_PROX
    					|| get_prox(FRONT_LEFT_IR) > TH_PROX){

    				left_motor_set_speed(SPEED_MOVE + SPEED_CORR);
    				right_motor_set_speed(SPEED_MOVE - SPEED_CORR);
    			}
    			else if(get_prox(DIAG_RIGHT_IR) > TH_PROX
    					|| get_prox(FRONT_RIGHT_IR) > TH_PROX){

    				left_motor_set_speed(SPEED_MOVE - SPEED_CORR);
    				right_motor_set_speed(SPEED_MOVE + SPEED_CORR);
    			}
    			else{
    				left_motor_set_speed(SPEED_MOVE);
    				right_motor_set_speed(SPEED_MOVE);
    			}
    		}
    		else{
    			if(get_color_detected() == get_color_selected()){
    				if(VL53L0X_get_dist_mm() > DIST_DETECTION){
    					if(get_prox(DIAG_LEFT_IR) > TH_PROX_COLOR
    							|| get_prox(FRONT_LEFT_IR) > TH_PROX_COLOR){

    						left_motor_set_speed(SPEED_MOVE + SPEED_CORR);
    						right_motor_set_speed(SPEED_MOVE - SPEED_CORR);
    					}
    					else if(get_prox(DIAG_RIGHT_IR) > TH_PROX
    							|| get_prox(FRONT_RIGHT_IR) > TH_PROX){

    						left_motor_set_speed(SPEED_MOVE - SPEED_CORR);
    						right_motor_set_speed(SPEED_MOVE + SPEED_CORR);
    					}
    					else{
    						left_motor_set_speed(SPEED_MOVE);
    						right_motor_set_speed(SPEED_MOVE);
    					}
    				}
    				else{
    					no_more_color_needed = 1;
    					left_motor_set_speed(SPEED_MOVE);
    					right_motor_set_speed(SPEED_MOVE);
    					for(i =0; i < 10000000; i++){
    					}
    					no_more_color_needed = 0;
    				}
    			}
    			else{
    				if(VL53L0X_get_dist_mm() > MIN_MAUVAISE_COULEUR){
    					if(get_prox(DIAG_LEFT_IR) > TH_PROX
    							|| get_prox(FRONT_LEFT_IR) > TH_PROX){
    						left_motor_set_speed(SPEED_MOVE + SPEED_CORR);
    						right_motor_set_speed(SPEED_MOVE - SPEED_CORR);
    					}
    					else if(get_prox(DIAG_RIGHT_IR) > TH_PROX
    							|| get_prox(FRONT_RIGHT_IR) > TH_PROX){

    						left_motor_set_speed(SPEED_MOVE - SPEED_CORR);
    						right_motor_set_speed(SPEED_MOVE + SPEED_CORR);
    					}
    					else{
    						left_motor_set_speed(SPEED_MOVE);
    						right_motor_set_speed(SPEED_MOVE);
    					}
    				}
    				else{
    					right_motor_set_pos(0);
    					left_motor_set_pos(0);
    					left_motor_set_speed(-SPEED_MOVE);
    					right_motor_set_speed(SPEED_MOVE);
    					while(right_motor_get_pos() != STEPS_90){}
    				}
    			}
    		}
    	}
    	else if(get_mode() == START){
    		chprintf((BaseSequentialStream *)&SD3, "MOVEMENT START \r\n");
    		if(get_color_selected() == 1){
    			right_motor_set_pos(0);
    			left_motor_set_pos(0);
    			left_motor_set_speed(-SPEED_MOVE);
    			right_motor_set_speed(SPEED_MOVE);
    			while(right_motor_get_pos() != 480){}
    			next_mode();
    		}
    		else if(get_color_selected() == 2){
    			right_motor_set_pos(0);
    			left_motor_set_pos(0);
    			left_motor_set_speed(-SPEED_MOVE);
    			right_motor_set_speed(SPEED_MOVE);
    			while(right_motor_get_pos() != 400){}
    			next_mode();
    		}
    		else if(get_color_selected() == 3){
    			right_motor_set_pos(0);
    			left_motor_set_pos(0);
    			left_motor_set_speed(-SPEED_MOVE);
    			right_motor_set_speed(SPEED_MOVE);
    			while(right_motor_get_pos() != 550){}
    			next_mode();
    		}
    	}
    	else if(get_mode() == FINISH){
    		if(VL53L0X_get_dist_mm() > 10 && find_the_end == 0){
    			if(get_prox(DIAG_LEFT_IR) > TH_PROX
    					|| get_prox(FRONT_LEFT_IR) > TH_PROX){

    				left_motor_set_speed(SPEED_MOVE + SPEED_CORR);
    				right_motor_set_speed(SPEED_MOVE - SPEED_CORR);
    			}
    			else if(get_prox(DIAG_RIGHT_IR) > TH_PROX
    					|| get_prox(FRONT_RIGHT_IR) > TH_PROX){

    				left_motor_set_speed(SPEED_MOVE - SPEED_CORR);
    				right_motor_set_speed(SPEED_MOVE + SPEED_CORR);
    			}
    			else{
    				left_motor_set_speed(SPEED_MOVE);
    				right_motor_set_speed(SPEED_MOVE);
    			}
    		}
    		else{
    			left_motor_set_speed(STOP);
    			right_motor_set_speed(STOP);
    			find_the_end = 1;
    			if(get_color_detected() == get_color_selected()){
    				find_the_end = 0;
    				set_body_led(1);
    				next_mode();
    			}
    			else{
    				right_motor_set_pos(0);
    				left_motor_set_pos(0);
    				left_motor_set_speed(-SPEED_MOVE);
    				right_motor_set_speed(SPEED_MOVE);
    				while(right_motor_get_pos() != 270){}
    			}
    		}
    	}
    	else{
    		left_motor_set_speed(STOP);
    		right_motor_set_speed(STOP);
    	}
    	//check if we can pass to FINISH mode
    	if(get_color_detected() == 3 && (get_mode() == MOVEMENT)){
    		next_mode();
		}
    	chThdSleepUntilWindowed(time, time + MS2ST(10)); // 100 Hz
    	//réfléchir si on met un sleep ou autre chose
    }

}


void movement_start(void){
	chThdCreateStatic(wamovement, sizeof(wamovement), NORMALPRIO, movement, NULL);
}


uint8_t get_no_more_color_needed(void){
	return no_more_color_needed;
}
