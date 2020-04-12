// AS5048.h
#ifndef AS5048_POSITION_ENCODER_H
#define AS5048_POSITION_ENCODER_H

#include "stm32f7xx_hal.h"

typedef struct {
	// Timer stuff
	TIM_HandleTypeDef *htim;
	HAL_TIM_ActiveChannel channel;
	TIM_TypeDef *instance;
	uint32_t channel_id;

	// Stuff for frequency detection
	uint8_t initialised, count;
	float frequency, period;

	// Stuff for position reading
	uint8_t cycleInitialised;
	uint32_t onRise, onFall, diff;
	float angle, prev_angle;
} AS5048_PWM_SENSOR;

void AS5048_initialise_struct(AS5048_PWM_SENSOR*, TIM_HandleTypeDef*, TIM_TypeDef*, HAL_TIM_ActiveChannel, uint32_t);
void AS5048_pwm_timer_interrupt(AS5048_PWM_SENSOR*);

#endif