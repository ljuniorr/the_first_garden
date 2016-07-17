/* --------------------------------------------------------------------------
+ Project	:	testLeandro
+ File		:	main
+ Author	:	Jonas Vieira de Souza
+ Created	:	7/5/2016 1:01:51 AM
+ Modified	:
+ Version	:
+ Purpose	:
+ -------------------------------------------------------------------------- */

// ----------------------------------------------------------------------------
// System definition ----------------------------------------------------------
#define F_CPU 16000000UL

// ----------------------------------------------------------------------------
// Header files ---------------------------------------------------------------
#include "globaldefines.h"
#include "atmega328.h"
#include "lcd4f.h"

// ----------------------------------------------------------------------------
// Macro functions ------------------------------------------------------------

// ----------------------------------------------------------------------------
// Project definition ---------------------------------------------------------
#define RELE_1_DDR	DDRC
#define RELE_1_PORT	PORTC
#define RELE_1_PIN	PINC
#define RELE_1_BIT	PC4

#define RELE_2_DDR	DDRC
#define RELE_2_PORT	PORTC
#define RELE_2_PIN	PINC
#define RELE_2_BIT	PC5

#define LEDS_DDR		DDRB
#define LEDS_PORT		PORTB
#define LED_GREEN_BIT	PB0
#define LED_YELLOW_BIT	PB1
#define LED_RED_BIT		PB2

// ----------------------------------------------------------------------------
// New data type --------------------------------------------------------------
typedef union systemFlags_t{
	struct{
		uint8 ledTimerUp	: 1;
		uint8 unusedBits	: 7;
	};
	uint8 allFlags;
}systemFlags_t;

// ----------------------------------------------------------------------------
// Function declaration -------------------------------------------------------
void timerLight( void );
void umidLed( uint16 rawAdc );
void lowLeds(void);

// ----------------------------------------------------------------------------
// Global variable ------------------------------------------------------------
volatile systemFlags_t systemFlags;
volatile unsigned long counterTimer;

// ----------------------------------------------------------------------------
// Main function

__attribute__ ((OS_main)) int main(void)
{
	// Variables Declaration
	uint16 rawAdc = 0;
	attachLcd(display);

	// CONFIGURATION RELES
	setBit(RELE_1_DDR,  RELE_1_BIT);	// SAIDA, BOMBA
	clrBit(RELE_1_PORT, RELE_1_BIT);	// 1
	
	setBit(RELE_2_DDR,  RELE_2_BIT);	// SAIDA, LUZ
	clrBit(RELE_2_PORT, RELE_2_BIT);	// 1

	// CONFIGURATION LEDS
	setBit(LEDS_DDR,  LED_GREEN_BIT);	// SAIDA
	setBit(LEDS_DDR,  LED_YELLOW_BIT);	// SAIDA
	setBit(LEDS_DDR,  LED_RED_BIT);		// SAIDA
	
	clrBit(LEDS_PORT, LED_GREEN_BIT);	// 0
	clrBit(LEDS_PORT, LED_YELLOW_BIT);	// 0
	clrBit(LEDS_PORT, LED_RED_BIT);		// 0
	

	// Variable Initialization
	lcdSetDataPort		(&display, &DDRD, &PORTD, &PIND, PD0 );
	lcdSetControlPort	(&display, &DDRB, &PORTB, PB3, PB4, PB5);
	lcdInit				(&display, LCD_16X2, LCD_FONT_5X8);
	lcdStdio			(&display);
	lcdClearScreen(&display);
	printf("TESTE LEADNRO\n\n");
	_delay_ms(1000);
	
	adcConfig(ADC_MODE_SINGLE_CONVERSION, ADC_REFERENCE_POWER_SUPPLY, ADC_PRESCALER_128);
	adcSelectChannel(ADC_CHANNEL_0);
	adcEnable();
	
	// hour init
	counterTimer = 86390;	
	
	for(;;)
	{
		
		adcStartConversion();
		adcWaitUntilConversionFinish();
		rawAdc = ADC;
		printf("DHT = %d\nHR = %d\n", rawAdc, (int)((counterTimer/60)/60) );
		timerLight();
		umidLed( rawAdc );
		
	}	
}

// ----------------------------------------------------------------------------
// Interruption handler -------------------------------------------------------

// ----------------------------------------------------------------------------
// Function definitions -------------------------------------------------------

void umidLed( uint16 rawAdc )
{
	if ( rawAdc > 0 && rawAdc < 400){ //Solo umido, acende o led verde
		lowLeds();
		setBit(LEDS_PORT, LED_GREEN_BIT);
	}
	if (rawAdc > 400 && rawAdc < 800){  //Solo com umidade moderada, acende led amarelo
		lowLeds();
		setBit(LEDS_PORT, LED_YELLOW_BIT);
	}
	if (rawAdc > 800 && rawAdc < 1024){ //Solo seco, acende led vermelho e chama função para irrigação
		lowLeds();
		setBit(LEDS_PORT, LED_RED_BIT);
		setBit(RELE_1_PORT, RELE_1_BIT);
	}
}

void lowLeds(void){
	clrBit(LEDS_PORT, LED_GREEN_BIT);
	clrBit(LEDS_PORT, LED_YELLOW_BIT);
	clrBit(LEDS_PORT, LED_RED_BIT);
}

void timerLight( void )
{
	_delay_ms(1000); // one second
	counterTimer++;
	
	if( counterTimer == 72000 ) // 20h
	{
		clrBit(RELE_2_PORT, RELE_2_BIT);	// desliga luz
	}
	if( counterTimer == 86400) // 2400
	{
		counterTimer = 0;
		setBit(RELE_2_PORT, RELE_2_BIT);	// liga luz
	}
}