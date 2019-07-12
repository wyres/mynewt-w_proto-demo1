/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */


#include <string.h>
#include <stdio.h>

#include "sysinit/sysinit.h"
#include "os/os.h"

#include <inttypes.h>
#include <mcu/mcu.h>

#include "hal/hal_gpio.h"
#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

#include "console/console.h"

#include "lorawan_api/lorawan_api.h"
#include "os/mynewt.h"

#include "bsp_defs.h"

#include "statemach.h"
#include "wutils.h"
#include "main.h"
#include "LoRa_message.h"


//#define DEBUG 1

/*Globale variable*/
static int button_tries_sent=0;
static int button_tries_error=0;
static int current_data=2;

/*Declaration of calback function of LoRa wait event result*/
static void tx_cb_fun (LORA_TX_RESULT_t e)
{
    console_printf("tx result %d \r\n",e);
    
    if (e != LORA_TX_OK) {
        console_printf("tx error \r\n");
        sendEvent(LORA_TX_STATUS, (void*)(e));
    }
    sendEvent(LORA_TX_STATUS, (void*)(LORA_TX_OK_ACKD));  
}

static void rx_cb_fun (uint8_t port, void* data, uint8_t sz)
{
    console_printf("rx received \r\n");
    //sendEvent(LORA_TX_STATUS, (void*)(LORA_TX_OK_ACKD));
}


/*Declaration of useful functions*/
int get_button_tries_sent()
{
    return button_tries_sent;
}

int inc_button_tries_sent()
{
    ++button_tries_sent;
    return button_tries_sent;
}
int reset_button_tries_sent()
{
    button_tries_sent=0;
    return button_tries_sent;
}
int get_button_tries_error()
{
    return button_tries_error;
}

int inc_button_tries_error()
{
    ++button_tries_error;
    return button_tries_error;
}
int reset_button_tries_error()
{
    button_tries_error=0;
    return button_tries_error;
}

int get_current_data()
{
    return current_data;
}

int set_current_data(int z)
{
    current_data = z;
    return current_data;
}



/* Declaration of the main of the code */
int
main(int argc, char **argv)
{
    int rc;
#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

#ifdef DEBUG
    uint32_t start = os_get_uptime_usec();
    uint32_t i=0;
    while ((start+10000000) > os_get_uptime_usec()) {
        i++;
    }
#endif

    sysinit();

    console_printf(":==================Console connected !===========================:\r\n");   
    
    lora_app_init(&tx_cb_fun, &rx_cb_fun);

    start_statemachine();

    while(1)
    {
        os_eventq_run(os_eventq_dflt_get());
    }
    
    return rc;
}