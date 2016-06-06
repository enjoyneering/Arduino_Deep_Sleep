/************************************************************************************************************/
/*
  "Arduino_deep_sleep" is an example of the proper and most advanced way to
  put ANY Arduino boards to sleep.
  
  Puts any arduino in to deep sleep for 30 sec. Wakes it up and turns on LED
  connected to the pin 0 and puts arduino back to sleep.
  
  Average ATtiny85 consumption in deep sleep mode is about 3 microamps
  at 3.3 volt.

  written by enjoyneering79
*/
/************************************************************************************************************/
#include <avr/sleep.h>     //AVR MCU power management.
#include <avr/power.h>     //AVR MCU peripheries (Analog comparator, ADC, USI, Timers/Counters etc) management.
#include <avr/wdt.h>       //AVR MCU watchdog timer management.
#include <avr/io.h>        //AVR MCU IO ports management.
#include <avr/interrupt.h> //AVR MCU interrupt flags management.


/************************************************************************************************************/
/*
    Constants

    Enables interrupts (instead of MCU reset), when watchdog is timed out.
    Used for wake-up MCU from power-down/sleep.
*/
/************************************************************************************************************/
#define WDTCR |= _BV(WDIE)


/************************************************************************************************************/
/*
    Variables
*/
/************************************************************************************************************/
byte led_pin = 0;
volatile byte watchdog_counter;


/************************************************************************************************************/
/*
    all_pins_output()
    
    Sets all pins as output to prevent MCU wake up from a random noise signal
    during a sleep. This function is optional.
    
    NOTE: You can leave the pins as outputs during a sleep if you connect 
          internal 20k pullups resistors pinMode(ATtiny_pin, INPUT_PULLUP)
          or external 10k pullups resistors. However it will increase the
          consumption by 1.25 microamps.
*/
/************************************************************************************************************/
void all_pins_output()
{
  for (byte ATtiny_pin; ATtiny_pin < 5; ATtiny_pin++)
  {
    pinMode(ATtiny_pin, OUTPUT);
  }
}


/************************************************************************************************************/
/*
    all_pins_input()
    
    Sets all ATtiny pins as input. This function is optional.
*/
/************************************************************************************************************/
void all_pins_input()
{
  for (byte ATtiny_pin; ATtiny_pin < 5; ATtiny_pin++)
  {
    pinMode(ATtiny_pin, INPUT);
  }
}


/************************************************************************************************************/
/*
    Setup()

    Main setup
*/
/************************************************************************************************************/
void setup()
{
  pinMode(led_pin,OUTPUT);
  setup_watchdog(WDTO_8S); //approximately 8 sec. of sleep
}


/************************************************************************************************************/
/*
    loop()

    Main loop
*/
/************************************************************************************************************/
void loop()
{
  while (watchdog_counter < 4) //wait for watchdog counter reched the limit (WDTO_8S * 4 = 32sec.)
  {
    ATtiny85_sleep();
  }
  
  watchdog_counter = 0;     //reset watchdog_counter
  power_all_enable();       //enable all peripheries (timer0, timer1, Universal Serial Interface, ADC)
  /*
  power_adc_enable();       //enable ADC
  power_timer0_enable();    //enable Timer0
  power_timer1_enable();    //enable Timer1
  power_usi_enable();       //enable the Universal Serial Interface module
  */
  delay(5);                 //to settle down the ADC and peripheries

  //all_pins_output();
  digitalWrite(led_pin,HIGH);  //led blink
  delay(3000);
  digitalWrite(led_pin,LOW);
  //all_pins_output();
}


/************************************************************************************************************/
/*
    ATtiny85_sleep()

    Puts MCU into the sleep state

    NOTE: There are 6 different sleeps modes:
          * SLEEP_MODE_IDLE..........The least power savings state. CPU stopped but Analog
                                     comparator, ADC, USI, Timer/Counter, Watchdog (if enabled),
                                     & the interrupt system continues operating. (by default in "sleep.h")
          * SLEEP_MODE_ADC...........ADC Noise Reduction. CPU stopped but the ADC, the external
                                     interrupts, & the Watchdog (if enabled) continue operating.
          * SLEEP_MODE_PWR_SAVE......Supported by Atiny25, Atiny45, Atiny85.
          * SLEEP_MODE_EXT_STANDBY...Not supported by Atiny25, Atiny45, Atiny85.
          * SLEEP_MODE_STANDBY.......Not supported by Atiny25, Atiny45, Atiny85.
          * SLEEP_MODE_PWR_DOWN......The most power savings state. All oscillators are stopped, only an
                                     External Reset, Watchdog Reset, Brown-out Reset, USI start condition
                                     interupt & external level interrupt on INT0 or a pin change interrupt
                                     can wake up the MCU.      
*/
/************************************************************************************************************/
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
  sleep_mode();                        /*system stops & sleeps here (automatically sets the SE (Sleep Enable) bit
                                         (so the sleep is possible), goes to sleep, wakes-up from sleep after an
                                         interrupt (if interrupts are enabled) or WDT timed out (if enabled) and
                                         clears the SE (Sleep Enable) bit afterwards).
                                         the sketch will continue from this point after interrupt or WDT timed out
                                       */
}


/************************************************************************************************************/
/*
    setup_watchdog()
    
    Sleeps intervals: WDTO_15MS, WDTO_30MS, WDTO_60MS, WDTO_120MS, WDTO_250MS,
                      WDTO_500MS, WDTO_1S, WDTO_2S, WDTO_4S, WDTO_8S
    
    NOTE: The MCU watchdog runs from internal 128kHz clock and continues to work
          during the deepest sleep modes to provide a wake up source.
*/
/************************************************************************************************************/
void setup_watchdog(byte sleep_time)
{
  wdt_enable(sleep_time);
}

/************************************************************************************************************/
/*
    ISR(WDT_vect)
    
    Watchdog Interrupt Service (automatically executed when watchdog is timed out)
*/
/************************************************************************************************************/
ISR(WDT_vect)
{
  watchdog_counter++;
}
