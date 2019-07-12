#include <string.h>
#include <stdbool.h>

#include "sysinit/sysinit.h"
#include "os/os.h"

#include <inttypes.h>
#include <mcu/mcu.h>

#include "console/console.h"

#include "lorawan_api/lorawan_api.h"
#include "os/mynewt.h"

#include "wutils.h"
#include "LoRa_message.h"


#define LORA_APP_PORT                     3
#define LORAAPP_TASK_PRIO       MYNEWT_VAL(LORAAPP_TASK_PRIO)
#define LORAAPP_TASK_STACK_SZ   MYNEWT_VAL(LORAAPP_STACK_SIZE)



static os_stack_t _loraapp_task_stack[LORAAPP_TASK_STACK_SZ];
static struct os_task _loraapp_task_str;
static struct os_sem _lora_tx_sem;

static lorawan_sock_t _sock_tx;
static lorawan_sock_t _sock_rx;
static bool _canTx = false;


static struct loraapp_config {
    bool useAck;
    bool useAdr;
    uint8_t txPort;
    uint8_t rxPort;
    uint8_t loraDR;
    int8_t txPower;
    uint32_t txTimeoutMs;
    uint8_t deveui[8];
    uint8_t appeui[8];
    uint8_t appkey[16];
    uint32_t devAddr;
    uint8_t nwkSkey[16];
    uint8_t appSkey[16];
} _loraCfg = 
{
    .useAck = false,
    .useAdr = false,
    .txPort=3,
    .rxPort=3,
    .loraDR=0,
    .txPower=14,
    .txTimeoutMs=10000,
    /*University keys*/
    
    .deveui = { 0x38, 0xb8, 0xeb, 0xe0, 0x00, 0x00, 0x00, 0xaa},
    .appeui = { 0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01  },
    .appkey = { 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 
                0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00  },
             
    .devAddr=0,
    .nwkSkey = {0},
    .appSkey = {0},
};
static LORA_RES_CB_FN_t _txcbfn=NULL;
static LORA_RX_CB_FN_t _rxcbfn=NULL;
static void loraapp_task(void* data);


uint16_t lora_getId(void) 
{
    return (_loraCfg.deveui[6] << 8) + _loraCfg.deveui[7];
}


// initialise lorawan stack with our config
void lora_app_init( LORA_RES_CB_FN_t txcb, LORA_RX_CB_FN_t rxcb)
{
    int status = lorawan_configure_OTAA(_loraCfg.deveui, _loraCfg.appeui, _loraCfg.appkey, 1, ((14-_loraCfg.txPower)/2),  MYNEWT_VAL(LORA_REGION));
    assert(status == LORAWAN_STATUS_OK);
    console_printf("Starting LoRaWAN in OTAA mode [with devEUI: %02x%02x%02x%02x%02x%02x%02x%02x] \r\n",
        _loraCfg.deveui[0],_loraCfg.deveui[1],_loraCfg.deveui[2],_loraCfg.deveui[3],_loraCfg.deveui[4],_loraCfg.deveui[5],_loraCfg.deveui[6],_loraCfg.deveui[7]);

    // TODO this is not reglo
    lorawan_set_dutycycle(false);
        // set cbfn, indcating lock of lora
    _txcbfn = txcb;
    _rxcbfn = rxcb;

    if (_txcbfn!=NULL) 
    {
        /* 1st action: obtain a socket from the LoRaWAN API */
        _sock_tx = lorawan_socket(SOCKET_TYPE_TX);
        assert(_sock_tx != 0);
        _canTx = true;
    }
    if (_rxcbfn!=NULL) 
    {
        /* 1st action: obtain a socket from the LoRaWAN API */
        _sock_rx = lorawan_socket(SOCKET_TYPE_RX);
        assert(_sock_rx != 0);
        // Doesn't need same config as for TX, but does need to be bound
        assert(lorawan_bind(_sock_rx, 0, _loraCfg.rxPort)==0);

    }
    
    // need a semaphore...
    os_sem_init(&_lora_tx_sem,0);
    // Create task to run TX/RX as KLK wrapper uses blocking calls... thanks guys...
    os_task_init(&_loraapp_task_str, "lw_eventq",
                 loraapp_task, NULL,
                 LORAAPP_TASK_PRIO, OS_WAIT_FOREVER,
                 _loraapp_task_stack,
                 LORAAPP_TASK_STACK_SZ);
}


// Use static buffer in case api doesn't copy it
static uint8_t _txBuffer[255]={0};
// tx a buffer. returns result enum indicating tx ok, ack rx, or error
LORA_TX_RESULT_t lora_app_tx(uint16_t* data, uint16_t sz, uint32_t timeoutMs) 
{
    assert(_sock_tx!=0);        // no txing if you didnt init for it
    if (!_canTx) 
    {
        return LORA_TX_ERR_RETRY;
    }

    _loraCfg.txTimeoutMs = timeoutMs;
    memcpy(_txBuffer, data, sz);

    console_printf("TX thread started\r\n");  

    /* Send a frame*/
    /* 
     * 3rd action: put the data into the queue, the message will be sent
     * when the LoRaWAN stack is ready. 
     */
    int ret = lorawan_send(_sock_tx, _loraCfg.txPort, _txBuffer, sz);
    switch(ret) 
    {
        case LORAWAN_STATUS_OK: 
        {
            console_printf("LoRaWAN API tx queued ok [with devAddr:%08lx]\r\n",
                lorawan_get_devAddr_unicast() );
            // release the task to wait for result if cb required
            if (timeoutMs!=0) 
            {
                os_sem_release(&_lora_tx_sem);
            }
            return LORA_TX_OK;
        }
        case LORAWAN_STATUS_PORT_BUSY: 
        {
            console_printf("LoRaWAN API tx has busy return code. \r\n");
            return LORA_TX_ERR_RETRY;
        }
        default: 
        {
            console_printf("LoRaWAN API tx has fatal error code (%d). \r\n",
                ret);
            return LORA_TX_ERR_FATAL;       // best you reset mate
        }
    }
}


static void loraapp_task(void* data) 
{
    while(1) 
    {
        if (_sock_tx!=0) 
        {
            // TODO should have sema to stop app trying a tx before we're ready to go back to listen for tx result...
            // but without blocking the tx caller.... just a flag?
            _canTx = true;
            // Wait till we tx something
            os_sem_pend(&_lora_tx_sem, OS_TIMEOUT_NEVER);
            _canTx = false;
            console_printf("send message, wait state \r\n");
            lorawan_event_t txev = lorawan_wait_ev(_sock_tx, (LORAWAN_EVENT_ERROR|LORAWAN_EVENT_SENT|LORAWAN_EVENT_ACK), _loraCfg.txTimeoutMs);
            // note sema is now taken (==0), so next time round will block until a tx call adds a token
            console_printf("tx ev returns, event is %02x \r\n", txev);
            assert(_txcbfn!=NULL);      // must have a cb fn if we created the socket...
            if (txev == LORAWAN_EVENT_ACK) 
            {
                (*_txcbfn)(LORA_TX_OK_ACKD);
            }
            if (txev == LORAWAN_EVENT_SENT) 
            {
                (*_txcbfn)(LORA_TX_OK);
            }
            if (txev == LORAWAN_EVENT_ERROR) 
            {
                (*_txcbfn)(LORA_TX_ERR_RETRY);
            }
            if (txev == LORAWAN_EVENT_NONE) 
            {
                // timeout
                (*_txcbfn)(LORA_TX_TIMEOUT);
            }
        }
        
        // TX done. try for an RX while we're here... 
        if (_sock_rx!=0) 
        {
            uint32_t devAddr;
            uint8_t port;
            uint8_t payload[256];
            assert(_rxcbfn!=NULL);      // Must have cb fn if created socket
            // Wait 10s as gotta wait for RX2 delay (up to 7s) + SF12 (2s)
            uint8_t rxsz = lorawan_recv(_sock_rx, &devAddr, &port, payload, 255, 9000);
            console_printf("lora rx says got [%d] bytes \r\n", rxsz);
            if (rxsz>0) 
            {
                (*_rxcbfn)(port, payload, rxsz);
            }
        }
        
        // If no socket open, then just wait
        if (!(_sock_tx!=0 || _sock_rx!=0)) 
        {
            os_time_delay(OS_TICKS_PER_SEC*60);
        }  
    
    }
}
