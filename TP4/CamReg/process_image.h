#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#define TARGET 10.0f

float get_distance_cm(void);
void process_image_start(void);
int line_detection(uint8_t* image);


#endif /* PROCESS_IMAGE_H */
