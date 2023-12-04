/* DESCRIPTION --------------------------------------------------------------------------------------------------------
 *
 * Gestione degli eventi relativi ai dati.
 *
 * COMMENTI:
 * -- i dati sono trasmessi a blocchi
 * -- i dati ricevuti vengono progressivamente accumulati in uno dei due buffer disponibili
 * -- quando tutti i dati di un blocco sono stati ricevuti, il buffer utilizzato per riceverli contiene l'intero blocco
 * -- questo modulo contiene funzioni per analizzare lo stato del blocco e dei buffer, così come la sensibilità ai trigger
 * 
 * ELENCO FUNZIONI:
 * -- void	DE_SystemEnable           ( );
 * -- void  DE_SystemDisable          ( );
 * -- int 	DE_GetSystemStatus        ( );
 * -- void  DE_SetTriggerAsDetected   ( );
 * -- void  DE_SetTriggerAsUndetected ( );
 * -- int 	DE_GetTriggerStatus       ( );
 * -- int 	DE_SetBufferAsFull        ( int buffer );
 * -- int 	DE_SetBufferAsEmpty       ( int buffer );
 * -- int 	DE_GetBufferStatus        ( int buffer );
 * -- int 	DE_SetActiveBuffer        ( int buffer );
 * -- int 	DE_GetActiveBuffer        ( );
 * -- int 	DE_SetBlockStatus         ( int state, int buffer );
 * -- int 	DE_GetBlockStatus         ( int buffer );
 * ----------------------------------------------------------------------------------------------------------------- */


#ifndef INC_DATA_EVENTS_H_
#define INC_DATA_EVENTS_H_

#define DE_OK                 0
#define DE_ERROR              3
#define DE_ADCBUFFER1         1
#define DE_ADCBUFFER2         2
#define DE_DETECTED           1
#define DE_UNDETECTED         0
#define DE_FULLBUFFER         1
#define DE_EMPTYBUFFER        0
#define DE_TRIGGERENABLED     1
#define DE_TRIGGERDISABLED    0
#define DE_INITIALBLOCK       0
#define DE_STARTBLOCK         1
#define DE_FOLLOWINGBLOCK     2


void DE_SystemEnable();
/* --------------------------------------------------------------------------------------------------------------------
 * DESCRIZIONE: abilita il sistema ad essere sensibile ai trigger (di default la sensibilità è disabilitata)
 * ----------------------------------------------------------------------------------------------------------------- */


void DE_SystemDisable(); 
/* --------------------------------------------------------------------------------------------------------------------
 * DESCRIZIONE: disabilita il sistema ad essere sensibile ai trigger (di default la sensibilità è disabilitata)
 * ----------------------------------------------------------------------------------------------------------------- */


int DE_GetSystemStatus();
/* --------------------------------------------------------------------------------------------------------------------
 * DESCRIZIONE: rileva se il sistema è attualmente sensibile ai trigger
 * RETURN VALUE: 
 * 	# DE_TRIGGERENABLED se il sitema è sensibile ai trigger
 * 	# DE_TRIGGERDISABLED altrimenti
-------------------------------------------------------------------------------------------------------------------- */


void DE_SetTriggerAsDetected();
/* --------------------------------------------------------------------------------------------------------------------
 * DESCRIZIONE: segnala rilevamento trigger
 * ----------------------------------------------------------------------------------------------------------------- */


void DE_SetTriggerAsUndetected();
/* --------------------------------------------------------------------------------------------------------------------
 * DESCRIZIONE: elimina segnalazione evento di trigger
 * ----------------------------------------------------------------------------------------------------------------- */


int DE_GetTriggerStatus();
/* --------------------------------------------------------------------------------------------------------------------
 * DESCRIZIONE: verifica rilevamento trigger
 * RETURN VALUE: 
 * 	# DE_DETECTED se l'evento di trigger è stato precedentemente rilevato
 * 	# DE_UNDETECTED altrimenti
 * ----------------------------------------------------------------------------------------------------------------- */


int DE_SetBufferAsFull( int buffer );
/* --------------------------------------------------------------------------------------------------------------------
 * DESCRIZIONE: segnala che il buffer è stato aggiornato
 * PARAMETRI: 
 * 	# buffer: quale dei 2 buffer deve essere considerato
 * RETURN VALUE: 
 * 	# DE_ERROR in caso di errore sul parametro
 * 	# DE_OK altrimenti
 * ----------------------------------------------------------------------------------------------------------------- */


int DE_SetBufferAsEmpty( int buffer );
/* --------------------------------------------------------------------------------------------------------------------
 * DESCRIZIONE: elimina segnalazione aggiornamento buffer
 * PARAMETRI: 
 * 	# buffer: quale dei 2 buffer deve essere considerato
 * RETURN VALUE: 
 * 	# DE_ERROR in caso di errore sul parametro
 * 	# DE_OK altrimenti
 * ----------------------------------------------------------------------------------------------------------------- */


int DE_GetBufferStatus( int buffer );
/* --------------------------------------------------------------------------------------------------------------------
 * DESCRIZIONE: ottieni stato buffer principale
 * PARAMETRI: 
 * 	# buffer: quale dei 2 buffer deve essere considerato
 * RETURN VALUE:
 * 	# DE_EMPTYBUFFER se non è stato aggiornato (default)
 * 	# DE_FULLBUFFER se è stato aggioranto e contiene un nuovo dato
 * 	# DE_ERROR in caso di errore sul parametro
 * ----------------------------------------------------------------------------------------------------------------- */


int DE_SetActiveBuffer( int buffer );
/* --------------------------------------------------------------------------------------------------------------------
 * DESCRIZIONE: imposta buffer attivo (di default è attivo il buffer 1)
 * PARAMETRI:
 * 	# buffer: quale dei 2 buffer deve essere considerato
 * RETURN VALUE:
 * 	# DE_ERROR in caso di errore sul parametro
 * 	# DE_OK altrimenti
 * ----------------------------------------------------------------------------------------------------------------- */


int DE_GetActiveBuffer();
/* --------------------------------------------------------------------------------------------------------------------
 * DESCRIZIONE: ottieni buffer attivo (di default è attivo il buffer 1)
 * RETURN VALUE:
 * 	# DE_ADCBUFFER1 se il buffer attivo è il buffer 1
 * 	# DE_ADCBUFFER2 altrimenti
 * ----------------------------------------------------------------------------------------------------------------- */


int DE_SetBlockStatus( int state, int buffer );
/* --------------------------------------------------------------------------------------------------------------------
 * DESCRIZIONE: imposta stato blocco
 * PARAMETRI:
 * 	# state: stato da impostare
 * 	# buffer: buffer di riferimento di cui impostare lo stato del blocco
 * RETURN VALUE:
 * 	# DE_ERROR in caso di errore sul parametro
 * 	# DE_OK altrimenti
 * ----------------------------------------------------------------------------------------------------------------- */


int DE_GetBlockStatus( int buffer );
/* --------------------------------------------------------------------------------------------------------------------
 * DESCRIZIONE: ottieni stato blocco
 * PARAMETRI:
 * 	# buffer: buffer di cui vogliamo ottenere lo stato del blocco
 * RETURN VALUE:
 * 	# DE_INITIALBLOCK (default per DE_ADCBUFFER1)
 * 	# DE_STARTBLOCK (default per DE_ADCBUFFER2)
 * 	# DE_FOLLOWINGBLOCK
 * 	# DE_ERROR in caso di errore sul parametro
 * ------------------------------------------------------------------------------------------------------------------ */


#endif /* INC_DATA_EVENTS_H_ */