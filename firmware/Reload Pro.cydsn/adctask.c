/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include <FreeRTOS.h>
#include <task.h>
#include "tasks.h"
#include "config.h"

typedef struct {
	int16 voltage;
	int16 current;
} adc_reading;

static adc_reading readings[ADC_MOVING_AVERAGE_LENGTH];
static uint8 num_readings = 0;
static uint8 next_reading = 0;
static int total_voltage = 0;
static int total_current = 0;

void vTaskADC(void *pvParameters) {
	portTickType lastWakeTime = xTaskGetTickCount();

	ADC_Start();
	ADC_StartConvert();

	while(1) {
		if(num_readings == ADC_MOVING_AVERAGE_LENGTH) {
			total_current -= readings[next_reading].current;
			total_voltage -= readings[next_reading].voltage;
		} else {
			num_readings++;
		}
		total_current += (readings[next_reading].current = ADC_GetResult16(0));
		total_voltage += (readings[next_reading].voltage = ADC_GetResult16(1));
		next_reading = (next_reading + 1) % ADC_MOVING_AVERAGE_LENGTH;
		
		ui_event event;
		event.type = UI_EVENT_ADC_READING;
		event.when = xTaskGetTickCountFromISR();
		xQueueSendToBack(ui_queue, &event, 0);
		
		vTaskDelayUntil(&lastWakeTime, configTICK_RATE_HZ / 10);
	}
}

int16 get_raw_current_usage() {
	return total_current / num_readings;
}

int get_current_usage() {
	int ret = (total_current / num_readings - settings->adc_current_offset) * settings->adc_current_gain;
	return (ret < 0)?0:ret;
}

int16 get_raw_voltage() {
	return total_voltage / num_readings;
}

int get_voltage() {
	int ret = (total_voltage / num_readings - settings->adc_voltage_offset) * settings->adc_voltage_gain;
	return (ret < 0)?0:ret;
}

/* [] END OF FILE */