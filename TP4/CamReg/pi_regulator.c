#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>


#include <main.h>
#include <motors.h>
#include <pi_regulator.h>
#include <process_image.h>

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    int16_t speed = 0;

    float distance = 0;
    float error = 0;
    float integral = 0;

    while(1){
        time = chVTGetSystemTime();
        
        distance = get_distance_cm();

        error =  distance - TARGET;
        integral = integral + error;

    	//we set a maximum and a minimum for the sum to avoid an uncontrolled growth
    	if(integral > MAX_SUM_ERROR){
    		integral = MAX_SUM_ERROR;
    	}else if(integral < -MAX_SUM_ERROR){
    		integral = -MAX_SUM_ERROR;
    	}

        speed = KP*error + KI*integral;

       // chprintf((BaseSequentialStream *)&SD3, "Integral = %f\r\n", integral);
       // chprintf((BaseSequentialStream *)&SD3, "distance = %f\r\n", distance);



        //applies the speed from the PI regulator
		right_motor_set_speed(speed);
		left_motor_set_speed(speed);

        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

void pi_regulator_start(void){
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}
