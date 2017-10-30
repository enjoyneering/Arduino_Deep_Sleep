/*****************************************************************************/
/*
  Arduino_deep_sleep is an example of the proper way to put ANY Arduino to sleep
  
  Sketch puts Arduino in to the deep sleep for ~30 sec. Wakes it up, turns on/off
  the LED connected to the pin0 & puts Arduino back to sleep.

  written by : enjoyneering79
  sourse code: https://github.com/enjoyneering/Arduino_Deep_Sleep

  BSD license, all text above must be included in any redistribution
*/
/*****************************************************************************/
#include <avr/sleep.h>     //AVR MCU power management
#include <avr/power.h>     //disable/anable AVR MCU peripheries (Analog Comparator, ADC, USI, Timers/Counters)
#include <avr/wdt.h>       //AVR MCU watchdog timer
#include <avr/io.h>        //includes the apropriate AVR MCU IO definitions
#include <avr/interrupt.h> //manipulation of the interrupt flags

/**************************************************************************/
/*
    variables
*/
/**************************************************************************/
volatile byte watchdogCounter = 0;
         byte led             = 0;

/**************************************************************************/
/*
    all_pins_output()
    
    Sets all pins as output, to prevent MCU wake up from a random noise
    signal during a sleep. Use it if needed.
    
    NOTE:
    - pinMode(ATtiny_pin, INPUT_PULLUP) connects internal 20k pullups resistors
    - it increases the consumption by 1.25uA
*/
/**************************************************************************/
void all_pins_output()
{
  for (byte ATtiny_pin; ATtiny_pin < 5; ATtiny_pin++)
  {
    pinMode(ATtiny_pin, INPUT_PULLUP);
  }
}

/**************************************************************************/
/*
    all_pins_input()

    Sets back all pins as input
*/
/**************************************************************************/
void all_pins_input()
{
  for (byte ATtiny_pin; ATtiny_pin < 5; ATtiny_pin++)
  {
    pinMode(ATtiny_pin, INPUT);
  }
}

/**************************************************************************/
/*
    setup()

    Main setup
*/
/**************************************************************************/
void setup()
{
  pinMode(led, OUTPUT);
  setup_watchdog(WDTO_8S); //approximately 8 sec. of sleep
}

/**************************************************************************/
/*
    loop()

    Main loop
*/
/**************************************************************************/
void loop()
{
  while (watchdogCounter < 4) //wait for watchdog counter reched the limit (WDTO_8S * 4 = 32sec.)
  {
    //all_pins_output();
    arduino_sleep();
  }
  
  watchdogCounter = 0;        //reset watchdogCounter
  power_all_enable();         //enable all peripheries (timer0, timer1, Universal Serial Interface, ADC)
  /*
  power_adc_enable();         //enable ADC
  power_timer0_enable();      //enable Timer0
  power_timer1_enable();      //enable Timer1
  power_usi_enable();         //enable the Universal Serial Interface module
  */
  delay(5);                   //to settle down ADC and peripheries

  digitalWrite(led,HIGH);     //led blink
  delay(3000);
  digitalWrite(led,LOW);
}

/**************************************************************************/
/*
    arduino_sleep()

    Puts system into the sleep state.
    
     There are 6 different sleeps modes:
     *  SLEEP_MODE_IDLE        - the least power savings (CPU stopped but Analog Comparator, ADC, USI, Timer/Counter,
                                 Watchdog (if enabled), and the interrupt system to continue operating) (default)
     *  SLEEP_MODE_ADC         - ADC Noise Reduction (CPU stopped but the ADC, the external interrupts,
                                 and the Watchdog (if enabled) to continue operating)
     *  SLEEP_MODE_PWR_SAVE    - supported by atiny25, atiny45, atiny85
     *  SLEEP_MODE_EXT_STANDBY - not supported by atiny25, atiny45, atiny85
     *  SLEEP_MODE_STANDBY     - not supported by atiny25, atiny45, atiny85
     *  SLEEP_MODE_PWR_DOWN    - the most power savings: all oscillators are stopped, only an External Reset, Watchdog Reset,
                                 Brown-out Reset, USI start condition interupt, an external level interrupt on INT0 or a pin
                                 change interrupt can wake up the MCU.      
*/
/**************************************************************************/
void arduino_sleep()
{
  power_all_disable();                 //disable all peripheries (timer0, timer1, Universal Serial Interface, ADC)
  /*              
  power_adc_disable();                 //disable ADC
  power_timer0_disable();              //disable Timer0
  power_timer1_disable();              //disable Timer2
  power_usi_disable();                 //disable the Universal Serial Interface module
  */
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //set the sleep type

  #if defined(__AVR_ATmega328P__) || (__AVR_ATtiny85__) || defined (__AVR_ATtiny45__) || defined (__AVR_ATtiny25__)
  sleep_bod_disable();                 //disable brown out detection during the sleep. saves more power
  #endif

  sleep_mode();                        /*system stops & sleeps here (automatically sets the SE (Sleep Enable) bit
                                         (so the sleep is possible), goes to sleep, wakes-up from sleep after the
                                         interrupt (if interrupts are enabled) or WDT timed out (if enabled) and
                                         clears the SE (Sleep Enable) bit afterwards).
                                         NOTE: sketch will continue from this point after interrupt or WDT timed out!!!
                                       */
}

/**************************************************************************/
/*
    setup_watchdog()

    Sets up watchdog to trigger interrupt after watchdog is timed out
    
    Sleeps intervals:
    - WDTO_15MS, WDTO_30MS, WDTO_60MS, WDTO_120MS, WDTO_250MS,
      WDTO_500MS, WDTO_1S, WDTO_2S, WDTO_4S, WDTO_8S
    
    NOTE:
    - MCU runs from internal 128kHz clock and continues to work during the
      deepest sleep modes to provide a wake up source.
*/
/**************************************************************************/
void setup_watchdog(byte sleep_time)
{
  /* enables interrupts instead of MCU reset, when watchdog is timed out */
  #if   defined(__AVR_ATmega328P__) || defined (__AVR_ATmega168p__)
  WDTCSR |= _BV(WDIE);
  #elif defined (__AVR_ATtiny85__) || defined (__AVR_ATtiny45__) || defined (__AVR_ATtiny25__)
  WDTCR  |= _BV(WDIE);
  #endif
  
  wdt_enable(sleep_time);
}

/**************************************************************************/
/*
    ISR(WDT_vect)

    Watchdog Interrupt Service, executed when watchdog is timed out
*/
/**************************************************************************/
ISR(WDT_vect)
{
  watchdogCounter++;
}
