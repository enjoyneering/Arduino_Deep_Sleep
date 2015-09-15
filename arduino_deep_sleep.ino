/*****************************************************************************/
/*
  Arduino_deep_sleep is example of the proper way to put ANY arduino to sleep
  
  Put arduino in deep sleep for 30 sec. Wake it up and turn on LED connected
  to the pin 0. After put arduino back to sleep

  written by enjoyneering79

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
byte pinLed = 0;
volatile byte watchdog_counter;

/**************************************************************************/
/*
    set all pins as output

    to prevent MCU wake up from a random noise signal during a sleep
    
    NOTE: you can leave the pins as outputs during a sleep if you connect 
          internal 20k pullups resistors pinMode(ATtiny_pin, INPUT_PULLUP)
          or external 10k pullups resistors. but it increase the consumption
          by 1.25uA
*/
/**************************************************************************/
void all_pins_output()
{
  for (byte ATtiny_pin; ATtiny_pin < 5; ATtiny_pin++)
  {
    pinMode(ATtiny_pin, OUTPUT);
  }
}

/**************************************************************************/
/*
    set back all pins as input
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
*/
/**************************************************************************/
void setup()
{
  pinMode(pinLed,OUTPUT);
  setup_watchdog(WDTO_8S); //approximately 8 sec. of sleep
}

/**************************************************************************/
/*
    main loop()
*/
/**************************************************************************/
void loop()
{
  ATtiny85_sleep();

  if (watchdog_counter >= 4)  //wait for watchdog counter reched the limit (WDTO_8S * 4 = 32sec.)
  {
    watchdog_counter = 0;     //reset watchdog_counter

    power_all_enable();       //enable all peripheries (timer0, timer1, Universal Serial Interface, ADC)
    /*
    power_adc_enable();       //enable ADC
    power_timer0_enable();    //enable Timer0
    power_timer1_enable();    //enable Timer1
    power_usi_enable();       //enable the Universal Serial Interface module
    */
    delay(5); //to settle down ADC and peripheries

    all_pins_output();
    digitalWrite(pinLed,HIGH);  //led blink
    delay(3000);
    digitalWrite(pinLed,LOW);

    all_pins_output();
  }
}

/**************************************************************************/
/*
    ATtiny85_sleep()

    put system into the sleep state
    There are 6 different sleeps modes:
     *  SLEEP_MODE_IDLE        - the least power savings (CPU stopped but Analog Comparator, ADC, USI, Timer/Counter, Watchdog (if enabled), and the interrupt system to continue operating) (default)
     *  SLEEP_MODE_ADC         - ADC Noise Reduction (CPU stopped but the ADC, the external interrupts, and the Watchdog (if enabled) to continue operating)
     *  SLEEP_MODE_PWR_SAVE    - supported by atiny25, atiny45, atiny85
     *  SLEEP_MODE_EXT_STANDBY - not supported by atiny25, atiny45, atiny85
     *  SLEEP_MODE_STANDBY     - not supported by atiny25, atiny45, atiny85
     *  SLEEP_MODE_PWR_DOWN    - the most power savings (all oscillators are stopped, only an External Reset, Watchdog Reset, Brown-out Reset, USI start condition interupt, an external level interrupt on INT0 or a pin change interrupt can wake up the MCU.      
*/
/**************************************************************************/
void ATtiny85_sleep()
{
  power_all_disable();                 //disable all peripheries (timer0, timer1, Universal Serial Interface, ADC)
  /*              
  power_adc_disable();                 //disable ADC
  power_timer0_disable();              //disable Timer0
  power_timer1_disable();              //disable Timer2
  power_usi_disable();                 //disable the Universal Serial Interface module.
  */
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //set the sleep type
  sleep_mode();                        /*system stops & sleeps here (automatically sets the sleep enable bit
                                         (so the sleep is possible), goes to sleep, wakes-up from sleep after an
                                         interrupt (if interrupts are enabled) or WDT timed out (if enabled) and
                                         clears the SE (Sleep Enable) bit afterwards).
                                         the sketch will continue from this point after interrupt or WDT timed out*/
}

/**************************************************************************/
/*
    setup_watchdog()
    
    Sleeps intervals: WDTO_15MS, WDTO_30MS, WDTO_60MS, WDTO_120MS, WDTO_250MS,
                      WDTO_500MS, WDTO_1S, WDTO_2S, WDTO_4S, WDTO_8S
    
    NOTE: runs from internal 128kHz clock and continues to work during the
          deepest sleep modes to provide a wake up source.
*/
/**************************************************************************/
void setup_watchdog(byte sleep_time)
{
  wdt_enable(sleep_time);
  WDTCR |= _BV(WDIE);     /*enable interrupts instead of MCU reset when watchdog is timed out
                            used for wake-up MCU from power-down*/
}

/**************************************************************************/
/*
    Watchdog Interrupt Service (executed when watchdog is timed out)
*/
/**************************************************************************/
ISR(WDT_vect)
{
  watchdog_counter++;
}
