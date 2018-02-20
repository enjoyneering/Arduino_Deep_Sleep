/***************************************************************************************************/
/*
  Arduino_deep_sleep is an example of the proper and most advanced way to put any AVR based Arduino
  boards in to sleep
  
  Sketch puts Arduino in to the deep sleep for ~30 sec. Wakes it up, turns on/off
  the LED connected to the pin0 or pin13 & puts Arduino back to sleep.

  written by : enjoyneering79
  sourse code: https://github.com/enjoyneering/Arduino_Deep_Sleep

  GNU GPL license, all text above must be included in any redistribution, see link below for details:
  - https://www.gnu.org/licenses/licenses.html
*/
/***************************************************************************************************/
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
#if defineddefined(__AVR_ATmega328P__) || defined(__AVR_ATmega168p__)
byte led = LED_BUILTIN;
#elif (__AVR_ATtiny85__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny25__)
byte led = 0;
#endif

volatile byte watchdogCounter = 0;

/**************************************************************************/
/*
    all_pins_output()
    
    Sets all pins as output, to prevent MCU wake up from random noise
    signal during sleep, use if needed
    
    NOTE:
    - INPUT_PULLUP connects internal 20k..30k pullups resistors & increase
      consumption by 1.25uA
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

    NOTE:
    - valid sleeps intervals:
      * WDTO_15MS
      * WDTO_30MS
      * WDTO_60MS
      * WDTO_120MS
      * WDTO_250MS
      * WDTO_500MS
      * WDTO_1S
      * WDTO_2S
      * WDTO_4S
      * WDTO_8S
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
  
  //wdt_disable();            //disable & stop wdt timer
  watchdogCounter = 0;        //reset counter

  power_all_enable();         //enable all peripheries (timer0, timer1, Universal Serial Interface, ADC)
  /*
  power_adc_enable();         //enable ADC
  power_timer0_enable();      //enable Timer0
  power_timer1_enable();      //enable Timer1
  power_usi_enable();         //enable the Universal Serial Interface module
  */
  delay(5);                   //to settle down ADC & peripheries

  digitalWrite(led, HIGH);    //led blink
  delay(3000);
  digitalWrite(led, LOW);

  //wdt_enable(WDTO_8S);      //enable wdt timer
}

/**************************************************************************/
/*
    arduino_sleep()

    Puts system into the sleep state
 
    NOTE:   
    - there are 6 different sleeps modes:
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
  cli();                               //disable interrupts for time critical operations below

  power_all_disable();                 //disable all peripheries (timer0, timer1, Universal Serial Interface, ADC)
  /*              
  power_adc_disable();                 //disable ADC
  power_timer0_disable();              //disable Timer0
  power_timer1_disable();              //disable Timer2
  power_usi_disable();                 //disable the Universal Serial Interface module
  */
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //set the sleep type

  #if defined(__AVR_ATmega328P__) || (__AVR_ATtiny85__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny25__)
  sleep_bod_disable();                 //disable brown out detection during the sleep to saves more power
  #endif

  sei();                               //re-enable interrupts

  sleep_mode();                        /*
                                         system stops & sleeps here (automatically sets the SE (Sleep Enable) bit
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
    
    NOTE:
    - WDT runs from internal 128kHz clock & continues to work during the
      deepest sleep modes to provide a wake up source
    - valid sleeps intervals:
      * WDTO_15MS
      * WDTO_30MS
      * WDTO_60MS
      * WDTO_120MS
      * WDTO_250MS
      * WDTO_500MS
      * WDTO_1S
      * WDTO_2S
      * WDTO_4S
      * WDTO_8S
*/
/**************************************************************************/
void setup_watchdog(byte sleep_time)
{
  cli();                  //disable interrupts for time critical operations below

  /* enable only watchdog interrupts */
  #if   defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168p__)
  WDTCSR |= _BV(WDIE);
  #elif defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny25__)
  WDTCR  |= _BV(WDIE);
  #endif

  wdt_enable(sleep_time); //enable wdt timer

  sei();                  //re-enable interrupts
}

/**************************************************************************/
/*
    ISR(WDT_vect)

    Watchdog Interrupt Service Routine, executed when watchdog is timed out

    NOTE:
    - if WDT ISR is not defined, MCU will reset after WDT
*/
/**************************************************************************/
ISR(WDT_vect)
{
  watchdogCounter++;
}
