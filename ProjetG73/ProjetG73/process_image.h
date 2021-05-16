#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

//Colors detected
#define WHITE 0
#define RED   1
#define BLUE  2
#define GREEN 3

void process_image_start(void);

//returns the detected color
uint8_t get_color_detected(void);

//returns the selected color
uint8_t get_color_selected(void);

//sets the detected color as selected color
void update_color_selected(void);

//Runs the color detection algorithm
void color_detection(uint32_t mean_r, uint32_t mean_b);




#endif /* PROCESS_IMAGE_H */
