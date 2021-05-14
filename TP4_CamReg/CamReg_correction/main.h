#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "camera/dcmi_camera.h"
#include "msgbus/messagebus.h"
#include "parameter/parameter.h"


//constants for the differents parts of the project
#define IMAGE_BUFFER_SIZE		640

//modes
#define IDLE 0
#define START 1
#define MOVEMENT 2
#define FINISH 3
#define WAIT_NEXT_ATTEMPT 4

#define SPEED_MOVE 500

/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;

void SendUint8ToComputer(uint8_t* data, uint16_t size);

uint8_t get_mode(void);

void next_mode(void);

#ifdef __cplusplus
}
#endif

#endif
