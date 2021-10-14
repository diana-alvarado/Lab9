/*
 * File:   Lab8.c
 * Author: Diana Alvarado
 *
 * Created on 27 de septiembre de 2021, 07:49 PM
 */

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

/*
 * --------------------------- Configracion de bits --------------------------
 */
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillador Interno sin salidas, XT
#pragma config WDTE = OFF       // WDT disabled (reinicio repetitivo del pic)
#pragma config PWRTE = OFF     // PWRT enabled  (espera de 72ms al iniciar)
#pragma config MCLRE = OFF      // El pin de MCLR se utiliza como I/O
#pragma config CP = OFF         // Sin protecci?n de c?digo
#pragma config CPD = OFF        // Sin protecci?n de datos
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit 
#pragma config FCMEN = OFF       // Fail-Safe Clock Monitor Enabled bit 
#pragma config LVP = OFF         // Low Voltage Programming Enable bit 

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit 
#pragma config WRT = OFF        // Flash Program Memory Self Write 

/*
 * --------------------------- Librerias --------------------------
 */

#include <xc.h>
#include <stdint.h>
#include <pic16f887.h>

#define _XTAL_FREQ 8000000  //8MHz

/*
 * ---------------------------- Constantes -----------------------------------
 */
#define _t0_valor 250
/*
 * ----------------------------- Variables ----------------------------------
 */
char casos;
uint8_t cont;
char pot;

/*
 * ------------------------------- Tabla ------------------------------------
 */


/*
 * -------------------------- Prototipos de funciones -----------------------
 */
void setup (void);
/*
 * ------------------------------ Interrupci?n -------------------------------
 */
void __interrupt () isr (void)
{
    if (PIR1bits.ADIF)   // Interrupcion ADC
    {  
        if (ADCON0bits.CHS == 0)
        {
            CCPR1L = (ADRESH>>1)+123;
            CCP1CONbits.DC1B = (ADRESH & 0b01);
            CCP1CONbits.DC1B0 = (ADRESL>>7);
        }
        else if (ADCON0bits.CHS == 1)
        {
            CCPR2L = (ADRESH>>1)+123;
            CCP1CONbits.DC1B= (ADRESH & 0b01);
            CCP1CONbits.DC1B0 = (ADRESL>>7);
        }
        else{
            pot = ADRESH;    
        }
        
        PIR1bits.ADIF = 0;
        
    }
    if (INTCONbits.T0IF)
    {
        cont ++;
        
        
        if (cont < pot)
            PORTCbits.RC4 = 1;
        else 
            PORTCbits.RC4 = 0;
        
        TMR0 = _t0_valor;
        INTCONbits.T0IF = 0;
        
    }
}

/*
 * ----------------------------- MAIN CONFIGURACION --------------------------- 
 */

void main (void)
{
    setup();
    ADCON0bits.GO = 1; 
    while(1)
    {
       // __delay_us(50);
        if (ADCON0bits.GO == 0) { // si esta en 0, revisa en qu� canal est� convirtiendo
            if (ADCON0bits.CHS == 0){
                __delay_us(100);
                ADCON0bits.CHS = 1;    
            }
            else if (ADCON0bits.CHS == 1){
                __delay_us(100);
                ADCON0bits.CHS = 2; 
            }
            else if (ADCON0bits.CHS == 2){
                __delay_us(100);
                ADCON0bits.CHS = 0;
            }
            __delay_us(100);
            ADCON0bits.GO =1;    //convertir  
        
        }
        
    }
}
/*
 * -------------------------------- Funciones --------------------------------
 */

void setup(void)
{
    //--------------- Configuraci?n de entradas y salidas --------------
    ANSEL = 0b00000111;
    ANSELH = 0;
    
    TRISA = 0xFF;  //Entrada
    TRISC = 0;
    
     //valores iniciales
    PORTA = 0;
    PORTC = 0;
    
    //---------------------- Configuraci?n del oscilador ----------------------
    OSCCONbits.IRCF = 0b0111;   //8MHz
    OSCCONbits.SCS = 1;
    
    //----------------------- Configuraci?n del ADC ---------------------------
    ADCON1bits.ADFM = 0;        //Justificai?n izquierda
    ADCON1bits.VCFG0 = 0;       //Vref en VSS Y VDD
    ADCON1bits.VCFG1 = 0;       
    
    ADCON0bits.ADCS = 0b10;     //FOSC/32
    ADCON0bits.CHS = 0;
    ADCON0bits.ADON = 1;
     __delay_us(50);
     
    //----------------------- Configuraci?n del PWM ---------------------------
     TRISCbits.TRISC2 = 1;          //RC2 /CCP1 entrada
     PR2 = 255; 
     CCP1CONbits.P1M = 0;           //Config modo PWM
     CCP1CONbits.CCP1M = 0b1100;
     CCPR1L = 0X0f;  
     CCP1CONbits.DC1B = 0;
     
     TRISCbits.TRISC1 = 1;                  
     CCP2CONbits.CCP2M = 0b1100;
     CCPR2L = 0X0f;  
     CCP2CONbits.DC2B1 = 0;
     
     //------------------------------- TMR2 ----------------------------------
    PIR1bits.TMR2IF = 0;           //apagamos la bandera
    T2CONbits.T2CKPS = 0b11;       //Prescaler 1:16
    T2CONbits.TMR2ON = 1;
     
    while (PIR1bits.TMR2IF == 0);
    PIR1bits.TMR2IF = 0;           //Esperamos un ciclo del tmr2
    
    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC1 = 0;
             //Salida del PWM
     
    //------------------ Configuraci?n de las interrupciones  -----------------
    INTCONbits.T0IF = 0;
    INTCONbits.T0IE = 1;
    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    
    //Configuraci?n de tmr0
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.T0SE =0;
    OPTION_REGbits.PSA = 0;
    OPTION_REGbits.PS2 = 0;
    OPTION_REGbits.PS1  = 0;
    OPTION_REGbits.PS0 = 1;
    TMR0 = _t0_valor;
    
    //INTCONbits.T0IF = 0;
    
    return;
}