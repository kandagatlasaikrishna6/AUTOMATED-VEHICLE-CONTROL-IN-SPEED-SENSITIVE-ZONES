/*
 * File:   RF_HC05.c
 * Author: Prg
 *
 * Created on November 2, 2018, 10:44 PM
 */

// Including the necessary Header files //
#include <xc.h> 
#include "Configuration_Bits.h" 
#include "USART.h"

// MOTOR Pins
#define M1_C LATDbits.LATD0       //PIN 0 of PORTD is assigned for Motor1 Clockwise 
#define M1_A LATDbits.LATD1       //PIN 1 of PORTD is assigned for Motor1 Anti Clockwise
#define M2_C LATDbits.LATD3       //PIN 3 of PORTD is assigned for Motor2 Clockwise
#define M2_A LATDbits.LATD2       //PIN 2 of PORTD is assigned for Motor2 Anti Clockwise
// LCD Pins
#define RS LATB0                    /*PIN 0 of PORTB is assigned for register select Pin of LCD*/
#define EN LATB1                    /*PIN 1 of PORTB is assigned for enable Pin of LCD */
#define ldata LATB                  /*PORTB(PB4-PB7) is assigned for LCD Data (D4-D7) Output*/ 
#define LCD_Port TRISB              /*define macros for PORTB Direction Register*/ 

// Declaring the necessary Functions //
void MSdelay_L(unsigned int );        /*Generate delay in ms*/
void LCD_Init();                    /*Initialize LCD*/
void LCD_Command(unsigned char );   /*Send command to LCD*/
void LCD_Char(unsigned char x);     /*Send data to LCD*/
void LCD_String(const char *);      /*Display data string on LCD*/
void LCD_String_xy(char, char , const char *);
void LCD_Clear();                   // LCD Clear
void mspeedfunc(int);               // Motor Speed funtion
void bluetoothint();                // Bluetooth Intialize function
int mspeedval=0;
char BT_data_in = '0'; // Input received from BT
int fbtext = 0; //Used to print msg only once in BT

//  Initializing the USART   //
void USART_Init(long baud_rate) 
{ 
    float temp; 
    ANSELC=1; 
    TRISC6=1; /*Make Tx pin as output*/ 
    TRISC7=1; /*Make Rx pin as input*/ 
    temp=Baud_value; 
    SPBRG=(int)temp; /* SPBRG=(F_CPU /(64*9600))-1*/ 
    TXSTA=0x20; /*Transmit Enable(TX) enable*/ 
    RCSTA=0x90; /*Receive Enable(RX) enable and serial port enable */ 
} 
void USART_TransmitChar(char out) 
{ 
    while(TXIF==0); /*wait for transmit interrupt flag*/ 
    TXREG=out; 
} 
char USART_ReceiveChar() 
{ 
    while(RCIF==0); /*wait for receive interrupt flag*/ 
    return(RCREG); /*receive data is stored in RCREG register and return to main program */ 
} 
void USART_SendString(const char *out) 
{ 
    while(*out!='\0') 
    { 
        USART_TransmitChar(*out);     // Printing of string at the exact location
        out++; 
    } 
} 
void MSdelay(unsigned int val) 
{ 
    unsigned int i,j;              /*This count Provide delay of 1 ms for 8MHz Frequency */
    for(i=0;i<=val;i++) 
    for(j=0;j<81;j++); 
} 

void main()
{
    OSCCON=0x62;              /* use internal oscillator frequency
                             which is set to * 8MHz */
    TRISC1 = 0;
    TRISC2 = 0;              /* set PORT as output port */
    PORTCbits.CCP1 = 0;
    PORTCbits.CCP2 = 0;
    PR2 = 0b00011000;
    TRISD = 0;               // set PORT as output port
    T2CON = 0b00000100;
    USART_Init(9600);         /* initialize USART operation with 9600 baud rate */ 
    
    USART_SendString(" Ready "); // Send message via Bluetooth
    LCD_Init();                           /*Initialize LCD to 5*8 matrix in 4-bit mode*/    
	LCD_String_xy(1,3," WELCOME");           /*Display string on 1st row, 5th location*/
    LCD_String_xy(2,0,"TESLA V2.0"); /*Display string on 2nd row,1st location*/           	
    MSdelay(25);  //Time granted for Settle before start
    
    while(1)
    {
        fbtext = 0;
        if(RCIF==1)                 //Check Bluetooth and receive it's value
        {
            BT_data_in = RCREG;
            fbtext = 1;
        }
        
        if(PORTCbits.RC0 == 1) // If RF signal == True (Entering School Zone)
        {
            //PWM [Higher value of CCPRxL = Higher Speed or Duty Cycle]
            
            CCPR1L = 0b00001101;	//Slow
            CCP1CON = 0b00001100;
            
            CCPR2L = 0b00001101;	//Slow
            CCP2CON = 0b00001100;
            MSdelay(1); //Time granted for Settle
            
            if(fbtext == 1)
            {
            USART_SendString(" School Zone ");
            fbtext = 0;
            } // Send message via Bluetooth 
            LCD_String_xy(1,3,"CAUTION!!!");           /*Display string on 1st row, 5th location*/
            LCD_String_xy(2,0,"SchoolZone 20MPH"); /*Display string on 2nd row,1st location*/
                
        }
        else //if (PORTCbits.RC0 == 0) 
        {
            
            LCD_String_xy(1,3," WELCOME       ");           /*Display string on 1st row, 5th location*/
            LCD_String_xy(2,0,"  TESLA V2.0     "); /*Display string on 2nd row,1st location*/
            CCPR1L = 0b11111111;	//100% speed of motor1
            CCP1CON = 0b00001100;
            
            CCPR2L = 0b11111111;	//100% speed of motor2
            CCP2CON = 0b00001100;
            MSdelay(1); //Time granted for Settle
             
        }
        
        MSdelay(1); //Time granted for Settle
        bluetoothint();   // initialise Bluetooth
    }
    
}

void LCD_Init()
{
    LCD_Port = 0;                   /*PORT as Output Port*/
    MSdelay_L(15);                    /*15ms,16x2 LCD Power on delay*/
    LCD_Command(0x02);              /*send for initialization of LCD 
                                     *for nibble (4-bit) mode */
    LCD_Command(0x28);              /*use 2 line and 
                                     *initialize 5*8 matrix in (4-bit mode)*/
	LCD_Command(0x01);              /*clear display screen*/
    LCD_Command(0x0c);              /*display on cursor off*/
	LCD_Command(0x06);              /*increment cursor (shift cursor to right)*/	   
}

void LCD_Command(unsigned char cmd )
{
	ldata = (ldata & 0x0f) |(0xF0 & cmd);   /*Send higher nibble of command first to PORT*/ 
	RS = 0;                                 /*Command Register is selected i.e.RS=0*/ 
	EN = 1;                                 /*High-to-low pulse on Enable pin to latch data*/ 
	NOP();
	EN = 0;
	MSdelay_L(1);
    ldata = (ldata & 0x0f) | (cmd<<4);      /*Send lower nibble of command to PORT */
	EN = 1;
	NOP();
	EN = 0;
	MSdelay_L(3);
}


void LCD_Char(unsigned char dat)
{
	ldata = (ldata & 0x0f) | (0xF0 & dat);   /*Send higher nibble of data first to PORT*/
	RS = 1;                                  /*Data Register is selected*/
	EN = 1;                                  /*High-to-low pulse on Enable pin to latch data*/
	NOP();
	EN = 0;
	MSdelay_L(1);
    ldata = (ldata & 0x0f) | (dat<<4);               /*Send lower nibble of data to PORT*/
	EN = 1;                         /*High-to-low pulse on Enable pin to latch data*/
	NOP();
	EN = 0;
	MSdelay_L(3);
}
void LCD_String(const char *msg)
{
	while((*msg)!=0)
	{		
	  LCD_Char(*msg);    // Printing of string at the exact location
	  msg++;	
    }
}

void LCD_String_xy(char row,char pos,const char *msg)
{
    char location=0;
    if(row<=1)
    {
        location=(0x80) | ((pos) & 0x0f);      /*Print message on 1st row and desired location*/
        LCD_Command(location);
    }
    else
    {
        location=(0xC0) | ((pos) & 0x0f);      /*Print message on 2nd row and desired location*/
        LCD_Command(location);    
    }  
    

    LCD_String(msg);

}
void LCD_Clear()
{
   	LCD_Command(0x01);     /*clear display screen*/
}

void MSdelay_L(unsigned int val)
{
 unsigned int i,j;
 for(i=0;i<=val;i++)
     for(j=0;j<81;j++);             /*This count Provide delay of 1 ms for 8MHz Frequency */
 }


void bluetoothint()
{
    

        if(BT_data_in=='1')
        {   
            mspeedval = 1;
            
            if(fbtext == 1)
            {
            USART_SendString(" Back ");//USART_SendString("Back"); // Send message via Bluetooth
            fbtext = 0;
            } // Send message via Bluetooth 
            
        }
        else if(BT_data_in=='2')
        {
            mspeedval = 2;
            
            if(fbtext == 1)
            {
            USART_SendString(" Forward ");//USART_SendString("Forward"); // Send message via Bluetooth
            fbtext = 0;
            } // Send message via Bluetooth 
           
        }
        else if(BT_data_in=='3')
        {
            mspeedval = 3;
            
            if(fbtext == 1)
            {
            USART_SendString(" Left ");//USART_SendString("Left"); // Send message via Bluetooth
            fbtext = 0;
            } // Send message via Bluetooth 
            
        }
        else if(BT_data_in=='4')
        {
            mspeedval = 4;
            
            if(fbtext == 1)
            {
            USART_SendString(" Right ");//USART_SendString("Right"); // Send message via Bluetooth
            fbtext = 0;
            } // Send message via Bluetooth 
            
        }
        else if(BT_data_in=='5')
        {
            mspeedval = 0;
            
            if(fbtext == 1)
            {
            USART_SendString(" Stop ");//USART_SendString("Stop"); // Send message via Bluetooth
            fbtext = 0;
            } // Send message via Bluetooth 
            
        }
        else
        {
            mspeedval = 0;//USART_SendString("_Wrong_"); // Send message via Bluetooth
            
        }
        
        mspeedfunc(mspeedval);
}

void mspeedfunc(mspeedval)
{
    if (mspeedval == 0) // Motor Stops
    {
        M1_C = 0;
        M1_A = 0;
        M2_C = 0;
        M2_A = 0;
      
      
    }
    else if (mspeedval == 1) // Motor goes Back
    {
        
        M1_C = 1;
        M1_A = 0;
        M2_C = 1;
        M2_A = 0;
     
    }
    else if (mspeedval == 2) // Motor goes Forward
    {
        M1_C = 0;
        M1_A = 1;
        M2_C = 0;
        M2_A = 1;
        
    }
    else if (mspeedval == 3) // Motor goes in Left direction
    {
        M1_C = 1;
        M1_A = 0;
        M2_C = 0;
        M2_A = 1;
 
    }
    else if (mspeedval == 4) // Motor goes in Right Direction
    {
        M1_C = 0;
        M1_A = 1;
        M2_C = 1;
        M2_A = 0;
        
    }
    else            // Motor Stops
    {
        M1_C = 0;
        M1_A = 0;
        M2_C = 0;
        M2_A = 0;
    
    }
}
