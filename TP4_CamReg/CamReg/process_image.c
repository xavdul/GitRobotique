#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>

#define PXTOCM 1570.0f

static float distance_cm = 0;


systime_t time;

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

static uint16_t line_position = IMAGE_BUFFER_SIZE/2;

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

		time = chVTGetSystemTime();

//		chprintf((BaseSequentialStream *)&SD3, "%Time=%d \n", time);

		chThdSleepMilliseconds(12);
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);



    }




}


static THD_WORKING_AREA(waProcessImage, 1024);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_buff_ptr;
	uint8_t image[IMAGE_BUFFER_SIZE] = {0};

    while(1){
    	//waits until an image has been captured
        chBSemWait(&image_ready_sem);
		//gets the pointer to the array filled with the last image in RGB565    
		img_buff_ptr = dcmi_get_last_image_ptr();

		for (int i=0; i < 2*IMAGE_BUFFER_SIZE; i+=2){
			image[i/2] = (img_buff_ptr[i+1] & 0x1F);
		}

//		SendUint8ToComputer(image, IMAGE_BUFFER_SIZE);

		int line_width = line_detection(image);
		if (line_width =0)
			distance_cm = 10;
		else
			distance_cm = (PXTOCM / line_width);


		chprintf((BaseSequentialStream *)&SD3, "Largeur = %7d\r\n", line_width);
		chprintf((BaseSequentialStream *)&SD3, "Distance = %f\r\n", distance_cm);

    }
}

float get_distance_cm(void){
	return distance_cm;
}

void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}

int line_detection(uint8_t* image){

	int line_width = 0, begin = 0, end = 0;
	int line_not_found = 1;

	uint32_t mean = 0;

	for(int i=0; i < IMAGE_BUFFER_SIZE; i++){
		mean += image[i];
	}

	mean = (mean/IMAGE_BUFFER_SIZE);

//	chprintf((BaseSequentialStream *)&SD3, "Mean = %7d\r\n", mean);


	int j=0;

	while(line_not_found){


		while(j < (IMAGE_BUFFER_SIZE - 5) && !begin){
			//chprintf((BaseSequentialStream *)&SD3, " trouvé1\r\n");
			if(image[j] > mean && image[j+5] < mean)
			{
				begin = j;
				//chprintf((BaseSequentialStream *)&SD3, "begin trouvé\r\n");
			}
			j++;
		}

		while(j < (IMAGE_BUFFER_SIZE-5) && !end){
			//chprintf((BaseSequentialStream *)&SD3, " trouvé2\r\n");

			if(image[j] < mean && image[j+5] > mean)
			{
				end = j;
				//chprintf((BaseSequentialStream *)&SD3, "end trouvé\r\n");
			}
			j++;
		}

		if((end-begin) > 40)
		{
			if(end-begin > 250)
				line_width = 250;
			else
			{
			line_not_found = 0;
			line_width = (end - begin);
			line_position = (end+begin)/2;
			}
		}
		else if (j>= (IMAGE_BUFFER_SIZE-5))
		{
			line_not_found = 0;
			line_width = 0;
			line_position =0;
		}
		else{
		end =0;
		begin =0;
		//chprintf((BaseSequentialStream *)&SD3, "fin de ligne\r\n");
		line_not_found = 1;
		}
	}



	return line_width;
}
