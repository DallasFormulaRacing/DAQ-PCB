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



void cppMain() {
	// Enable `printf()` using USART
//	RetargetInit(&huart1);


	for(;;) {
		HAL_Delay(2000);
		HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_2);

	}
}
