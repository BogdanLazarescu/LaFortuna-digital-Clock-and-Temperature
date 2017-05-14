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

#include "SimpleDHT.h"

// confirm the OUTPUT is level in us,
// for example, when DHT11 start sample, it will
//    1. PULL LOW 80us, call confirm(pin, 80, LOW)
//    2. PULL HIGH 80us, call confirm(pin, 80, HIGH)
// @return 0 success; oterwise, error.
// @remark should never used to read bits,
//    for function call use more time, maybe never got bit0.
// @remark please use simple_dht11_read().
int __simple_dht11_confirm(int pin, int us, uint8_t level) {
    // wait one more count to ensure.
    int cnt = us / 10 + 1;

    int ok = 0;
    int i;
    for (i = 0; i < cnt; i++) {
      //  if (digitalRead(pin) != level) {
      if (( (PIND  & _BV(PD7)) >0) != level) {
            ok = 1;
            break;
        }
        _delay_us(10);
    }

    if (!ok) {
        return -1;
    }
    return 0;
}

// @data the bits of a uint8_t.
// @remark please use simple_dht11_read().
uint8_t __simple_dht11_bits2uint8_t(uint8_t data[8]) {
    uint8_t v = 0;
    int i;
    for (i = 0; i < 8; i++) {
        v += data[i] << (7 - i);
    }
    return v;
}

// read temperature and humidity from dht11.
// @param pin the pin for DHT11, for example, 2.
// @param data a uint8_t[40] to read bits to 5uint8_ts.
// @return 0 success; otherwise, error.
// @remark please use simple_dht11_read().
int __simple_dht11_sample(int pin, uint8_t data[40]) {
    // empty output data.
    memset(data, 0, 40);

    // notify DHT11 to start:
    //    1. PULL LOW 20ms.
    //    2. PULL HIGH 20-40us.
    //    3. SET TO INPUT.

    PORTD  |=  _BV(PD7);
    _delay_us(30);

    //pinMode(pin, OUTPUT);
    DDRD  |=  _BV(PD7);
    //digitalWrite(pin, LOW);
    PORTD  &=  ~_BV(PD7);
    _delay_ms(20);
    //digitalWrite(pin, HIGH);
   PORTD  |=  _BV(PD7);
    _delay_us(30);
    //pinMode(pin, INPUT);
    PORTD  &=  ~_BV(PD7);
    DDRD  &=  ~_BV(PD7);

    // DHT11 starting:
    //    1. PULL LOW 80us
    //    2. PULL HIGH 80us
    if (__simple_dht11_confirm(pin, 80, LOW)) {
      tfp_printf("%s\n", "iesie aici primul if");
        return 100;
    }
    if (__simple_dht11_confirm(pin, 80, HIGH)) {
      tfp_printf("%s\n", "iesie aici primul if");
        return 101;
    }

        // DHT11 data transmite:
        //    1. 1bit start, PULL LOW 50us
        //    2. PULL HIGH 26-28us, bit(0)
        //    3. PULL HIGH 70us, bit(1)
        int j;
        for (j = 0; j < 40; j++) {
            if (__simple_dht11_confirm(pin, 50, LOW)) {
              tfp_printf("%s\n", "iesie aici");
                return 102;
            }

            // read a bit, should never call method,
            // for the method call use more than 20us,
            // so it maybe failed to detect the bit0.
            int ok = 0;
            int tick = 0,i;
            for (i = 0; i < 8; i++, tick++) {
                // if (digitalRead(pin) != HIGH) {
                if ( ((PIND  & _BV(PD7)) >0) != HIGH) {
                    ok = 1;
                    break;
                }
                _delay_us(10);
            }

            if (!ok) {
                return 103;
            }
            data[j] = (tick > 3? 1:0);
        }

        // DHT11 EOF:
        //    1. PULL LOW 50us.
        if (__simple_dht11_confirm(pin, 50, LOW)) {
            return 104;
        }

        return 0;
    }

    // parse the 40bits data to temperature and humidity.
    // @remark please use simple_dht11_read().
    int __simple_dht11_parse(uint8_t data[40], uint8_t* ptemperature, uint8_t* phumidity) {
        uint8_t humidity = __simple_dht11_bits2uint8_t(data);
        uint8_t humidity2 = __simple_dht11_bits2uint8_t(data + 8);
        uint8_t temperature = __simple_dht11_bits2uint8_t(data + 16);
        uint8_t temperature2 = __simple_dht11_bits2uint8_t(data + 24);
        uint8_t check = __simple_dht11_bits2uint8_t(data + 32);
        uint8_t expect = humidity + humidity2 + temperature + temperature2;
        if (check != expect) {
            return 105;
        }
        *ptemperature = temperature;
        *phumidity = humidity;
        return 0;
    }

    // print the raw data of dht11, 5uint8_ts(40bits).
    void simple_dht11_serial_print(uint8_t data[40]) {
      int i;
        for (i = 0; i < 40; i++) {
          //  Serial.print((int)data[i]);
            tfp_printf("%d\n", (int)data[i]);
            if (i > 0 && ((i + 1) % 4) == 0) {
                tfp_printf("%s", " ");
            }
        }
        tfp_printf("%s\n", "");
    }

    // to read from dht11.
    // @param pin the DHT11 pin.
    // @param ptemperature output, NULL to igore.
    // @param phumidity output, NULL to ignore.
    // @param pdata output 40bits sample, NULL to ignore.
    // @remark the min delay for this method is 200ms.
    int simple_dht11_read(int pin, uint8_t* ptemperature, uint8_t* phumidity, uint8_t pdata[40]) {
        int ret = 0;

        uint8_t data[40] = {0};
        if ((ret = __simple_dht11_sample(pin, data)) != 0) {

            return ret;
        }

        uint8_t temperature = 0;
        uint8_t humidity = 0;
        if ((ret = __simple_dht11_parse(data, &temperature, &humidity)) != 0) {
            return ret;
        }

        if (pdata) {
            memcpy(pdata, data, 40);
        }
        if (ptemperature) {
            *ptemperature = temperature;
        }
        if (phumidity) {
            *phumidity = humidity;
        }

        return ret;
    }

//
//
//     // DHT11 starting:
//     //    1. PULL LOW 80us
//     //    2. PULL HIGH 80us
//     if (__simple_dht11_confirm(pin, 80, LOW)) {
//       tfp_printf("%s\n", "exits first IF");
//         return 100;
//     }
//     if (__simple_dht11_confirm(pin, 80, HIGH)) {
//
//         tfp_printf("%s\n", "exit second if");
//         return 101;
//     }
//
//     // DHT11 data transmite:
//     //    1. 1bit start, PULL LOW 50us
//     //    2. PULL HIGH 26-28us, bit(0)
//     //    3. PULL HIGH 70us, bit(1)
//     int j;
//     for (j = 0; j < 40; j++) {
//         if (__simple_dht11_confirm(pin, 50, LOW)) {
//
//             tfp_printf("%s\n", "face break aiciiiii");
//             return 102;
//         }
//         // read a bit, should never call method,
//         // for the method call use more than 20us,
//         // so it maybe failed to detect the bit0.
//         int ok =0;
//         int tick = 0;
//         int i;
//         for (i = 0; i < 8; i++, tick++) {
//           //  if (digitalRead(pin) != HIGH) {
//               if ( ( PORTD  & _BV(PD7)) != _BV(PD7)) {
//                 ok = 1;
//                   tfp_printf("%s\n", "face break");
//                 break;
//             }
//             _delay_us(10);
//
//               tfp_printf("%s\n", "ajunge dupa break;");
//         }
//         if (!ok) {
//             return 103;
//         }
//         data[j] = (tick > 3? 1:0);
//     }
//
//     // DHT11 EOF:
//     //    1. PULL LOW 50us.
//     if (__simple_dht11_confirm(pin, 50, LOW)) {
//
//         tfp_printf("%s\n", "ajunge if jos");
//         return 104;
//     }
//
//     return 0;
// }
//
// // parse the 40bits data to temperature and humidity.
// // @remark please use simple_dht11_read().
// int __simple_dht11_parse(uint8_t data[40], uint8_t* ptemperature, uint8_t* phumidity) {
//     uint8_t humidity = __simple_dht11_bits2uint8_t(data);
//     uint8_t humidity2 = __simple_dht11_bits2uint8_t(data + 8);
//     uint8_t temperature = __simple_dht11_bits2uint8_t(data + 16);
//     uint8_t temperature2 = __simple_dht11_bits2uint8_t(data + 24);
//     uint8_t check = __simple_dht11_bits2uint8_t(data + 32);
//     uint8_t expect = humidity + humidity2 + temperature + temperature2;
//     if (check != expect) {
//         return 105;
//     }
//     *ptemperature = temperature;
//     *phumidity = humidity;
//     return 0;
// }
//
// // print the raw data of dht11, 5uint8_ts(40bits).
// void simple_dht11_serial_print(uint8_t data[40]) {
//
//   int i;
//     for (i = 0; i < 40; i++) {
//       //  Sdisplay_string((int)data[i]);
//       tfp_printf("%d", "(int)data[i]");
//         if (i > 0 && ((i + 1) % 4) == 0) {
//           tfp_printf("%s", " ");
//         }
//     }
//   tfp_printf("%s", "");
// }
//
// // to read from dht11.
// // @param pin the DHT11 pin.
// // @param ptemperature output, NULL to igore.
// // @param phumidity output, NULL to ignore.
// // @param pdata output 40bits sample, NULL to ignore.
// // @remark the min delay for this method is 200ms.
// int simple_dht11_read(int pin, uint8_t* ptemperature, uint8_t* phumidity, uint8_t pdata[40]) {
//     int ret = 0;
//
//     uint8_t data[40] = {0};
//     if ((ret = __simple_dht11_sample(pin, data)) != 0) {
//       tfp_printf("%s\n", "ajunge primul if ");
//         return ret;
//     }
//
//     int h;
//     for (h=0;h<40;h++)
//     tfp_printf("%d ", data[h]);
//     uint8_t temperature = 0;
//     uint8_t humidity = 0;
//     tfp_printf("%s\n", "ajunge si aici");
//     if ((ret = __simple_dht11_parse(data, &temperature, &humidity)) != 0) {
//       tfp_printf("%s\n","eroare al citire");
//         return ret;
//     }
//     tfp_printf("%s\n", "ajunge pana si aici");
//     tfp_printf("%s %d %s %d\n", "temp:",temperature,"unid:",humidity);
//
//
//     if (pdata) {
//         memcpy(pdata, data, 40);
//     }
//     if (ptemperature) {
//         *ptemperature = temperature;
//     }
//     if (phumidity) {
//         *phumidity = humidity;
//     }
//
//     return ret;
// }
