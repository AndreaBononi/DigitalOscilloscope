#ifndef INC_DATA_OPERATION_H_
#define INC_DATA_OPERATION_H_

#include "main.h"


// SHIFT A SINISTRA DI UNA POSIZIONE, considerando che gli estremi (primo e ultimo campione) non sono da spostare
void DO_BufferLeftShift(uint8_t *buffer, int dimension);


// SHIFT A DESTRA DI UNA POSIZIONE, considerando che gli estremi (primo e ultimo campione) non sono da spostare
void DO_BufferRightShift(uint8_t *buffer, int dimension);


#endif /* INC_DATA_OPERATION_H_ */
