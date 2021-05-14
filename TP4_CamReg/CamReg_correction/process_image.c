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



//Leds
#define RED		255, 0, 0
#define GREEN	0, 255, 0
#define	BLUE	0, 0, 255
#define WHITE	255, 255, 255
#define NB_LEDS	4

//Defining the static variables to store the colors seen by the camera and the selected color mode on the robot

static uint8_t color_detected = 0;
static uint8_t color_selected = 0;

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);


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


static THD_WORKING_AREA(waProcessImage, 4096); //3072
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_buff_ptr;

	//Initializing the array to extract each colors pixels
	uint8_t image_r[IMAGE_BUFFER_SIZE] = {0};
	uint8_t image_g[IMAGE_BUFFER_SIZE] = {0};
	uint8_t image_b[IMAGE_BUFFER_SIZE] = {0};

	//Initializing the variables for the future calculations of the colors mean
	uint32_t mean_r = 0;
	uint32_t mean_g = 0;
	uint32_t mean_b = 0;


    while(1){


    	mean_r = 0;
    	mean_g = 0;
    	mean_b = 0;

    	//waits until an image has been captured
        chBSemWait(&image_ready_sem);
		//gets the pointer to the array filled with the last image in RGB565    
		img_buff_ptr = dcmi_get_last_image_ptr();

		//Extracts the pixels of each color in an array
		for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){
			image_b[i/2] = (uint8_t)img_buff_ptr[i+1]&0x1F;
			image_r[i/2] = (((uint8_t)img_buff_ptr[i]&0xF8) >> 3);
			image_g[i/2] = ((((uint8_t)img_buff_ptr[i+1]&0xE0) >> 5) | (((uint8_t)img_buff_ptr[i]&0x07 ) << 3));
		}

		// Computing the mean value of the red, green, and blue pixels
		for(int i = 0; i < IMAGE_BUFFER_SIZE; i++){
			mean_r += image_r[i];
		}

		for(int i = 0; i < IMAGE_BUFFER_SIZE; i++){
			mean_g += image_g[i];
						}

		for(int i = 0; i < IMAGE_BUFFER_SIZE; i++){
			mean_b += image_b[i];
		}
		mean_r = mean_r / IMAGE_BUFFER_SIZE;
     	mean_g = mean_g / IMAGE_BUFFER_SIZE;
     	mean_b = mean_b / IMAGE_BUFFER_SIZE;



//DETECTION COULEUR AVEC SEUILS
     	if( !get_no_more_color_needed() ){
     		if(VL53L0X_get_dist_mm() > 100){
     			color_detected = 0;
     			if(get_mode() != FINISH){
     				clear_leds();
     				for(uint8_t i = 0; i < NB_LEDS; i++)
     				{
     					set_rgb_led(i, WHITE);
     				}
     			}
     			else
     				clear_leds();
     		}
     		else{
     			if(mean_r > 20){
     				color_detected = 1;
     				//				chprintf((BaseSequentialStream *)&SD3, "C'est du rouge \r\n");
     				if(get_mode() != FINISH){
     					clear_leds();
     					for(uint8_t i = 0; i < NB_LEDS; i++)
     					{
     						set_rgb_led(i, RED);
     					}
     				}
     				else
     					clear_leds();
     			}
     			else if(mean_b > 10){
     				color_detected = 2;
     				//				chprintf((BaseSequentialStream *)&SD3, "C'est du bleu \r\n");
     				if(get_mode() != FINISH){
     					clear_leds();
     					for(uint8_t i = 0; i < NB_LEDS; i++)
     					{
     						set_rgb_led(i, BLUE);
     					}
     				}
     				else
     					clear_leds();
     			}
     			else if(mean_b < 7 && mean_r < 12){
     				color_detected = 3;
     				//				chprintf((BaseSequentialStream *)&SD3, "C'est du vert \r\n");
     				if(get_mode() != FINISH){
     					clear_leds();
     					for(uint8_t i = 0; i < NB_LEDS; i++)
     					{
     						set_rgb_led(i, GREEN);
     					}
     				}
     				else
     					clear_leds();
     			}
     			else{
     				//				chprintf((BaseSequentialStream *)&SD3, "C'est du blanc \r\n");
     				color_detected = 0;
     				if(get_mode() != FINISH){
     					clear_leds();
     					for(uint8_t i = 0; i < NB_LEDS; i++)
     					{
     						set_rgb_led(i, WHITE);
     					}
     				}
     				else
     					clear_leds();
     			}
     		}
     	}


		//sends to the computer the image
		//SendUint8ToComputer(image_r, IMAGE_BUFFER_SIZE);
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
//	chprintf((BaseSequentialStream *)&SD3, "color = %d \r\n", color_selected);
}


void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage+1, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
