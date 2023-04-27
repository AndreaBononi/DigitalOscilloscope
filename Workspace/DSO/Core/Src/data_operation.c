#include "data_operation.h"

// I valori in posizione '0' e in posizione 'dimension' NON VENGONO TOCCATI, in quanto contengono i caratteri che delimitano il frame


void DO_BufferLeftShift(uint8_t *buffer, int dimension)
{
	int i = 0;
	int box = buffer[1];						// salviamo il primo campione in una variabile temporanea
	for (i = 2; i < dimension - 1; i++)
	{
		buffer[i-1] = buffer[i];				// spostiamo ogni campione a sinistra di una posizione, partendo dall'inizio
	}
	buffer[dimension - 2] = box;				// salviamo il primo campione in ultima posizione
}


void DO_BufferRightShift(uint8_t *buffer, int dimension)
{
	int j = 0;
	int box = buffer[dimension - 2];			// salviamo l'ultimo campione in una variabile temporanea
	for(j = (dimension - 3); j > 0; j--)
	{
		buffer[j+1] = buffer[j];				// spostiamo ogni campione a destra di una posizione, partendo dal fondo
	}
	buffer[1] = box;							// salviamo l'ultimo campione in prima posizione
}


/* END OF FILE */
