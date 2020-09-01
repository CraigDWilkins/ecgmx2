#pragma once
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void ecgMain(void);
EXTERNC void ecgInit(SPI_HandleTypeDef *handle, UART_HandleTypeDef *huart2);

#define PrintText(text) {char buffer[] = (text);HAL_UART_Transmit_DMA(phuart2, (uint8_t *)buffer, sizeof(buffer));}

#define PrintTextInt(text,integer) {char buffer[] = (text);\
HAL_UART_Transmit_DMA(phuart2, (uint8_t *)buffer, sizeof(buffer));\
char buf[10];itoa((integer),buf,10);\
HAL_UART_Transmit_DMA(phuart2, (uint8_t *)buf, strlen(buf));}

#define PrintInt(integer) {char buf[10];itoa((integer),buf,10);HAL_UART_Transmit_DMA(phuart2, (uint8_t *)buf, strlen(buf));}

#define PrintC(c) HAL_UART_Transmit_DMA(phuart2, (uint8_t *)&(c), sizeof((c)))

//extern UART_HandleTypeDef *phuart2;

#define ev2V8Hi() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET)
#define ev2V8Lo() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET)
#define ev3V3Hi() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET)
#define ev3V3Lo() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET)
#define LED1Hi() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)
#define LED1Lo() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET)
#define LED2Hi() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET)
#define LED2Lo() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET)