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

/* Linker script for STM32L152 when running from flash and using the bootloader */

/* Linker script to configure memory regions. */
MEMORY
{
  FLASH (rx) :  ORIGIN = 0x0800c000, LENGTH = 100K /*0x0800a000 104K*/ /* First image slot. */
  RAM (rwx) :   ORIGIN = 0x20000000, LENGTH = 32K
}

/* This linker script is used for images and thus contains an image header */
_imghdr_size = 0x20;
