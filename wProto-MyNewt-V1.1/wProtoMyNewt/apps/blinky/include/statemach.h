#ifndef H_STATEMACH_H
#define H_STATEMACH_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {  
                NOTINIT, 
                CURRENT_STATE, 
                JOINING, 
                STARTING, 
                OP_WAITING, 
                OP_TX_AND_WAIT_RESULT, 
                ST_TEST_DOOR, 
                ST_TX_AND_WAIT_RESULT, 
                ST_SIGNAL_TIMEOUT, 
                ST_RETRY_SENT,
                ST_SIGNAL_OK, 
                ST_SIGNAL_ERROR, 
                ST_TRIES_ERROR,
                FATAL_ERROR,
                OP_SIGNAL_OK,
                OP_SIGNAL_ERROR,
                OP_SIGNAL_TIMEOUT,
            
            } STATE;

typedef enum { ENTER, EXIT, TIMEOUT, LORA_TX_STATUS, LORA_RX, IRQ_HALL, IRQ_BUTT } EVENT;



/*public functions*/
/*init the statemachine*/
void start_statemachine(void) ;
/*make transition betwenn the event*/
void sendEvent(EVENT e, void* data); 



#ifdef __cplusplus
}
#endif

#endif  /* H_STATEMACH_H */
