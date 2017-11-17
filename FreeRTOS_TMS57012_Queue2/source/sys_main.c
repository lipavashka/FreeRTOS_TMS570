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
#include "gio.h"
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

/* Номера функций-передатчиков сообщений */
#define mainSENDER_1 1
#define mainSENDER_2 2

typedef struct
{
    unsigned char ucValue;
    unsigned char ucSource;
} xData;

static const xData xStructsToSend[2] =
{
  {
    100, mainSENDER_1
  }, /* Используется задачей-передатчиком 1 */

  {
    200, mainSENDER_2
  } /* Используется задачей-передатчиком 2 */
};

void vSenderTask(void *pvParameters)
{

    long lValueToSend;

    /* xQueueSendToBack() */
    portBASE_TYPE xStatus;

    lValueToSend = (long) pvParameters;

    while (1)
    {
        vTaskDelay(300); // 300ms
        xStatus = xQueueSendToBack(xQueue, &lValueToSend, 0);
        if (xStatus != pdPASS)
        {
            serialSendln("Could not send to the queue.\r\n");
        }
        taskYIELD();
    }
}

void vReceiverTask(void *pvParameters)
{

    long lReceivedValue;
    char _arr[10] = "";
    portBASE_TYPE xStatus;

    while (1)
    {

        lReceivedValue = uxQueueMessagesWaiting(xQueue);
        if (lReceivedValue > 0)
        {
            //serialSendln("Queue should have been empty!\r\n");
            serialSendln("Queue = "); ltoa(lReceivedValue, _arr); serialSendln(&_arr[0]); serialSendln(" element(s)\r\n");
        }

        xStatus = xQueueReceive(xQueue, &lReceivedValue, 310 /portTICK_RATE_MS);
        if (xStatus == pdPASS)
        {
            // printf("Received = %ld\r\n", lReceivedValue);
            serialSendln("Received = "); ltoa(lReceivedValue, _arr); serialSendln(&_arr[0]); serialSendln("\r\n");
        }
        else
        {
            serialSendln("Could not receive from the queue.\r\n");
        }
    }
}


void vApplicationIdleHook(void)
{
    ulIdleCycleCount++;
}


int main(void)
{
    char arr[10] ="" ;
    long heapSize;

    _enable_IRQ();
    serialInit(115200);

    serialSendln("\r\nQueue example running!\r\n");

    gioInit();

    xQueue = xQueueCreate(5, sizeof(long));

    if (xQueue != NULL)
    {
        xTaskCreate(vSenderTask, "Sender1", configMINIMAL_STACK_SIZE, (void *) 100, 1, NULL);
        xTaskCreate(vSenderTask, "Sender2", configMINIMAL_STACK_SIZE, (void *) 200, 1, NULL);
        xTaskCreate(vSenderTask, "Sender3", configMINIMAL_STACK_SIZE, (void *) 300, 1, NULL);
        xTaskCreate(vSenderTask, "Sender4", configMINIMAL_STACK_SIZE, (void *) 400, 1, NULL);
        xTaskCreate(vSenderTask, "Sender5", configMINIMAL_STACK_SIZE, (void *) 500, 1, NULL);
        xTaskCreate(vSenderTask, "Sender6", configMINIMAL_STACK_SIZE, (void *) 600, 1, NULL);
        xTaskCreate(vSenderTask, "Sender7", configMINIMAL_STACK_SIZE, (void *) 700, 1, NULL);

        xTaskCreate(vReceiverTask, "Receiver", configMINIMAL_STACK_SIZE, NULL, 2, NULL);

        heapSize = xPortGetFreeHeapSize();
        serialSendln("Heap Size = "); ltoa(heapSize, arr); serialSendln(&arr[0]); serialSendln("\r\n");

        vTaskStartScheduler();
    }
    else
    {
        serialSendln("Couldn't Create Queue\r\n");
    }


    while (1);

    // return 0;
}

