#include <stdint.h>
#include "stm32f4xx.h"
volatile uint32_t ms_ticks = 0;
#define NUM_COLS         8
#define FIL_BASE         7      // PE7 = primera fila
#define COL_BASE         8      // PD8 = primera columna
#define DEBOUNCE_DELAY_MS 20
int b=16;
int h=0;
int col=0;
int count=0;
char contra[4]={'1','2','3','4'};
char v[4];
int f=1;
// Un byte por columna. bit0 = fila superior (PE7) ... bit7 = fila inferior (PE14)
static const uint8_t figura[21][NUM_COLS] = {
		{0x00, 0x40, 0x42, 0x7F, 0x7F, 0x40, 0x40, 0x00},//1
		{0x00, 0x18, 0x14, 0x12, 0x7F, 0x7F, 0x10, 0x00},//4
		{0x00, 0x01, 0x71, 0x79, 0x0D, 0x07, 0x03, 0x00},//7
		{0x00, 0x00, 0x36, 0x1C, 0x7F, 0x1C, 0x36, 0x00},//*
		{0x00, 0x42, 0x63, 0x71, 0x59, 0x4F, 0x46, 0x00},//2
		{0x00, 0x2F, 0x6F, 0x49, 0x49, 0x79, 0x31, 0x00},//5
		{0x00, 0x36, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00},//8
		{0x00, 0x3E, 0x7F, 0x41, 0x41, 0x7F, 0x3E, 0x00},//0
		{0x00, 0x22, 0x63, 0x49, 0x49, 0x7F, 0x36, 0x00},//3
		{0x00, 0x3E, 0x7F, 0x49, 0x49, 0x7B, 0x32, 0x00},//6
		{0x00, 0x26, 0x6F, 0x49, 0x49, 0x7F, 0x3E, 0x00},//9
		{0x00, 0x28, 0xFE, 0xFE, 0x28, 0xFE, 0xFE, 0x28},//#
		{0x00, 0x7C, 0x7E, 0x13, 0x13, 0x7E, 0x7C, 0x00},//A
		{0x00, 0x7F, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00},//B
		{0x00, 0x3E, 0x7F, 0x41, 0x41, 0x63, 0x22, 0x00},//C
		{0x00, 0x7F, 0x7F, 0x41, 0x63, 0x3E, 0x1C, 0x00},//D
		{0x00, 0xFE, 0xFE, 0x22, 0x22, 0x3E, 0x1C, 0x00},//P
		{0x00, 0xC6, 0xEE, 0x38, 0x10, 0x38, 0xEE, 0xC6},//X
		{0x30, 0x60, 0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06},
		{0x3C, 0x42, 0xA5, 0x91, 0x91, 0xA5, 0x42, 0x3C},
		{0x00, 0x02, 0x03, 0xD1, 0xD9, 0x0F, 0x06, 0x00}
};
void Figuras(void);
void Pins_Init(void);
char Keypad_Scan(void);
void SysTick_Init(void);
void SysTick_Handler(void);
char Keypad_Read_Debounced(void);
void maquina2(void);
typedef enum {
    STATE_IDLE,
    STATE_DEBOUNCE_PRESS,
    STATE_PRESSED,
    STATE_DEBOUNCE_RELEASE
} KeypadState_t;

KeypadState_t current_state = STATE_IDLE;
uint32_t debounce_timer = 0;
uint32_t tem = 0;
typedef enum {
    estado_inicial,
    estado_guardando,
    estado_error,
    estado_correcto,
	estado_peligro,
	easter_egg
} State;

State estado = estado_inicial;

char last_valid_key = '\0';



int main(void) {
    Pins_Init();
    SysTick_Init();
    int a = ms_ticks;
        while (1) {


        	maquina2();

            if (ms_ticks-a >= 1 && h==0) {

            	Figuras();
                a = ms_ticks;
            }


        }
}

void maquina2(void){
	char tecla = '\0';
	if(h==0) tecla = Keypad_Read_Debounced();


	switch (estado) {

		case estado_inicial:
			if (tecla != '\0') {
				tem = ms_ticks; // Registrar marca de tiempo
				v[count]= tecla;
				if (count == 3) {
					estado=estado_guardando;
				break;
				}
				count++;
			}
			if(ms_ticks-tem >= 600){
				b=16;
					}
			break;

		case estado_guardando:
			// Si han pasado 20ms
			if ((ms_ticks - tem) >= DEBOUNCE_DELAY_MS) {

				if (count == 3 && v[0] == contra[0] && v[1] == contra[1] && v[2] == contra[2] && v[3] == contra[3]) {
					 // ¡Tecla validada!
					estado = estado_correcto;
				}else if(count == 3 && v[0] == v[1] && v[1] == v[2] && v[2] == v[3]){
					estado = easter_egg;
				}
				else{
					// Fue un pico de ruido o se soltó rápido
					estado = estado_error;
				}
			}
			break;

		case estado_error:
			if((ms_ticks-tem) <= 3000){
				if((ms_ticks-debounce_timer)>=1){
				h=1;
				b=17;
				Figuras();
				}
			}
			else {
				tem=ms_ticks;
				if(f==3){
				estado=estado_peligro;
				f=1;
				}
				else{
				estado=estado_inicial;
				h=0;
				b=16;
				count=0;
				f++;}
			}
			break;

		case estado_correcto:
			if((ms_ticks-tem) <= 3000){
				if((ms_ticks-debounce_timer)>=1){
				h=1;
				b=18;
				Figuras();
				debounce_timer=ms_ticks;
				}
			}
			else {
				tem=ms_ticks;
				estado=estado_inicial;
				h=0;
				b=16;
				f=1;
				count=0;
				}

			break;

		case estado_peligro:
			if((ms_ticks-tem) <= 10000){
				if((ms_ticks-debounce_timer)>=1){
				h=1;
				b=19;
				Figuras();
				debounce_timer=ms_ticks;
				}
			}
			else {
				tem=ms_ticks;
				estado=estado_inicial;
				h=0;
				b=16;
				count=0;
				}
			break;
		case easter_egg:
				if((ms_ticks-tem) <= 3000){
					if((ms_ticks-debounce_timer)>=1){
					h=1;
					b=20;
					Figuras();
					debounce_timer=ms_ticks;
					}
				}
				else {
					tem=ms_ticks;
					estado=estado_inicial;
					h=0;
					b=16;
					count=0;
					}
	}

}
void Pins_Init(void) {
    // 1. Habilitar reloj para GPIOD (Bit 3)
    RCC->AHB1ENR |= (1 << 4);
    RCC->AHB1ENR |= (1 << 3);

    // 2. Configurar PD0 a PD3 como salidas (Filas)
    //----->Tu trabajo aquí consiste en mejorar el código usando máscaras <----
    GPIOE->MODER &= ~(0x3FFFC000); // Limpiar bits 7-14
    GPIOE->MODER |= 0x15554000;    // Establecer como 01 (General purpose output)
    //Filas

    GPIOD->MODER &= ~(0xFFFF0000); // Limpiar bits 8-15
    GPIOD->MODER |= 0x55550000;    // Columnas

    GPIOD->MODER &= ~(0x000000FF); // Limpiar bits 0-7
	GPIOD->MODER |= 0x00000055;    // Establecer como 01 (General purpose output)


	GPIOD->MODER &= ~(0x0000FF00); // Limpiar bits 8-15 (00 = Input)

	// 4. Activar resistencias Pull-Up internas para PD4-PD7
	GPIOD->PUPDR &= ~(0x0000FF00); // Limpiar registros PUPDR
	GPIOD->PUPDR |= 0x00005500;    // Establecer como 01 (Pull-up)


    for (int i=7;i<15;i++){
    	GPIOE->BSRR = (1 << (i + 16));
    };
    for (int i=8;i<16;i++){
        GPIOD->BSRR = (1 << (i + 16));
        };
}
void Figuras(void){
	// Apaga todo antes de dibujar la nueva columna
	                GPIOE->BSRR = (0xFFU << FIL_BASE);              // filas en alto -> LEDs apagados
	                GPIOD->BSRR = (0xFFU << (COL_BASE + 16));         // apaga todas las columnas

	                // Dibuja la columna actual
	                GPIOE->BSRR = ((uint32_t)figura[b][col] << (FIL_BASE + 16));  // enciende filas del patrón
	                GPIOD->BSRR = (1U << (COL_BASE + col));                     // enciende esta columna

	                col++;



	                if (col >= NUM_COLS) col = 0;

}
char Keypad_Scan(void) {
    // Mapa de caracteres del teclado 4x4
    const char keys[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    for (int row = 0; row < 4; row++) {
        // Poner las 4 filas en HIGH usando la mitad inferior de BSRR (Bits 0-3)
        GPIOD->BSRR = 0x000F;

    // Poner la fila actual en LOW usando la mitad superior de BSRR (Bits 16-19)
        GPIOD->BSRR = (1 << (row + 16));

        // Breve retardo para estabilización eléctrica de las señales
        //--------Ya debes saber que esto es una MALA práctica--------
        //--------------En algún momento debes corregirlo-------------
        // Leer las columnas desplazando los bits al inicio (PD4-PD7 -> Bits 0-3)
        uint16_t cols = (GPIOD->IDR >> 4) & 0x0F;

        // Si el pin cae a LOW (0), indica que se conectó con la fila actual
        if (!(cols & 0x01)) {
        	if(keys[row][0]=='1') b=0;
        	if(keys[row][0]=='4') b=1;
        	if(keys[row][0]=='7') b=2;
        	if(keys[row][0]=='*') b=3;
        	return keys[row][0];} // Columna 1
        if (!(cols & 0x02)){
			if(keys[row][1]=='2') b=4;
			if(keys[row][1]=='5') b=5;
			if(keys[row][1]=='8') b=6;
			if(keys[row][1]=='0') b=7;
        	return keys[row][1];} // Columna 2
        if (!(cols & 0x04)){
        	if(keys[row][2]=='3') b=8;
			if(keys[row][2]=='6') b=9;
			if(keys[row][2]=='9') b=10;
			if(keys[row][2]=='#') b=11;
        	return keys[row][2];} // Columna 3
        if (!(cols & 0x08)){
        	if(keys[row][3]=='A') b=12;
			if(keys[row][3]=='B') b=13;
			if(keys[row][3]=='C') b=14;
			if(keys[row][3]=='D') b=15;
        	return keys[row][3];} // Columna 4
    }

    return '\0'; // Retorna nulo si ninguna tecla fue presionada
}
void SysTick_Init(void) {
    // 16 MHz / 1000 = 16000 ticks por milisegundo
    SysTick->LOAD = 16000 - 1;

    // Limpiar el valor actual
    SysTick->VAL = 0;

    // Habilitar SysTick, su interrupción, y usar el reloj del procesador
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}
void SysTick_Handler(void) {
    ms_ticks++;
}
char Keypad_Read_Debounced(void) {
    char raw_key = Keypad_Scan();
    char validated_key = '\0';

    switch (current_state) {
        case STATE_IDLE:
            if (raw_key != '\0') {
                last_valid_key = raw_key;
                debounce_timer = ms_ticks; // Registrar marca de tiempo
                current_state = STATE_DEBOUNCE_PRESS;
            }
            break;

        case STATE_DEBOUNCE_PRESS:
            // Si han pasado 20ms
            if ((ms_ticks - debounce_timer) >= DEBOUNCE_DELAY_MS) {
                if (raw_key == last_valid_key) {
                    validated_key = last_valid_key; // ¡Tecla validada!
                    current_state = STATE_PRESSED;
                } else {
                    // Fue un pico de ruido o se soltó rápido
                    current_state = STATE_IDLE;
                }
            }
            break;

        case STATE_PRESSED:
            if (raw_key == '\0') {
                debounce_timer = ms_ticks;
                current_state = STATE_DEBOUNCE_RELEASE;
            }
            break;

        case STATE_DEBOUNCE_RELEASE:
            if ((ms_ticks - debounce_timer) >= DEBOUNCE_DELAY_MS) {
                if (raw_key == '\0') {
                    current_state = STATE_IDLE; // Liberación confirmada
                } else {
                    current_state = STATE_PRESSED; // Rebote al soltar
                }
            }
            break;
    }

    return validated_key; // Solo retorna el char una vez por pulsación
}
