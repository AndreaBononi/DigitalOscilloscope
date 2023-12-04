/* DESCRIPTION --------------------------------------------------------------------------------------------------------
 * 
 * Funzioni di shift dei byte contenuti in un buffer di dimensione arbitraria.
 * 
 * ELENCO FUNZIONI:
 * -- void DO_BufferLeftShift  ( uint8_t *buffer, int dimension );
 * -- void DO_BufferRightShift ( uint8_t *buffer, int dimension );
 * 
 * ----------------------------------------------------------------------------------------------------------------- */


#ifndef INC_DATA_OPERATION_H_
#define INC_DATA_OPERATION_H_

#include "main.h"


void DO_BufferLeftShift( uint8_t *buffer, int dimension );
/* --------------------------------------------------------------------------------------------------------------------
 * DESCRIZIONE: shift a sinistra di una posizione di tutti i byte, esclusi quelli in prima ed ultima posizione
 * PARAMETRI: 
 * 	# *buffer: puntatore alla locazione del primo elemento del buffer
 *   # dimension: dimensione del buffer
 * ----------------------------------------------------------------------------------------------------------------- */


void DO_BufferRightShift( uint8_t *buffer, int dimension );
/* --------------------------------------------------------------------------------------------------------------------
 * DESCRIZIONE: shift a destra di una posizione di tutti i byte, esclusi quelli in prima ed ultima posizione
 * PARAMETRI: 
 * 	# *buffer: puntatore alla locazione del primo elemento del buffer
 *   # dimension: dimensione del buffer
 * ----------------------------------------------------------------------------------------------------------------- */


#endif /* INC_DATA_OPERATION_H_ */