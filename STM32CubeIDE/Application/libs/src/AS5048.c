#include "AS5048.h"


void AS5048_initialise_struct(AS5048_PWM_SENSOR *sensorStruct, TIM_HandleTypeDef *htim, TIM_TypeDef *TIMER, HAL_TIM_ActiveChannel CHANNEL, uint32_t channel_id){
	sensorStruct->htim = htim;
	sensorStruct->channel = CHANNEL;
	sensorStruct->channel_id = channel_id;
	sensorStruct->instance = TIMER;
	sensorStruct->initialised = 0;
	sensorStruct->count = 0;
	sensorStruct->initialised = 0;
}

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
				if(sensorStruct->onFall > sensorStruct->onRise) sensorStruct->diff = sensorStruct->onFall - sensorStruct->onRise;
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
				sensorStruct->period += (float) HAL_TIM_ReadCapturedValue(sensorStruct->htim, sensorStruct->channel_id)/1000000; // each clock count is equivalent to 1 us
			}
			else {
				// mean of periods
				sensorStruct->period /= (float)sensorStruct->count;
				sensorStruct->frequency = 1.0 / sensorStruct->period;
				sensorStruct->initialised++;
				__HAL_TIM_SET_CAPTUREPOLARITY(sensorStruct->htim, sensorStruct->channel_id, TIM_INPUTCHANNELPOLARITY_RISING);
			}
			sensorStruct->count++;
		}
}
