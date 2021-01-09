/* Reads thermometer from probe and receives data for another probe via wifi */

// PIC16F887 Configuration Bit Settings
#include <xc.h>
#include <math.h>
// CONFIG1
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)
// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)
//TABLES//
const char TABLE[]={'0','1','2','3','4','5','6','7','8','9',' '};
const char FTABLE[]={'.','-'};
const char OUTTABLE[8]={'O','u','t','s','i','d','e',':',};
const char INTABLE[8]={'I','n','s','i','d','e',' ',':',};
const char TEMP[16]={'T','e','m','p','e','r','a','t','u','r','e',':',' ',' ',' ',' '};
const char FAR[16]={'F','a','h','r','e','n','h','e','i','t',' ',' ',' ',' ',' ',' '};
const char Cels[16]={'C','e','l','s','i','u','s',' ',' ',' ',' ',' ',' ',' ',' ',' '};
const char BADSUM[]={'B','A','D',' ','C','H','E','C','K',' ','S','U','M'};
const char FTABLE2[]={'0','x'};
const char DEGF[]= {0xDF, 'F'};
const char DEGC[]= {0xDF, 'C'};
int negbit;

//variables//
double beta=3950.0;
double R_0=10000.0;
double R_known=10086.0;
double T_0=298.0;
float TK;           //inside kelvin temperature storage
float TF;           //storage for fahrenheit temperature calculation
float TC;           //storage for celsius temperature calculation
float TKOUT;        //outside kelvin temperature storage
int OUTDATA[4];     //received data array from wifi
char FOURDIGIT[4];  //array of temperature data to print to lcd
int CelTog;         //toggle bit between celsius and fahrenheit
int badsum;         //toggle bit 1 when bad check sum occurs
int ADRESOUT;       //sum of 1st and 2nd element of outdata array
int STATE;          //state bit within program

//LCD pins
#define rs RA1
#define rw RA2
#define e  RA3
#define  _XTAL_FREQ 4000000
//--------------------------------------
//delay
void delay()
 {
   int i;
   for(i=0;i<3000;i++);
 }
//--------------------------------------
//lcd display setting
void LCDsendCom(char x)
 {
   PORTD=x;                  //command out to PORTD
   rs=0;                     //is command not data
   rw=0;                     //is write not read.
   e=0;                      //pull low enable signal.
   delay();                  //for a while.
   e=1;                      //pull high to build the rising edge
 }
//LCD init
void lcd_init()
 {
    LCDsendCom(0x1);            //clr screen
    LCDsendCom(0X38);           //8 bits 2 lines 5*7 mode.
    LCDsendCom(0X0e);          //display on,cursor on,blink on.
    LCDsendCom(0X06);                //character not move,cursor rotate right.
    LCDsendCom(0X80);                //Go to top line, first position
 }
//--------------------------------------
//write a byte to lcd.
void LCDcharWrite(char x)
 {
  PORTD=x;                   //data send to PORTD
  rs=1;                      //is data not command.
  rw=0;                      //is write not read.
  e=0;                       //pull low enable signal.
  delay();                   //for a while.
  e=1;                       //pull high to build the rising edge.
 }
 
void __interrupt() ISR() //ISR
{
    LCDsendCom(0x1);            //clr screen
    LCDsendCom(0X80);                //Go to top line, first position
    if(CelTog==1)                   //if celtog WAS set
    {
        for(int i=0; i<16;i++)
        {
            LCDcharWrite(TEMP[i]);      //write "Temperature: Fahrenheit"
        }
        LCDsendCom(0xC0);
        for(int i=0; i<16;i++)
        {
            LCDcharWrite(FAR[i]);
        }
        CelTog=0;                   //Clear CelTog
    }
    else if(CelTog==0)              //if celtog wasn't set
    {
       for(int i=0; i<16;i++)
        {
            LCDcharWrite(TEMP[i]);  //write "Temperature: Celsius"
        }
        LCDsendCom(0xC0);
         for(int i=0; i<16;i++)
        {
            LCDcharWrite(Cels[i]);
        }
       
        CelTog=1;                   //set CelTog
    }
    STATE=1;                        //set state variable to 1
    INTF=0;                         //clear interrupt flag
      
}
void MATHING(int x,int y)
 {
    if(y==0)
    {
     float part1=(R_known*((float)(x))/(R_0*(1023-x)));
     float natlog= log(part1);
     float part2=(natlog/beta)+(1/T_0);
     TK=1/part2;
    }
    if(y==1)
    {
     float part1=((float)(x))/((1023-x));
     float natlog= log(part1);
     float part2=(natlog/beta)+(1/T_0);
     TKOUT=1/part2;   
    }
 }
 void MATHTWO(float x)
 {
    
     int  hun,ten,one,tenths; //variables to hold thousands,  hundreds, tens, and ones place of result
     int roundT=x*10;
  hun= roundT/0x3E8;   //get display hundreds digit
     ten= (roundT%0x3E8)/0x64;   //get display tens digit
     one= roundT%0x64/0xA;          //get display ones digit
     tenths= roundT%0xA;
    
     if(hun==0)             //if hundreds place is zero put blank there
     {
         if(ten==0)
         {
             hun=10;
             ten=10;
         }
         hun=10;            //set hun to 10 for 10th table index
     }
    
  FOURDIGIT[0]=hun;      //store digits in array
  FOURDIGIT[1]=ten;
  FOURDIGIT[2]=one;
     FOURDIGIT[3]=tenths;
 }
 
 void KtoFandC(float x)
 {
     TF=((x-273.2)*1.8)+32;     //conversion of Kelvin to Fahrenheit
     TC=((x-273.2));            //convert K to C
     if (TC<0&&CelTog==1)
     {
         TC=fabs(TC);
         negbit=1;
     }
     else
     {
         TC=TC;
         negbit=0;
     }
 }

  void DegreesPrint()
 {
      if (negbit==1)
      {
          LCDcharWrite(FTABLE[1]);
      }
      else
      {
          LCDcharWrite(' ');
      }
    
         
 for (int i=0; i<3; i++)   
     {
         LCDcharWrite(TABLE[FOURDIGIT[i]]);     //print values in fourdigit array
     }
     LCDcharWrite(FTABLE[0]);                   //table containing format char
     LCDcharWrite(TABLE[FOURDIGIT[3]]);
         
     switch (CelTog)
     {
         case 0:
          for(int i=0;i<2; i++)             //if celtog is zero
     {
         LCDcharWrite(DEGF[i]);             //print fahrenheit
     }
    
          case 1:                           //if celtog is one
         for(int i=0;i<2; i++)
     {
         LCDcharWrite(DEGC[i]);             //print celsius
     }
    
     }
  }
 
  void checksum()
  {
      if (OUTDATA[0]+OUTDATA[1]!=(256*OUTDATA[2])+OUTDATA[3])   //checks if data matches check sum
      {
          badsum=1;         //toggle badsum variable
      }
      else
      {
          badsum=0;
      }
  }
 
  void PRINTBAD()
  {
      for(int i=0; i<13; i++)
      {
          LCDcharWrite(BADSUM[i]);          //print "Bad Check Sum"
      }
  }
  void OUTDATAADDER()
  {
      ADRESOUT=(OUTDATA[0]*256)+OUTDATA[1];     //generate ad result from outside data
  
  }
 
 
  void init()               //port,register, and variable initializations
  {
    
    TRISA=0xF1;     //outputs for LCD control
    TRISB=0x01;
    TRISC=0xBF;   //input on Rx, output on Tx
    TRISD=0x00;     //outputs for LCD data/commands
    ANSEL =0x01;      //all digital
    ANSELH=0x00;
    ADCON1=0x80;              //Right justify A/D result, use VDD and VSS as voltage references
    ADCON0=0x81;              //system clock Fosc/32,select RA0(AN0) as analog channel,turn on A/D module, but don't set GO yet
    GIE=1;          //allow interrupts
    INTE=1;         //INTCONSETUP
    INTF=0;         //clear int flag
    BRG16=0;        //Set BAUD RATE of 9600
    BRGH=1;
    SPBRG= 25;
    SPBRGH=0;
    SYNC=0;         //asynchronous mode
    SPEN=1;         //enable serial transmission
    TX9=0;          //8-bit mode
    RX9=0;          //
    STATE=1;        //set state variable
    for(int i=0;i<4;i++)
    {
        FOURDIGIT[i]=0;
        OUTDATA[i]=0;
    } 
  }
  void GETINSIDETEMP()      //measures temperature from thermistor
  {
      while(STATE==1)
      {
      lcd_init();                //initialize LCD
     int result;
     result=0x00;             //clear the convert result
     for(int i=10;i>0;i--)     //get the average of ten conversion results
     {                                                                    
     GO=0X1;               //start conversion
     while(GO);            //wait for conversion to finish
     result=result+ADRESL+(256*ADRESH); //accumulative total convert results          
     }                                                                   
     result=result/10;      //get the average of ten convert results
     MATHING(result,0);       //convert result to temperature Kelvin
     KtoFandC(TK);              //convert kelvin to fahrenheit
      STATE++;
      }
  }
void PRINTINSIDETEMP()      //prints calculated temperature
  {
    while(STATE==2)
    {
    switch(CelTog)
        {
        case 0:
            {
            for(int i=0; i<8; i++)
                {
                LCDcharWrite(INTABLE[i]);           //write "Inside:"
                }
            MATHTWO(TF);           //convert value to digits for display
            DegreesPrint();
            }
        case 1:
            {
            for(int i=0; i<8; i++)
                {
                LCDcharWrite(INTABLE[i]);
                }
            MATHTWO(TC);
            DegreesPrint();
            }
        }
    STATE++;            //increment state value
    }
  }
 
  ////////Receive Outside Temperature Routines///////////////
void RECEIVEOUTSIDETEMP()
  {
    while(STATE==3)
  {
    char readIn;                                //byte of data read in
    char readIn2;
    CREN=1;                                     //enable continuous receive
    TXEN=0;                                     //disable transmission               
READLOOP:                                       //label for passkey check
    do                     
        {
        while(!RCIF);                           //waits for Receive flag to go up
        readIn=RCREG;                           //write RCREG to readIn
        }
    while(readIn != 0xcc);                      //waits for 0xcc to be read in
    while(!RCIF);                               //waits for RCIF flag again
    readIn2=RCREG;                              //writes RCREG to readIn2
    if(readIn2 != 0xdd)                         //if readIn2 isn't DD
        goto READLOOP;                          //go back to waiting for CC DD sequence
    for (int i=0; i<4; i++)                     //after dd pull in 4 hex values
        {
        while(!RCIF);                           //wait to receive data one byte at a time
        OUTDATA[i]=(RCREG);                     //takes in hex values for outdoor temperature ADC result
        }
      for(int i=0;i<4;i++)                        //Preset Print array to 0
        {
        FOURDIGIT[i]=0;
        }
    CREN=0;                                     //end receive mode
    checksum();                                 //check that values are good
    OUTDATAADDER();                             //generate outside ad result  
    MATHING(ADRESOUT,1);  
    KtoFandC(TKOUT);
    STATE++;         //increment state value
  }
  }
 
void PRINTOUTSIDETEMP()
  {
    while(STATE==4)
    {
    LCDsendCom(0xC0);
    if(badsum==1)
        {
        PRINTBAD(); 
        }
    else
    {
    switch(CelTog)
        {
        case 0:
            for(int i=0; i<8 ; i++)
                {
                LCDcharWrite(OUTTABLE[i]);      //write "Outside:"
                }
            MATHTWO(TF);                        //convert value to fahrenheit for display
            DegreesPrint();                     //print temperature in fahrenheit
        case 1:
            for(int i=0; i<8 ; i++)
                {
                LCDcharWrite(OUTTABLE[i]);      //write "Outside:"
                }
            MATHTWO(TC);                        //convert value to celsius for display
            DegreesPrint();                     //print temperature in celsius
        }
    }
    STATE=1;             //reset state value
    }
  }
 
 
void main()
{
    init();                     //initialize ports and variables
    __delay_ms(100);
           
    while(1)                    //infinite loop
    {
     
        while(STATE==1)         //only call getinsidetemp if STATE is 1
        {
        GETINSIDETEMP();
        }
        while(STATE==2)         //only call printinsidetemp if STATE is 2
        {
        PRINTINSIDETEMP();
        }
        while(STATE==3)         //only call RECEIVEOUTSIDETEMP if STATE is 3
        {
        RECEIVEOUTSIDETEMP();
        }
        while(STATE==4)         //only call PRINTOUTSIDETEMP if STATE is 4
        {
        PRINTOUTSIDETEMP();
        }
       
        __delay_ms(1000);
    }
}
