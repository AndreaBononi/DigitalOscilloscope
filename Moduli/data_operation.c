/* DESCRIPTION --------------------------------------------------------------------------------------------------------
 * 
 * Funzioni di shift dei byte contenuti in un buffer di dimensione arbitraria.
 * 
 * ELENCO FUNZIONI:
 * -- void DO_BufferLeftShift  ( uint8_t *buffer, int dimension );
 * -- void DO_BufferRightShift ( uint8_t *buffer, int dimension );
 * 
 * ----------------------------------------------------------------------------------------------------------------- */


#include "data_operation.h"


/* ----------------------------------------------------------------------------------------------------------------- */

void DO_BufferLeftShift( uint8_t *buffer, int dimension ) {
	int i = 2;
	int box = buffer[1];						
	for ( i = 2; i < dimension - 1; i++ ) {
		buffer[i-1] = buffer[i];				
	}
	buffer[dimension - 2] = box;
}

/* ----------------------------------------------------------------------------------------------------------------- */

void DO_BufferRightShift( uint8_t *buffer, int dimension )
{
	int j = 0;
	int box = buffer[dimension - 2];
	for ( j = (dimension - 3); j > 0; j-- ) {
		buffer[j+1] = buffer[j];
	}
	buffer[1] = box;
}

/* ----------------------------------------------------------------------------------------------------------------- */