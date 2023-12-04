/* DESCRIPTION ----------------------------------------------------------------------------------------------------------------------------
 * 
 * DSO application source file.
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
 * ------------------------------------------------------------------------------------------------------------------------------------- */


#include "DSO_application.h"
#include "data_events.h"
#include "data_operation.h"
#include "hwHandles.h"
#include "serial.h"

static uint32_t       reloadValue;
static uint32_t       samplingPeriod;
static int            triggerLevel = 128;
static int            mode = AUTO;
static volatile int   triggerPosition;
static volatile int   missingSamples = BUFFER_DIMENSION - 2;

static volatile HAL_StatusTypeDef HAL_RXstatus;
static volatile HAL_StatusTypeDef HAL_TXstatus;

static volatile unsigned int adcRegularChannel_BufferIndex = 1;     /* indice utilizzato da ADC regular channel */
static volatile unsigned int adcInjectedChannel_BufferIndex = 2;    /* indice utilizzato da ADC injected channel */

static volatile uint8_t adcBuffer1[BUFFER_DIMENSION] = {0};         /* buffer1 per accumulare i risultati durante l'elaborazione di buffer2 */
static volatile uint8_t adcBuffer2[BUFFER_DIMENSION] = {0};			    /* buffer2 per accumulare i risultati durante l'elaborazione di buffer1 */
static volatile uint8_t OPC_buffer[OPCODE_DIMENSION + 1];   		    /* buffer contenente l'operation code ricevuto */
static volatile uint8_t TT[TT_DIMENSION + 1];   						        /* buffer contenente il trigger type ricevuto */
static volatile uint8_t TL[TL_DIMENSION + 1];   						        /* buffer contenente il trigger level ricevuto */
static volatile uint8_t SP[SP_DIMENSION + 1];   						        /* buffer contenente il samping period ricevuto */

static uint8_t mainBuffer[BUFFER_DIMENSION] = {0};                  /* buffer per la trasmissione */

static int isTrigger( int index );		                              /* funzione locale che verifica se un campione corrisponde al trigger */


/* ------------------------------------------------------------------------------------------------------------------------------------ */


void DSO_UserCode() {

	int buffer1_stat, buffer2_stat, shift, RX_stat, parity, bufferIndex, bufferIndexBound, blockStatus;

	adcBuffer1[0] = '*';
  adcBuffer2[0] = '*';
	adcBuffer1[BUFFER_DIMENSION - 1] = '#';
	adcBuffer2[BUFFER_DIMENSION - 1] = '#';

  buffer1_stat = DE_GetBufferStatus( DE_ADCBUFFER1 );
  buffer2_stat = DE_GetBufferStatus( DE_ADCBUFFER2 );

  /* CONTROLLO DISPONIBILITÀ NUOVO DATO DA INVIARE ----------------------------------------------------------------------------------- */
	if ( buffer1_stat == DE_FULLBUFFER || buffer2_stat == DE_FULLBUFFER ) {       
    /* 
     * COPIA DI ADCBUFFER IN MAINBUFFER -------------------------------------------------------------------------------------------------
     * Se la posizione del trigger è pari allora parity=0, altrimenti parity=1
     * Se lo stato del blocco è DE_STARTBLOCK, il buffer contiene dei dati validi ma non è mai stato del tutto riempito
     * Di conseguenza una parte dei campioni passati si trova già in mainBuffer, per cui adcBuffer non deve essere interamente copiato
     * Inoltre, la posizione del trigger dovrebbe sempre essere minore della metà del buffer
     */
    parity = triggerPosition % 2;
		if ( buffer1_stat == DE_FULLBUFFER ) {  /* il buffer contenente il dato da trasmettere è adcBuffer1 */
			blockStatus = DE_GetBlockStatus( DE_ADCBUFFER1 );
			if ( blockStatus == DE_STARTBLOCK ) {
				bufferIndexBound = triggerPosition + ((BUFFER_DIMENSION - 4) / 2) + parity;
        if ( triggerPosition > ((BUFFER_DIMENSION - 2) / 2) ) {             
          Error_Handler(); 
        }
			}
			else if ( blockStatus == DE_FOLLOWINGBLOCK ) {
				bufferIndexBound = BUFFER_DIMENSION - 1;
			}
			else {
				Error_Handler();
			}
			for ( bufferIndex = 0; bufferIndex <= bufferIndexBound; bufferIndex++ ) {
				mainBuffer[bufferIndex] = adcBuffer1[bufferIndex];
			}
		}
		else {  /* il buffer contenente il dato da trasmettere è adcBuffer2 */
			blockStatus = DE_GetBlockStatus( DE_ADCBUFFER2 );
			if ( blockStatus == DE_STARTBLOCK ) {
				bufferIndexBound = triggerPosition + ((BUFFER_DIMENSION - 4) / 2) + parity;
        if ( triggerPosition > ((BUFFER_DIMENSION - 2) / 2) ) { 
          Error_Handler(); 
        }
			}
			else if ( blockStatus == DE_FOLLOWINGBLOCK ) {
				bufferIndexBound = BUFFER_DIMENSION - 1;
			}
			else {
				Error_Handler();
			}
			for ( bufferIndex = 0; bufferIndex <= bufferIndexBound; bufferIndex++ ) {
				mainBuffer[bufferIndex] = adcBuffer2[bufferIndex];
			}
		}
		/* 
     * ALLINEAMENTO BUFFER ------------------------------------------------------------------------------------------------------------
     * Se il canale di trasmissione non è libero, mainBuffer è stato aggiornato prima che la trasmissione precedente fosse finita (errore)
     * Se il canale di trasmissione è libero possiamo shiftare il buffer (allineamento) per poi procedere ad inviarlo
     */
		if ( SER_GetTxState() == SER_TX_EMPTY ) {                               /* canale di trasmissione libero */
			if ( triggerPosition < ((BUFFER_DIMENSION - 2) / 2) ) {               /* shift da effettuare a destra */
				if ( parity == 0 ) {									                              /* indice pari, campione associato al CH1 */
					shift = ((BUFFER_DIMENSION - 2) / 2) + 1 - triggerPosition;
				}
				else {    											                                    /* indice dispari, campione associato al CH0 */
					shift = ((BUFFER_DIMENSION - 2) / 2) - triggerPosition;
				}
				for ( int i = 0; i < shift; i++ ) {
					DO_BufferRightShift( mainBuffer, BUFFER_DIMENSION );
				}
			}
			else {                                                                /* shift da effettuare a sinistra */
				if ( parity == 0 ) {                                                /* indice pari, campione associato al CH1 */
					shift = triggerPosition - ((BUFFER_DIMENSION - 2) / 2) + 1;
				}
				else {                                                              /* indice dispari, campione associato al CH0 */
					shift = triggerPosition - ((BUFFER_DIMENSION - 2) / 2);
				}
				for ( int i = 0; i < shift; i++ ) {
					DO_BufferLeftShift( mainBuffer, BUFFER_DIMENSION );
				}
			}
      /* INVIO DATO ----------------------------------------------------------------------------------------------------------------- */
			HAL_GPIO_WritePin( LED_GPIO_Port, LED_Pin, GPIO_PIN_SET );
			HAL_TXstatus = HAL_UART_Transmit_IT( &huart2, (uint8_t *)mainBuffer, sizeof( mainBuffer ) );   /* invio dato */
			if ( DE_GetBufferStatus(DE_ADCBUFFER1) == DE_FULLBUFFER ) {            
				if ( DE_SetBufferAsEmpty(DE_ADCBUFFER1) == DE_ERROR ) {   /* rimozione segnalazione buffer pieno */
					Error_Handler();
				}
			}
			else {
				if ( DE_SetBufferAsEmpty(DE_ADCBUFFER2) == DE_ERROR ) {
					Error_Handler();
				}
			}
			if(SER_SetTxState(SER_TX_FULL) != SER_OK) {   /* impostazione canale di trasmissione come occupato */
				Error_Handler();
			}
		}
		else {
			Error_Handler();
		}
	} /* FINE CONTROLLO DISPONIBILITÀ NUOVO DATO DA INVIARE --------------------------------------------------------------------------- */

	/* CONTROLLO DISPONIBILITÀ NUOVO DATO IN RICEZIONE -------------------------------------------------------------------------------- */
	RX_stat = SER_GetRxState();
	if (RX_stat == SER_RX_SP || RX_stat == SER_RX_TL || RX_stat == SER_RX_TT) {
		/* 
     * STOP E RI-INIZIALIZZAZIONE DEL SISTEMA ------------------------------------------------------------------------------------------
     * sensibilità ai trigger disattivata
     * segnalazione trigger non rilevato
     * segnalazione buffer vuoti
     */
		DE_SystemDisable();
		missingSamples = BUFFER_DIMENSION - 2;
		DE_SetTriggerAsUndetected();
		if ( DE_SetBufferAsEmpty(DE_ADCBUFFER1) == DE_ERROR ) {
			Error_Handler();
		}
		if ( DE_SetBufferAsEmpty(DE_ADCBUFFER2) == DE_ERROR ) {
			Error_Handler();
		}
		/* 
     * AGGIORNAMENTO SISTEMA -----------------------------------------------------------------------------------------------------------
     * potrebbe essere necessario aggiornare il sampling period, il trigger level oppure la modalità di lavoro
     * se la modalità di lavoro è diversa da STOP, la sensibilità ai trigger deve essere nuovamente abilitata
     * infine, la linea di ricezione deve essere segnalata come libera
     */
		if ( RX_stat == SER_RX_SP ) {                                               /* aggiornamento sampling period */
			samplingPeriod = (SP[0] << 24) | (SP [1] << 16) | (SP[2] << 8) | SP[3];
			reloadValue = samplingPeriod / 2000;
			__HAL_TIM_SET_AUTORELOAD( &htim3, reloadValue );
		}
		else if ( RX_stat == SER_RX_TL ) {                                          /* aggiornamento trigger level */
			triggerLevel = TL[0];
		}
		else if ( RX_stat == SER_RX_TT ) {                                          /* cambio modalità di lavoro */
			if ( TT[0] == 0x00 ) {
				mode = AUTO;
			}
			else if ( TT[0] == 0x01 ) {
				mode = NORMAL;
			}
			else if ( TT[0] == 0x02 ) {
				mode = SINGLE;
				prova = 1;
			}
			else {
				mode = STOP;
			}
		}
		if ( mode != STOP ) {
			DE_SystemEnable();							                                          /* abilitazione sensibilità ai trigger */
		}
		if ( SER_SetRxState( SER_RX_EMPTY ) != SER_OK )   	                        /* impostiamo linea di ricezione libera */
		{
			Error_Handler();
		}
	} /* FINE CONTROLLO DISPONIBILITÀ NUOVO DATO IN RICEZIONE ------------------------------------------------------------------------- */
} /* FINE FUNZIONE DSO_UserCode() --------------------------------------------------------------------------------------------------- */


/* ---------------------------------------------------------------------------------------------------------------------------------- */


void DSO_StartRX() {
	HAL_RXstatus = HAL_UART_Receive_IT( &huart2, (uint8_t *)OPC_buffer, sizeof( OPC_buffer ) );     /* avvio ricezione */
}


/* ---------------------------------------------------------------------------------------------------------------------------------- */


void HAL_UART_TxCpltCallback( UART_HandleTypeDef *huart ) {
	if ( HAL_TXstatus != HAL_OK ) {
		Error_Handler();
	}
	else if ( huart == &huart2 ) {
		if ( SER_SetTxState( SER_TX_EMPTY ) != SER_OK ) {		            /* impostazione canale di trasmissione libero */
			Error_Handler();
		}
		if ((mode != SINGLE) && (mode != STOP)) {
			DE_SystemEnable();    						                            /* riabilitazione sensibilità ai trigger */
		}
		HAL_GPIO_WritePin (LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	}
}


/* ---------------------------------------------------------------------------------------------------------------------------------- */


void HAL_UART_RxCpltCallback( UART_HandleTypeDef *huart) {
	int SER_Status, RX_status;
	RX_status = SER_GetRxState();
  if ( HAL_RXstatus != HAL_OK ) {
   	Error_Handler();
  }
  else if ( huart == &huart2 ) {
		if ( RX_status == SER_RX_EMPTY ) {                                              /* IL DATO RICEVUTO È UN OPCODE */
			if ( OPC_buffer[0] == '*' ) {
				if ( (OPC_buffer[1] == 'T') && (OPC_buffer[2] == 'T') ) {   					      /* trigger type opcode */
					SER_Status = SER_SetRxState(SER_RX_OPCODE_TT);   							            /* segnalazione ricezione opcode */ 
					HAL_RXstatus = HAL_UART_Receive_IT(&huart2, (uint8_t *)TT, sizeof(TT)); 	/* avvio ricezione trigger type value */
				}
				else if ( (OPC_buffer[1] == 'T') && (OPC_buffer[2] == 'L') ) {   				    /* trigger level opcode */
					SER_Status = SER_SetRxState(SER_RX_OPCODE_TL);   							            /* segnalazione ricezione opcode */ 
					HAL_RXstatus = HAL_UART_Receive_IT(&huart2, (uint8_t *)TL, sizeof(TL)); 	/* avvio ricezione trigger level value */
				}
				else if ( (OPC_buffer[1] == 'S') && (OPC_buffer[2] == 'P') ) {  				    /* sampling period opcode */
					SER_Status = SER_SetRxState(SER_RX_OPCODE_SP);   							            /* segnalazione ricezione opcode */ 
					HAL_RXstatus = HAL_UART_Receive_IT(&huart2, (uint8_t *)SP, sizeof(SP)); 	/* avvio ricezione sampling period value */
				}
			}
		}
		else if ( RX_status == SER_RX_TT || RX_status == SER_RX_TL || RX_status == SER_RX_SP ) { 	  /* OVERRUN (errore) */
			Error_Handler();
		}
		else {                                              /* IL DATO RICEVUTO È IL VALORE ASSOCIATO ALL'OPCODE RICEVUTO IN PRECEDENZA */
			if ( RX_status == SER_RX_OPCODE_TT ) { 				    /* trigger type value */
				if ( TT[TT_DIMENSION] == '#' ) {  			        /* valid value */
					SER_Status = SER_SetRxState( SER_RX_TT) ;   	/* segnalazione ricezione trigger type value */
				}
				else {  											                  /* invalid value */
					SER_Status = SER_SetRxState( SER_RX_EMPTY );	/* il ricevitore rimane vuoto */
				}
			}
			else if ( RX_status == SER_RX_OPCODE_TL ) {  			/* trigger level value */
				if ( TL[TL_DIMENSION] == '#' ) {   					    /* valid value */
					SER_Status = SER_SetRxState( SER_RX_TL );  	  /* segnalazione ricezione trigger level value */
				}
				else {  											                  /* invalid value */
					SER_Status = SER_SetRxState( SER_RX_EMPTY );  /* il ricevitore rimane vuoto */
				}
			}
			else if ( RX_status == SER_RX_OPCODE_SP ) {  			/* sampling period value */
				if ( SP[SP_DIMENSION] == '#' ) {  					    /* valid value */
					SER_Status = SER_SetRxState( SER_RX_SP );   	/* segnalazione ricezione sampling period value */
				}
				else { 											                    /* invalid value */
					SER_Status = SER_SetRxState( SER_RX_EMPTY );  /* il ricevitore rimane vuoto */
				}
			}
			HAL_RXstatus = HAL_UART_Receive_IT( &huart2, (uint8_t *)OPC_buffer, sizeof( OPC_buffer ) );     /* INIZIO NUOVA RICEZIONE */
		}
		if ( SER_Status != SER_OK ) {
			Error_Handler();
		}
	}
}


/* ---------------------------------------------------------------------------------------------------------------------------------- */


void HAL_UART_ErrorCallback( UART_HandleTypeDef *huart ) {
	Error_Handler();
}


/* ---------------------------------------------------------------------------------------------------------------------------------- */


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
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
