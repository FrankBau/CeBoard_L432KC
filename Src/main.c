#include <stm32l432xx.h>

#include <stdio.h>
#include <math.h>

#include <gpio.h>
#include <delay.h>
#include <tone.h>
#include <uart.h>
#include <adc.h>

// this project is for the Nucleo-L432KC board mounted on the
// old (designed for Nucleo-F042) green CE base board.

// PB3 is a replacement for the LED routed to PF1 which is not connected on the Nucleo-L432KC.
const uint8_t led[] = {PB0, PB1, PA8, PB3, PB5, PB4, PA1, PA0};

const uint8_t button[2] = {PA6, PA5};

// buzzer and analog (ADC) are swapped compared to the old F0 board
const uint8_t buzzer = PA3; // TIM15 CH2 (AF14)

const uint8_t analog = PA4; // ADC1_IN9

#define NOTE_F4 349
#define NOTE_A4 440
#define NOTE_C5 523
#define NOTE_E5 659
#define NOTE_F5 698

void imperial_march()
{
  static const int melody[] = {NOTE_A4, NOTE_A4, NOTE_A4, NOTE_F4, NOTE_C5, NOTE_A4, NOTE_F4, NOTE_C5, NOTE_A4, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_F5, NOTE_C5, NOTE_A4, NOTE_F4, NOTE_C5, NOTE_A4};
  static const int durations[] = {4, 4, 4, 5, 16, 4, 5, 16, 2, 4, 4, 4, 5, 16, 4, 5, 16, 2};
  
  int bpm = 103; // beats (== quarter note duration) per minute

  for (unsigned int i = 0; i < sizeof(melody) / sizeof(melody[0]); ++i)
  {
    uint32_t freq = melody[i];
    uint32_t duration_ms = (1000 * 60 * 4) / (bpm * durations[i]);
    tone(freq, duration_ms);
  }
}

int main(void)
{
  for (int i = 0; i < 8; ++i)
    gpio_output(led[i]);

  for (int i = 0; i < 8; ++i)
    gpio_set_1(led[i]);

  for (int i = 0; i < 2; ++i)
  {
    gpio_input(button[i]);
    gpio_pull_up(button[i]);
  }

  /* Loop forever */
  for (;;)
  {
    char ch = getch();  // input from terminal prog? 
    if ((gpio_get(button[0]) == 0) || (ch=='A'))
    {
      imperial_march();
    }

    if ((gpio_get(button[1]) == 0) || (ch=='B'))
    {
      for(int k=0; k<10; ++k) {
        for (int i = 0; i < 8; ++i) {
          gpio_toggle(led[i]);
          delay(100);
        }
      }
    }

    uint32_t mV = adc_measure(analog);
    printf("measured voltage at pin PA4: %5lu mV\n", mV);

    delay(100);
  }
}
