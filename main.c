 

 #define F_CPU                           (4000000UL)         /* using default clock 4MHz*/
 #include <avr/io.h>
 #include <util/delay.h>
 #include <string.h>
 #include "util/delay.h"
 #include "UART_1_AVR128DA64.h"
// #include "ADC_AVR128DA64.h"
   #define channel_3 0x03
    #define channel_5 0x05
   #define channel_4 0x04
   #define START_TOKEN 0x03
   #define END_TOKEN 0xFC

 #define stepPin PA4            //Define Step pin
 #define dirPin PA6             //Define Direction pin
 #define Enable PA5             //Define Enable pin
 int d;

 const float PI = 3.14159265359;
 float radius=14.25, height=2.00, volume;			
 const float flowrate=50.0;
 int d;


 extern unsigned long ReadCount(void);
 extern unsigned int Get_Weight();
 extern void Get_OFFSET();

 long HX711_Buffer = 0;
 long Weight_OFFSET = 0,Weight_Real = 0;
 int Weight = 0;
 unsigned long M_Weight=0, load_cell_raw_Value=0;
 unsigned long Count;
 long map(long x, long in_min, long in_max, long out_min, long out_max)
 {
	 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
 }
  void ADC0_init(void)
   {
  	 VREF.ADC0REF = 0x5 | (1 << 7);
  	 ADC0.CTRLC = ADC_PRESC_DIV4_gc;        // CLK_PER divided by 4
  	 ADC0.CTRLA = ADC_ENABLE_bm;
 	 PORTD.IN  = ADC_RESSEL_12BIT_gc;////////////////////////////////////////////////////////////////////////////
  	ADC0.MUXPOS = ADC_MUXPOS_AIN3_gc;
   }
 
   void ADC0_start(void)
   {
  	 ADC0.COMMAND = ADC_STCONV_bm;
   }
 
   float ADC0_read(int pin)
   {
 
  	 ADC0.MUXPOS = pin;
 	 ADC0_start();
 
 
 	 while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));
 
 	 return ADC0.RES;
   }

 void Get_OFFSET()
 {
	 HX711_Buffer = ReadCount();
	 Weight_OFFSET = HX711_Buffer;
 }
 unsigned int Get_Weight()
 {
	 HX711_Buffer = ReadCount();
	 HX711_Buffer = HX711_Buffer;

	 Weight_Real = HX711_Buffer;
	 Weight_Real = Weight_Real - Weight_OFFSET; //Get AD sampling value in kind .

	 Weight_Real = (unsigned int)((float)Weight_Real/7.35+0.05);
	 // Calculate the actual physical weight
	 // Because different sensor characteristic curve is not the same, so that each sensor and to correct the divisor 4.30 here
	 // When they find out of the test weight is too large, increase the value.
	 // If the test out of the weight is too small, decrease the value change .
	 // This value is generally about 7.16 . Because different sensors may be.
	 // +0.05 Percentile to rounding

	 return Weight_Real;
 }
 void delay()
 {
	 d =18*(volume)/flowrate;
	 while (d>0)
	 {
		 _delay_us(5);
		 d--;
	 }
 }
 void clockwise(void)
 {
	 volume =PI * radius * radius * height; //d =18*(volume)/flowrate;
	 int i = 0;
	 PORTA.OUTSET |= PIN6_bm;
	 for (i = 0; i < 50 * 5; i++)
	 {
		 PORTA.OUTSET = PIN4_bm;
		 delay();

		 PORTA.OUTCLR = PIN4_bm;
		 delay();
	 }
 }
 void anticlockwise(void)
 {
	 volume =PI * radius * radius * height; //d =18*(volume)/flowrate;
	 int i = 0;
	 PORTA.OUTCLR |= PIN6_bm;
	 for (i = 0; i < 50 * 5; i++)
	 {
		 PORTA.OUTSET = PIN4_bm;
		 delay();
		 PORTA.OUTCLR = PIN4_bm;
		 delay();
	 }
 }

 void motoroff(void)//////////////////////////////////////////////////////////////////////////////////////////////////
 {
	 PORTA.OUTCLR |= PIN6_bm; // dir pin low
	 PORTA.OUTCLR  |=PIN5_bm; // eneble pin as low
 }

 unsigned long ReadCount(void)
 {
	 unsigned long Count;
	 unsigned char i;
	 //PORTA.OUT |= (1 << 2); // DT AS HIGH
	 PORTA.OUT &= ~(1 << 3); // SCK AS LOW
	 Count=0;
	 while((PORTA.IN & (1 << 2)));
	 for (i=0;i<24;i++)
	 {
		 PORTA.OUT |=(1 << 3);
		 _delay_us(5);
		 Count=Count<<1;
		 PORTA.OUT &= ~(1 << 3);
		 _delay_us(5);
		 if((PORTA.IN & (1 << 2)))
		 {
			 Count++;
		 }
	 }

	 PORTA.OUT |= (1 << 3);
	 Count=Count^0x800000;
	 PORTA.OUT &= ~(1 << 3);
	 // USART1_sendInt(Count);
	 return(Count);
 }

 int main(void)
 {
	 USART1_init(9600);
	 ADC0_init();
	 PORTA.DIR |= PIN4_bm | PIN5_bm | PIN6_bm ;
	 PORTA.DIR &= ~(1 << 2); //DT AS INPUT
	 PORTA.DIR |= (1 << 3);  // SCK AS OUTPUT
	 
	 PORTC.DIRCLR = PIN7_bm;	 // for syringe detection switch
	 PORTC.DIRSET = PIN6_bm;    // led blink after occlusion detection
	 PORTC.PIN7CTRL |= PORT_PULLUPEN_bm; // pull up switch Portc 7
	 while (1)
	 {
		 clockwise();
		//anticlockwise();
		 if (!(PORTC.IN & (PIN7_bm)))         // for syringe detection
		 {
			 
			 USART1_sendString("SYRINGE DETECTED");
			 //	_delay_us(200);
			 
		 }
		 else
		 {
			 USART1_sendString("SYRINGE NOT DETECTED");
			 //	_delay_us(200);
		 }
		 float adc_size= ADC0_read( channel_5);   // syringe size detection
		 float sum=0.00;
		 for(int i=1;i<=500;i++)
		 {
			 sum=sum+adc_size;
		 }
		 sum=sum/500.00;
		 float size=sum/100.00;
	  	 USART1_sendFloat(size,2);
		 if(size>=21.00 && size<=28.00)
		 {
			 USART1_sendString("60 ml syringe detected ");
			 
			 int adc_bridge= ADC0_read( channel_4);
		//	 USART1_sendInt(adc_bridge);
			 int y=map(adc_bridge,1410,4095,0,40);
			 USART1_sendString(" Liquid in Syringe in ml ");
			 USART1_sendInt(y);
			 //	_delay_ms(500);
			 int z=map(y,0,40,0,100);
			 USART1_sendString("liquid in syringe in %  ");
			 //	USART1_sendString("\n");
			 USART1_sendInt(z);
		 }
		 if(size>=16.00 && size<=21.00)
		 {
			 USART1_sendString("20 ml syringe detected ");
			 
			 int adc_bridge= ADC0_read( channel_4);   // meter bridge using for Liquid in syringe in ml
			 USART1_sendInt(adc_bridge);
			 int y=map(adc_bridge,1390,4095,0,20);
			 USART1_sendString("Liqid in Syringe in ml ");
			 USART1_sendInt(y);
			 //	_delay_ms(500);
			 int z=map(y,0,20,0,100);
			 USART1_sendString("liquid in syringe in % ");
			 USART1_sendInt(z);
		 }
		 if(size>=13.00 && size<=15.50)
		 {
			 USART1_sendString("10 ml syringe detected");
			 
			 int adc_bridge= ADC0_read( channel_4);
			 USART1_sendInt(adc_bridge);
			 int y=map(adc_bridge,1116,3925,0,10);
			 USART1_sendString("Liqid in Syringe in ml ");
			 USART1_sendInt(y);
			 //	_delay_ms(500);
			 int z=map(y,0,10,0,100);
			 USART1_sendString("liquid in syringe in % ");
			 USART1_sendInt(z);
		 }
		 //	float voltage = (adc*12.60)/4095.00;
		 //USART1_sendFloat(adc, 2);
		 //	USART1_sendFloat(voltage, 2);
		 //	USART1_sendString(" Syringe Size");
		 //	USART1_sendFloat(size, 2);
		 
		 //	_delay_us(200);
		 
		 // 	    int adc_bridge= ADC0_read( channel_4);
		 // 		int y=map(adc_bridge,1410,4095,0,40);
		 // 		USART1_sendString("Liqid in Syringe in ml");
		 // 		USART1_sendInt(y);
		 // 	//	_delay_ms(500);
		 // 		int z=map(y,0,40,0,100);
		 // 		USART1_sendString("liquid in syringe in % ");
		 // 		USART1_sendInt(z);
		 // _delay_ms(500);
		 
		 
		 load_cell_raw_Value=ReadCount();
		 M_Weight=(load_cell_raw_Value-8401819)*0.00793457031;
		 
		 M_Weight=(M_Weight-1500.00);
		 USART1_sendFloat(M_Weight,2);
		 _delay_ms(4);
		 if(M_Weight<=10)
		 {
			 PORTC.OUTCLR= PIN6_bm;
			 USART1_sendString("Kg.");
			 USART1_sendFloat(0,3 );
			 _delay_ms(5);
		 }
		 else
		 {
			 M_Weight=M_Weight/1000.00;
			 USART1_sendString("Kg.");
			 USART1_sendFloat(M_Weight,3);
			 
			 _delay_us(200);
		 }
		 

	 }
 }

 

 
 