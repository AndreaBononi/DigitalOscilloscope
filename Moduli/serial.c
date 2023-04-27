#include "serial.h"

static volatile int TxState = SER_TX_EMPTY;
static volatile int RxState = SER_RX_EMPTY;


int SER_GetRxState()
{
	return RxState;
}


int SER_GetTxState()
{
	return TxState;
}


int SER_SetRxState(int state)
{
	if((state != SER_RX_EMPTY) && (state != SER_RX_OPCODE_TT) && (state != SER_RX_OPCODE_TL) &&
	   (state != SER_RX_OPCODE_SP) && (state != SER_RX_TT) && (state != SER_RX_TL) && (state != SER_RX_SP))
	{
		return SER_ERR;
	}
	else
	{
		RxState = state;
		return SER_OK;
	}
}


int SER_SetTxState(int state)
{
	if((state != SER_TX_EMPTY) && (state != SER_TX_FULL))
		{
			return SER_ERR;
		}
		else
		{
			TxState = state;
			return SER_OK;
		}
}

/* END OF FILE */
