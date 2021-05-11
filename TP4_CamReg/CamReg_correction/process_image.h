#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

float get_distance_cm(void);
uint16_t get_line_position(void);
void process_image_start(void);
uint16_t get_line_not_found(void);

typedef struct line_info{
	uint16_t begin;
	uint16_t end;
	uint16_t width;
	uint16_t width_pi;
	uint16_t position;
	uint8_t line_not_found;
}line_info;

#endif /* PROCESS_IMAGE_H */
