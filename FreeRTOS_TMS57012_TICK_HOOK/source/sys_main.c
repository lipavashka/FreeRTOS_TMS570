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

// Mutex Descriptor
volatile xSemaphoreHandle xMutex;
xSemaphoreHandle xRecursiveMutex;



static void prvPrintTask(void *pvParameters);

static void prvStdioGatekeeperTask(void *pvParameters);

static char *pcStringsToPrint[] = {
        "Task 1 ****************************************************\r\n",
        "Task 2 ----------------------------------------------------\r\n",
        "Message printed from the tick hook interrupt ##############\r\n" };


xQueueHandle xPrintQueue;

void vApplicationIdleHook(void)
{
    ulIdleCycleCount++;
}


static void prvStdioGatekeeperTask(void *pvParameters)
{
    char *pcMessageToPrint;
    while (1)
    {
        xQueueReceive(xPrintQueue, &pcMessageToPrint, portMAX_DELAY);
        serialSendln("Inherited priority = "); serialSendln(pcMessageToPrint);  serialSendln("\r\n");
    }
}

void vApplicationTickHook(void)
{
    static int iCount = 0;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    iCount++;
    if (iCount >= 200)
    {
        xQueueSendToFrontFromISR(xPrintQueue, &(pcStringsToPrint[2]), &xHigherPriorityTaskWoken);
        iCount = 0;
    }
}

static void prvPrintTask(void *pvParameters)
{
    int iIndexToString;

    iIndexToString = (int) pvParameters;
    while (1)
    {
        xQueueSendToBack(xPrintQueue, &(pcStringsToPrint[iIndexToString]), 0);
        vTaskDelay((rand() % 500));
    }
}

int main(void)
{
    char arr[10] = "";
    long heapSize;

    _enable_IRQ();
    serialInit(115200);

    serialSendln("\r\n\r\n Gatekeeper example running!\r\n");

    gioInit();


///////////////////////////////////////////////////////////////////////
    xPrintQueue = xQueueCreate(5, sizeof(char *));

    if (xPrintQueue != NULL) {

        if ((xTaskCreate(prvPrintTask, "Print1", configMINIMAL_STACK_SIZE, 0, 1, NULL)) != pdTRUE)
        {
            serialSendln("Couldn't Create prvPrintTask1\r\n");
            while (1);
        }
        else
        {
            serialSendln("Created prvPrintTask1\r\n");
        }
        if ((xTaskCreate(prvPrintTask, "Print2", configMINIMAL_STACK_SIZE, 0, 2, NULL)) != pdTRUE)
        {
            serialSendln("Couldn't Create prvPrintTask2\r\n");
            while (1);
        }
        else
        {
            serialSendln("Created prvPrintTask2\r\n");
        }
    if ((xTaskCreate(prvStdioGatekeeperTask, "Gatekeeper", configMINIMAL_STACK_SIZE, NULL, 0, NULL)) != pdTRUE)
    {
        serialSendln("Couldn't Create prvStdioGatekeeperTask\r\n");
        while (1);
    }
    else
    {
        serialSendln("Created prvStdioGatekeeperTask\r\n");
    }

    heapSize = xPortGetFreeHeapSize(); serialSendln("Heap Size = "); ltoa(heapSize, arr); serialSendln(&arr[0]); serialSendln("\r\n");

    vTaskStartScheduler();
    }
    return 1;
}

