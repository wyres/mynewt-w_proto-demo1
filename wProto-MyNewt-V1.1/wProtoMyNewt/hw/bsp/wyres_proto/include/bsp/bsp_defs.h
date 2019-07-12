/*
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
#ifndef H_BSP_DEFS_H
#define H_BSP_DEFS_H


#ifdef __cplusplus
extern "C" {
#endif


/* Pin management de wyres_proto */

/*LED pins*/
/*LED_BLINK_PIN (PA.0 on the board)*/
#define LED_D1              MCU_GPIO_PORTA(0)
/*LED_2 (PA.3 ont the board)*/
#define LED_D2              MCU_GPIO_PORTA(3)

/*IRQ gpio*/
/*BUTTON_PIN (PB.8 on the board)*/
#define BUTTON_PIN          MCU_GPIO_PORTB(12)
/*HALL_EFFECT (PA.9 on the board)*/
#define HALL_EFFECT         MCU_GPIO_PORTB(14)

/* UART */
#define UART_CNT 1
/*UART pins for console*/
#define BSP_UART0_TX MCU_GPIO_PORTA(9)
#define BSP_UART0_RX MCU_GPIO_PORTA(10)

/* SPI1 */
/*SPI pins for LoRaWAN message*/
#define SPI_0_CS MCU_GPIO_PORTB(0)
#define SPI_0_MOSI MCU_GPIO_PORTA(7)
#define SPI_0_MISO MCU_GPIO_PORTA(6)
#define SPI_0_CLK MCU_GPIO_PORTA(5)


#ifdef __cplusplus
}
#endif

#endif  /* H_BSP_DEFS_H */
