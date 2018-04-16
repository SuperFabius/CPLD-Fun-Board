/* ------------------------------------------------------------------------------

S161017 - HW ref: A020216 / A020216-R280617  CPLD Fun Board

NOTE: It is required the H161017 configuration file loaded into the CPLD.

Blink test.
Blink LED_PB1, LED1 and LED2.
LED_PB1 blinks driven by the MCU.
LED1 blinks using the clock generated from the STM32F103 MCU (PA8/MCO pin 29) and divided inside the CPLD.
LED2 uses the clock from the optional 50MHz oscillator and divided inside the CPLD, so it blinks only if the oscillator is present.

--------------------------------------------------------------------------------- */


// ------------------------------------------------------------------------------
//
// Hardware definitions for A020216-R280617
//
// ------------------------------------------------------------------------------

#define LED_PB1 PB1       // HIGH = ON
#define USER1   PC13      // USER1 button (LOW when pressed)
#define USER2   PC14      // USER2 button (LOW when pressed)
#define USER3   PC15      // USER3 button (LOW when pressed)
#define BUT     PB8       // BUT button (HIGH when pressed)

void setup() 
{
  // Set the MCU side for the the H161017 CPLD configuration file
  pinMode(LED_PB1, OUTPUT);
  pinMode(USER1, INPUT_PULLUP);
  pinMode(USER2, INPUT_PULLUP);
  pinMode(USER3, INPUT_PULLUP);
  pinMode(BUT, INPUT);
  digitalWrite(LED_PB1, LOW);  // Led OFF
  clockOut(2);                 // Set a 36MHz output clock on MCO/PA8 pin
}

void loop() 
{
  // Blink LED_PB1
  delay(500);
  digitalWrite(LED_PB1, !digitalRead(LED_PB1)); 
}

void clockOut(byte mode)
// Set the MCO pin to output the clock to the CPLD (GCLK1 pin 14). See STM32F103 Reference Manual RM0008.
// Valid mode values:
//  mode = 1 -> 8MHz clock
//  mode = 2 -> 36MHz clock
{
  switch (mode)
  {
    case 1:
      RCC_BASE->CFGR =  RCC_BASE->CFGR | 0x06000000;    // Set MCO[2:0] = 110 (inside RCC_CFGR register) for 8MHz clock output
    break;

    case 2:
      RCC_BASE->CFGR =  RCC_BASE->CFGR | 0x07000000;    // Set MCO[2:0] = 111 (inside RCC_CFGR register) for 36MHz clock output
    break;

    default:
      return;                                           // Do nothing and exit
    break;
  }
  GPIOA->regs->CRH = GPIOA->regs->CRH | 0x00000003;     // Set output mode on PA8 @ max freq 50MHz (MODE8[1:0] = 11)
  GPIOA->regs->CRH = GPIOA->regs->CRH & 0xFFFFFFF3;     // Set output mode on PA8 as general push-pull output (cnf8[1:0] = 00)
  GPIOA->regs->CRH = GPIOA->regs->CRH | 0x00000008;     // Set output mode on PA8 as alternate push-pull output (cnf8[1:0] = 10)
}

