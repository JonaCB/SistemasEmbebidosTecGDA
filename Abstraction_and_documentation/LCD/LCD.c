
#include "LCD.h"
#include "../uc_i2c/uc_i2c.h"
#include "../delay/delay.h"
#include "../miniprintf/miniprintf.h"

//based and adapted from https://deepbluembedded.com/interfacing-i2c-lcd-16x2-tutorial-with-pic-microcontrollers-mplab-xc8/

unsigned char RS, i2c_add, BackLight_State = LCD_BACKLIGHT;

void LCD_Init(unsigned char I2C_Add)
{
  i2c_add = I2C_Add;
  IO_Expander_Write(0x00);
  delay_ms(50);  // wait for >40ms
  LCD_CMD(0x03);
  delay_ms(5);
  LCD_CMD(0x03);
  delay_ms(1);
  LCD_CMD(0x03);
  delay_ms(1);
  LCD_CMD(LCD_RETURN_HOME);
  delay_ms(1);
  LCD_CMD(0x20 | (LCD_TYPE << 2));
  delay_ms(1);
  LCD_CMD(LCD_TURN_ON);
  delay_ms(1);
  LCD_CMD(LCD_CLEAR);
  delay_ms(1);
  LCD_CMD(LCD_ENTRY_MODE_SET | LCD_RETURN_HOME);
  delay_ms(1);
}

void IO_Expander_Write(unsigned char Data)
{
  i2c_write_8bits(i2c_add, Data | BackLight_State);
}

void LCD_Write_4Bit(unsigned char Nibble)
{
  // Get The RS Value To LSB OF Data
  Nibble |= RS;
  IO_Expander_Write(Nibble | 0x04);
  IO_Expander_Write(Nibble & 0xFB);
  delay_ms(1);
}

void LCD_CMD(unsigned char CMD)
{
  RS = 0; // Command Register Select
  LCD_Write_4Bit(CMD & 0xF0);
  LCD_Write_4Bit((CMD << 4) & 0xF0);
}

void LCD_Write_Char(char Data)
{
  RS = 1; // Data Register Select
  LCD_Write_4Bit(Data & 0xF0);
  LCD_Write_4Bit((Data << 4) & 0xF0);
}

void LCD_Write_String(char* Str)
{
  for(int i=0; Str[i]!='\0'; i++)
    LCD_Write_Char(Str[i]);
}

void LCD_Set_Cursor(unsigned char ROW, unsigned char COL)
{
  switch(ROW) 
  {
    case 2:
      LCD_CMD(0xC0 + COL-1);
      break;
    case 3:
      LCD_CMD(0x94 + COL-1);
      break;
    case 4:
      LCD_CMD(0xD4 + COL-1);
      break;
    // Case 1
    default:
      LCD_CMD(0x80 + COL-1);
  }
}


void LCD_putc(char ch)  {
    LCD_Write_Char(ch);
}

/**
 * Prints UART message
 * param[in] format
 * param[out] rc
 */
int LCD_printf(const char *format, ...)  {
    va_list args;
    int rc;

    va_start(args, format);
    rc = mini_vprintf_cooked(LCD_putc, format, args);
    va_end(args);
    return rc;
}

