#define HISTEREZIS (5)

void initiale() //afisare initiale BCD 7 seg
{ 
  //LITERA C
    PORTD|=1<<PD4|1<<PD5|1<<PD7;
     PORTB|=(1<<PB0);

/*
 * STINGERE C SI DUPA APRINDE D
    _delay_ms(1000); 
 PORTD&=~(1<<PD4|1<<PD5|1<<PD7) ;
    PORTB&=~(1<<PB0);
   _delay_ms(1000);

    //LITERA d
  PORTD|=1<<PD3|1<<PD6|1<<PD7;
     PORTB|=1<<PB0|1<<PB1;
      _delay_ms(1000);

      PORTD&=~(1<<PD3|1<<PD6|1<<PD7);
     PORTB&=~(1<<PB0|1<<PB1);
    */
}

void T1() //timer1
{
 cli(); 
 TCCR1A=0;//setarea valorilor registrului pe 0
 TCCR1B=0;//setarea valorilor registrului pe 0
 TCNT1=0;//initializam valoarea counter-ului pe 0
 OCR1A=15624;//1s=16MHz, 1024 prescaler => 15624
 TCCR1B|=(1<<WGM12);//CTC folosind OCR1A
 TCCR1B|=(1<<CS12)|(1<<CS10);//prescaler 1024
 TIMSK1|=(1<<OCIE1A);//enable timer compare interrupt
 sei();
} 

void T2() //timer2 PWM
{
  TCCR2A = 0;//setarea valorilor registrului pe 0
  DDRB|=(1<<PB3);//pin 11 prestabilit
  TCCR2A |= (1 << COM2A1); //clear OC2B on compare match
  TCCR2A |= (1 << WGM21)|(1 << WGM20);//fast pwm/factor de  umplere
  TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);//prescaler
}

void PWM() //pwm timp crestere 4secunde
{
  //fade in
    for(int i=0; i<256; i++)
    {
      OCR2A =i;
       _delay_ms(4);//255*4
    }
    OCR2A =255;
    _delay_ms(1000);//1 secunda
  //fade out
    for(int i=256; i>0; i--)
    {
      OCR2A =i;
      _delay_ms(4);//255*4
    }
    OCR2A =0;
    _delay_ms(1000);
}

void adc_init() //citire pin exterior 
{
//set division factor between system clock frequency and the input clock to the ADC‐ 128
ADCSRA |= ((1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0));//set division factor between system clock
ADMUX |= (1<<REFS0); //AVcc with external capacitor at Aref pin
ADCSRA |= (1<<ADEN);//enable ADC
ADCSRA |= (1<<ADSC);//pornire conversie ADC
}

void USART_init() //comunicare seriala
{
  //la 16MHz
  UBRR0 = 103; //setare viteza transmisie date 9600 biti/sec
  UCSR0B = (1<<RXEN0)|(1<<TXEN0); //enable transmisie/receptie
  UCSR0C = (1<<USBS0)|(3<<UCSZ00);//setare biti date si biti stop
}

void tastatura() //comunicare seriala
{
  unsigned char input;
  input=UDR0;//preia valoarea introdusa de la tastatura
  if(input=='A')
  {
     PORTB|=(1<<5);//aprinde ledul 
  }
  else if(input=='S')
  {
     PORTB&=~(1<<5);//stinge ledul
  }
}

uint16_t read_adc(uint8_t channel) //citire pin
{
ADMUX &= 0xF0; // selectare input A0->A5
ADMUX |= channel; // selectare canal
ADCSRA |= (1<<ADSC); //pornire conversie
while(ADCSRA & (1<<ADSC));//asteptam pana conversiile nu sunt actualizate
return ADC; //returneaza tensiunea
}

void temp() //calcul senzor temperatura
{
  int val;
  int tempPin = 0;
  val = analogRead(tempPin);
  float mv = ( val/1024.0)*500;
  float cel = mv/10;
  float farh = (cel*9)/5 + 32;
  Serial.print("TEMPERATURE = ");
  Serial.print(cel);
  Serial.print("*C");
  Serial.println();

  if(cel>=(35+HISTEREZIS)) //aprindere led
  {
    PORTD|=(1<<2);//aprindere led pentru temperatura
  }
  else
      if(cel<(35-HISTEREZIS))
  {
    PORTD &=~(1<<2);//stingere led pentru temperatura
  }  
}

void setup()
{
cli();//oprire intreruperi globale
//setare pini intrare iesire
DDRB = 0x3F;
DDRD = 0xFC;
DDRC = 0x00;
//functii
T1();//timer1
adc_init();
T2();//timer2
USART_init();
sei();//pornire intreruperi
}

void loop()
{
initiale();
tastatura();
temp();
PWM();
}
ISR(TIMER1_COMPA_vect) //vector comparare
{
  //toggle la pinul de la led
   PORTB ^=( 1 << 4);
}