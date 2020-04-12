#include "AS5048.h"

// Initialises struct PWM sensor struct
void AS5048_initialise_struct(AS5048_PWM_SENSOR *sensorStruct, TIM_HandleTypeDef *htim, TIM_TypeDef *TIMER, HAL_TIM_ActiveChannel CHANNEL, uint32_t channel_id, uint32_t tim_freq){
	sensorStruct->htim = htim;
	sensorStruct->channel = CHANNEL;
	sensorStruct->channel_id = channel_id;
	sensorStruct->instance = TIMER;
	sensorStruct->initialised = 0;
	sensorStruct->count = 0;
	sensorStruct->initialised = 0;
	sensorStruct->valid = 0;
	sensorStruct->timer_frequency = tim_freq;
}

// Handles PWM cycles, should be called on HAL_TIM_IC_CaptureCallback
void AS5048_pwm_timer_interrupt(AS5048_PWM_SENSOR *sensorStruct){

		if(sensorStruct->initialised){
			// Sensor reading
			if(!sensorStruct->cycleInitialised){
				// If it's the beginning of the PWM cycle, get time it occurs and change next interrupt to falling edge (starts with rising
				sensorStruct->onRise = HAL_TIM_ReadCapturedValue(sensorStruct->htim, sensorStruct->channel_id);
				__HAL_TIM_SET_CAPTUREPOLARITY(sensorStruct->htim, sensorStruct->channel_id, TIM_INPUTCHANNELPOLARITY_FALLING);
				sensorStruct->cycleInitialised = 1;
			}else{
				// If its on the end of the cycle, get time in which it occurred and save on variable
				sensorStruct->onFall = HAL_TIM_ReadCapturedValue(sensorStruct->htim, sensorStruct->channel_id);
				// reset timer
				__HAL_TIM_SET_COUNTER(sensorStruct->htim, 0);
				// Calculate time difference
				if(sensorStruct->onFall > sensorStruct->onRise){
					sensorStruct->diff = sensorStruct->onFall - sensorStruct->onRise;
					AS5048_pwm_to_rad(sensorStruct);
				}
				// Set next interrupt to be on rising edge
				__HAL_TIM_SET_CAPTUREPOLARITY(sensorStruct->htim, sensorStruct->channel_id, TIM_INPUTCHANNELPOLARITY_RISING);
				sensorStruct->cycleInitialised = 0;

			}
		}
		else { // Initialise sensor
			// After a sensor is connected, discard first interrupt (since we did not know the state of the timer) and read a few
			// more interrupt times, take a mean of that and assert frequency
			if(sensorStruct->count == 0){
				// Reset counter
				__HAL_TIM_SET_COUNTER(sensorStruct->htim, 0);
				sensorStruct->period = 0;
			}
			else if(sensorStruct->count <= 100){
				sensorStruct->period += (float) HAL_TIM_ReadCapturedValue(sensorStruct->htim, sensorStruct->channel_id)/sensorStruct->timer_frequency; // each clock count is equivalent to 1 us
			}
			else {
				// mean of periods
				sensorStruct->period /= (float)sensorStruct->count;
				sensorStruct->frequency = 1.0 / sensorStruct->period;
				sensorStruct->initialised++;
			}
			sensorStruct->count++;
		}
}

// Interpret raw value and convert to radians when possible
void AS5048_pwm_to_rad(AS5048_PWM_SENSOR* sensorStruct){
	// Acording to datasheet: https://ams.com/documents/20143/36005/AS5048_DS000298_4-00.pdf
	// Each PWM cycle has 4120 clock periods, lets count for how many of those clocks the PWM was up
	sensorStruct->valid = 255;
	if(sensorStruct->onFall < 110000){
		sensorStruct->pwm_clocks =  roundf(sensorStruct->diff * 4120 / (sensorStruct->period * sensorStruct->timer_frequency));
		// First 16bits of the duty cycles are reserved for error handling and init
		if(sensorStruct->pwm_clocks >= 15) {
			sensorStruct->angle = sensorStruct->prev_angle;
			sensorStruct->angle = (sensorStruct->pwm_clocks - 15.0) * 2 * M_PI / 4096.0;
			sensorStruct->valid = 0;
		} else {
			sensorStruct->valid = sensorStruct->pwm_clocks;
		}
	}

}
