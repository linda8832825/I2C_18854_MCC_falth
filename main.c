#include "mcc_generated_files/mcc.h"
#include "I2C_LCD.c"
#include <htc.h>
#include "I2C_LCD.h"
void main(void)
{
    SYSTEM_Initialize();
    TRISAbits.TRISA1=0;
    I2C_Master_Init();
    LCD_Init(0x4E); // Initialize LCD module with I2C address = 0x4E/////////////////////////////////
    while (1)
    {
        PORTAbits.RA1=0;
        __delay_ms(1000);
        PORTAbits.RA1=1;
        __delay_ms(1000);
        LCD_Set_Cursor(1, 1);
        LCD_Write_String(" Khaled Magdy");
        LCD_Set_Cursor(2, 1);
        LCD_Write_String(" DeepBlue");
    }
        return;
}