/* ------------------------------------------------------------------------------

S311017 - HW ref: A020216-R280617  CPLD Fun Board

NOTE: It is required the H311017 configuration file to be loaded into the CPLD.

Driving a custom 7-segments 4 digits led display interface.
Drive a custom interface to display four BCD digits to a 7-segments display.
USR1 button increments units, USR2 button increments tens, USR3 button increments hundreds and BUT button increments thousands.
Holding a button pressed will cause auto increments.
The display multiplexing is done internally in the CPLD (see H311017 CPLD configuration file).

--------------------------------------------------------------------------------- */


// ------------------------------------------------------------------------------
//
// Hardware definitions for A020216-R280617
//
// ------------------------------------------------------------------------------

#define LED_PB1 PB1       // HIGH = ON
#define USR1    PC13      // USR1 button (LOW when pressed)
#define USR2    PC14      // USR2 button (LOW when pressed)
#define USR3    PC15      // USR3 button (LOW when pressed)
#define BUT     PB8       // BUT button (HIGH when pressed)

// ------------------------------------------------------------------------------
//
// Hardware definitions for the H311017 CPLD configuration file
//
// ------------------------------------------------------------------------------

#define DATA_0  PA0        // BCD data line 0
#define DATA_1  PA1        // BCD data line 1
#define DATA_2  PA2        // BCD data line 2
#define DATA_3  PA3        // BCD data line 3
#define SEL_0   PA5        // Digit register address line 0
#define SEL_1   PA4        // Digit register address line 1
#define SEL_WE  PB0        // Digit register write enable

// ------------------------------------------------------------------------------
//
//  Global variables
//
// ------------------------------------------------------------------------------

byte    digit0, digit1, digit2, digit3;
boolean keyPressed = false;

// ------------------------------------------------------------------------------

void setup() 
{
  // Set the MCU side for the the H311017 CPLD configuration file
  // Note: the pull-up resistors are already set inside the CPLD
  pinMode(DATA_0, OUTPUT);
  pinMode(DATA_1, OUTPUT);
  pinMode(DATA_2, OUTPUT);
  pinMode(DATA_3, OUTPUT);
  pinMode(SEL_0, OUTPUT);
  pinMode(SEL_1, OUTPUT);
  pinMode(SEL_WE, OUTPUT);
  digitalWrite(SEL_WE, HIGH);
 
  // Set the MCU buttons and led
  pinMode(LED_PB1, OUTPUT);
  pinMode(USR1, INPUT_PULLUP);
  pinMode(USR2, INPUT_PULLUP);
  pinMode(USR3, INPUT_PULLUP);
  pinMode(BUT, INPUT);
  digitalWrite(LED_PB1, LOW);  // Led OFF

  // Set a 36MHz output clock on MCO/PA8 pin
  clockOut(2);                 

  // Initialize all the digits
  digit0 = 0;
  digit1 = 0;
  digit2 = 0;
  digit3 = 0;
}

void loop() 
{
  // Check if any digit button is pressed
  if (!digitalRead(USR3)) 
  {
    digit0++;
    keyPressed = true;
  }
  if (!digitalRead(USR2)) 
  {
    digit1++;
    keyPressed = true;
  }
  if (!digitalRead(USR1)) 
  {
    digit2++;
    keyPressed = true;
  }
  if (digitalRead(BUT)) 
  {
    digit3++;
    keyPressed = true;
  }

  // Adjust BCD digits overrun if any digit button was pressed
  if (keyPressed)
  {
    digitalWrite(LED_PB1, HIGH);
    if (digit0 > 9)
    {
      digit0 = 0;
      digit1++;
    }
    if (digit1 > 9)
    {
      digit1 = digit1 % 10;
      digit2++;
    }
    if (digit2 > 9)
    {
      digit2 = digit2 % 10;
      digit3++;
    }
    if (digit3 > 9)
    {
      digit3 = digit3 % 10;
    }

    // Write current digits into the display registers
    writeDigit(digit0,0);
    writeDigit(digit1,1);
    writeDigit(digit2,2);
    writeDigit(digit3,3);
    
    delay(300);   // Simple debounce delay
    keyPressed = false;
    digitalWrite(LED_PB1, LOW);
  }
}

// ------------------------------------------------------------------------------

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
      return;                                           // Do nothing and exit immediately
    break;
  }
  GPIOA->regs->CRH = GPIOA->regs->CRH | 0x00000003;     // Set output mode on PA8 @ max freq 50MHz (MODE8[1:0] = 11)
  GPIOA->regs->CRH = GPIOA->regs->CRH & 0xFFFFFFF3;     // Set output mode on PA8 as general push-pull output (cnf8[1:0] = 00)
  GPIOA->regs->CRH = GPIOA->regs->CRH | 0x00000008;     // Set output mode on PA8 as alternate push-pull output (cnf8[1:0] = 10)
}

// ------------------------------------------------------------------------------

void writeDigit(byte digitBCD, byte regAddress)
// Write a BCD digit into one of the 7-segments display registers. The are 4 registers, each store a BCD digit. Each register is 4 bit wide.
// The address of each register (two bit wide) is equal of the weight of the BCD digit (e.g. the adress 0x0 is for the least significant BCD digit register)
// Usage:
//  digitBCD is the 4 bit BCD digit to write, in the least significant nibble of digitBCD;
//  regAddress is the two bit address of the display register to write (from 0x0 to 0x3).
{
  // Write DATA lines (BCD digit value)
  digitalWrite(DATA_3, HIGH && (digitBCD & B00001000));
  digitalWrite(DATA_2, HIGH && (digitBCD & B00000100));
  digitalWrite(DATA_1, HIGH && (digitBCD & B00000010));
  digitalWrite(DATA_0, HIGH && (digitBCD & B00000001));

  // Write SEL lines (display register address)
  digitalWrite(SEL_1, HIGH && (regAddress & B00000010));
  digitalWrite(SEL_0, HIGH && (regAddress & B00000001));

  // Write into the display register using a pulse on the SEL_WE line
  digitalWrite(SEL_WE, LOW);
  digitalWrite(SEL_WE, HIGH);
}

