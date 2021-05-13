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
#define MIN_DIST_FRONT 50
#define DIST_DETECTION 30
#define DISTANCE_PASSAGE_COULEUR 500

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

#define SPEED_MOVE 600
#define SPEED_TURN 500
#define SPEED_CORR 400
#define STOP 0


// movement thread

static THD_WORKING_AREA(wamovement, 256);
static THD_FUNCTION(movement, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    while(1){

    	time = chVTGetSystemTime();

    	uint16_t distance = VL53L0X_get_dist_mm();

//    	chprintf((BaseSequentialStream *)&SD3, "distance %d \n\n", distance);
//    	chprintf((BaseSequentialStream *)&SD3, "DIAG_LEFT_IR :%d \n\n", get_prox(DIAG_LEFT_IR));
//    	chprintf((BaseSequentialStream *)&SD3, "DIAG_RIGHT_IR :%d \n\n", get_prox(DIAG_RIGHT_IR));
//    	chprintf((BaseSequentialStream *)&SD3, "FRONT_LEFT_IR :%d \n\n", get_prox(FRONT_LEFT_IR));
//    	chprintf((BaseSequentialStream *)&SD3, "FRONT_RIGHT_IR :%d \n\n", get_prox(FRONT_RIGHT_IR));
//    	chprintf((BaseSequentialStream *)&SD3, "distance %d : \r\n", distance);
    	chprintf((BaseSequentialStream *)&SD3, "couleur trouvée %d : \r\n", get_couleur_trouvee());

    	if(get_selector()){
    		if(get_couleur_trouvee() == 0){  //ou distance > 3 dans le cas ou une couleur a été trouvé, on corrige quand meme
    			if(distance < MIN_DIST_FRONT && get_couleur_trouvee() == 0){
    				left_motor_set_speed(-SPEED_MOVE);
    				right_motor_set_speed(SPEED_MOVE);
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
    		else if(get_couleur_trouvee() == get_selector() && distance < DIST_DETECTION){
    			for(int i = 0; i < DISTANCE_PASSAGE_COULEUR; i++){   //TROUVER i pour passer le mur (on peut jouer sur les leds pour montrer qu'on bloque le robot en mode passage de couleur)
    				left_motor_set_speed(SPEED_MOVE);   // JOUER SUR LE FAIT QUE CE THREAD AURA UNE PRIORITE PLUS GRANDE QUE LES AUTRES THREADS POUR POUVOIR "BLOQUER"
    				right_motor_set_speed(SPEED_MOVE);
    			}
    		}
    		else if(get_couleur_trouvee() != get_selector() && distance < MIN_DIST_FRONT){
    			left_motor_set_speed(-SPEED_TURN);
    			right_motor_set_speed(SPEED_TURN);
    		}
    	}
    	else{
    		left_motor_set_speed(STOP);
    		right_motor_set_speed(STOP);
    	}



    	//JE TESTE UN TRUC POUR LA COULEUR

 //   				else if(distance < DIST_DETECTION){
  //  					for(int j = 0; j < 1000; j++){
  //  						toggle_rgb_led(1, RED_LED, RED)
  //						toggle_rgb_led(1, RED_LED, RED)
 //							toggle_rgb_led(1, RED_LED, RED)
 //   						toggle_rgb_led(1, RED_LED, RED)
 //						}
 //   					if(get_couleur_trouvee() == get_selector()){
 //   	    				for(int i = 0; i < DISTANCE_PASSAGE_COULEUR; i++){
 //   	    				left_motor_set_speed(SPEED_MOVE);
  //  	    				right_motor_set_speed(SPEED_MOVE);
 //   	    				}
 //						}
  //  	    			else{
 //   	    				left_motor_set_speed(-SPEED_TURN);
 //   	    				right_motor_set_speed(SPEED_TURN);
 //   	    			}
//    	    		}


    	chThdSleepUntilWindowed(time, time + MS2ST(10)); // 100 Hz
    	//réfléchir si on met un sleep ou autre chose
    }

}


void movement_start(void){
	chThdCreateStatic(wamovement, sizeof(wamovement), NORMALPRIO, movement, NULL);
}
