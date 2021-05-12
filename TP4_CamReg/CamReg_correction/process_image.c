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


static float distance_cm = 0;
static float test = 0;
static uint16_t line_position = IMAGE_BUFFER_SIZE/2;	//middle
static uint8_t line_not_found = 0;
static uint8_t couleur_trouvee =0;
static uint8_t couleur_selectionnee =0;

//Leds
#define RED		255, 0, 0
#define GREEN	0, 255, 0
#define	BLUE	0, 0, 255
#define WHITE	255, 255, 255
#define NB_LEDS	4

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

/*
 *  Returns the line's width extracted from the image buffer given
 *  Returns 0 if line not found
 */
line_info extract_line_width(uint8_t *buffer){

	uint16_t i = 0, begin = 0, end = 0, width = 0;
	uint8_t stop = 0, wrong_line = 0;
	uint32_t mean = 0;

	line_info line;

	//performs an average
	for(uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE ; i++){
		mean += buffer[i];
	}
	mean /= IMAGE_BUFFER_SIZE;

//	chprintf((BaseSequentialStream *)&SD3, "mean :%d \n\n", mean);

		line_not_found = 0;
		i = 0;
		//search for a begin
		while(stop == 0 && i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE))
		{ 
			//the slope must at least be WIDTH_SLOPE wide and is compared
		    //to the mean of the image
//			chprintf((BaseSequentialStream *)&SD3, "valeur if :%d \n\n", buffer[i]);

		    if(buffer[i] < mean && buffer[i+WIDTH_SLOPE] > mean)
		    {
		        begin = i;
		        stop = 1;
		    }
		    i++;
		}
		//if a begin was found, search for an end
		if (i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE) && begin)
		{
		    stop = 0;
		    
		    while(stop == 0 && i < IMAGE_BUFFER_SIZE)
		    {
		        if(buffer[i] < mean && buffer[i-WIDTH_SLOPE] > mean)
		        {
		            end = i;
		            stop = 1;
		        }
		        i++;
		    }
		    //if an end was not found
		    if (i > IMAGE_BUFFER_SIZE || !end)
		    {
		        line_not_found = 1;
//		        chprintf((BaseSequentialStream *)&SD3, "jai pas de ligne \n\n");
		    }
		}
		else//if no begin was found
		{
		    line_not_found = 1;
//		    chprintf((BaseSequentialStream *)&SD3, "jai pas de ligne \n\n");
		}

		if(end - begin < MIN_LINE_WIDTH)
			line_not_found = 1;


	 //gives the line position.

		if(!line_not_found){
			width = (end - begin);
			line_position = (begin + end)/2;
			line.width = width;
			if(PXTOCM/line.width > MAX_DISTANCE)
				line.width_pi = MAX_DISTANCE;
			else
				line.width_pi = line.width;
//			chprintf((BaseSequentialStream *)&SD3, "jai une ligne \n\n");

		}
		else {
			width = 0;
			begin = 0;
			end = 0;
			line_position = IMAGE_BUFFER_SIZE/2;
		}

		line.begin = begin;
		line.end = end;
		line.position = line_position;
		line.line_not_found = line_not_found;
//		chprintf((BaseSequentialStream *)&SD3, "position : %d \n\n", line_position);






	return line;
}

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


static THD_WORKING_AREA(waProcessImage, 3072);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_buff_ptr;
	uint8_t image_r[IMAGE_BUFFER_SIZE] = {0};
	uint8_t image_g[IMAGE_BUFFER_SIZE] = {0};
	uint8_t image_b[IMAGE_BUFFER_SIZE] = {0};
	uint16_t lineWidth = 0;

	uint32_t mean_r = 0;
	uint32_t mean_g = 0;
	uint32_t mean_b = 0;

	line_info line_r = {0};
	line_info line_g = {0};
	line_info line_b = {0};

	bool send_to_computer = false ;

    while(1){
    	//waits until an image has been captured

    	mean_r = 0;
    	mean_g = 0;
    	mean_b = 0;

        chBSemWait(&image_ready_sem);
		//gets the pointer to the array filled with the last image in RGB565    
		img_buff_ptr = dcmi_get_last_image_ptr();

		//Extracts the pixels of each color in array
		for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){
			image_b[i/2] = (uint8_t)img_buff_ptr[i+1]&0x1F;
			image_r[i/2] = (((uint8_t)img_buff_ptr[i]&0xF8) >> 3);
			image_g[i/2] = ((((uint8_t)img_buff_ptr[i+1]&0xE0) >> 5) | (((uint8_t)img_buff_ptr[i]&0x07 ) << 3));
		}


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

//     	if(mean_r > 3)
//     		chprintf((BaseSequentialStream *)&SD3, "wesh bien moyenne calculée red:%d \r\n", mean_r);
//     	if(mean_g > 3)
//     		chprintf((BaseSequentialStream *)&SD3, "wesh bien moyenne calculée green :%d \r\n", mean_g);
//     	if(mean_b > 3)
//     		chprintf((BaseSequentialStream *)&SD3, "wesh bien moyenne calculée blue:%d \r\n", mean_b);



//DETECTION COULEUR AVEC SEUILS

			if(mean_r > 20){
     			couleur_trouvee = 1;
				chprintf((BaseSequentialStream *)&SD3, "C'est du rouge \r\n");
            	clear_leds();
    			for(uint8_t i = 0; i < NB_LEDS; i++)
    			{
    				set_rgb_led(i, RED);
    			}

			}
			else if(mean_b > 10){
     			couleur_trouvee = 2;
				chprintf((BaseSequentialStream *)&SD3, "C'est du bleu \r\n");
				clear_leds();
            	clear_leds();
    			for(uint8_t i = 0; i < NB_LEDS; i++)
    			{
    				set_rgb_led(i, BLUE);
    			}
			}
			else if(mean_b < 3){
     			couleur_trouvee = 3;
				chprintf((BaseSequentialStream *)&SD3, "C'est du vert \r\n");
            	clear_leds();
    			for(uint8_t i = 0; i < NB_LEDS; i++)
    			{
    				set_rgb_led(i, GREEN);
    			}
			}
			else{
				chprintf((BaseSequentialStream *)&SD3, "C'est du blanc \r\n");
				couleur_trouvee = 0;
            	clear_leds();
    			for(uint8_t i = 0; i < NB_LEDS; i++)
    			{
    				set_rgb_led(i, WHITE);
    			}
    		}

//
		//sends to the computer the image
		//SendUint8ToComputer(image_r, IMAGE_BUFFER_SIZE);

    }
}

float get_distance_cm(void){
	return distance_cm;
}

uint16_t get_line_position(void){
	return line_position;
}

void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage+1, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}


uint16_t get_line_not_found(void){
	return line_not_found;
}

uint8_t get_couleur_trouvee(void){
	return couleur_trouvee;
}




