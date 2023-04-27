#include "data_events.h"


static volatile int enabling_flag = DE_TRIGGERDISABLED;
static volatile int trigger_flag = DE_UNDETECTED;
static volatile int mainBuffer1_flag = DE_EMPTYBUFFER;
static volatile int mainBuffer2_flag = DE_EMPTYBUFFER;
static volatile int activeBuff = DE_ADCBUFFER1;
static volatile int buffer1_blockStatus = DE_INITIALBLOCK;
static volatile int buffer2_blockStatus = DE_STARTBLOCK;


void DE_SystemEnable()
{
	enabling_flag = DE_TRIGGERENABLED;
}


void DE_SystemDisable()
{
	enabling_flag = DE_TRIGGERDISABLED;
}


int DE_GetSystemStatus()
{
	return enabling_flag;
}


void DE_SetTriggerAsDetected()
{
	trigger_flag = DE_DETECTED;
}


int DE_GetTriggerStatus()
{
	return trigger_flag;
}


void DE_SetTriggerAsUndetected()
{
	trigger_flag = DE_UNDETECTED;
}


int DE_SetBufferAsFull(int buffer)
{
	if(buffer == DE_ADCBUFFER1)
	{
		mainBuffer1_flag = DE_FULLBUFFER;
		return DE_OK;
	}
	else if (buffer == DE_ADCBUFFER2)
	{
		mainBuffer2_flag = DE_FULLBUFFER;
		return DE_OK;
	}
	else
	{
		return DE_ERROR;
	}
}


int DE_GetBufferStatus(int buffer)
{
	if(buffer == DE_ADCBUFFER1)
	{
		return mainBuffer1_flag;
	}
	else if (buffer == DE_ADCBUFFER2)
	{
		return mainBuffer2_flag;
	}
	else
	{
		return DE_ERROR;
	}
}


int DE_SetBufferAsEmpty(int buffer)
{
	if(buffer == DE_ADCBUFFER1)
	{
		mainBuffer1_flag = DE_EMPTYBUFFER;
		return DE_OK;
	}
	else if (buffer == DE_ADCBUFFER2)
	{
		mainBuffer2_flag = DE_EMPTYBUFFER;
		return DE_OK;
	}
	else
	{
		return DE_ERROR;
	}
}


int DE_GetActiveBuffer()
{
	return activeBuff;
}


int DE_SetActiveBuffer(int buffer)
{
	if((buffer == DE_ADCBUFFER1) || (buffer == DE_ADCBUFFER2))
	{
		activeBuff = buffer;
		return DE_OK;
	}
	else
	{
		return DE_ERROR;
	}
}


int DE_SetBlockStatus(int state, int buffer)
{
	if(state == DE_INITIALBLOCK || state == DE_STARTBLOCK || state == DE_FOLLOWINGBLOCK)
	{
		if(buffer == DE_ADCBUFFER1)
		{
			buffer1_blockStatus = state;
			return DE_OK;
		}
		else if(buffer == DE_ADCBUFFER2)
		{
			buffer2_blockStatus = state;
			return DE_OK;
		}
		else
		{
			return DE_ERROR;
		}
	}
	else
	{
		return DE_ERROR;
	}
}


int DE_GetBlockStatus(int buffer)
{
	if(buffer == DE_ADCBUFFER1)
	{
		return buffer1_blockStatus;
	}
	else if(buffer == DE_ADCBUFFER2)
	{
		return buffer2_blockStatus;
	}
	else
	{
		return DE_ERROR;
	}
}


/* END OF FILE */
