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
#include "gio.h"
#include "adc.h"
#include "SFU_Serial.h"


volatile unsigned long ulIdleCycleCount = 0;

xSemaphoreHandle xBinarySemaphore;

/* Дескрипторы очередей */
xQueueHandle xIntegerQueue;
xQueueHandle xAdcQueue;

static void vIntegerGenerator(void *pvParameters)
{
    portTickType xLastExecutionTime;
    unsigned portLONG ulValueToSend = 0;
    int i;

    xLastExecutionTime = xTaskGetTickCount();
    while (1)
    {
        vTaskDelayUntil(&xLastExecutionTime, 100);
        for (i = 0; i < 3; i++)
        {
            xQueueSendToBack(xIntegerQueue, &ulValueToSend, 0);
            ulValueToSend++;
        }
        serialSendln("\r\nGenerator task - About to generate an interrupt\r\n");
        adcStartConversion(adcREG1, 1U);  /* Эта инструкция сгенерирует прерывание. */
        serialSendln("Generator task - Interrupt generated\r\n");
        gioSetBit(gioPORTB, 1, gioGetBit(gioPORTB, 1) ^ 1);
    }
}

static void vStringPrinter(void *pvParameters)
{
    long  AdcValue;
    char _arr[10] = "";

    while (1)
    {
        xQueueReceive(xAdcQueue, &AdcValue, portMAX_DELAY);
        serialSendln("ADC Data = "); ltoa(AdcValue, _arr); serialSendln(&_arr[0]); serialSendln("\r\n");
        // taskENTER_CRITICAL();
        vTaskSuspendAll();
        gioSetBit(gioPORTB, 2, gioGetBit(gioPORTB, 2) ^ 1);
        // taskEXIT_CRITICAL();
        xTaskResumeAll();
    }
}


void vApplicationIdleHook(void)
{
    ulIdleCycleCount++;
}

int main(void)
{
    char arr[10] = "";
    long heapSize;

    _enable_IRQ();
    serialInit(115200);

    serialSendln("\r\n\r\nCounting Semaphore example running!\r\n");

    gioInit();

    adcInit();


    xIntegerQueue = xQueueCreate(10, sizeof(unsigned long));
    xAdcQueue = xQueueCreate(10, sizeof(int *));

    if ((xTaskCreate(vIntegerGenerator, "IntGen", configMINIMAL_STACK_SIZE, NULL, 1, NULL)) != pdTRUE)
    {
        serialSendln("Couldn't Create vIntegerGenerator\r\n");
        while (1);
    }
    else
    {
        serialSendln("Created IntGen\r\n");
    }

    if ((xTaskCreate(vStringPrinter, "Printer", configMINIMAL_STACK_SIZE, NULL, 2, NULL)) != pdTRUE)
    {
        serialSendln("Couldn't Create vStringPrinter\r\n");
        while (1);
    }
    else
    {
        serialSendln("Created vStringPrinter\r\n");
    }

    heapSize = xPortGetFreeHeapSize();
    serialSendln("Heap Size = "); ltoa(heapSize, arr); serialSendln(&arr[0]);serialSendln("\r\n");

    vTaskStartScheduler();


    while (1);

    // return 0;
}

