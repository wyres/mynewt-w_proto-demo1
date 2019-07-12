#ifndef H_LORA_MESSAGE_H
#define H_LORA_MESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif
      
typedef enum { 
                LORA_TX_OK, 
                LORA_TX_OK_ACKD, 
                LORA_TX_TIMEOUT, 
                LORA_TX_ERR_RETRY, 
                LORA_TX_ERR_FATAL 
              
            } LORA_TX_RESULT_t;

typedef void (*LORA_RES_CB_FN_t)(LORA_TX_RESULT_t e);

typedef void (*LORA_RX_CB_FN_t)(uint8_t port, void* data, uint8_t sz);  

uint16_t lora_getId(void);


void lora_app_init( LORA_RES_CB_FN_t txcb, LORA_RX_CB_FN_t rxcb);

// tx a buffer. calls the callback fn (in init()) with result : 
 LORA_TX_RESULT_t lora_app_tx(uint16_t* data, uint16_t sz, uint32_t timeoutMs);


#ifdef __cplusplus
}
#endif

#endif  /* H_LORA_MESSAGE_H */
