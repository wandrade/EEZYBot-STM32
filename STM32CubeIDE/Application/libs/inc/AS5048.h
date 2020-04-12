// AS5048.h
#ifndef AS5048_POSITION_ENCODER_H
#define AS5048_POSITION_ENCODER_H

#include "stm32f7xx_hal.h"
#include <math.h>

typedef struct {
	// Timer stuff
	TIM_HandleTypeDef *htim;
	HAL_TIM_ActiveChannel channel;
	TIM_TypeDef *instance;
	uint32_t channel_id;
	uint32_t timer_frequency;

	// Stuff for frequency detection
	uint8_t initialised, count;
	float frequency, period;

	// Stuff for position reading
	uint8_t cycleInitialised;
	uint32_t onRise, onFall, diff;

	// Stuff for converting PWM into an actual angle (in radians)
	uint8_t valid; // Zero when valid, different than zero means error code (see data sheet)
	uint16_t pwm_clocks;
	float angle, prev_angle;
} AS5048_PWM_SENSOR;

// Initialises struct PWM sensor struct
void AS5048_initialise_struct(AS5048_PWM_SENSOR *sensorStruct, TIM_HandleTypeDef *htim, TIM_TypeDef *TIMER, HAL_TIM_ActiveChannel CHANNEL, uint32_t channel_id, uint32_t tim_freq);

// Handles PWM cycles, should be called on HAL_TIM_IC_CaptureCallback
void AS5048_pwm_timer_interrupt(AS5048_PWM_SENSOR *sensorStruct);

// Interpret raw value and convert to radians when possible
void AS5048_pwm_to_rad(AS5048_PWM_SENSOR* sensorStruct);

#endif
