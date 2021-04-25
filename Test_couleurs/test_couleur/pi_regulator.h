#ifndef PI_REGULATOR_H
#define PI_REGULATOR_H

#define KP 500.0f
#define KI 0.2f
#define MAX_SUM_ERROR (1100/3.5f)

//start the PI regulator thread
void pi_regulator_start(void);

#endif /* PI_REGULATOR_H */
