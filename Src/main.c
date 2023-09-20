#include "stm32f30x_conf.h" // STM16 config
#include "30010_io.h" 		// Input/output library for this course
#include "lcd.h"			// LCD library
#include "ansi.h"			// homebrew
#include "flash.h"

volatile static uint8_t result = 0;
volatile static uint8_t __DEBUG__ = 0;

volatile stopwatch_time sw_time;
volatile stopwatch_time sw_split;

extern bool LCD_flag;


void init(void) {

	__disable_irq(); //Disable global interrupts.

	uart_init(115200); // Initialize USB serial at 115200 baud
	//initJoystick();
	//initLed();
	//setLed(off);
	//init_Interrupt();
	setup_TIM15();
	init_spi_lcd();
	ADC_setup_PA();
	ADC_CAL();
	__enable_irq();	//Enable global interrupts.
}


int main(void) {

	init();


	while (1) {

	}



//        OPG 2.1
//		uint8_t fbuffer[512];
//		memset(fbuffer,0xAA,512); // Sets each element of the buffer to 0xAA
//	    //uint8_t fbuffer[512] = {0x7F, 0x31, 0x34, 0x3C, 0x34, 0x30, 0x78, 0x00};
//	    lcd_push_buffer(fbuffer);

	//        OPG 2.2

//	    uint8_t fbuffer[512] = {0};
//	    lcd_write_string("1. line - indented", fbuffer, 10, 0);
//	    lcd_write_string("2. line", fbuffer, 0, 1);
//	    lcd_write_string("3. line - indented", fbuffer, 30, 1);
//	    lcd_write_string("4. line", fbuffer, 0, 3);
//	    lcd_push_buffer(fbuffer);

//	    uint8_t printout[512] = {0};
//	    uint8_t a = 10;
//	    char str_test[7] = {0};
//	    sprintf(str_test, "a = %2d", a);
//	    lcd_write_string(str_test, printout, 50, 1);
//	    lcd_push_buffer(printout);



	//TODO Lav nedenstående til funktioner
	//		OPG 2.3

//	uint32_t address = 0x0800F800;
//	uint16_t data[10] = {16, 0xA1 , 0xA2 , 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9};
//
//	FLASH_Unlock(); FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
//	FLASH_ErasePage( address ); for ( int i = 0; i < 10; i++ )
//	{ FLASH_ProgramHalfWord(address + i * 2, data[i]); }
//	FLASH_Lock();
//
//	address = 0x0800F800;
//	uint16_t tempVal;
//	for ( int i = 0 ; i < 10 ; i++ ){
//		tempVal = *(uint16_t *) (address + i * 2); // Read Command
//		printf("%d \n", tempVal); }

//	float tempfloat = 2.5;
//	uint32_t tempval32 = 1500;
//	uint16_t tempval16 = 120;
//
//
//	init_page_flash(PG31_BASE);
//	FLASH_Unlock();
//	for ( int i = 0; i < 10 ; i++) {
//	write_float_flash(PG31_BASE, 0x00+i, tempfloat+0.1*i);
//	write_word_flash(PG31_BASE, 0x10+i, tempval32+i);
//	//write_hword_flash(PG31_BASE, 0x20+i, tempval16+i);
//	}
//	FLASH_Lock();
//
//
//	for (int i = 0; i < 0 ; i++) {
//	volatile float tempfloatout = read_float_flash(PG31_BASE, 0x00+i);
//	volatile uint32_t tempval32out = read_word_flash(PG31_BASE, 0x10+i);
//	printf("tempfloat: %f \n tempval32: %ld \n", tempfloatout, tempval32out);
//	}

}

