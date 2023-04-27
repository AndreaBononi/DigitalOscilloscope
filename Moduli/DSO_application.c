#include "data_events.h"
#include "data_operation.h"
#include "DSO_application.h"
#include "hwHandles.h"
#include "serial.h"



static uint32_t reloadValue, samplingPeriod;
static int triggerLevel = 128;
static int mode = AUTO;
static volatile int triggerPosition;

static volatile unsigned int adcRegularChannel_BufferIndex = 1;			// Indice utilizzato dal canale REGULAR del ADC
static volatile unsigned int adcInjectedChannel_BufferIndex = 2;		// Indice utilizzato dal canale INJECTED del ADC
static volatile uint8_t adcBuffer1[BUFFER_DIMENSION] = {0};				// buffer1 per accumulare i risultati del ADC durante l'elaborazione di buffer2
static volatile uint8_t adcBuffer2[BUFFER_DIMENSION] = {0};				// buffer2 per accumulare i risultati del ADC durante l'elaborazione di buffer1
static volatile int missingSamples = BUFFER_DIMENSION - 2;

static volatile uint8_t OpCode_Buffer[OPCODE_DIMENSION + 1];   			// buffer contenente l'operation code ricevuto
static volatile uint8_t TT[TT_DIMENSION + 1];   						// buffer contenente il trigger type ricevuto
static volatile uint8_t TL[TL_DIMENSION + 1];   						// buffer contenente il trigger level ricevuto
static volatile uint8_t SP[SP_DIMENSION + 1];   						// buffer contenente il samping period ricevuto

static uint8_t mainBuffer[BUFFER_DIMENSION] = {0};						// buffer per la trasmissione

static volatile HAL_StatusTypeDef HAL_RXstatus, HAL_TXstatus;

static int isTrigger(int index);		// definizione funzione locale che verifica se un campione corrisponde al trigger



void DSO_UserCode()
{
	int shift, RX_Stat, bufferIndex, parity, bufferIndexBound, blockStatus;

	adcBuffer1[0] = '*';
	adcBuffer1[BUFFER_DIMENSION - 1] = '#';
	adcBuffer2[0] = '*';
	adcBuffer2[BUFFER_DIMENSION - 1] = '#';

	// CONTROLLIAMO SE È DISPONIBILE UN NUOVO DATO DA INVIARE:
	if ((DE_GetBufferStatus(DE_ADCBUFFER1) == DE_FULLBUFFER) || (DE_GetBufferStatus(DE_ADCBUFFER2) == DE_FULLBUFFER))
	{
		parity = triggerPosition % 2;								// triggerPosition PARI --> parity = 0, altrimenti parity = 1;
		// COPIA DI ADCBUFFER IN MAINBUFFER:
		if(DE_GetBufferStatus(DE_ADCBUFFER1) == DE_FULLBUFFER)		// il buffer contenente il dato da trasmettere è adcBuffer1
		{
			// Se blockStatus è STARTBLOCK, il buffer contiene dati validi ma non è mai stato del tutto riempito
			// In questo caso una parte dei campioni passati si trova già in mainBuffer, quindi adcBuffer non deve essere interamente copiato
			blockStatus = DE_GetBlockStatus(DE_ADCBUFFER1);
			if(blockStatus == DE_STARTBLOCK)
			{
				// Arrivati a questo punto, la posizione del trigger dovrebbe sempre essere < della metà del buffer
				// Per sicurezza è sempre meglio inserire un controllo
				if(triggerPosition > ((BUFFER_DIMENSION - 2) / 2))
				{
					Error_Handler();
				}
				bufferIndexBound = triggerPosition + ((BUFFER_DIMENSION - 4) / 2) + parity;
			}
			else if (blockStatus == DE_FOLLOWINGBLOCK)
			{
				bufferIndexBound = BUFFER_DIMENSION - 1;
			}
			else
			{
				Error_Handler();
			}
			for (bufferIndex = 0; bufferIndex <= bufferIndexBound; bufferIndex++)
			{
				mainBuffer[bufferIndex] = adcBuffer1[bufferIndex];
			}
		}
		else	// il buffer contenente il dato da trasmettere è adcBuffer1, ripetiamo le operazioni precedenti
		{
			blockStatus = DE_GetBlockStatus(DE_ADCBUFFER2);
			if(blockStatus == DE_STARTBLOCK)
			{
				if(triggerPosition > ((BUFFER_DIMENSION - 2) / 2))
				{
					Error_Handler();
				}
				bufferIndexBound = triggerPosition + ((BUFFER_DIMENSION - 4) / 2) + parity;
			}
			else if (blockStatus == DE_FOLLOWINGBLOCK)
			{
				bufferIndexBound = BUFFER_DIMENSION - 1;
			}
			else
			{
				Error_Handler();
			}
			for (bufferIndex = 0; bufferIndex <= bufferIndexBound; bufferIndex++)
			{
				mainBuffer[bufferIndex] = adcBuffer2[bufferIndex];
			}
		}
		// ALLINEAMENTO BUFFER:
		if(SER_GetTxState() == SER_TX_EMPTY)						// CANALE DI TRASMISSIONE LIBERO
		{
			// SHIFT:
			if (triggerPosition < ((BUFFER_DIMENSION - 2) / 2))		// SHIFT A DESTRA
			{
				if (parity == 0)									// indice pari, campione associato al CH1, deve portarsi in posizione 512
				{
					shift = (((BUFFER_DIMENSION - 2) / 2) + 1) - triggerPosition;
				}
				else    											// indice dispari, campione associato al CH0, deve portarsi in posizione 511
				{
					shift = ((BUFFER_DIMENSION - 2) / 2) - triggerPosition;
				}
				for (int i = 0; i < shift; i++)
				{
					DO_BufferRightShift(mainBuffer, BUFFER_DIMENSION);
				}
			}
			else    												// SHIFT A SINISTRA
			{
				if (parity == 0)									// indice pari, campione associato al CH1, deve portarsi in posizione 512
				{
					shift = triggerPosition - (((BUFFER_DIMENSION - 2) / 2) + 1);
				}
				else    											// indice dispari, campione associato al CH0, deve portarsi in posizione 511
				{
					shift = triggerPosition - ((BUFFER_DIMENSION - 2) / 2);
				}
				for (int i = 0; i < shift; i++)
				{
					DO_BufferLeftShift(mainBuffer, BUFFER_DIMENSION);
				}
			}
			// INVIO DATO:
			HAL_GPIO_WritePin (LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
			HAL_TXstatus = HAL_UART_Transmit_IT(&huart2, (uint8_t *)mainBuffer, sizeof(mainBuffer));
			// Rimozione segnalazione buffer pieno:
			if(DE_GetBufferStatus(DE_ADCBUFFER1) == DE_FULLBUFFER)
			{
				if(DE_SetBufferAsEmpty(DE_ADCBUFFER1) == DE_ERROR)
				{
					Error_Handler();
				}
			}
			else
			{
				if(DE_SetBufferAsEmpty(DE_ADCBUFFER2) == DE_ERROR)
				{
					Error_Handler();
				}
			}
			// Impostazione canale di trasmissione come occupato:
			if(SER_SetTxState(SER_TX_FULL) != SER_OK)
			{
				Error_Handler();
			}
		}
		else	// mainBuffer è stato aggiornato prima che la trasmissione precedente fosse finita
		{
			Error_Handler();
		}
	}

	// CONTROLLIAMO LO STATO DEL RICEVITORE:
	RX_Stat = SER_GetRxState();
	if (RX_Stat == SER_RX_SP || RX_Stat == SER_RX_TL || RX_Stat == SER_RX_TT)	// se la condizione è vera, è disponibile un nuovo dato
	{
		// Fermiamo il sistema e ri-inizializziamolo:
		DE_SystemDisable();
		missingSamples = BUFFER_DIMENSION - 2;
		DE_SetTriggerAsUndetected();
		if(DE_SetBufferAsEmpty(DE_ADCBUFFER1) == DE_ERROR)
		{
			Error_Handler();
		}
		if(DE_SetBufferAsEmpty(DE_ADCBUFFER2) == DE_ERROR)
		{
			Error_Handler();
		}
		// Aggiornamento:
		if (RX_Stat == SER_RX_SP)						// AGGIORNAMENTO SAMPLING PERIOD
		{
			samplingPeriod = (SP[0] << 24) | (SP [1] << 16) | (SP[2] << 8) | SP[3];
			reloadValue = samplingPeriod / 2000;
			__HAL_TIM_SET_AUTORELOAD(&htim3, reloadValue);
		}
		else if (RX_Stat == SER_RX_TL)					// AGGIORNAMENTO TRIGGER LEVEL
		{
			triggerLevel = TL[0];
		}
		else if (RX_Stat == SER_RX_TT)					// CAMBIO MODALITÀ DI LAVORO
		{
			if(TT[0] == 0x00)
			{
				mode = AUTO;
			}
			else if(TT[0] == 0x01)
			{
				mode = NORMAL;
			}
			else if(TT[0] == 0x02)
			{
				mode = SINGLE;
				prova = 1;
			}
			else
			{
				mode = STOP;
			}
		}
		if(mode != STOP)
		{
			DE_SystemEnable();							// facciamo ripartire il rilevamento del trigger
		}
		if(SER_SetRxState(SER_RX_EMPTY) != SER_OK)   	// impostiamo linea di ricezione libera
		{
			Error_Handler();
		}
	}
}



void DSO_StartRX()
{
	HAL_RXstatus = HAL_UART_Receive_IT(&huart2, (uint8_t *)OpCode_Buffer, sizeof(OpCode_Buffer));
}



void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(HAL_TXstatus != HAL_OK)
	{
		Error_Handler();
	}
	else if(huart == &huart2)
	{
		if(SER_SetTxState(SER_TX_EMPTY) != SER_OK)		// impostiamo il canale di trasmissione come libero
		{
			Error_Handler();
		}
		if ((mode != SINGLE) && (mode != STOP))
		{
			DE_SystemEnable();    						// riabilita il sistema ad essere sensibile ai trigger
		}
		// Segnaliamo le varie trasmissioni facendo lampeggiare il LED:
		HAL_GPIO_WritePin (LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	}
}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	int SER_Status, RX_Status;
	RX_Status = SER_GetRxState();

    if(HAL_RXstatus != HAL_OK)
    {
    	Error_Handler();
    }
    else if(huart == &huart2)
	{
		if(RX_Status == SER_RX_EMPTY)   // Il dato ricevuto è un OpCode
		{
			if(OpCode_Buffer[0] == '*')
			{
				if((OpCode_Buffer[1] == 'T') && (OpCode_Buffer[2] == 'T'))   					// TRIGGER TYPE OPCODE
				{
					SER_Status = SER_SetRxState(SER_RX_OPCODE_TT);   							// Segnaliamo ricezione OpCode
					HAL_RXstatus = HAL_UART_Receive_IT(&huart2, (uint8_t *)TT, sizeof(TT)); 	// Avvio ricezione trigger type
				}
				else if((OpCode_Buffer[1] == 'T') && (OpCode_Buffer[2] == 'L'))   				// TRIGGER LEVEL OPCODE
				{
					SER_Status = SER_SetRxState(SER_RX_OPCODE_TL);   							// Segnaliamo ricezione OpCode
					HAL_RXstatus = HAL_UART_Receive_IT(&huart2, (uint8_t *)TL, sizeof(TL)); 	// Avvio ricezione trigger level
				}
				else if((OpCode_Buffer[1] == 'S') && (OpCode_Buffer[2] == 'P'))   				// SAMPLING PERIOD OPCODE
				{
					SER_Status = SER_SetRxState(SER_RX_OPCODE_SP);   							// Segnaliamo ricezione OpCode
					HAL_RXstatus = HAL_UART_Receive_IT(&huart2, (uint8_t *)SP, sizeof(SP)); 	// Avvio ricezione sampling period
				}
			}
		}
		else if(RX_Status == SER_RX_TT || RX_Status == SER_RX_TL || RX_Status == SER_RX_SP) 	// OverRun
		{
			Error_Handler();
		}
		else   // Il dato ricevuto è il valore associato all'OpCode precedentemente ricevuto
		{
			if(RX_Status == SER_RX_OPCODE_TT)   				// IL DATO RICEVUTO È IL TRIGGER TYPE
			{
				if(TT[TT_DIMENSION] == '#')   					// dato valido
				{
					SER_Status = SER_SetRxState(SER_RX_TT);   	// segnaliamo la ricezione del TT
				}
				else   											// dato non valido
				{
					SER_Status = SER_SetRxState(SER_RX_EMPTY);	// il ricevitore rimane vuoto
				}
			}
			else if(RX_Status == SER_RX_OPCODE_TL)   			// IL DATO RICEVUTO È IL TRIGGER LEVEL
			{
				if(TL[TL_DIMENSION] == '#')   					// dato valido
				{
					SER_Status = SER_SetRxState(SER_RX_TL);  	// segnaliamo la ricezione del TL
				}
				else   											// dato non valido
				{
					SER_Status = SER_SetRxState(SER_RX_EMPTY);  // il ricevitore rimane vuoto
				}
			}
			else if(RX_Status == SER_RX_OPCODE_SP)   			// IL DATO RICEVUTO È IL SAMPLING PERIOD
			{
				if(SP[SP_DIMENSION] == '#')   					// dato valido
				{
					SER_Status = SER_SetRxState(SER_RX_SP);   	// segnaliamo la ricezione del SP
				}
				else   											// dato non valido
				{
					SER_Status = SER_SetRxState(SER_RX_EMPTY);  // il ricevitore rimane vuoto
				}
			}
			// Iniziamo una nuova ricezione:
			HAL_RXstatus = HAL_UART_Receive_IT(&huart2, (uint8_t *)OpCode_Buffer, sizeof(OpCode_Buffer));
		}
		// Controllo errori:
		if(SER_Status != SER_OK)
		{
			Error_Handler();
		}
	}
}



void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	Error_Handler();
}



void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	int activeBuffer, localError;
	if(hadc == &hadc1)
	{
		activeBuffer = DE_GetActiveBuffer();
		// Salvataggio del valore campionato:
		if (activeBuffer == DE_ADCBUFFER1)
		{
			adcBuffer1[adcRegularChannel_BufferIndex] = HAL_ADC_GetValue(&hadc1);
		}
		else
		{
			adcBuffer2[adcRegularChannel_BufferIndex] = HAL_ADC_GetValue(&hadc1);
		}
		// Elaborazione valore campionato:
		if (DE_GetSystemStatus() == DE_TRIGGERENABLED)    							// IL SISTEMA È SENSIBILE AI TRIGGER
		{
			missingSamples --;
			if (DE_GetTriggerStatus() == DE_DETECTED)    							// IL TRIGGER È GIÀ STATO RILEVATO
			{
				if((DE_GetBufferStatus(DE_ADCBUFFER1) == DE_EMPTYBUFFER) && (DE_GetBufferStatus(DE_ADCBUFFER2) == DE_EMPTYBUFFER))
				{
					if (missingSamples == 0)										// buffer valido
					{
						DE_SystemDisable();    										// disabilita il sistema ad essere sensibile ai trigger
						// Segnala disponibiltà di un nuovo blocco:
						if(activeBuffer == DE_ADCBUFFER1)
						{
							localError = DE_SetBufferAsFull(DE_ADCBUFFER1);						// segnala che il buffer1 è pronto
							localError = localError + DE_SetActiveBuffer(DE_ADCBUFFER2);		// imposta il buffer2 per i prossimi campionamenti
							localError = localError + DE_SetBlockStatus(DE_STARTBLOCK, DE_ADCBUFFER2);		// Quando un buffer viene attivato, blockStatus viene impostato a STARTBLOCK
							if(localError != DE_OK)
							{
								Error_Handler();
							}
						}
						else
						{
							localError = DE_SetBufferAsFull(DE_ADCBUFFER2);						// segnala che il buffer1 è pronto
							localError = localError + DE_SetActiveBuffer(DE_ADCBUFFER1);		// imposta il buffer2 per i prossimi campionamenti
							localError = localError + DE_SetBlockStatus(DE_STARTBLOCK, DE_ADCBUFFER1);		// Quando un buffer viene attivato, blockStatus viene impostato a STARTBLOCK
							if(localError != DE_OK)
							{
								Error_Handler();
							}
						}
						adcRegularChannel_BufferIndex = 1;
						// Prepariamo il sistema per quando sarà nuovamente abilitato a sentire i trigger:
						missingSamples = BUFFER_DIMENSION - 2;
						DE_SetTriggerAsUndetected();
					}
				}
				else
				{
					Error_Handler();
				}
			}
			else    // IL TRIGGER NON È ANCORA STATO RILEVATO
			{
				if (isTrigger(adcRegularChannel_BufferIndex) == TRUE || (mode == AUTO && missingSamples <= (((BUFFER_DIMENSION - 2) / 2) - 1)))
				{
					// Il campione è il trigger, oppure siamo in AUTO e abbiamo acquisito metà campioni senza trovare il trigger
					DE_SetTriggerAsDetected();    							// segnala acquisizione trigger
					triggerPosition = adcRegularChannel_BufferIndex;		// memorizza posizione trigger
					missingSamples = ((BUFFER_DIMENSION - 2) / 2);
				}
				else
				{
					if (missingSamples <= 0)
					{
						missingSamples = BUFFER_DIMENSION - 2;
					}
				}
			}
		}
		// Aggiornamento indice:
		adcRegularChannel_BufferIndex = adcRegularChannel_BufferIndex + 2;
		if (adcRegularChannel_BufferIndex > BUFFER_DIMENSION - 3)
		{
			adcRegularChannel_BufferIndex = 1;
			// Il sistema inizia ad essere sensibile ai trigger solo dopo aver aquisito 1022 campioni dall'accensione
			if((DE_GetActiveBuffer() == DE_ADCBUFFER1) && (DE_GetBlockStatus(DE_ADCBUFFER1) == DE_INITIALBLOCK))
			{
				if(DE_SetBlockStatus(DE_STARTBLOCK, DE_ADCBUFFER1) == DE_ERROR)
				{
					Error_Handler();
				}
				DE_SystemEnable();
			}
			// Quando un buffer viene attivato, blockStatus viene impostato a STARTBLOCK
			// Quando il buffer viene riempito la prima volta dopo l'attivazione, blockStatus diventa FOLLOWINGBLOCK
			else if((DE_GetActiveBuffer() == DE_ADCBUFFER1) && (DE_GetBlockStatus(DE_ADCBUFFER1) == DE_STARTBLOCK))
			{
				if(DE_SetBlockStatus(DE_FOLLOWINGBLOCK, DE_ADCBUFFER1) == DE_ERROR)
				{
					Error_Handler();
				}
			}
			else if((DE_GetActiveBuffer() == DE_ADCBUFFER2) && (DE_GetBlockStatus(DE_ADCBUFFER2) == DE_STARTBLOCK))
			{
				if(DE_SetBlockStatus(DE_FOLLOWINGBLOCK, DE_ADCBUFFER2) == DE_ERROR)
				{
					Error_Handler();
				}
			}
		}
	}
}



void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	int activeBuffer, localError;
	if (hadc == &hadc1)
	{
		activeBuffer = DE_GetActiveBuffer();
		// Salvataggio del valore campionato:
		if (activeBuffer == DE_ADCBUFFER1)
		{
			adcBuffer1[adcInjectedChannel_BufferIndex] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
		}
		else
		{
			adcBuffer2[adcInjectedChannel_BufferIndex] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
		}
		// Elaborazione del valore campionato:
		if (DE_GetSystemStatus() == DE_TRIGGERENABLED)    					// IL SISTEMA È SENSIBILE AI TRIGGER
		{
			missingSamples --;   		 									// decremento del numero di campioni rimanenti da acquisire dal CH0
			if (DE_GetTriggerStatus() == DE_DETECTED)    					// IL TRIGGER È GIÀ STATO RILEVATO
			{
				if ((DE_GetBufferStatus(DE_ADCBUFFER1) == DE_EMPTYBUFFER) && (DE_GetBufferStatus(DE_ADCBUFFER2) == DE_EMPTYBUFFER))
				{
					if (missingSamples <= 0)								// buffer valido
					{
						DE_SystemDisable();    								// disabilita il sistema ad essere sensibile ai trigger
						// Segnala disponibiltà di un nuovo blocco:
						if(activeBuffer == DE_ADCBUFFER1)
						{
							localError = DE_SetBufferAsFull(DE_ADCBUFFER1);						// segnala che il buffer1 è pronto
							localError = localError + DE_SetActiveBuffer(DE_ADCBUFFER2);		// imposta il buffer2 per i prossimi campionamenti
							localError = localError + DE_SetBlockStatus(DE_STARTBLOCK, DE_ADCBUFFER2);		// Quando un buffer viene attivato, blockStatus viene impostato a STARTBLOCK
							if(localError != DE_OK)
							{
								Error_Handler();
							}
						}
						else
						{
							localError = DE_SetBufferAsFull(DE_ADCBUFFER2);						// segnala che il buffer1 è pronto
							localError = localError + DE_SetActiveBuffer(DE_ADCBUFFER1);		// imposta il buffer2 per i prossimi campionamenti
							localError = localError + DE_SetBlockStatus(DE_STARTBLOCK, DE_ADCBUFFER1);		// Quando un buffer viene attivato, blockStatus viene impostato a STARTBLOCK
							if(localError != DE_OK)
							{
								Error_Handler();
							}
						}
						adcInjectedChannel_BufferIndex = 2;
						// Prepariamo il sistema per quando sarà nuovamente abilitato a sentire i trigger:
						missingSamples = BUFFER_DIMENSION - 2;
						DE_SetTriggerAsUndetected();
					}
				}
			}
			else    // IL TRIGGER NON È ANCORA STATO RILEVATO
			{
				if (isTrigger(adcInjectedChannel_BufferIndex) == TRUE || (mode == AUTO && missingSamples <= (((BUFFER_DIMENSION - 2) / 2) - 1)))
				{
					// Il campione è il trigger, oppure siamo in AUTO e abbiamo acquisito metà campioni senza trovare il trigger
					DE_SetTriggerAsDetected();    							// segnala acquisizione trigger
					triggerPosition = adcInjectedChannel_BufferIndex;		// memorizza posizione trigger
					missingSamples = (((BUFFER_DIMENSION - 2) / 2) - 1);	// il numero di campioni da acquisire dopop il trigger è noto
				}
				else
				{
					if (missingSamples <= 0)
					{
						missingSamples = BUFFER_DIMENSION - 2;
					}
				}
			}
		}
		// Aggiornamento indice:
		adcInjectedChannel_BufferIndex = adcInjectedChannel_BufferIndex + 2;
		if (adcInjectedChannel_BufferIndex > BUFFER_DIMENSION - 2)
		{
			adcInjectedChannel_BufferIndex = 2;
		}
	}
}



void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
	Error_Handler();
}



static int isTrigger(int index)
{
	// Confrontiamo il valore attuale con quello precedente
	if(DE_GetActiveBuffer() == DE_ADCBUFFER1)
	{
		// Per gli indici 1 e 2 il valore precedente si trova alla fine del buffer
		if((index > 2))
		{
			if((adcBuffer1[index] >= triggerLevel) && (adcBuffer1[index - 2] < triggerLevel))
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			if((adcBuffer1[index] >= triggerLevel) && (adcBuffer1[BUFFER_DIMENSION - 4 + index] < triggerLevel))
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}
	else
	{
		if((index > 2))
		{
			if((adcBuffer2[index] >= triggerLevel) && (adcBuffer2[index - 2] < triggerLevel))
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			if((adcBuffer2[index] >= triggerLevel) && (adcBuffer2[BUFFER_DIMENSION - 4 + index] < triggerLevel))
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}
}



/* END OF FILE */
