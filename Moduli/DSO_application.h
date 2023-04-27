#ifndef INC_DSO_APPLICATION_H_
#define INC_DSO_APPLICATION_H_

#include "hwHandles.h"

#define BUFFER_DIMENSION 1024
#define OPCODE_DIMENSION 2
#define TT_DIMENSION 1
#define TL_DIMENSION 1
#define SP_DIMENSION 4

#define AUTO 0
#define NORMAL 1
#define SINGLE 2
#define STOP 3

#define TRUE 0
#define FALSE 1


// Codice utente da inserire nel loop:
void DSO_UserCode();

// Funzione per far partire la ricezione dei comandi da seriale
void DSO_StartRX();

// Usart TX callback
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

// Usart RX callback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

// Usart error callback
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);

// ADC end of regular channel conversion callback
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);

// ADC end of injected channel conversion callback
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc);

// ADC error callback
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc);


#endif /* INC_DSO_APPLICATION_H_ */

/* END OF FILE */
