#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>


#include <main.h>
#include <motors.h>
#include <pi_regulator.h>
#include <process_image.h>

#define RED		100, 0, 0
#define NB_LEDS	4

//simple PI regulator implementation
int16_t pi_regulator(float distance, float goal){

	float error = 0;
	float speed = 0;

	static float sum_error = 0;

	error = distance - goal;

	//disables the PI regulator if the error is to small
	//this avoids to always move as we cannot exactly be where we want and 
	//the camera is a bit noisy
	if(fabs(error) < ERROR_THRESHOLD){
		return 0;
	}

	sum_error += error;

	//we set a maximum and a minimum for the sum to avoid an uncontrolled growth
	if(sum_error > MAX_SUM_ERROR){
		sum_error = MAX_SUM_ERROR;
	}else if(sum_error < -MAX_SUM_ERROR){
		sum_error = -MAX_SUM_ERROR;
	}

	speed = KP * error + KI * sum_error;
//	speed = KP * error;
//	chprintf((BaseSequentialStream *)&SD3, "speed :%f \n\n", speed);
    return (int16_t)speed;
}

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    int16_t speed = 0;
    int16_t speed_correction = 0;
    float mm =0;


    while(1){
        time = chVTGetSystemTime();
        
    	mm = (VL53L0X_get_dist_mm());
//    	chprintf((BaseSequentialStream *)&SD3, "mm = %f \n", mm);
    	uint8_t test = get_line_not_found();
    	if(!test){
    		if(get_selector() && mm != 10){

    			//computes the speed to give to the motors
    			//distance_cm is modified by the image processing thread
    			//speed = pi_regulator(get_distance_cm(), GOAL_DISTANCE);
    			speed = pi_regulator(mm, GOAL_DISTANCE);
    			//computes a correction factor to let the robot rotate to be in front of the line
    			speed_correction = (get_line_position() - (IMAGE_BUFFER_SIZE/2));

    			//if the line is nearly in front of the camera, don't rotate
    			if(abs(speed_correction) < ROTATION_THRESHOLD){
    				speed_correction = 0;
    			}

    			//applies the speed from the PI regulator and the correction for the rotation
    			right_motor_set_speed(speed - ROTATION_COEFF * speed_correction);
    			left_motor_set_speed(speed + ROTATION_COEFF * speed_correction);
    			//        	right_motor_set_speed(speed);
    			//        	left_motor_set_speed(speed);
    		}
    		else{
    			right_motor_set_speed(0);
    			left_motor_set_speed(0);
    			if(mm== 10){
    				set_front_led(10);
    			}
    		}
    	}



        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

void pi_regulator_start(void){
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}
