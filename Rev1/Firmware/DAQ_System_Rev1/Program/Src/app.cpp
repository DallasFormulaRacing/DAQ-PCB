/*
* Data Acquisition (DAQ) System — Revision 1
* (c) 2024 Dallas Formula Racing - Embedded Firmware Team
* Formula SAE International Collegiate Chapter
* GPL-3.0 License
*/


#include "app.hpp"

// Standard Libraries
#include <cstdio>
#include <cinttypes>

#include <memory>
#include <vector>

// ST HAL Dependencies
#include "gpio.h"

#include "usart.h"
extern UART_HandleTypeDef huart1;


// 3rd Party Libraries and Frameworks
#include "cmsis_os.h"


// DFR Custom Dependencies
#include "../../Core/Inc/retarget.h"


void RtosInit();
void DataLoggingThread(void *argument);


void cppMain() {
	// Enable `printf()` using USART
	RetargetInit(&huart1);

	RtosInit();
	/*
	 * When `RtosInit()` is enabled, the rest of this function does NOT execute.
	 */


	for(;;) {
		HAL_Delay(2000);
		HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_2);

	}
}


/**************************************************************
 * 					RTOS Thread Properties
 **************************************************************/
osThreadId_t dataLoggingTaskHandle;
const osThreadAttr_t dataLoggingTask_attributes = {
  .name = "dataLoggingTask",
  .stack_size = 128 * 20,
  .priority = (osPriority_t) osPriorityHigh,
};


/**************************************************************
 * 						RTOS Threads
 **************************************************************/
void DataLoggingThread(void *argument) {
	for (;;) {
		HAL_Delay(2000);
		HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_2);
	}
}


void RtosInit() {
	NVIC_SetPriorityGrouping( 0 );	// For allowing hardware (not RTOS/software) interrupts while the Kernel is running
	osKernelInitialize(); 			// Initialize scheduler

	// Threads
	dataLoggingTaskHandle = osThreadNew(DataLoggingThread, NULL, &dataLoggingTask_attributes);
//	timestampTaskHandle = osThreadNew(TimestampThread, NULL, &timestampTask_attributes);
//	ecuTaskHandle = osThreadNew(EcuThread, NULL, &ecuTask_attributes);

	// Mutexes
//	queue_mutex->Create();
//	data_mutex->Create();

	// Hardware Timers
//	HAL_TIM_Base_Start_IT(&htim7);

	osKernelStart(); 				// Start scheduler
}
