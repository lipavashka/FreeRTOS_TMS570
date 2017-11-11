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
#include "gio.h"
#include "SFU_Serial.h"

xTaskHandle xTask1Handle, xTask2Handle, xUARTTaskHandle, xUARTEchoHandle;


typedef struct TaskParam_t
{
    char string[32];
    long period;
}TaskParam;

TaskParam xTP1, xTP2, xTP3, xTP4;

volatile unsigned long ulIdleCycleCount = 0;

void vTask1(void *pvParameters)
{
    volatile long ul;
    volatile TaskParam *pxTaskParam;

    pxTaskParam = (TaskParam *) pvParameters;
    serialSendln( (const char*)pxTaskParam->string);

    while (1)
    {
        gioSetBit(gioPORTB, 1, gioGetBit(gioPORTB, 1) ^ 1);
        serialSendln("1\r\n");
        vTaskDelay(1000); // 100ms
    }
}

void vTask2(void *pvParameters)
{
    volatile long ul;
    volatile TaskParam *pxTaskParam;
    portTickType xLastWakeTime;
    char arr[10] ="" ;

    pxTaskParam = (TaskParam *) pvParameters;
    serialSendln( (const char*)pxTaskParam->string);

    xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        gioSetBit(gioPORTB, 2, gioGetBit(gioPORTB, 2) ^ 1);
        serialSendln("Count = ");
        ltoa(ulIdleCycleCount, arr) ;
        serialSendln (&arr[0] );
        serialSendln("\r\n");
        vTaskDelayUntil( &xLastWakeTime, ( 300 / portTICK_RATE_MS ) );
    }
}

void UART_Task(void *pvParameters)
{
    volatile long ul;
    volatile TaskParam *pxTaskParam;

    pxTaskParam = (TaskParam *) pvParameters;
    serialSendln( (const char*)pxTaskParam->string);

    while (1)
    {
        gioSetBit(gioPORTB, 2, gioGetBit(gioPORTB, 2) ^ 1);
        serialSendln("UART\r\n");
        vTaskDelay(5000);
    }
}

void UARTEcho(void *pvParameters)
{
    static char currChar = 0;
    volatile long ul;
    volatile TaskParam *pxTaskParam;

    pxTaskParam = (TaskParam *) pvParameters;
    serialSendln( (const char*)pxTaskParam->string);

    while (1)
    {
        while ((scilinREG->FLR & (uint32) SCI_RX_INT) == 0U)
        {
            vTaskDelay(10);
        }
        currChar = (uint8) (scilinREG->RD & 0x000000FFU);
        scilinREG->RD = 0;
        scilinREG->FLR = ((uint32) SCI_FE_INT | (uint32) SCI_OE_INT  | (uint32) SCI_PE_INT);
        serialSendln(&currChar);
    }
}

void vApplicationIdleHook(void)
{
    ulIdleCycleCount++;
}

int main(void)
{
    strcpy(xTP1.string, "Task 1 is running\r\n");
    xTP1.period = 10000000L;

    strcpy(xTP2.string, "Task 2 is running\r\n");
    xTP2.period = 30000000L;

    strcpy(xTP3.string, "Task 3 is running\r\n");
    strcpy(xTP4.string, "Task 4 is running\r\n");

    _enable_IRQ(); // global interrupt enable
    serialInit(115200); // SFU Serial

    serialSendln("hello!\r\n");

    gioInit();

    if (xTaskCreate(vTask1, "Task1", configMINIMAL_STACK_SIZE, (void*)&xTP1, 1, &xTask1Handle) != pdTRUE)
    {
        serialSendln("Couldn't Create Task1\r\n");
        while (1);
    }
    else
    {
        serialSendln("Created task 1\r\n");
    }
    vTaskDelay(1000);

    if (xTaskCreate(vTask2, "Task2", configMINIMAL_STACK_SIZE, (void*)&xTP2, 2, &xTask2Handle) != pdTRUE)
    {
        serialSendln("Couldn't Create Task2\r\n");
        while (1);
    }
    else
    {
        serialSendln("Created task 2\r\n");
    }
    vTaskDelay(1000);

    if (xTaskCreate(UART_Task, "UART_Task", configMINIMAL_STACK_SIZE, (void*)&xTP3, 3, &xUARTTaskHandle) != pdTRUE)
    {
        serialSendln("Couldn't Create UART Task\r\n");
        while (1);
    }
    else
    {
        serialSendln("Created UART Task\r\n");
    }
    vTaskDelay(1000);

    if (xTaskCreate(UARTEcho, "UART_Echo", configMINIMAL_STACK_SIZE, (void*)&xTP4, 4, &xUARTEchoHandle) != pdTRUE)
    {
        serialSendln("Couldn't Create UART Echo Task\r\n");
        while (1);
    }
    else
    {
        serialSendln("Created UART Echo Task\r\n");
    }
    vTaskStartScheduler();
    vTaskDelay(1000);

    while (1);

    return 0;
}

/* USER CODE BEGIN (4) */
/* USER CODE END */
