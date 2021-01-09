// PIC16F887 Configuration Bit Settings
#include <xc.h>
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

int TimeCount;
int bpm;
int result;
int prev;
int ARRAY[5];
int index;
int sumPeriod;
int Timer1Sum;
unsigned long TimeCountMicroseconds;
int Periodmilliseconds;
int avgPeriod;

#define  _XTAL_FREQ 4000000

 void MATH()
 { 
    TimeCountMicroseconds=TimeCount*65536ULL;
    Timer1Sum=((TMR1H*256)+TMR1L);
    unsigned long Period=TimeCountMicroseconds + Timer1Sum;        //calculate period
    Periodmilliseconds=Period/1000ULL;
    ARRAY[index]=Periodmilliseconds;       //write bpm to current index
    index=((index+1)%5);                    //increment index by 1 take modulus
    
   for(int i=0; i<5;i++)
   {
    sumPeriod+=ARRAY[i];      //sum bpm is the sum of the array indices
   }
    avgPeriod=sumPeriod/5;
   
   bpm=60000.0/avgPeriod;
  }
 
void __interrupt() ISR(void)
{
 
   if (ADIF)                         //when ad conversion finishes do this
  {
   result=ADRESL+(256*ADRESH); //accumulative total convert results
 
     if((result>=0x2b7)&&(prev<0x2b7))      //ADC result above threshold and increasing
     {                                  //on the normal high peak
        MATH();                        //calculate average bpm  
        TimeCount=0;                     //reset timer1 overflow variable
        TMR1H=0;                         //reset timer1
        TMR1L=0;
     }
   
    prev=result;                        //write previous value as result each time
    ADIF=0;                            //take down ad flag
    GO=1;                               //restart ad conversion
         
  }
   else if (TMR1IF==1)                   //timer1 flag tripped
  {
     TimeCount++;             //increment overflow variable
     TMR1IF=0;                          //reset timer1 flag
  }
 
}
//Table containing lookup values to display 0-9 on LEDs
 const char TABLE[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0X82,0XF8,0X80,0X90};

//delay program
void  DELAY()              //delay program          
    {                                                
     for(int i=0x100;i--;);     //delay
    }

 void display(int x)        
   {
    int  hun,ten,one;   //variables to hold hundreds, tens, and ones place of result
    hun= (x)/0x64;        //get display hundreds digit
    ten= (x%0x64)/0xa;  //get display tens digit
    one= x%0xa;         //get display ones digit

     if(hun==0)
     {
        PORTD=0xFF; //blank most left display if 0
     }

     else
     {
        PORTD=TABLE[hun];   //retrieve lookup table code from table for hundreds digit
     }

     PORTA=0x37;         //RA4 OUTPUT low,light hundred bit display     0011 0111

     DELAY();            //delay some time,ensure display brightness  

     PORTD=TABLE[ten];   //get the display tens digit code from table

     PORTA=0x2f;         //RA5 OUTPUT low,light ten bit display         0010 1111

     DELAY();            //delay some time,ensure display brightness    

     PORTD=TABLE[one];   //get the display ones digit code from table

     PORTA=0x1f;         //RA6 OUTPUT low,light ones digit display      0001 1111

     DELAY();            //delay some time,ensure display brightness    */
   }
 
 void init()
 {
     ANSEL=0x01;        //RA0 = analog
     ANSELH=0x00;       //All digital
     TRISA=0x01;        //RA0 input
     PORTA=0xFF;        //PORTA = FF    7 Segment config start off
     PORTD=0xFF;        //PORTD = FF    7 segment config
     TRISD=0x00;        //PortD all output
     ADCON0=0x41;       //Fosc/8
     ADCON1=0x80;       //Right Justified
     GIE=1;             //global interrupts enabled
     PEIE=1;            //peripheral interrupts enabled
     ADIE=1;            //analog to digital interrupt enabled
     ADIF=0;            //preset ad flag as down
     index=0;           //preset index as 0
     T1CON=0x01;        //enable timer1
     TMR1IE=1;          //timer1 interrupt enabled
     TMR1IF=0;          //preset timer1 flag as down
bpm=0;      //initial value set to 0
prev=0;             //preset prev to 0
for(int i=0; i<5;i++)       //array values initially zero
{
ARRAY[i]=0;
}
     GO=1;              //must tell to GO first time!
   
 }
void main() {
    init();             //initialize ports and stuff
   
    while(1)                    //continuously display bpm
    {
    display(bpm); //send bpm to 7 segment display, send bpm for debug
	
}
}