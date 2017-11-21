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
#include "os_timer.h"
#include "gio.h"
#include "adc.h"
#include "SFU_Serial.h"


volatile unsigned long ulIdleCycleCount = 0;

xSemaphoreHandle xBinarySemaphore;

/* Дескрипторы очередей */
xQueueHandle xIntegerQueue;
xQueueHandle xAdcQueue;


#define NUMBER_OF_TIMERS 3
#define ID_TIMER_1 111
#define ID_TIMER_2 222
#define ID_TIMER_3 333

xTimerHandle xAutoReloadTimer;
xTimerHandle xOneShotTimers[NUMBER_OF_TIMERS];
const unsigned portBASE_TYPE uxOneShotTimersIDs [NUMBER_OF_TIMERS] = { ID_TIMER_1, ID_TIMER_2, ID_TIMER_3 };
unsigned int uiAutoReloadTimerPeriod = 1000 / portTICK_RATE_MS;
void vAutoReloadTimerFunction(xTimerHandle xTimer)
{
    char _arr[10] = "";
    serialSendln("AutoReload timer. Time = ");
    ltoa((xTaskGetTickCount() / configTICK_RATE_HZ), _arr);
    serialSendln(&_arr[0]);
    serialSendln("\r\n");
    uiAutoReloadTimerPeriod += 1000 / portTICK_RATE_MS;
    xTimerChangePeriod(xTimer, uiAutoReloadTimerPeriod, 0);
}

void vOneShotTimersFunctio_n(xTimerHandle xTimer)
{
    char _arr[10] = "";
    unsigned portBASE_TYPE *pxTimerID;

    pxTimerID = pvTimerGetTimerID(xTimer);

    switch (*pxTimerID)
    {
    case ID_TIMER_1:
        serialSendln("\t\t\t\tOneShot timer ID = "); ltoa(*pxTimerID, _arr); serialSendln(&_arr[0]); serialSendln(" "); ltoa((xTaskGetTickCount() / configTICK_RATE_HZ), _arr); serialSendln(&_arr[0]); serialSendln(" sec\r\n");
        xTimerStart(xOneShotTimers[1], 0);
        break;
    case ID_TIMER_2:
        serialSendln("\t\t\t\tOneShot timer ID = "); ltoa(*pxTimerID, _arr); serialSendln(&_arr[0]); serialSendln(" "); ltoa((xTaskGetTickCount() / configTICK_RATE_HZ), _arr); serialSendln(&_arr[0]); serialSendln(" sec\r\n");
        xTimerStart(xOneShotTimers[2], 0);
        break;
    case ID_TIMER_3:
        serialSendln("\t\t\t\tOneShot timer ID = "); ltoa(*pxTimerID, _arr); serialSendln(&_arr[0]); serialSendln(" "); ltoa((xTaskGetTickCount() / configTICK_RATE_HZ), _arr); serialSendln(&_arr[0]); serialSendln(" sec\r\n");
        serialSendln("\n\r\t\t\t\tAbout to delete AutoReload timer!");
        xTimerDelete(xAutoReloadTimer, 0);
        break;
    }
}
/*-----------------------------------------------------------*/
void vOneShotTimersFunction(xTimerHandle xTimer)
{
    char _arr[10] = "";
    unsigned portBASE_TYPE *pxTimerID;

    pxTimerID = pvTimerGetTimerID(xTimer);
    if (xTimer == xOneShotTimers[0])
    {
        serialSendln("\t\t\t\tOneShot timer ID = "); ltoa(*pxTimerID, _arr); serialSendln(&_arr[0]); serialSendln(" "); ltoa((xTaskGetTickCount() / configTICK_RATE_HZ), _arr); serialSendln(&_arr[0]); serialSendln(" sec\r\n");
        xTimerChangePeriod(xAutoReloadTimer, 6000, 0);
        xTimerStart(xOneShotTimers[1], 0);
    }
    else if (xTimer == xOneShotTimers[1])
    {
        serialSendln("\t\t\t\tOneShot timer ID = "); ltoa(*pxTimerID, _arr); serialSendln(&_arr[0]); serialSendln(" "); ltoa((xTaskGetTickCount() / configTICK_RATE_HZ), _arr); serialSendln(&_arr[0]); serialSendln(" sec\r\n"); xTimerStart(xOneShotTimers[2], 0);
    }
    else if (xTimer == xOneShotTimers[2])
    {
        serialSendln("\t\t\t\tOneShot timer ID = "); ltoa(*pxTimerID, _arr); serialSendln(&_arr[0]); serialSendln(" "); ltoa((xTaskGetTickCount() / configTICK_RATE_HZ), _arr); serialSendln(&_arr[0]); serialSendln(" sec\r\n");
        serialSendln("\n\r\t\t\t\tAbout to delete AutoReload timer!");
        xTimerDelete(xAutoReloadTimer, 0);
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

    unsigned portBASE_TYPE i;

    _enable_IRQ();
    serialInit(115200);

    serialSendln("\r\n\r\n Timer example running!\r\n");

    gioInit();

    //adcInit();

    xAutoReloadTimer = xTimerCreate("AutoReloadTimer", uiAutoReloadTimerPeriod, pdTRUE, 0,vAutoReloadTimerFunction);

    xTimerReset(xAutoReloadTimer, 0);

    for (i = 0; i < NUMBER_OF_TIMERS; i++)
    {
        xOneShotTimers[i] = xTimerCreate("OneShotTimer_n", 12000 / portTICK_RATE_MS, pdFALSE, (void*) &uxOneShotTimersIDs[i], vOneShotTimersFunction);
    }
    xTimerReset(xOneShotTimers[0], 0);
    serialSendln("Timers start! Time =  "); ltoa((xTaskGetTickCount() / configTICK_RATE_HZ), arr); serialSendln(&arr[0]);  serialSendln(" sec\n\r\n\r");

    heapSize = xPortGetFreeHeapSize();
    serialSendln("Heap Size = "); ltoa(heapSize, arr); serialSendln(&arr[0]);serialSendln("\r\n");

    vTaskStartScheduler();

    return 1;

}

