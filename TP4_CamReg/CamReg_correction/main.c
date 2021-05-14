#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <arm_math.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <main.h>
#include <motors.h>
#include <camera/po8030.h>
#include <chprintf.h>
#include <sensors/proximity.h>
#include <sensors/VL53L0X/VL53L0X.h>
#include <selector.h>
#include <spi_comm.h>
#include <i2c_bus.h>
#include <leds.h>

#include <process_image.h>
#include <movement.h>
#include <camera/po8030.h>
messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

static 	uint8_t mode = IDLE;

void SendUint8ToComputer(uint8_t* data, uint16_t size) 
{
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)"START", 5);
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)&size, sizeof(uint16_t));
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)data, size);
}

static void serial_start(void)
{
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};

	sdStart(&SD3, &ser_cfg); // UART3.
}

int main(void)
{

    halInit();
    chSysInit();
    mpu_init();

    //starts the serial communication
    serial_start();
    //start the USB communication
    usb_start();
    // inits the inter process communication bus
    messagebus_init(&bus, &bus_lock, &bus_condvar);
    //starts the camera
    dcmi_start();
	po8030_start();
    po8030_set_awb(0); //disable auto white balance
	//inits the motors
	motors_init();
	//inits the proximity sensors
	proximity_start();
	//inits the rgb leds
	spi_comm_start();
    // starts the time of flight sensor
    VL53L0X_start();

	//inits the movement thread
	movement_start();
	//inits the image processing thread to detect the colors
	process_image_start();

    /* Infinite loop. */
    while (1) {

    	switch(mode){

    	case IDLE :
    		chprintf((BaseSequentialStream *)&SD3, "IDLE \r\n");
    		set_body_led(0);
    		if(get_selector() == 0)
    			update_color_selected();
    		else
    			mode = START;
    		break;

    	case START :
    		chprintf((BaseSequentialStream *)&SD3, "START \r\n");
    		break;

    	case MOVEMENT :
    		chprintf((BaseSequentialStream *)&SD3, "MOVEMENT \r\n");
    		break;

    	case FINISH :
    		chprintf((BaseSequentialStream *)&SD3, "FINISH \r\n");
    		break;
    	case WAIT_NEXT_ATTEMPT :
    		chprintf((BaseSequentialStream *)&SD3, "ATTENTE \r\n");
    		if(get_selector() == 0)
    			mode = IDLE;
    	}

    	//waits 1 second
        chThdSleepMilliseconds(1000);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}

uint8_t get_mode(void){
	return mode;
}

void next_mode(void){
	mode += 1;
}
