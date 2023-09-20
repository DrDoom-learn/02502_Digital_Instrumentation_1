#include "stm32f30x_conf.h" // STM32 config
#include "30010_io.h" // Input/output library for this course
#include "ansi.h"


const uint8_t LED_DELAY_MAX = 10;
uint8_t LED_COUNTER = 0;

const uint8_t LCD_DELAY_MAX = 100;
uint8_t LCD_COUNTER = 0;

bool LCD_flag = FALSE;

float VDDA;

uint16_t ADC_measure_PA(uint8_t ch) {
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_1Cycles5);
	ADC_StartConversion(ADC1); // Start ADC read
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0) {} // Wait for ADC read
	uint16_t x = ADC_GetConversionValue(ADC1); // Read the ADC value into a variable

	return x;
}


void ADC_CAL(void) {
	ADC_Cmd(ADC1, DISABLE);
	while (ADC_GetDisableCmdStatus(ADC1)) {
	}

	ADC_VrefintCmd(ADC1, ENABLE); // setup ref voltage to channel 18

	for (uint32_t i = 0; i < 20000; i++)
		; // I think this is needed...

	ADC_Cmd(ADC1, ENABLE); // turn on ADC
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY)) {
	}
	ADC_RegularChannelConfig(ADC1, ADC_Channel_18, 1, ADC_SampleTime_19Cycles5);

	ADC_StartConversion(ADC1); // Start ADC read
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0) {
	} // Wait for ADC read

	uint16_t VREFINT_DATA = ADC_GetConversionValue(ADC1); // Read the ADC value into a variable

	VDDA = 3.3f * ((float)VREFINT_CAL / (float)VREFINT_DATA);
}

void ADC_setup_PA(void) {
	RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div8);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructAll; // Define typedef struct for setting pins
	GPIO_StructInit(&GPIO_InitStructAll); // Initialize GPIO struct

	// Sets PA0 to input for the potentiometer
	GPIO_InitStructAll.GPIO_Mode = GPIO_Mode_AN;    // Set as input
	GPIO_InitStructAll.GPIO_Pin = GPIO_Pin_0; // Set so the configuration is on pin 0
	GPIO_InitStructAll.GPIO_Speed = GPIO_Speed_10MHz; // Set speed to 10 MHz
	GPIO_Init(GPIOA, &GPIO_InitStructAll);
	GPIO_InitStructAll.GPIO_Pin = GPIO_Pin_1; // Set so the configuration is on pin 1
	GPIO_Init(GPIOA, &GPIO_InitStructAll);

	// See RM [p.47]
	ADC_InitTypeDef ADC_InitStruct;
	ADC_StructInit(&ADC_InitStruct);
	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE; // Single mode
	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b; // 12 bit resolution
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right; // Align right
	ADC_InitStruct.ADC_NbrOfRegChannel = 1;
	ADC_Init(ADC1, &ADC_InitStruct);

	ADC_Cmd(ADC1, ENABLE);
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY)) {}

	ADC_VoltageRegulatorCmd(ADC1, ENABLE); //Wait for at least 10uS before continuing...
	// set internal reference voltage source and wait

	for (uint32_t i = 0; i < 10000; i++);

	ADC_Cmd(ADC1, DISABLE);

	while (ADC_GetDisableCmdStatus(ADC1)) {} // wait for disable of ADC

	ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Single);

	ADC_StartCalibration(ADC1);

	while (ADC_GetCalibrationStatus(ADC1)) {}

	for (uint32_t i = 0; i < 100; i++);

	ADC_Cmd(ADC1, ENABLE);

}






void EXTI1_IRQHandler(void) {
	__disable_irq();
	//split timer / LEFT
	if (EXTI_GetITStatus(EXTI_Line1) != RESET) {
		sw_split.hours = sw_time.hours;
		sw_split.minutes = sw_time.minutes;
		sw_split.seconds = sw_time.seconds;
		sw_split.hundredths = sw_time.hundredths;

		EXTI_ClearITPendingBit(EXTI_Line1);
	}
	__enable_irq();
}

// Stop and clear timer / DOWN
void EXTI0_IRQHandler(void) {
	__disable_irq();
	if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
		TIM_Cmd(TIM2, DISABLE);
		sw_time.hours = 0;
		sw_time.minutes = 0;
		sw_time.seconds = 0;
		sw_time.hundredths = 0;

		EXTI_ClearITPendingBit(EXTI_Line0);
	}
	__enable_irq();
}

// Start/Stop timer - Up
void EXTI4_IRQHandler(void) {
	__disable_irq();
	if (EXTI_GetITStatus(EXTI_Line4) != RESET) {

		TIM2->CR1 ^= TIM_CR1_CEN;

		EXTI_ClearITPendingBit(EXTI_Line4);
	}
	__enable_irq();
}

void init_Interrupt(void) {
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // define and set setting for EXTI
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource5); // sets port B pin 5 to the IRQ (center)
    EXTI_InitStructure.EXTI_Line = EXTI_Line5; // line 5 see [RM p. 215]
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStructure);

    // define and set setting for EXTI
    //SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC,EXTI_PinSource0); // sets port C pin 0 to the IRQ (right)
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource0); // sets port B pin 0 to the IRQ (down)
    EXTI_InitStructure.EXTI_Line = EXTI_Line0; // line 0 see [RM p. 215]
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // define and set setting for EXTI
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource1); // sets port C pin 5 to the IRQ (left)
    EXTI_InitStructure.EXTI_Line = EXTI_Line1; // line 1 see [RM p. 215]
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // define and set setting for EXTI
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4); // sets port A pin 4 to the IRQ (up)
    EXTI_InitStructure.EXTI_Line = EXTI_Line4; // line 4 see [RM p. 215]
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStructure);
        // setup NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    // setup NVIC 0
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);

    // setup NVIC 1
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);

    // setup NVIC 4
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);

}

void setup_TIM15(void){
	// Enable Timer 15 clock at 1 ms / 1 kHz
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);

	TIM15->CR1 = 0;
	TIM15->ARR = 63999;
	TIM15->PSC = 0;

	TIM15->DIER |= TIM_IT_Update;

	NVIC_SetPriority(TIM1_BRK_TIM15_IRQn, 0);

	NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);

	TIM_Cmd(TIM15, ENABLE);
}

void TIM1_BRK_TIM15_IRQHandler(void) {
	//TODO sæt et flag her og gør ting i main.c
	__disable_irq();

	uint16_t x = ADC_measure_PA(1);
	uint16_t y = ADC_measure_PA(2);
	char str_test[512] = { 0 };
	uint8_t printout[512] = { 0 };
	if (++LCD_COUNTER == LCD_DELAY_MAX) {
		__disable_irq();

		float VCH1 = (VDDA/4095)*x;
		float VCH2 = (VDDA/4095)*y;


		//TODO lav en funktion til at samle nedenstående
		sprintf(str_test, "VCH1 = %.2f", VCH1);
		lcd_write_string(str_test, printout, 1, 1);
		lcd_push_buffer(printout);


		sprintf(str_test, "VCH2 = %.2f", VCH2);
		lcd_write_string(str_test, printout, 1, 2);
		lcd_push_buffer(printout);
		LCD_flag = FALSE;
		__enable_irq();
		LCD_COUNTER = RESET;
	}



//	printf("%d\n",__LINE__);
//
//	if (++LCD_COUNTER == LCD_DELAY_MAX) {
//		printf("*******\n%d***********\n",__LINE__);
//		LCD_flag = TRUE;
//		LCD_COUNTER = RESET;
//	}

// Joystick input
/*	if (TIM_GetITStatus(TIM15, TIM_IT_Update) != RESET) {

		// up
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4)) {
			set_LED(green);
		}

		// down
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0)) {
			set_LED(magenta);
		}

		// left
		if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1)) {
			set_LED(blue);
		}

		// up_left
		if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) && GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4)) {
			set_LED(cyan);
		}

		// right
		if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0)) {
			set_LED(red);
		}

		// up_right
		if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0) && GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4)) {
			set_LED(yellow);
		}

		//center
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5)) {
			set_LED(white);
		}

		if (++LED_COUNTER == LED_DELAY_MAX) {
			set_LED(off);
			LED_COUNTER = RESET;
		}

		TIM_ClearITPendingBit(TIM15, TIM_IT_Update);
	} */

	__enable_irq();
}

void setupTIM2PWM(void) {
	// Enable Timer 2 clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_ICInitTypeDef TIM_InitStructure;
	TIM_ICStructInit(&TIM_InitStructure);
	TIM_InitStructure.TIM_Channel = TIM_Channel_1;
	TIM_InitStructure.TIM_ICPolarity = TIM_ICPolarity_BothEdge;
	TIM_InitStructure.TIM_ICFilter = DISABLE;
	TIM_InitStructure.TIM_ICPrescaler = DISABLE;
	TIM_PWMIConfig(TIM2, &TIM_InitStructure);

	TIM_TimeBaseInitTypeDef TIM_TimeInitStructure;
	TIM_TimeBaseStructInit(&TIM_TimeInitStructure);
	TIM_TimeInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeInitStructure.TIM_Period = 0xFFFF;
	TIM_TimeInitStructure.TIM_Prescaler = 64;

	TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);

	// Set Timer 2 interrupt priority (adjust as needed)
	NVIC_SetPriority(TIM2_IRQn, 0);

	// Enable Timer 2 interrupt in NVIC
	NVIC_EnableIRQ(TIM2_IRQn);

}

void setup_TIM2(void) {
	// Enable Timer 2 clock at 10 ms / 100 Hz
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	// Configure Timer 2
	TIM2->CR1 = 0x00;        // No UIFREMAP, No clock division, Edge-aligned up-counting mode
	TIM2->ARR = 639999;       // Set reload value (100 Hz frequency)
	TIM2->PSC = 0;           // Set prescaler value (no prescaling)

	// Enable Timer 2 interrupts
	TIM2->DIER |= TIM_IT_Update;

	// Set Timer 2 interrupt priority (adjust as needed)
	NVIC_SetPriority(TIM2_IRQn, 0);

	// Enable Timer 2 interrupt in NVIC
	NVIC_EnableIRQ(TIM2_IRQn);
}

volatile static uint32_t ICValue1;
volatile static uint32_t ICValue1_old;
volatile static uint32_t ICValue2;


void TIM2_IRQHandler(void) {
	__disable_irq();



	// Update the time structure


	ICValue1_old = ICValue1;
	ICValue2 = TIM_GetCapture2(TIM2); // Duty/Width
	ICValue1 = TIM_GetCapture1(TIM2);

	//uint8_t ICValid = 1;

	TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

	printf("a: %d b: %d c: %d\n", ICValue1, ICValue1_old, (ICValue1 - ICValue1_old));
	// Clear the interrupt flag

	__enable_irq();
}

//void TIM2_IRQHandler(void) {
//	__disable_irq();
//	// Update the time structure
//
//	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
//		if (++sw_time.hundredths == 100) {
//			sw_time.hundredths = 0;
//
//			if (++sw_time.seconds == 60) {
//				sw_time.seconds = 0;
//
//				if (++sw_time.minutes == 60) {
//					sw_time.minutes = 0;
//
//					if (++sw_time.hours == 100) {
//						sw_time.hours = 0;
//					}
//				}
//			}
//		}
//	}
//	// Clear the interrupt flag
//	TIM2->SR &= ~TIM_SR_UIF;
//
//	__enable_irq();
//}



void set_LED(LED_Color color){
	switch(color){
	case red:
		GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_SET); // Blue LED
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_SET); // Green LED
		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_RESET); // Red LED
		break;
	case green:
		GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_SET);
		break;
	case blue:
		GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_SET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_SET);
		break;
	case cyan:
		GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_SET);
		break;
	case magenta:
		GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_SET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_RESET);
		break;
	case yellow:
		GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_RESET);
		break;
	case white:
		GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_RESET);
		break;
	case off:
	default:
		GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_SET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_SET);
		break;
	}

}

void init_LED(void) {
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE); // Enable clock for GPIO Port A
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE); // Enable clock for GPIO Port B
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE); // Enable clock for GPIO Port C

	GPIO_InitTypeDef GPIO_InitStructAll;  // Define typedef struct for setting pins
	GPIO_StructInit(&GPIO_InitStructAll); // Initialize GPIO struct

	// Sets PA9 to output Blue LED
	GPIO_InitStructAll.GPIO_Mode = GPIO_Mode_OUT;    // Set as output
	GPIO_InitStructAll.GPIO_OType = GPIO_OType_PP;   // Set as Push-Pull
	GPIO_InitStructAll.GPIO_Pin = GPIO_Pin_9;        // Set so the configuration is on pin 9
	GPIO_InitStructAll.GPIO_Speed = GPIO_Speed_2MHz; // Set speed to 2 MHz

	GPIO_Init(GPIOA, &GPIO_InitStructAll);           // Setup of GPIO with the settings chosen

	// Sets PC7 to output Green LED
	GPIO_InitStructAll.GPIO_Mode = GPIO_Mode_OUT;    // Set as output
	GPIO_InitStructAll.GPIO_OType = GPIO_OType_PP;   // Set as Push-Pull
	GPIO_InitStructAll.GPIO_Pin = GPIO_Pin_7;        // Set so the configuration is on pin 7
	GPIO_InitStructAll.GPIO_Speed = GPIO_Speed_2MHz; // Set speed to 2 MHz

	GPIO_Init(GPIOC, &GPIO_InitStructAll);           // Setup of GPIO with the settings chosen

	// Sets PB4 to output Red LED
	GPIO_InitStructAll.GPIO_Mode = GPIO_Mode_OUT;    // Set as output
	GPIO_InitStructAll.GPIO_OType = GPIO_OType_PP;   // Set as Push-Pull
	GPIO_InitStructAll.GPIO_Pin = GPIO_Pin_4;        // Set so the configuration is on pin 4
	GPIO_InitStructAll.GPIO_Speed = GPIO_Speed_2MHz; // Set speed to 2 MHz

	GPIO_Init(GPIOB, &GPIO_InitStructAll);           // Setup of GPIO with the settings chosen
}

uint8_t read_Joystick(void) {
	uint8_t j_up = GPIO_ReadInputDataBit( GPIOA, GPIO_Pin_4);
	uint8_t j_down = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
	uint8_t j_left = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1);
	uint8_t j_right = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0);
	uint8_t j_center = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);

	return ((j_center << 4) | (j_right << 3) | (j_left << 2) | (j_down << 1) | j_up);
}

// prints the byte as binary
void print_byte(uint8_t input) {
	if (input == 0)
		return;

	printf("DEC: %d\n", input);
	printf("0b");
	for (int i = 0; i < 8; i++) {
		if (((input << i) & 0b10000000)) {
			printf("1");
		} else {
			printf("0");
		}
	}
	printf("\n");
}

void GPIO_set_AF_PA5(void) {
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE); // Enable clock for GPIO Port A

	GPIO_InitTypeDef GPIO_InitStructAll; // Define typedef struct for setting pins

	// Pin PA5
	GPIO_InitStructAll.GPIO_Mode = GPIO_Mode_AF;      // Set as alternative function
	GPIO_InitStructAll.GPIO_PuPd = GPIO_PuPd_DOWN;    // Set as pull down
	GPIO_InitStructAll.GPIO_Pin = GPIO_Pin_5;         // Set so the configuration is on pin 5
	GPIO_InitStructAll.GPIO_Speed = GPIO_Speed_50MHz; // 50 MHz
	GPIO_Init(GPIOA, &GPIO_InitStructAll);            // Setup of GPIO with the settings chosen

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_1); //Sets pin 5 at port A to alternative function 1
}

void init_Joystick(void) {
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE); // Enable clock for GPIO Port A
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE); // Enable clock for GPIO Port B
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE); // Enable clock for GPIO Port C

	GPIO_InitTypeDef GPIO_InitStructAll; // Define typedef struct for setting pins

	// Pin j_up PA4
	GPIO_StructInit(&GPIO_InitStructAll); // Initialize GPIO struct

	GPIO_InitStructAll.GPIO_Mode = GPIO_Mode_IN;   // Set as input
	GPIO_InitStructAll.GPIO_PuPd = GPIO_PuPd_DOWN; // Set as pull down
	GPIO_InitStructAll.GPIO_Pin = GPIO_Pin_4;      // Set so the configuration is on pin 4
	GPIO_Init(GPIOA, &GPIO_InitStructAll);         // Setup of GPIO with the settings chosen

	// Pin j_down PB0
	GPIO_InitStructAll.GPIO_Mode = GPIO_Mode_IN;   // Set as input
	GPIO_InitStructAll.GPIO_PuPd = GPIO_PuPd_DOWN; // Set as pull down
	GPIO_InitStructAll.GPIO_Pin = GPIO_Pin_0;      // Set so the configuration is on pin 0
	GPIO_Init(GPIOB, &GPIO_InitStructAll);         // Setup of GPIO with the settings chosen

	// Pin j_left PC1
	GPIO_InitStructAll.GPIO_Mode = GPIO_Mode_IN;   // Set as input
	GPIO_InitStructAll.GPIO_PuPd = GPIO_PuPd_DOWN; // Set as pull down
	GPIO_InitStructAll.GPIO_Pin = GPIO_Pin_1;      // Set so the configuration is on pin 1
	GPIO_Init(GPIOC, &GPIO_InitStructAll);         // Setup of GPIO with the settings chosen

	// Pin j_right PC0
	GPIO_InitStructAll.GPIO_Mode = GPIO_Mode_IN;   // Set as input
	GPIO_InitStructAll.GPIO_PuPd = GPIO_PuPd_DOWN; // Set as pull down
	GPIO_InitStructAll.GPIO_Pin = GPIO_Pin_0;      // Set so the configuration is on pin 0
	GPIO_Init(GPIOC, &GPIO_InitStructAll);         // Setup of GPIO with the settings chosen

	// Pin j_center PB5
	GPIO_InitStructAll.GPIO_Mode = GPIO_Mode_IN;   // Set as input
	GPIO_InitStructAll.GPIO_PuPd = GPIO_PuPd_DOWN; // Set as pull down
	GPIO_InitStructAll.GPIO_Pin = GPIO_Pin_5;      // Set so the configuration is on pin 5
	GPIO_Init(GPIOB, &GPIO_InitStructAll);         // Setup of GPIO with the settings chosen
}

// calculates a^exp
int16_t power(int16_t a, int16_t exp) {
	int16_t i, r = a;
	for (i = 1; i < exp; i++)
		r *= a;
	return (r);
}


