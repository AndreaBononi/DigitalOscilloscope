/* DESCRIPTION --------------------------------------------------------------------------------------
 * 
 * DSO application header file.
 * 
 * ELENCO FUNZIONI:
 * -- void DSO_UserCode                         ( );
 * -- void DSO_StartRX                          ( );
 * -- void HAL_UART_TxCpltCallback              ( UART_HandleTypeDef *huart );
 * -- void HAL_UART_RxCpltCallback              ( UART_HandleTypeDef *huart );
 * -- void HAL_UART_ErrorCallback               ( UART_HandleTypeDef *huart );
 * -- void HAL_ADC_ConvCpltCallback             ( ADC_HandleTypeDef* hadc );
 * -- void HAL_ADCEx_InjectedConvCpltCallback   ( ADC_HandleTypeDef* hadc );
 * -- void HAL_ADC_ErrorCallback                ( ADC_HandleTypeDef *hadc );
 *
 * ----------------------------------------------------------------------------------------------- */


#ifndef INC_DSO_APPLICATION_H_
#define INC_DSO_APPLICATION_H_

#include "hwHandles.h"

#define BUFFER_DIMENSION  1024
#define OPCODE_DIMENSION  2
#define TT_DIMENSION      1
#define TL_DIMENSION      1
#define SP_DIMENSION      4
#define AUTO              0
#define NORMAL            1
#define SINGLE            2
#define STOP              3
#define TRUE              0
#define FALSE             1


void DSO_UserCode();
/* -------------------------------------------------------------------------------------------------
 * DESCRIZIONE: codice utente da inserire nel loop del main
 * ---------------------------------------------------------------------------------------------- */


void DSO_StartRX();
/* -------------------------------------------------------------------------------------------------
 * DESCRIZIONE: funzione per avviare la ricezione dei comandi da seriale
 * ---------------------------------------------------------------------------------------------- */


void HAL_UART_TxCpltCallback( UART_HandleTypeDef *huart );
/* -------------------------------------------------------------------------------------------------
 * DESCRIZIONE: usart TX callback
 * ---------------------------------------------------------------------------------------------- */


void HAL_UART_RxCpltCallback( UART_HandleTypeDef *huart );
/* -------------------------------------------------------------------------------------------------
 * DESCRIZIONE: usart RX callback
 * ---------------------------------------------------------------------------------------------- */


void HAL_UART_ErrorCallback( UART_HandleTypeDef *huart );
/* -------------------------------------------------------------------------------------------------
 * DESCRIZIONE: usart error callback
 * ---------------------------------------------------------------------------------------------- */


void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef* hadc );
/* -------------------------------------------------------------------------------------------------
 * DESCRIZIONE: ADC end of regular channel conversion callback
 * ---------------------------------------------------------------------------------------------- */


void HAL_ADCEx_InjectedConvCpltCallback( ADC_HandleTypeDef* hadc );
/* -------------------------------------------------------------------------------------------------
 * DESCRIZIONE: ADC end of injected channel conversion callback
 * ---------------------------------------------------------------------------------------------- */


void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc );
/* -------------------------------------------------------------------------------------------------
 * DESCRIZIONE: ADC error callback
 * ---------------------------------------------------------------------------------------------- */


#endif /* INC_DSO_APPLICATION_H_ */