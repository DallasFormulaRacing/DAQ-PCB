/*
* DataAcquisition2.0 — 2023 Vehicle
* (c) 2022 Dallas Formula Racing - Embedded Firmware Team
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

// 3rd Party Libraryes and Frameworks
#include "cmsis_os.h"

#include "fatfs.h"
extern char USBHPath[4];   // USBH logical drive path
extern FATFS USBHFatFS;    // File system object for USBH logical drive
extern FIL USBHFile;       // File object for USBH

#include "usb_host.h"
//extern uint8_t usb_connected_observer; // USB connected/ejected interrupt
extern int to_unmount;
extern int to_log;


// DFR Custom Dependencies
#include "Application/circular_queue.hpp"
#include "Application/data_payload.hpp"
#include "Application/FileSystem/fat_fs.hpp"
#include "Application/Mutex/mutex_cmsisv2.hpp"
#include "Platform/GPIO/igpio.hpp"
#include "Platform/GPIO/gpio_stmf4.hpp"


void RtosInit();
void DataLoggingThread(void *argument);


/**************************************************************
 * 				Toggle Switch Interrupt Callback
 **************************************************************/
std::shared_ptr<platform::GpioStmF4> gpio_callback_ptr(nullptr);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	gpio_callback_ptr->InterruptCallback(GPIO_Pin);
}



void cppMain() {
	RtosInit();

	/*
	 * When `RtosInit()` is enabled, the rest of this function does NOT execute.
	 */

	for(;;){

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
	MX_USB_HOST_Init();

	auto file_system = std::make_shared<application::FatFs>(USBHPath, USBHFatFS, USBHFile);

	auto toggle_switch = std::make_shared<platform::GpioStmF4>(GPIOG, GPIO_PIN_8);
	gpio_callback_ptr = toggle_switch;

	for (;;) {

		if(to_log == 1) {
			file_system->Mount();

			file_system->CreateFile((char*)"please_work.txt\0");
			file_system->OpenFile((char*)"please_work.txt\0", (char*)"a");
			file_system->WriteFile((char*)"Hi there\n");
			file_system->CloseFile();

			to_log = 0;

		} else if (to_unmount == 1) {
			file_system->Unmount();
			to_unmount = 0;
		}

		if (toggle_switch->ToggleDetected()) {

			if(toggle_switch->Read()) {
				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
			} else {
				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);
			}


		}

//		osDelay(500);
//		HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_2);

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
