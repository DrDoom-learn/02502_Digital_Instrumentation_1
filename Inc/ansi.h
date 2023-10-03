#ifndef ANSI_H_
#define ANSI_H_

typedef enum {
	off = 0,     // turn LED off
	green = 1,   // up
	magenta = 2, // down
	blue = 4,    // left
	cyan = 5,    // up_left
	red = 8,     // right
	yellow = 9,  // up_right
	white = 16   // center
} LED_Color;

typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t hundredths;
} stopwatch_time;

extern float VDDA;

//ADC VREF_CAL
#define VREFINT_CAL *((uint16_t*) ((uint32_t) 0x1FFFF7BA)) //calibrated at 3.3V@ 30C

#define bool _Bool
#define TRUE 1
#define FALSE !TRUE



// PWM function
void GPIO_set_AF1_PA6(void);
void GPIO_set_AF1_PB11(void);
void timer16_clock_init(void);
void timer2_clock_init(void);
void TIM16_PWM_init(uint16_t duty);
void TIM2_PWM_init(uint16_t duty);



// ADC functions
void ADC_setup_PA(void);
void ADC_CAL(void);
uint16_t ADC_measure_PA(uint8_t ch);


int16_t power(int16_t a, int16_t exp);
void init_Joystick(void);
void print_byte(uint8_t input);
void init_LED(void);
void set_LED(LED_Color color);
void init_TIM2(void);
void setupTIM2PWM(void);
void setup_TIM15(void);
void GPIO_set_AF_PA5(void);
void init_Interrupt(void);
uint8_t read_Joystick(void);




extern volatile stopwatch_time sw_time;
extern volatile stopwatch_time sw_split;

#endif /* ANSI_H_ */
