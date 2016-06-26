/*-
 * Copyright (c) 2015 lex
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "libgpio.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PIN 17

int main(int argc, char *argv[]) {
  // these values seem to work best
  int firstSleep = 18000;
  int secondSleep = 120;

  uint8_t rawData[40] = {0};
  uint8_t values[4] = {0};

  if (argc == 3) {
    firstSleep = atoi(argv[1]);
    secondSleep = atoi(argv[2]);
  }

  gpio_handle_t handle = gpio_open(0);

  // wake up the sensor
  gpio_pin_output(handle, PIN);
  gpio_pin_low(handle, PIN);
  usleep(firstSleep);
  gpio_pin_high(handle, PIN);
  usleep(secondSleep);

  // begin to receive data
  gpio_pin_input(handle, PIN);

  int total = 0;

  for (int i = 0; i < 500; ++i) {
    int c = 0;

    while (gpio_pin_get(handle, PIN) != 1) {
    }

    while (gpio_pin_get(handle, PIN) != 0 && c < 256) {
      // transmitting, 26-28 us = 0, 70 us = 1
      // results should be something like 3-4 c for 0, 12-14 for 1
      ++c;
    }

    if (c == 256) {
      // reading probably failed
      break;
    }

    ++total;
    rawData[i] = c;

    if (total == 40) {
      // got all we need
      break;
    }
  }

  if (total != 40) {
    printf("failed, got %d/40 bits\n", total);
    gpio_close(handle);
    return 1;
  }

  for (int i = 0; i < 4; ++i) {
    uint8_t value = 0;

    for (int j = 0; j < 8; ++j) {
      uint8_t measuredLength = rawData[i * 8 + j];

      // hax
      if (measuredLength > 9)
        value |= 0x01;

      if (j != 7)
        value <<= 1;
    }

    values[i] = value;
  }

  printf("Humidity: %d.%d%%\n", values[0], values[1]);
  printf("Temperature: %d.%d c\n", values[2], values[3]);

  gpio_close(handle);
  return 0;
}
