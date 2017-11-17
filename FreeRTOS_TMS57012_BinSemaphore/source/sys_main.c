/** @file sys_main.c 
 *   @brief Application main file
 *   @date 07-July-2017
 *   @version 04.07.00
 *
 *   This file contains an empty main function,
 *   which can be used for the application.
 */

/* 
 * Copyright (C) 2009-2016 Texas Instruments Incorporated - www.ti.com
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* USER CODE BEGIN (0) */
/* USER CODE END */

/* Include Files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sys_common.h"
#include "FreeRTOS.h"
#include "os_task.h"
#include "os_queue.h"
#include "os_semphr.h"
//#include "os_portasm.asm"
#include "gio.h"
#include "adc.h"
#include "SFU_Serial.h"

xTaskHandle xTask1Handle, xTask2Handle;
xQueueHandle xQueue;

typedef struct TaskParam_t
{
    char string[32];
    long period;
}TaskParam;

TaskParam xTP1, xTP2, xTP3, xTP4;
volatile unsigned long ulIdleCycleCount = 0;


xSemaphoreHandle xBinarySemaphore;


static void vPeriodicTask(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(500 / portTICK_RATE_MS);
        serialSendln("Periodic task - About to generate an interrupt.\r\n");
        //__asm{   int 0x82} /* Сгенерировать прерывание MS-DOS */
        serialSendln("Periodic task - Interrupt generated.\r\n\r\n\r\n");
    }
}

/* Обработчик прерывания */
/*static void __interrupt __far vExampleInterruptHandler( void )
{
    static portBASE_TYPE xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR (xBinarySemaphore, &xHigherPriorityTaskWoken );
    if( xHigherPriorityTaskWoken == pdTRUE )
    {
        portSWITCH_CONTEXT();
    }
}*/

static void vHandlerTask(void *pvParameters)
{

    for (;;)
    {

        xSemaphoreTake(xBinarySemaphore, portMAX_DELAY);

        serialSendln("Handler task - Processing event.\r\n");
    }
}
void vApplicationIdleHook(void)
{
    ulIdleCycleCount++;
}


int main(void)
{
    unsigned char command[8];
    char arr[10] = "";
    long heapSize;

    _enable_IRQ();
    serialInit(115200);

    serialSendln("\r\nBinary Semaphore example running!\r\n");

    gioInit();
    // adcInit();
    // adcStartConversion(adcREG1, adcGROUP0);

    adcData_t adc_data; //ADC Data Structure
    adcData_t *adc_data_ptr = &adc_data; //ADC Data Pointer
    volatile unsigned int NumberOfChars, value; //Declare variables

    adcInit(); //Initializes the ADC module
    while (1)
    {
        adcStartConversion(adcREG1, 1U); //Start ADC conversion
        while (!adcIsConversionComplete(adcREG1, 1U))
            ; //Wait for ADC conversion
        adcGetData(adcREG1, 1U, adc_data_ptr); //Store conversion into ADC pointer
        value = (unsigned int) adc_data_ptr->value;
        NumberOfChars = ltoa(value, (char *) command);
        serialSendln("Adc = "); ltoa(value, arr); serialSendln(&arr[0]); serialSendln("\r\n");
    }

    vSemaphoreCreateBinary(xBinarySemaphore);
    //_dos_setvect(0x82, vExampleInterruptHandler);

    if (xBinarySemaphore != NULL)
    {

        if ((xTaskCreate(vHandlerTask, "Handler", configMINIMAL_STACK_SIZE, NULL, 3, NULL)) != pdTRUE)
        {
            serialSendln("Couldn't Create vHandlerTask\r\n");
            while (1);
        }
        else
        {
            serialSendln("Created vHandlerTask\r\n");
        }

        if ((xTaskCreate(vPeriodicTask, "Periodic", configMINIMAL_STACK_SIZE, NULL, 1, NULL)) != pdTRUE)
        {
            serialSendln("Couldn't Create vPeriodicTask\r\n");
            while (1);
        }
        else
        {
            serialSendln("Created vPeriodicTask\r\n");
        }

        heapSize = xPortGetFreeHeapSize();
        serialSendln("Heap Size = ");
        ltoa(heapSize, arr);
        serialSendln(&arr[0]);
        serialSendln("\r\n");

        vTaskStartScheduler();
    }

    while (1);

    // return 0;
}

