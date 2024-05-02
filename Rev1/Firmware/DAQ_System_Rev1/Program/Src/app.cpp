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

#include "tim.h"
extern TIM_HandleTypeDef htim7;

// 3rd Party Libraryes and Frameworks
#include "cmsis_os.h"

#include "fatfs.h"
extern char USBHPath[4];   // USBH logical drive path
extern FATFS USBHFatFS;    // File system object for USBH logical drive
extern FIL USBHFile;       // File object for USBH

#include "usb_host.h"
extern uint8_t usb_connected_observer; // USB connected/ejected interrupt


// DFR Custom Dependencies
#include "Application/circular_queue.hpp"
#include "Application/data_payload.hpp"
#include "Application/DataLogger/DataLogger.hpp"
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
		HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_2);
	}
}




/**************************************************************
 * 						RTOS Mutexes
 **************************************************************/
const osMutexAttr_t queue_mutex_attributes = {
  "myThreadMutex",
  osMutexRecursive | osMutexPrioInherit,
  NULL,
  0U
};

const osMutexAttr_t data_mutex_attributes = {
  "myThreadMutex",
  osMutexRecursive | osMutexPrioInherit,
  NULL,
  0U
};

auto queue_mutex = std::make_shared<application::MutexCmsisV2>(queue_mutex_attributes);
auto data_mutex = std::make_shared<application::MutexCmsisV2>(data_mutex_attributes);



/**************************************************************
 * 					Shared Components
 **************************************************************/
static constexpr uint8_t kQueueSize = 20;
application::CircularQueue<application::DataPayload> queue(kQueueSize, queue_mutex);

application::DataPayload data_payload(data_mutex);

bool is_logging_flag = false;


/**************************************************************
 * 					RTOS Thread Properties
 **************************************************************/
osThreadId_t dataLoggingTaskHandle;
const osThreadAttr_t dataLoggingTask_attributes = {
  .name = "dataLoggingTask",
  .stack_size = 128 * 20,
  .priority = (osPriority_t) osPriorityHigh,
};

uint32_t timestamp_thread_flag = 0x00000001U;
osThreadId_t timestampTaskHandle;
const osThreadAttr_t timestampTask_attributes = {
  .name = "timestampTask",
  .stack_size = 128 * 8,
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

	application::DataLogger data_logger(file_system, toggle_switch, queue, usb_connected_observer, is_logging_flag);

	for (;;) {
		data_logger.Run();
		osDelay(1000);
	}
}

void TimestampThread(void *argument) {
	int count = 0;
	static constexpr float kTimeDuration = 2.0f; // seconds

	for(;;) {
		osThreadFlagsWait(timestamp_thread_flag, osFlagsWaitAny, osWaitForever);
		HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_2);
		if (is_logging_flag) {
			count++;

			data_payload.Lock();
			data_payload.timestamp_ = count * kTimeDuration;
			printf("Time: %f seconds\n", data_payload.timestamp_);


			queue.Lock();
			if(queue.IsFull()) {
				printf("Queue is full! Data samples are being dropped...\n");
			}
			queue.Enqueue(data_payload);
			queue.Unlock();
			data_payload.Unlock();
		}
		else {
			count = 0;
		}
	}
}


void RtosInit() {
	NVIC_SetPriorityGrouping( 0 );	// For allowing hardware (not RTOS/software) interrupts while the Kernel is running
	osKernelInitialize(); 			// Initialize scheduler

	// Threads
	dataLoggingTaskHandle = osThreadNew(DataLoggingThread, NULL, &dataLoggingTask_attributes);
	timestampTaskHandle = osThreadNew(TimestampThread, NULL, &timestampTask_attributes);
//	ecuTaskHandle = osThreadNew(EcuThread, NULL, &ecuTask_attributes);

	// Mutexes
	queue_mutex->Create();
	data_mutex->Create();

	// Hardware Timers
	HAL_TIM_Base_Start_IT(&htim7);

	osKernelStart(); 				// Start scheduler
}
