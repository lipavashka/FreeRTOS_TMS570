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
#include "os_croutine.h"


xSemaphoreHandle xBinarySemaphore;

xQueueHandle xIntegerQueue;
xQueueHandle xAdcQueue;


void vCoRoutine1( xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex)
{
    static long i;
    crSTART(xHandle);
    while (1)
    {
        serialSendln("Co-routine #1 runs!\r\n");
        for (i = 0; i < 5000000; i++);
        crDELAY(xHandle, 0);
    }
    crEND();
}

void vCoRoutine2( xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex)
{
    static long i;
    crSTART( xHandle );
    while (1)
    {
        serialSendln("Co-routine #2 runs!\r\n");
        for (i = 0; i < 5000000; i++);
        crDELAY(xHandle, 0);
    }
    crEND();
}

void vApplicationIdleHook(void)
{
    while (1)
    {
        vCoRoutineSchedule();
    }
}

int main(void)
{
    char arr[10] = "";
    long heapSize;

    //_enable_IRQ();
    serialInit(115200);

    serialSendln("\r\n\r\n CoRoutine example running!\r\n");

    gioInit();


    if (xCoRoutineCreate(vCoRoutine1, 1, 0) != pdTRUE)
    {
        serialSendln("Couldn't Create vCoRoutine1\r\n");
        while (1);
    }
    else
    {
        serialSendln("Created vCoRoutine1\r\n");
    }
    if (xCoRoutineCreate(vCoRoutine2, 1, 0) != pdTRUE)
    {
        serialSendln("Couldn't Create vCoRoutine2\r\n");
        while (1);
    }
    else
    {
        serialSendln("Created vCoRoutine2\r\n");
    }

    heapSize = xPortGetFreeHeapSize(); serialSendln("Heap Size = "); ltoa(heapSize, arr); serialSendln(&arr[0]); serialSendln("\r\n");
    vTaskStartScheduler();

    return 1;
}

