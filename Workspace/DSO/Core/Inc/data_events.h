#ifndef INC_DATA_EVENTS_H_
#define INC_DATA_EVENTS_H_

#define DE_OK 0
#define DE_ADCBUFFER1 1
#define DE_ADCBUFFER2 2
#define DE_ERROR 3

#define DE_DETECTED 1
#define DE_UNDETECTED 0

#define DE_FULLBUFFER 1
#define DE_EMPTYBUFFER 0

#define DE_TRIGGERENABLED 1
#define DE_TRIGGERDISABLED 0

#define DE_INITIALBLOCK 0
#define DE_STARTBLOCK 1
#define DE_FOLLOWINGBLOCK 2


// ABILITA IL SISTEMA AD ESSERE SENSIBILE AI TRIGGER
void DE_SystemEnable();
// Di default il trigger è segnalato come disabilitato


// DISABILITA IL SISTEMA AD ESSERE SENSIBILE AI TRIGGER
void DE_SystemDisable();


// RICONOSCI SE IL SITEMA È SENSIBILE AI TRIGGER
int DE_GetSystemStatus();
// RETURN VALUE:
	// DE_TRIGGERENABLED (1) se il sitema è sensibile ai trigger, DE_TRIGGERDISABLED (0) altrimenti


// SEGNALA RILEVAMENTO TRIGGER
void DE_SetTriggerAsDetected();


// ELIMINA SEGNALAZIONE EVENTO DI TRIGGER
void DE_SetTriggerAsUndetected();


// OTTIENI RILEVAMENTO TRIGGER
int DE_GetTriggerStatus();
// RETURN VALUE:
	// DE_DETECTED (1) se l'evento di trigger è stato precedentemente rilevato
	// DE_UNDETECTED (0) altrimenti


// SEGNALA CHE IL BUFFER PRINCIPALE È STATO AGGIORNATO
int DE_SetBufferAsFull(int buffer);
// PARAMETRI:
	// BUFFER: quale dei 2 buffer deve essere considerato
// RETURN VALUE:
	// DE_ERROR (3) in caso di errore sul parametro
	// DE_OK (0) altrimenti


// ELIMINA SEGNALAZIONE AGGIORNAMENTO BUFFER PRINCIPALE
int DE_SetBufferAsEmpty(int buffer);
// PARAMETRI:
	// BUFFER: quale dei 2 buffer deve essere considerato
// RETURN VALUE:
	// DE_ERROR (3) in caso di errore sul parametro
	// DE_OK (0) altrimenti


// OTTIENI STATO BUFFER PRINCIPALE
int DE_GetBufferStatus(int buffer);
// PARAMETRI:
	// BUFFER: quale dei 2 buffer deve essere considerato
// RETURN VALUE:
	// DE_EMPTYBUFFER (1) se non è stato aggiornato
	// DE_FULLBUFFER (0) se è stato aggioranto e contiene un nuovo dato
	// DE_ERROR (3) in caso di errore sul parametro
// EMPTYBUFFER è la configurazione di default


// IMPOSTA BUFFER ATTIVO
int DE_SetActiveBuffer(int buffer);
// PARAMETRI:
	// BUFFER: quale dei 2 buffer deve essere considerato
// RETURN VALUE:
	// DE_ERROR (3) in caso di errore sul parametro
	// DE_OK (0) altrimenti
// Di default è attivo il buffer 1


// OTTIENI BUFFER ATTIVO
int DE_GetActiveBuffer();
// RETURN VALUE:
	// DE_ADCBUFFER1 se il buffer attivo è il buffer 1, altrimenti DE_ADCBUFFER2
// Di default è attivo il buffer 1


// IMPOSTA STATO BLOCCO
int DE_SetBlockStatus(int state, int buffer);
// PARAMETRI:
	// STATE: stato da impostare
	// BUFFER: buffer di riferimento di cui impostare lo stato del blocco
// RETURN VALUE:
	// DE_ERROR (3) in caso di errore sul parametro
	// DE_OK (0) altrimenti


// OTTIENI STATO BLOCCO
int DE_GetBlockStatus(int buffer);
// PARAMETRI:
	// BUFFER: buffer di cui vogliamo ottenere lo stato del blocco
// RETURN VALUE:
	// DE_INITIALBLOCK (0)
	// DE_STARTBLOCK (1)
	// DE_FOLLOWINGBLOCK (2)
	// DE_ERROR (3) in caso di errore sul parametro
// Di default il blocco relativo a buffer1 vale INITIALBLOCK, quello relativo a buffer2 vale STARTBLOCK


#endif /* INC_DATA_EVENTS_H_ */

/* END OF FILE */
