#include <xc.h>
#include "I2C_LCD.h"
unsigned char RS, i2c_add, BackLight_State = LCD_BACKLIGHT;
//---------------[ I2C Routines ]-------------------
//--------------------------------------------------
void I2C_Master_Init(){
    SSP1CON1 = 0x28; //ssp控制暫存器 //0010 1000 
                     //bit7(WCOL：寫衝突檢測bit)=0 =發送時無衝突
                     //bit6(SSPOV：接收溢出指示符bit)=0=無溢出
                     //bit5(SSPEN：同步串列阜啟用bitSSPEN：同步串列阜啟用bit)=1= 啟用串列阜，並將SDA和SCL引腳配置為串列阜引腳的源
                     //bit4(CKP：clock極性選擇bit) master下不需要
                     //bit3~0(SSPM <3：0>：同步串列阜模式選擇bit)=1000=I2C master模式，clock= FOSC/（4 *（SSPxADD + 1））
    SSP1CON2 = 0x00; //ssp控制暫存器 //0000 0000 
                     //bit7(通用呼叫智能bit)=0 禁用通用呼叫地址
                     //bit6(acknowledge(確認) 狀態位) = ack 1 =未收到確認 0 =已收到確認
                     //bit5(acknowledge(確認)data bit)=在接收模式下：當用戶在接收結束時啟動確認序列時發送的值 1 =沒有確認 0 =確認
                     //bit4(ACKEN：應答序列智能bit)=0 =確認序列空閒
                     //bit3(RCEN：接收智能bit)=0 =先不接收
                     //bit2(PEN：停止條件智能bit)=0 =先不停止
                     //bit1(RSEN：重複啟動條件智能bit)=0=先不重複啟動條件
                     //bit0(SEN：啟動條件智能bit)=0 =先不起始
    
    SSP1STAT = 0x00; //狀態暫存器 //0000 0000 
                     //bit7(SM：SPI數據輸入採樣位)=0 =在數據輸出時間的中間對輸入數據進行採樣=在高速模式（400 kHz）下啟用slew rate control
                     //bit6(CKE: SPI時鐘邊沿選擇位)=0=0 =禁用SMBus特定的輸入
                     //bit5(D/A：數據/地址位)=0=指示接收或發送的最後一個字節是Address
                     //bit4(P: Stop bit)=0 =上次未檢測到stop bit
                     //bit3(S : Start bit)=0=最後未檢測到start bit
                     //bit2(R/W : 讀/寫位信息)=0=0 =發送不在進行中將該位與SEN，RSEN，PEN，RCEN或ACKEN進行或運算，將指示是否 MSSP處於空閒模式。
                     //bit0(BF： Buffer Full Status bit) = 0 = 接收未完成，SSPxBUF為空 = 數據發送完成（不包括ACK和stop bit），SSPxBUF為空
    
    SSP1ADD = ((_XTAL_FREQ *4)/I2C_BaudRate) - 1; //設定SCL的CLOCK
    SCL_D = 1;//設定SCL的腳位初始為1
    SDA_D = 1;//設定SDA的腳位初始為1
}
void I2C_Master_Wait(){
  while ((SSP1STAT & 0x04) || (SSP1CON2 & 0x1F));//SSP1STAT & 0x04代表S=1 =指示最後一次檢測到start bit（復位時該位為0）
                                                 //或
                                                 //ssp控制暫存器 //0001 1111
                                                 //bit7(通用呼叫智能bit)=0 禁用通用呼叫地址
                                                 //bit5(acknowledge(確認)data bit)=在接收模式下：當用戶在接收結束時啟動確認序列時發送的值 =0 =確認
                                                 //bit4(ACKEN：應答序列智能bit)=1 =在SDA和SCL引腳上啟動應答序列，並發送ACKDT data bit
                                                 //bit3(RCEN：接收智能bit)=1 =智能I2C的接收模式
                                                 //bit2(PEN：停止條件智能bit)=1 =在SDA和SCL引腳上啟動停止條件
                                                 //bit1(RSEN：重複啟動條件智能bit)=1 =在SDA和SCL引腳上啟動重複啟動條件
                                                 //bit0(SEN：啟動條件智能bit)=1 =在SDA和SCL引腳上啟動啟動條件
                                                 //結論:可能是start bit為1 或是 在智能腳位 或是 傳送有問題
}
void I2C_Master_Start(){//開始//讓scl=1 SDA: 1→0
  I2C_Master_Wait();
  SSP1CON2bits.SEN = 1; //在master模式下：1 =在SDA和SCL引腳上啟動啟動條件
}
void I2C_Master_RepeatedStart(){//重新開始//讓scl=1 SDA: 1→0
  I2C_Master_Wait();
  SSP1CON2bits.RSEN = 1; //1 =在SDA和SCL引腳上啟動重複啟動條件。
}
void I2C_Master_Stop(){//停止//讓scl=1 SDA: 0→1
  I2C_Master_Wait();
  SSP1CON2bits.PEN = 1; //1 =在SDA和SCL引腳上啟動停止條件
}
void I2C_ACK(void){//傳送沒問題
  SSP1CON2bits.ACKDT = 0; // 0 -> ACK
  I2C_Master_Wait();
  SSP1CON2bits.ACKEN = 1; // Send ACK
}
void I2C_NACK(void){//傳送有問題
  SSP1CON2bits.ACKDT = 1; // 1 -> NACK
  I2C_Master_Wait();
  SSP1CON2bits.ACKEN = 1; // Send NACK
}
unsigned char I2C_Master_Write(unsigned char data){//寫資料和回傳ACK
  I2C_Master_Wait();
  SSP1BUF = data;  //把要傳的東西丟到mssp1的buffer
  while(!SSP1IF); //SSP1IF=0=等待傳輸/接收/bus 狀態進度
  SSP1IF = 0;  //SSP1IF=1 =傳輸/接收/bus 狀態已完成
  return SSP1CON2bits.ACKSTAT;//回傳有沒有收到確認//1=沒//0=有
}
unsigned char I2C_Read_Byte(void){//打開接收和回傳接收到的資料
  I2C_Master_Wait();
  SSP1CON2bits.RCEN = 1; // 1 =智能I2C的接收模式
  while(!SSP1IF); //SSP1IF=0=等待傳輸/接收/bus 狀態進度
  SSP1IF = 0; //SSP1IF=1 =傳輸/接收/bus 狀態已完成
  I2C_Master_Wait();
  return SSP1BUF; // 回傳接收到的資料
}
//======================================================
//---------------[ LCD Routines ]----------------
//-----------------------------------------------
void LCD_Init(unsigned char I2C_Add){
  i2c_add = I2C_Add;//slave的位置
  IO_Expander_Write(0x00);
  __delay_ms(30);
  LCD_CMD(0x03);
  __delay_ms(5);
  LCD_CMD(0x03);
  __delay_ms(5);
  LCD_CMD(0x03);
  __delay_ms(5);
  LCD_CMD(LCD_RETURN_HOME);
  __delay_ms(5);
  LCD_CMD(0x20 | (LCD_TYPE << 2));
  __delay_ms(50);
  LCD_CMD(LCD_TURN_ON);
  __delay_ms(50);
  LCD_CMD(LCD_CLEAR);
  __delay_ms(50);
  LCD_CMD(LCD_ENTRY_MODE_SET | LCD_RETURN_HOME);
  __delay_ms(50);
}
void IO_Expander_Write(unsigned char Data)//寫資料
{
  I2C_Master_Start();//開始//讓scl=1//sda=1->0
  I2C_Master_Write(i2c_add);//slave的位置
  I2C_Master_Write(Data | BackLight_State);//有資料的話就寫資料，不然就依照BackLight_State 的狀態讓螢幕亮或暗
  I2C_Master_Stop();//停止//讓scl=1//sda=0->1
}
void LCD_Write_4Bit(unsigned char Nibble){
  // Get The RS Value To LSB OF Data
  Nibble |= RS;// Nibble = Nibble | RS
  IO_Expander_Write(Nibble | 0x04);
  IO_Expander_Write(Nibble & 0xFB);
  __delay_us(50);
}
void LCD_CMD(unsigned char CMD){//命令暫存器選擇
  RS = 0; //命令暫存器選擇
  LCD_Write_4Bit(CMD & 0xF0);
  LCD_Write_4Bit((CMD << 4) & 0xF0);
}
void LCD_Write_Char(char Data){
  RS = 1; // 數據暫存器選擇
  LCD_Write_4Bit(Data & 0xF0);
  LCD_Write_4Bit((Data << 4) & 0xF0);
}
void LCD_Write_String(char* Str){
  for(int i=0; Str[i]!='\0'; i++)
    LCD_Write_Char(Str[i]);//把字串一個一個字拆開，再寫出來
}
void LCD_Set_Cursor(unsigned char ROW, unsigned char COL){
  switch(ROW) {
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
void Backlight(){
  BackLight_State = LCD_BACKLIGHT;
  IO_Expander_Write(0);
}
void noBacklight(){
  BackLight_State = LCD_NOBACKLIGHT;
  IO_Expander_Write(0);
}
void LCD_SL(){
  LCD_CMD(0x18);
  __delay_us(40);
}
void LCD_SR(){
  LCD_CMD(0x1C);
  __delay_us(40);
}
void LCD_Clear(){
  LCD_CMD(0x01);
  __delay_us(40);
}