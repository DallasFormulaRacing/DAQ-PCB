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


// DFR Custom Dependencies
#include "../../Core/Inc/retarget.h"


void cppMain() {
	// Enable `printf()` using USART
	RetargetInit(&huart1);


	for(;;) {
		HAL_Delay(2000);
		HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_2);

	}
}
