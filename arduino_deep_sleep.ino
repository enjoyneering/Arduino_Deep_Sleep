/***************************************************************************************************/
/*
  Arduino example of the proper and most advanced way to put any AVR based Arduino
  boards into sleep
  
  Sketch puts Arduino in to the deep sleep for ~30 sec. Wakes it up, turns on/off LED connected
  to pin0/pin13 & puts Arduino back to sleep.

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

#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#define LED 0
#else
#define LED LED_BUILTIN
#endif

/**************************************************************************/
/*
    variables
*/
/**************************************************************************/
volatile byte watchdogCounter = 0;

/**************************************************************************/
/*
    all_pins_output()
    
    Sets all pins as output, to prevent the MCU from waking up from a
    random noise signal during sleep, use if needed
    
    NOTE:
    - INPUT_PULLUP connects internal 20k..30k pullups resistors & increase
      consumption by 1.25uA
*/
/**************************************************************************/
void all_pins_output()
{
  for (byte pin = 0; pin < 5; pin++) //ATtiny85 has 5 pins
  {
    pinMode(pin, INPUT_PULLUP);
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
  for (byte pin = 0; pin < 5; pin++) //ATtiny85 has 5 pins
  {
    pinMode(pin, INPUT);
  }
}

/**************************************************************************/
/*
    setup()

    Main setup

    NOTE:
    - valid sleep intervals: WDTO_15MS,  WDTO_30MS,  WDTO_60MS, WDTO_120MS
                             WDTO_250MS, WDTO_500MS, WDTO_1S,   WDTO_2S
                             WDTO_4S,    WDTO_8S
*/
/**************************************************************************/
void setup()
{
  pinMode(LED, OUTPUT);
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
  while (watchdogCounter < 4) //wait for watchdog counter reached the limit, WDTO_8S * 4 = 32sec.
  {
    //all_pins_output();
    arduino_sleep();
  }
  
  //wdt_disable();            //disable & stop wdt timer
  watchdogCounter = 0;        //reset counter

  power_all_enable();         //enable all peripheries (ADC, Timer0, Timer1, Universal Serial Interface)
  /*
  power_adc_enable();         //enable ADC
  power_timer0_enable();      //enable Timer0
  power_timer1_enable();      //enable Timer1
  power_usi_enable();         //enable the Universal Serial Interface module
  */
  delay(5);                   //to settle down ADC & peripheries

  digitalWrite(LED, HIGH);    //led blink
  delay(3000);
  digitalWrite(LED, LOW);

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
    - another useful functions:
      *  sleep_enable()         - enable sleep, set Sleep Enable (SE) bit
      *  sleep_cpu()            - system stops & sleeps here, but doesn't automatically sets/clears Sleep Enable (SE) bit like sleep_mode()
      *  sleep_disable()        - disable sleep, clears Sleep Enable (SE) bit
      
*/
/**************************************************************************/
void arduino_sleep()
{
  cli();                               //disable interrupts for time critical operations below

  power_all_disable();                 //disable all peripheries (ADC, Timer0, Timer1, Universal Serial Interface)
  /*              
  power_adc_disable();                 //disable ADC
  power_timer0_disable();              //disable Timer0
  power_timer1_disable();              //disable Timer2
  power_usi_disable();                 //disable the Universal Serial Interface module
  */
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //set sleep type

  #if defined(BODS) && defined(BODSE)  //if MCU has bulit-in BOD it will be disabled, ATmega328P, ATtiny85, AVR_ATtiny45, ATtiny25  
  sleep_bod_disable();                 //disable Brown Out Detector (BOD) before going to sleep, saves more power
  #endif

  sei();                               //re-enable interrupts

  sleep_mode();                        /*
                                         system stops & sleeps here, it automatically sets Sleep Enable (SE) bit, 
                                         so sleep is possible, goes to sleep, wakes-up from sleep after interrupt,
                                         if interrupt is enabled or WDT enabled & timed out, than clears the SE bit.
                                       */

  /*** NOTE: sketch will continue from this point after sleep ***/
}

/**************************************************************************/
/*
    setup_watchdog()

    Sets up watchdog to trigger interrupt, not reset, after watchdog is
    timed out
    
    NOTE:
    - WDT runs from internal 128kHz clock & continues to work during the
      deepest sleep modes to provide a wake up source
    - Your can configure watchdog timer through Watchdog Timer Control
      Register WDTCSR/WDTCR 
    - Watchdog timer control register has seven configuration bits:
      7     6     5     4     3    2     1     0
      WDIF, WDIE, WDP3, WDCE, WDE, WDP2, WDP1, WDP0
    - Following  prescaler 4 bits determine how long the timer will count
      for before resetting:
      WDP  WDP  WDP  WDP  Time-out  Macro
      0    0    0    0    16ms      WDTO_15MS
      0    0    0    1    32ms      WDTO_30MS
      0    0    1    0    64ms      WDTO_60MS
      0    0    1    1    125ms     WDTO_120MS
      0    1    0    0    250ms     WDTO_250MS
      0    1    0    1    500ms     WDTO_500MS
      0    1    1    0    1000ms    WDTO_1S
      0    1    1    1    2000ms    WDTO_2S
      1    0    0    0    4000ms    WDTO_4S
      1    0    0    1    8000ms    WDTO_8S
    - WDE bit Watchdog System Reset Enable, it is overridden by WDRF bit
      in MCUSR/MCU Status Register. This means that WDE is always set
      when WDRF is set. To clear WDE, WDRF must be cleared first. This
      feature ensures multiple resets during conditions causing failure
      & safe start-up after the failure
    - WDCE bit is Watchdog Change Enable, it is used in timed sequences for
      changing WDE & prescaler bits. To clear the WDE bit or change 
      prescaler bits, WDCE must be set. Once written to one, hardware will 
      clear WDCE after four clock cycles.
    - WDIE bit is Watchdog Interrupt Enable. When this bit is written to
      one & I-bit in MCUSR is set, the Watchdog Interrupt is enabled.
      If WDE is cleared in combination with this setting, the Watchdog Timer
      is in Interrupt Mode & corresponding interrupt is executed if time-out
      in the Watchdog Timer occurs.
      If WDE is set, the Watchdog Timer is in Interrupt & System Reset Mode.
      The first time-out in the Watchdog Timer will set WDIF.
      Executing corresponding interrupt vector will clear WDIE & WDIF
      automatically by hardware, Watchdog goes to System Reset Mode.
      This is useful for keeping the Watchdog Timer security while using
      interrupt. To stay in Interrupt & System Reset Mode, WDIE must be set
      after each interrupt. This shouldn't be done within interrupt
      service routine itself, as this might compromise the safety-function
      of Watchdog System Reset mode. If the interrupt is not executed before
      next time-out, a System Reset will be applied.
    - Watchdog Timer Configuration:
      WDTON  WDE  WDIE  Mode Action    Time-out Action
      1      0    0     Stopped        None
      1      0    1     Interrupt      Interrupt
      1      1    0     System Reset   Reset
      1      1    1     Interrupt &    Interrupt then go to System Reset
                        System Reset
      0      x    x     System Reset   Reset
      WDTON fuse set to "0" means programmed & "1" means unprogrammed
    - WDIF is Watchdog System Interrupt Flag & automatically flagged high
      and low by the system if interrupt happend.
    - MCUSR is MCU Status Register, it provides information on which reset
      source caused an MCU reset. MCUSR bit 3 is WDRF Watchdog System Reset
      Flag. his bit is set if a Watchdog System Reset occurs. The bit is
      reset by a Power-on Reset, or by writing a logic zero to the flag.
    - Watchdog is a timer, if you donʼt reset it regularly it will time-out.
      Just call "wdt_reset()" before timer expires, otherwise a watchdog
      will reset MCU.
    - Macros _BV(bit)=(1 << (bit)) converts bit number into a byte value.
*/
/**************************************************************************/
void setup_watchdog(byte sleep_time)
{
  cli();                           //disable interrupts for time critical operations below

  MCUSR &= ~_BV(WDRF);             //must be cleared first, to clear WDE

  wdt_enable(sleep_time);          //set WDCE, WDE change prescaler bits

  #if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
  WDTCR  |= _BV(WDCE) & ~_BV(WDE); //set WDCE first, clear WDE second, changes have to be done within 4-cycles
  WDTCR  |= _BV(WDIE);             //set WDIE to Watchdog Interrupt
  #else
  WDTCSR |= _BV(WDCE) & ~_BV(WDE); //set WDCE first, clear WDE second, changes have to be done within 4-cycles
  WDTCSR |= _BV(WDIE);             //set WDIE to Watchdog Interrupt
  #endif

  sei();                           //re-enable interrupts
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
