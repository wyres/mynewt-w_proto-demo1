#include <string.h>
#include <stdio.h>

#include "sysinit/sysinit.h"
#include "os/os.h"

#include <inttypes.h>
#include <mcu/mcu.h>

#include "hal/hal_bsp.h"
#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

#include "console/console.h"

#include "lorawan_api/lorawan_api.h"
#include "os/mynewt.h"

#include "bsp_defs.h"

#include "gpiomgr.h"
#include "ledmgr.h"
#include "wutils.h"
#include "statemach.h"
#include "main.h"
#include "adc.h"
#include "LoRa_message.h"

/*Define task stack of the state machine*/
#define MY_SM_TASK_PRIO        MYNEWT_VAL(STATE_MACH_TASK_PRIO)
#define MY_SM_TASK_STACK_SZ    MYNEWT_VAL(STATE_MACH_STACK_SIZE)


// callout & queue
static struct os_callout _sm_timer;
static struct os_eventq _sm_eq;

static os_stack_t my_sm_task_stack[MY_SM_TASK_STACK_SZ];
static struct os_task my_sm_task_str;

static EVENT _zeEvent;
static void* _zeData;

static STATE statemachine(EVENT e, void* data); 


/* For LED toggling */
static int8_t g_led_orange = LED_D1;
static int8_t g_led_red = LED_D2;
static int8_t g_button_pin = BUTTON_PIN;
static int8_t g_hall_pin = HALL_EFFECT;

/*Send different payloads for cage state*/
// cage Id (devEUI), opened/closed, battery, temperature
static uint16_t payload [5] = {0x0000, 0x0002, 0x0000, 0x0000};


/*Globale variable*/
static STATE _currentState = NOTINIT;

static void my_button_ev_cb(struct os_event *);
static void my_hall_ev_cb(struct os_event *);
static void sm_evt_cb(struct os_event *); 
static void sm_timer_stop(void); 


/* Decalare and initialize the event with the callback function*/
static struct os_event gpio_ev0 = {
    .ev_cb = my_button_ev_cb,
};
static struct os_event gpio_ev1 = {
    .ev_cb = my_hall_ev_cb,
};

static struct os_event _sm_evt = {
    .ev_cb = sm_evt_cb,
};



static STATE changeState(STATE n) 
{
    if (n==CURRENT_STATE) 
    {
        return _currentState;
    }
    else
    {
        //oops
    }
    console_printf("Leaving state %d, entering state %d\r\n", _currentState, n);
    sm_timer_stop();
    statemachine(EXIT, NULL);
    _currentState = n;
    changeState(statemachine(ENTER, NULL));
    return _currentState;
}


static void sm_timer_start(uint32_t t) 
{
    // create callout calling the cb executed on default q
    os_callout_reset(&_sm_timer, ((t*OS_TICKS_PER_SEC)/1000));
}
static void sm_timer_stop(void) 
{
    os_callout_stop(&_sm_timer);
}


static void sm_callout_cb(struct os_event *ev) 
{
    changeState(statemachine(TIMEOUT, NULL));
}


static void my_sm_task(void *arg)
{
    while (1) 
    {
        os_eventq_run(&_sm_eq);
    }
}
void 
sendEvent(EVENT e, void* data) 
{
    if (_sm_evt.ev_queued==false) 
    {
        _zeEvent=e;
        _zeData=data;
        os_eventq_put(&_sm_eq, &_sm_evt);
    } 
    else 
    {
        //oops
        console_printf("failed to q sm event %d as already on q\r\n", e);
    }
}

static void sm_evt_cb(struct os_event *ev) 
{
    changeState(statemachine(_zeEvent, _zeData));
}


static void my_button_ev_cb(struct os_event *ev)
{
    assert(ev!=NULL);
    sendEvent(IRQ_BUTT, NULL);
}
static void my_hall_ev_cb(struct os_event *ev)
{
    assert(ev!=NULL);
    sendEvent(IRQ_HALL, NULL);
}


static void
my_button_irq(void *arg)
{
    os_eventq_put(&_sm_eq, &gpio_ev0);
}

static void 
my_hall_irq(void *arg)
{
    os_eventq_put(&_sm_eq, &gpio_ev1);
}
 


void 
start_statemachine(void) 
{
    payload[0]=lora_getId();

    // init q, event
    /* Use a dedicate event queue for timer and interrupt events */
    os_eventq_init(&_sm_eq);

    //  create task
    /* 
     * Create the task to process timers and interrupts events from 
     * the my_button_eventq event queue and the 
     * my_hall_eventq.
     */

    os_task_init(&my_sm_task_str, "_sm_task", 
                 my_sm_task, NULL, 
                 MY_SM_TASK_PRIO, OS_WAIT_FOREVER, 
                 my_sm_task_stack, 
                 MY_SM_TASK_STACK_SZ);

    // define callout
    /* 
     * Initialize the callout for a timer event.  
     * The my_timer_ev_cb callback function processes the timer event.
     */

    os_callout_init(&_sm_timer, &_sm_eq, sm_callout_cb, NULL);

    // start up state machine, firstly join attempt
    changeState(JOINING);
}



static void
init_tasks(void)
{    
    /* 
     * Initialize and enable interrupt for the pin for the push button
     * and the hall effect sensor and configure the button and the hall
     * effect sensor with pull up resistor on the mcu.
     */

    GPIO_define_irq("push_button_irq", g_button_pin, my_button_irq, NULL, 
                    HAL_GPIO_TRIG_FALLING, HAL_GPIO_PULL_UP, LP_SLEEP);
    GPIO_irq_enable(g_button_pin);

    GPIO_define_irq("hall_effect_irq", g_hall_pin, my_hall_irq, NULL,
                    HAL_GPIO_TRIG_BOTH, HAL_GPIO_PULL_UP, LP_SLEEP);
    GPIO_irq_enable(g_hall_pin);


    ledRequest(g_led_orange, ON, 4, LED_REQ_INTERUPT);
    ledRequest (g_led_red, ON, 4, LED_REQ_INTERUPT);
}






static STATE statemachine(EVENT e, void* data) 
{
    switch(_currentState) 
    {
        case NOTINIT: 
        {
            return CURRENT_STATE;
        }
        case JOINING:
        {
            switch(e) 
            {
                case ENTER:
                {
                    // send LoRa message
                    payload[2]=BoardBatteryMeasureVolage();
                    console_printf("level battery = %d mV \r\n",payload[2]);
                    console_printf("payload = %04x %04x %04x %04x\r\n", payload[0],payload[1],payload[2], payload[3]);
                    lora_app_tx(payload, sizeof(payload), 8000);
                    sm_timer_start(20000);
                    ledRequest(g_led_red, FLASH_4HZ, 0, LED_REQ_INTERUPT);
                    return CURRENT_STATE;
                }
                case EXIT: {
                    sm_timer_stop();
                    ledCancel(g_led_red);
                    return CURRENT_STATE;
                }
                case TIMEOUT: {
                    console_printf("JOIN sent but no result, retry\r\n");
                    payload[2]=BoardBatteryMeasureVolage();
                    console_printf("level battery = %d mV \r\n",payload[2]);
                    console_printf("payload = %04x %04x %04d %04x\r\n", payload[0],payload[1],payload[2], payload[3]);
                    lora_app_tx(payload, sizeof(payload), 8000);
                    sm_timer_start(300000);
                    return CURRENT_STATE;
                }
                case LORA_TX_STATUS:
                {
                    // status in data
                    // deal with it
                    LORA_TX_RESULT_t result = (LORA_TX_RESULT_t)data;
                    if (result==LORA_TX_OK_ACKD) 
                    {
                        console_printf("JOIN/SEND/RX ok, starting\r\n");
                        return STARTING;
                    } 
                    else if (result==LORA_TX_ERR_FATAL) 
                    {
                        console_printf("All bad, reboot\r\n");
                        return FATAL_ERROR;
                    }
                    // else the timer will retry in a bit
                    return CURRENT_STATE;

                }
                default: {
                    console_printf("in state %d got unprocessed event %d \r\n", _currentState, e);
                    return CURRENT_STATE;
                }
            }
            return CURRENT_STATE;
        }

        case STARTING:
        {
            switch(e) 
            {
                case ENTER:
                {
                    // init stuff
                    init_tasks();
                    return OP_WAITING;
                }

                default: {
                    console_printf("in state %d got unprocessed event %d \r\n", _currentState, e);
                    return CURRENT_STATE;
                }
            }
            return CURRENT_STATE;
        }
        case OP_WAITING:
        {
            switch(e) 
            {
                case ENTER:
                {
                    sm_timer_start(300000);
                    return CURRENT_STATE;
                }
                case TIMEOUT:
                {
                    sm_timer_stop();
                    return OP_TX_AND_WAIT_RESULT;
                }
                case IRQ_BUTT:
                {
                    return ST_TEST_DOOR;
                }
                case IRQ_HALL: 
                {
                    // check hall input and set led appropriately
                    // TODO
                    if (GPIO_read(g_hall_pin)==0)
                    {
                        ledCancel(g_led_orange);            
                        ledCancel(g_led_red);
                        console_printf("Cage opened !! \r\n");
                        if (get_current_data()!=0) {
                            set_current_data(0);
                            // State change so LoRa message sent
                            return OP_TX_AND_WAIT_RESULT;
                        }
                    }
                    else if (GPIO_read(g_hall_pin)==1)
                    {
                        ledCancel(g_led_orange);                    
                        ledRequest(g_led_red, ON, 30, LED_REQ_INTERUPT);
                        console_printf("Cage closed !! \r\n");
                        if (get_current_data()!=1) {
                            set_current_data(1);
                            // State change so LoRa message sent
                            return OP_TX_AND_WAIT_RESULT;
                        }
                    }
                    return CURRENT_STATE;
                }

                default: 
                {
                    console_printf("in state %d got unprocessed event %d \r\n", _currentState, e);
                    return CURRENT_STATE;
                }
            }
            return CURRENT_STATE;
        }
        case OP_TX_AND_WAIT_RESULT:
        {
            switch(e) 
            {
                case ENTER:
                {
                    if (GPIO_read(g_hall_pin)==0)
                    {
                        // send LoRa message
                        payload[1]=0x0001;
                        payload[2]=BoardBatteryMeasureVolage();
                        console_printf("level battery = %d",payload[2]);
                        console_printf("payload = %02x%02x%02x%02x\r\n", payload[0],payload[1],payload[2], payload[3]);
                        if (lora_app_tx(payload, sizeof(payload), 10000)==LORA_TX_OK) {
                            sm_timer_start(20000);
                            return CURRENT_STATE;
                        }
                        return OP_SIGNAL_ERROR;
                    }
                    if (GPIO_read(g_hall_pin)==1)
                    {
                        // send LoRa message
                        payload[1]=0x0000;
                        payload[2]=BoardBatteryMeasureVolage();
                        console_printf("level battery = %d mV \r\n",payload[2]);
                        console_printf("payload = %02x%02x%02x%02x\r\n", payload[0],payload[1],payload[2], payload[3]);
                        if (lora_app_tx(payload, sizeof(payload), 10000)==LORA_TX_OK) {
                            sm_timer_start(20000);
                            return CURRENT_STATE;
                        }
                        return OP_SIGNAL_ERROR;
                    }                    
                }
                case TIMEOUT:
                {
                    // no answer
                    return OP_SIGNAL_TIMEOUT;
                }
                case LORA_TX_STATUS:
                {
                    // status in data
                    // deal with it
                    LORA_TX_RESULT_t result = (LORA_TX_RESULT_t)data;
                    if (result==LORA_TX_OK_ACKD) 
                    {
                        return OP_SIGNAL_OK;
                    } 
                    else if (result==LORA_TX_TIMEOUT) 
                    {
                        return OP_SIGNAL_TIMEOUT;
                    }  
                    else 
                    {
                        return OP_SIGNAL_ERROR;
                    }
                    return CURRENT_STATE;

                }
                default: {
                    console_printf("in state %d got unprocessed event %d \r\n", _currentState, e);
                    return CURRENT_STATE;
                }
            }
            return CURRENT_STATE;
        }
        case OP_SIGNAL_OK:
        {
            switch(e) 
            {
                case ENTER :
                {
                    ledRequest(g_led_orange, ON, 10, LED_REQ_INTERUPT);
                    return OP_WAITING;
                } 

                default: {
                    console_printf("in state %d got unprocessed event %d \r\n", _currentState, e);
                    return CURRENT_STATE;
                }
            }
            return CURRENT_STATE;
        }   
        case OP_SIGNAL_TIMEOUT:
        case OP_SIGNAL_ERROR:
        {
            switch(e) 
            {
                case ENTER : 
                {
                    ledRequest(g_led_orange, FLASH_4HZ, 10, LED_REQ_INTERUPT);
                    if (get_current_data()==0) 
                    {
                        // ok too bad
                        return OP_WAITING;
                    } else {
                        // retry in 10s
                        sm_timer_start(10000);
                        return CURRENT_STATE;
                    }               
                }
                case EXIT : 
                {
                    sm_timer_stop();
                    ledCancel(g_led_orange);
                    ledCancel (g_led_red);
                    return CURRENT_STATE;
                }
                case TIMEOUT:
                {
                    return OP_TX_AND_WAIT_RESULT;
                }
                default: {
                    console_printf("in state %d got unprocessed event %d \r\n", _currentState, e);
                    return CURRENT_STATE;
                }
            }
            return CURRENT_STATE;
        }
     
        case ST_TEST_DOOR:
        {
            switch(e) 
            {
                case ENTER:
                {
                    reset_button_tries_error();
                    reset_button_tries_sent();
                    ledRequest(g_led_orange, FLASH_4HZ, 30, LED_REQ_INTERUPT);
                    sm_timer_start(30000);
                    return CURRENT_STATE;
                }
                case TIMEOUT: 
                {
                    return ST_TX_AND_WAIT_RESULT;
                }
                case EXIT: 
                {
                    ledCancel(g_led_orange);
                    ledCancel(g_led_red);
                    sm_timer_stop();
                    return CURRENT_STATE;
                }
                case IRQ_HALL: 
                {
                    // check hall input and set led appropriately
                    // TODO
                    if (GPIO_read(g_hall_pin)==0)
                    {
                        ledCancel(g_led_red);
                        console_printf("Cage opened !! \r\n");
                    }
                    else if (GPIO_read(g_hall_pin)==1)
                    {
                        ledRequest(g_led_red, ON, 30, LED_REQ_INTERUPT);
                        console_printf("Cage closed !! \r\n");
                    }
                    return CURRENT_STATE;
                }
                default: {
                    console_printf("in state %d got unprocessed event %d \r\n", _currentState, e);
                    return CURRENT_STATE;
                }
            }
            return CURRENT_STATE;
        }
        case ST_TX_AND_WAIT_RESULT:
        {
            switch(e) 
            {
                case ENTER: 
                {
                    // timeout for lora send
                    ledCancel(g_led_orange);
                    ledCancel(g_led_red);
                    sm_timer_start(20000);
                    payload[1] = 0x0000;
                    payload[2]=BoardBatteryMeasureVolage();
                    console_printf("level battery = %d mV \r\n",payload[2]);
                    if (lora_app_tx(payload, sizeof(payload), 10000)==LORA_TX_OK) {
                        return CURRENT_STATE;
                    }
                    return ST_SIGNAL_ERROR;
                }
                case EXIT: 
                {
                    sm_timer_stop();
                    return CURRENT_STATE;
                }
                case TIMEOUT: {
                    return ST_SIGNAL_TIMEOUT;
                }
                case LORA_TX_STATUS:
                {
                    // deal with it
                    LORA_TX_RESULT_t result = (LORA_TX_RESULT_t)data;
                    if (result==LORA_TX_OK_ACKD) 
                    {
                        return ST_SIGNAL_OK;
                    } 
                    else if (result==LORA_TX_TIMEOUT) 
                    {
                        return ST_SIGNAL_TIMEOUT;
                    }  
                    else 
                    {
                        return ST_SIGNAL_ERROR;
                    }
                    return CURRENT_STATE;
                } 
                default: {
                    console_printf("in state %d got unprocessed event %d \r\n", _currentState, e);
                    return CURRENT_STATE;
                }
            }
            return CURRENT_STATE;
        }
        case ST_SIGNAL_TIMEOUT:
        {
            switch(e) 
            {
                case ENTER :
                {
                    ledRequest(g_led_orange, FLASH_1HZ, 5, LED_REQ_INTERUPT);
                    sm_timer_start(5000);
                    console_printf("Message sent but not receive \r\n");
                    console_printf("attempts = %x \r\n", get_button_tries_sent());
                    return CURRENT_STATE;                          
                } 

                case TIMEOUT : 
                {
                    if (inc_button_tries_sent()>5) {
                        return ST_RETRY_SENT;
                    }
                    return ST_TX_AND_WAIT_RESULT;
                }

                case EXIT : 
                {
                    ledCancel(g_led_red);
                    ledCancel(g_led_orange);
                    sm_timer_stop();
                    return CURRENT_STATE;
                }

                default: {
                    console_printf("in state %d got unprocessed event %d \r\n", _currentState, e);
                    return CURRENT_STATE;
                }
            }
            return CURRENT_STATE;
        }

        case ST_RETRY_SENT:
        {
            switch (e)
            {
                case ENTER : 
                {
                    sm_timer_start(5000);
                    ledCancel(g_led_orange);
                    ledRequest(g_led_red, FLASH_4HZ, 5, LED_REQ_INTERUPT);
                    console_printf("Message failed, prototype reboot\r\n");    
                    return CURRENT_STATE;
                }
                
                case TIMEOUT :
                {        
                    os_reboot(0);
                }

                case EXIT : 
                {
                    ledCancel(g_led_orange);
                    ledCancel(g_led_red);
                    sm_timer_stop();
                    return CURRENT_STATE;
                }

                default: {
                    console_printf("in state %d got unprocessed event %d \r\n", _currentState, e);
                    return CURRENT_STATE;
                }                
            }
            return CURRENT_STATE;
        }
        case ST_SIGNAL_OK:
        {
            switch(e) 
            {
                case ENTER : 
                {
                    ledRequest(g_led_orange, ON, 10, LED_REQ_INTERUPT);
                    sm_timer_start(10000);
                    console_printf("Message sent correctly \r\n");
                    return CURRENT_STATE;                
                }
                case TIMEOUT : 
                {
                    return OP_WAITING;
                }
                case EXIT : 
                {
                    sm_timer_stop();
                    ledCancel(g_led_orange);
                    ledCancel (g_led_red);
                    return CURRENT_STATE;
                }
                default: {
                    console_printf("in state %d got unprocessed event %d \r\n", _currentState, e);
                    return CURRENT_STATE;
                }
            }
            return CURRENT_STATE;
        }
        case ST_SIGNAL_ERROR:
        {
            switch(e) 
            {
                case ENTER :
                {
                    ledRequest(g_led_red, FLASH_05HZ, 10, LED_REQ_INTERUPT);
                    sm_timer_start(10000);
                    console_printf("Message sent error \r\n");
                    console_printf("attempts = %x \r\n", get_button_tries_error());
                    return CURRENT_STATE;                          
                } 
                case TIMEOUT : 
                {
                    if (inc_button_tries_error()>5) {
                        return ST_TRIES_ERROR;
                    }
                    return ST_TX_AND_WAIT_RESULT;
                }
                case EXIT : 
                {
                    ledCancel(g_led_red);
                    ledCancel(g_led_orange);
                    sm_timer_stop();
                    return CURRENT_STATE;
                }
                default: {
                    console_printf("in state %d got unprocessed event %d \r\n", _currentState, e);
                    return CURRENT_STATE;
                }
            }
            return CURRENT_STATE;
        }

        case ST_TRIES_ERROR :
        {
            switch (e)
            {
                case ENTER : 
                {
                    ledRequest(g_led_red, ON, 30, LED_REQ_INTERUPT);
                    sm_timer_start(30000);        
                    console_printf("Message error, prototype reboot\r\n");
                    return CURRENT_STATE;
                }            
                case TIMEOUT :
                {
                    return FATAL_ERROR;                    
                }
                case EXIT : 
                {
                    ledCancel(g_led_orange);
                    ledCancel(g_led_red);
                    return CURRENT_STATE;
                }
                default: {
                    console_printf("in state %d got unprocessed event %d \r\n", _currentState, e);
                    return CURRENT_STATE;
                }
            return CURRENT_STATE;
            }
        }
 
        case FATAL_ERROR :
        {
            switch (e)
            {
                case ENTER : 
                {
                    os_reboot(0);                    
                }
                default: {
                    console_printf("in state %d got unprocessed event %d \r\n", _currentState, e);
                    return CURRENT_STATE;
                }

            return CURRENT_STATE;
            }
        }

        default: 
        {
            console_printf("in state %d got unprocessed event %d \r\n", _currentState, e);
            assert(0);      // STOP NOW
            return CURRENT_STATE;
        }        
    }
    return CURRENT_STATE;

}