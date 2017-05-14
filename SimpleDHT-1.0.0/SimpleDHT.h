/*
 The MIT License (MIT)

 Copyright (c) 2016 winlin

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "printf.h"


#ifndef SIMPLE_DHT_H
#define SIMPLE_DHT_H
typedef bool boolean;
#define HIGH 0x1
#define LOW 0x0
#define INPUT 0x0
#define OUTPUT 0x1

// to read from dht11.
// @param pin the DHT11 pin.
// @param ptemperature output, NULL to igore.
// @param phumidity output, NULL to ignore.
// @param pdata output 40bits sample, NULL to ignore.
// @remark the min delay for this method is 200ms.
extern int simple_dht11_read(int pin, uint8_t* ptemperature, uint8_t* phumidity, uint8_t pdata[40]);

// print the raw data of dht11, 5uint8_ts(40bits).
extern void simple_dht11_serial_print(uint8_t data[40]);

#endif
