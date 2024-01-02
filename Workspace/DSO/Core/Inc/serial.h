/* DESCRIPTION ---------------------------------------------------------------------------------------
 * 
 * Gestione dello stato del trasmettitore e del ricevitore.
 * 
 * ELENCO FUNZIONI:
 * -- int SER_GetRxState   ();
 * -- int SER_GetTxState   (); 
 * -- int SER_SetRxState   ( int state ); 
 * -- int SER_SetTxState   ( int state ); 
 * 
 * ------------------------------------------------------------------------------------------------ */


#ifndef INC_SERIAL_H_
#define INC_SERIAL_H_

#define SER_TX_EMPTY        1             
#define SER_TX_FULL         0                  
#define SER_RX_EMPTY        0                  
#define SER_RX_OPCODE_TT    1
#define SER_RX_OPCODE_TL    2 
#define SER_RX_OPCODE_SP    3   
#define SER_RX_TT           4       
#define SER_RX_TL           5        
#define SER_RX_SP           6          
#define SER_OK              0       
#define SER_ERR             1         


int SER_GetRxState(); 
/* ---------------------------------------------------------------------------------------------------
 * DESCRIZIONE: ottieni stato ricevitore
 * RETURN VALUE:
 *   # SER_RX_EMPTY        : nessun nuovo dato ricevuto (DEFAULT)
 *   # SER_RX_OPCODE_TT    : TriggerType operation code ricevuto 
 *   # SER_RX_OPCODE_TL    : TriggerLevel operation code ricevuto
 *   # SER_RX_OPCODE_SP    : SamplingPeriod operation code ricevuto
 *   # SER_RX_TT           : TriggerType ricevuto 
 *   # SER_RX_TL           : TriggerLevel ricevuto
 *   # SER_RX_SP           : SamplingPeriod ricevuto
 * ------------------------------------------------------------------------------------------------ */


int SER_GetTxState(); 
/* ---------------------------------------------------------------------------------------------------
 * DESCRIZIONE: ottieni stato trasmettitore
 * RETURN VALUE:
 *   # SER_TX_EMPTY    : canale di trasmissione libero (DEFAULT)
 *   # SER_TX_FULL     : canale di trasmissione impegnato
 * ------------------------------------------------------------------------------------------------ */


int SER_SetRxState( int state ); 
/* ---------------------------------------------------------------------------------------------------
 * DESCRIZIONE: imposta stato ricevitore
 * PARAMETRI:
 *   # state: stato da impostare, deve assumere uno dei seguenti valori
 *     -- SER_RX_EMPTY        : nessun nuovo dato ricevuto
 *     -- SER_RX_OPCODE_TT    : TriggerType operation code ricevuto 
 *     -- SER_RX_OPCODE_TL    : TriggerLevel operation code ricevuto
 *     -- SER_RX_OPCODE_SP    : SamplingPeriod operation code ricevuto
 *     -- SER_RX_TT           : TriggerType ricevuto 
 *     -- SER_RX_TL           : TriggerLevel ricevuto
 *     -- SER_RX_SP           : SamplingPeriod ricevuto
 * RETURN VALUE:
 *   # SER_ERR in caso di errore
 *   # SER_OK altrimenti
 * ------------------------------------------------------------------------------------------------ */


int SER_SetTxState( int state ); 
/* ---------------------------------------------------------------------------------------------------
 * DESCRIZIONE: imposta stato trasmettitore
 * PARAMETRI:
 *   # state: stato da impostare, deve assumere uno dei seguenti valori
 *     -- SER_TX_EMPTY   : canale di trasmissione libero
 *     -- SER_TX_FULL    : canale di trasmissione impegnato
 * RETURN VALUE:
 *   # SER_ERR in caso di errore
 *   # SER_OK altrimenti
 * ------------------------------------------------------------------------------------------------ */


#endif /* INC_SERIAL_H_ */