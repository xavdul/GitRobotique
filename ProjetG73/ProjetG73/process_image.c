#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>
#include <motors.h>
#include <process_image.h>
#include <movement.h>
#include <leds.h>
#include <sensors/VL53L0X/VL53L0X.h>


//Thresholds for color detection
#define RED_TH    20
#define BLUE_TH   10
#define GREEN_TH1 7
#define GREEN_TH2 12

//RGB colors for the leds
#define RED_LED	    255, 0, 0
#define GREEN_LED	0, 255, 0
#define	BLUE_LED	0, 0, 255
#define WHITE_LED	255, 255, 255


//Security threshold to detect color under a certain distance
#define SECURITY_TH 100

//Defining the static variables to store the colors seen by the camera and the selected color mode on the robot

static uint8_t color_detected = 0;
static uint8_t color_selected = 0;

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

//Thread to capture the image from the camera, code taken from TP4
static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

    while(1){
        //starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
    }
}

//Thread to process the image and extract the color of the image, adapted from TP4

static THD_WORKING_AREA(waProcessImage, 2048);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_buff_ptr;

	//Initializing the array to extract each colors pixels
	uint8_t image_r[IMAGE_BUFFER_SIZE] = {0};
	uint8_t image_b[IMAGE_BUFFER_SIZE] = {0};

	//Initializing the variables for the future calculations of the colors mean
	uint32_t mean_r = 0;
	uint32_t mean_b = 0;


    while(1){


    	mean_r = 0;
    	mean_b = 0;

    	//waits until an image has been captured
        chBSemWait(&image_ready_sem);
		//gets the pointer to the array filled with the last image in RGB565    
		img_buff_ptr = dcmi_get_last_image_ptr();

		//Extracts the blue and red pixels in the respective array
		for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){
			image_b[i/2] = (uint8_t)img_buff_ptr[i+1]&0x1F;
			image_r[i/2] = (((uint8_t)img_buff_ptr[i]&0xF8) >> 3);
		}

		// Computing the mean value of the red and blue pixels over the lines
		for(int i = 0; i < IMAGE_BUFFER_SIZE; i++){
			mean_r += image_r[i];
			mean_b += image_b[i];
		}

		mean_r = mean_r / IMAGE_BUFFER_SIZE;
     	mean_b = mean_b / IMAGE_BUFFER_SIZE;



//Detection of the colors with thresholds on the average for every color

     if(get_mode() != WAIT_NEXT_ATTEMPT){
     	if(!get_no_more_color_needed()){				//checking if we want a color detection or not
     		if(VL53L0X_get_dist_mm() > SECURITY_TH){	//no detection over 10cm to avoid false detections
     			color_detected = WHITE;					//white as default color when no detection
     			if(get_mode() == FINISH )				//set leds to detected color unless in FINISH mode
     				clear_leds();
     			else
     				set_all_leds(WHITE_LED);
     		}
     		else{
     			color_detection(mean_r, mean_b);		//color detection
     			if(get_mode() != FINISH){				//set leds to detected color unless in FINISH mode
     				if(color_detected == RED)
     					set_all_leds(RED_LED);
     				if(color_detected == GREEN)
     					set_all_leds(GREEN_LED);
     				if(color_detected == BLUE)
     					set_all_leds(BLUE_LED);
     				if(color_detected == WHITE)
     					set_all_leds(WHITE_LED);
     			}
     			else
     				clear_leds();
     		}
     	}
     }
     else
    	 clear_leds();
    }
}


uint8_t get_color_detected(void){
	return color_detected;
}

uint8_t get_color_selected(void){
	return color_selected;
}

void update_color_selected(void){
	color_selected = color_detected;
}


void color_detection(uint32_t mean_r, uint32_t mean_b){
	if(mean_r > RED_TH){								//RED detection
		color_detected = RED;
	}
	else if(mean_b > BLUE_TH){							//BLUE detection
		color_detected = BLUE;
	}
	else if(mean_b < GREEN_TH1 && mean_r < GREEN_TH2){  //GREEN detection
		color_detected = GREEN;
	}
	else{												//WHITE as default (walls) colors if no other color detected
		color_detected = WHITE;
	}
}

void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
