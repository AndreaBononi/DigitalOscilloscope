#ifndef INC_SERIAL_H_
#define INC_SERIAL_H_

#define SER_TX_EMPTY 1   	// canale di trasmissione libero
#define SER_TX_FULL 0   	// canale di trasmissione impegnato

#define SER_RX_EMPTY 0   	// nessun nuovo dato è stato ricevuto
#define SER_RX_OPCODE_TT 1  // TriggerType ricevuto
#define SER_RX_OPCODE_TL 2  // TriggerLevel ricevuto
#define SER_RX_OPCODE_SP 3  // SamplingPeriod ricevuto
#define SER_RX_TT 4   		// TriggerType ricevuto
#define SER_RX_TL 5   		// TriggerLevel ricevuto
#define SER_RX_SP 6   		// SamplingPeriod ricevuto

#define SER_OK 0
#define SER_ERR 1


// Ottieni stato ricevitore, di default è SER_RX_EMPTY
int SER_GetRxState();

// Ottieni stato trasmettitore, di default è SER_TX_EMPTY
int SER_GetTxState();

// Imposta stato ricevitore
int SER_SetRxState(int state);
// RETURN VALUE:
	// SER_ERR in caso di errore, altrimenti SER_OK

// Imposta stato trasmettitore
int SER_SetTxState(int state);
// RETURN VALUE:
	// SER_ERR in caso di errore, altrimenti SER_OK


#endif /* INC_SERIAL_H_ */

/* END OF FILE */
