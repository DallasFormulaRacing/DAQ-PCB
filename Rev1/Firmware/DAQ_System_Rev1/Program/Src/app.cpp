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


#include "File_Handling.h"

void RtosInit();
void DataLoggingThread(void *argument);

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

//	auto file_system = std::make_shared<application::FatFs>(USBHPath, USBHFatFS, USBHFile);

	for (;;) {

		if(to_log == 1) {
				Mount_USB();

					Check_USB_Details();   // check space details

					Scan_USB("/");   // scan for files and directories

					Create_File("/ROOTFILE.txt");
					Write_File("/ROOTFILE.txt", "This data should be in root file\n");

					Create_Dir("/DIR1");
					Create_File("/DIR1/DIR1FILE.txt");
					Write_File("/DIR1/DIR1FILE.txt", "This data should be in DIR1 file\n");

					Create_Dir("/DIR2");
					Create_Dir("/DIR2/SUBDIR1");
					Create_File("/DIR2/SUBDIR1/DIR2FILE.txt");
					Write_File("/DIR2/SUBDIR1/DIR2FILE.txt", "This data should be in DIR2/SUBDIR1 file\n as i have nothing better to write/n so i just wrote this\n");

					Update_File("/ROOTFILE.txt", "This updated data must be in second line of Root File\n");
					to_log = 0;

		          } else if (to_unmount == 1) {
		        	  Unmount_USB();
		              to_unmount = 0;
		          }

				osDelay(200);
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
