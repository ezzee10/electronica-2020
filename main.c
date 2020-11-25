
// PIC18F26J50
// XC8 1.34

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <delays.h>
#define _XTAL_FREQ 8000000
#include "lcd.h"
#include <rtcc.h>

#pragma config CPUDIV=OSC1
#pragma config CP0=OFF
#pragma config WDTEN=OFF
#pragma config PLLDIV=1
#pragma config STVREN=ON
#pragma config XINST=OFF
#pragma config XINST=OFF
#pragma config OSC=INTOSC
#pragma config T1DIG=ON
#pragma config LPT1OSC=OFF
#pragma config FCMEN=OFF
#pragma config IESO=OFF
#pragma config WDTPS=32768
#pragma config DSWDTOSC=INTOSCREF
#pragma config RTCOSC=T1OSCREF// RTCC Clock Select (RTCC uses T1OSC/T1CKI)
#pragma config DSBOREN=OFF
#pragma config DSWDTEN=OFF
#pragma config DSWDTPS=G2
#pragma config IOL1WAY=OFF
#pragma config MSSP7B_EN=MSK7
#pragma config WPCFG=OFF
#pragma config WPDIS=OFF

unsigned char buffer1[20];


rtccTimeDate RtccTimeDate ,RtccAlrmTimeDate, Rtcc_read_TimeDate ;

char segundo_u;         //variables BCD donde se guradan o leen los
char segundo_d;         //datos del RTCC
char minuto_u;
char minuto_d;
char hora_u;
char hora_d;
char fecha_u;
char fecha_d;
char mes_u;
char mes_d;
char dia_semana;
char anio_u;
char anio_d;
int anio=20;                //año 2020 -> 00001110
int mes=11;                  //mes enero-> 00000001 (1) .... dici-> 00001100 (12)
int diasem=2;               //0 Dom , 1 Lun , 2 Mar......, 6 Sab
int dia=16;                  //dia 01 -> 00000001.....31 -> 00011111
int hora=18;                 //hora 00 -> 00000000.....24 -> 00011000
int minuto=55;               //variables de numeros enteros que permiten
int segundo=0;              //representar los valores en el LCD

rtccTime RtccTime; // Inicializa la estructura de tiempo
rtccTime RtccTimeVal;
rtccDate RtccDate;//Inicializa la estructura de Fecha

char  Sw_Up = 0;
char  Sw_Down = 0;
char  Sw_Left = 0;
char  Sw_Right = 0;
char  Sw_Center = 0;
char  Flag_1 = 0;
int   i;
char  CuentamSeg = 0;       //Para lectura del RTCC
char  lectura = 0;          //Lectura del conversor analogico
float Temp_prog = 22.5;     //Temperatura programada
float Grados = 20;          //Temperatura leida

#define ILUM_on         LATBbits.LATB7 = 1
#define ILUM_off        LATBbits.LATB7 = 0
#define RIEGO_on        LATBbits.LATB6 = 1
#define RIEGO_off       LATBbits.LATB6 = 0
#define TEMP_on         LATAbits.LA1 = 1
#define TEMP_off        LATAbits.LA1 = 0
#define SW_Up           Sw_Up = PORTAbits.RA3
#define SW_Down         Sw_Down = PORTCbits.RC4
#define SW_Left         Sw_Left = PORTAbits.RA0
#define SW_Right        Sw_Right = PORTAbits.RA2
#define SW_Center       Sw_Center = PORTAbits.RA5

void V_Principal(void);
void V_ProgReloj(void);
char* Convert_diasem(int dia);
void lee_temperatura(void);
void Write_RTC(void);
void Read_RTC(void);
void Delay_6seg(void);

/**********************************************************************************
 COMANDOS DEL LCD
***********************************************************************************
 Lcd_Cmd(0x01);             //Clear Display, Cursor al inicio
 Lcd_Cmd(0x02);             //Cursor al inicio
 Lcd_Cmd(0x00);             //Apaga Display
 Lcd_Cmd(0x08);             //Enciende Display
 lcd_Cmd(0x0C);             //Enciende display sin cursor y sin blink
 lcd_Cmd(0x0D);             //Enciende display sin cursor y con blink
 lcd_Cmd(0x0E);             //Enciende display con cursor y sin blink
 lcd_Cmd(0x0F);             //Enciende display con cursor y con blink
 lcd_Cmd(0x06);             //Incrementa Direccion, Display fijo
 lcd_Cmd(0x04);             //Decrementa Direccion, Display fijo
 lcd_Cmd(0x07);             //Incrementa Direccion, Cursor fijo
 lcd_Cmd(0x05);             //Decrementa Direccion, Cursor fijo
 lcd_Cmd(0x10);             //Cursor a la Izquierda
 lcd_Cmd(0x14);             //Cursor a la Derecha
 lcd_Cmd(0x18);             //Display a la Izquierda
 lcd_Cmd(0x1C);             //Display a la Derecha
 Lcd_Init();                //inicializa el lcd
 Lcd_Out(x,y,"string");     //posiciona cursor,fila, col, escribe sring
 Lcd_Out2(x,y,buffer1);     //posiciona cursor,fila, col, escribe dato
 Lcd_Chr_CP(c);             //escribe 1 caracter en el cursor
 *********************************************************************************/

void main()
{
OSCTUNEbits.INTSRC=1;           //setea el oscilador de 32768 para el RTC
OSCTUNEbits.PLLEN=0;            //desactiva PLL
OSCCONbits.IRCF0=1;             //selecciona el clock en 8MHz
OSCCONbits.IRCF1=1;
OSCCONbits.IRCF2=1;
OSCCONbits.SCS0=0;              //oscilator INTRC
OSCCONbits.SCS1=0;

RTCCFGbits.RTCEN=1;             //Seteos del RTCC
RTCCFGbits.RTCWREN=1;
T1CONbits.T1OSCEN=1;

TRISA = 0b11111101;             //A7...A2 entradas, A1 y A0 salidas
TRISB = 0;                      //Todas salidas
TRISC = 0b11111111;             //Todas entradas
ANCON0 = 0b11111111;            //Entradas digitales
ANCON1 = 0b11110111;            //Entradas AN11 analógica, el resto digitales
ADCON0 = 0b00101111;            //Ref:0-3,3v Conversor apunta a A11
ADCON1 = 0b00111001;            //Justif izquierda, tiempo adquisicion 20TAD y FOSC/8

Write_RTC();                    //Escribe los valores por defecto en el RTCC
Lcd_Init();
Lcd_Cmd(LCD_CLEAR);
Lcd_Cmd(LCD_CURSOR_OFF);
__delay_ms(98);
/*
Lcd_Out(1, 1, "Primera fila");
Lcd_Out(2, 1, "Segunda fila");
Lcd_Out(3, 1, "Tercera fila");
Lcd_Out(4, 2, "Cuarta fila");
 */
__delay_ms(98);__delay_ms(98);__delay_ms(98);__delay_ms(98);__delay_ms(98);__delay_ms(98);__delay_ms(98);__delay_ms(98);
Lcd_Cmd(LCD_CLEAR);

while(1){
    
    //CuentamSeg++;
    //if (CuentamSeg >= 26){CuentamSeg = 0;V_Principal();}
    V_Principal();
    if(SW_Left == 0){
            
        Lcd_Out(1, 0, "Program del reloj");
        char diasem_tr = diasem;
        for(i=0 ;i<30;i++){
            __delay_ms(90);
            if(SW_Up == 0){
                i=0;
                diasem_tr++;
                Lcd_Out(2, 0, Convert_diasem(diasem_tr) );
                //while(SW_Up == 0);
            }
            if(SW_Right == 0){
                
            }
            if(SW_Center == 0){
                diasem = diasem_tr;
                Write_RTC();
                CuentamSeg = 26;
                break;
            }
            CuentamSeg = 26;
        }
             
    }

    if(SW_Down == 0){
    //lectura = 222;
        sprintf(buffer1,"%03u",lectura);
        Lcd_Out(4,0,buffer1);
    }

    if(SW_Right == 0){
        Lcd_Out(3, 0, "RIGHT             ");
    }

    if(SW_Up == 0){
        Lcd_Out(4, 0, "UP               ");
    }

    if(SW_Center == 0){
        Lcd_Cmd(LCD_CLEAR);
        Lcd_Out(2, 0, "CENTER             ");
    }

    ILUM_on;
    RIEGO_on;
    TEMP_on;
    __delay_ms(10);
    ILUM_off;
    RIEGO_off;
    TEMP_off;
    __delay_ms(10);
    }

}

/**********************************************************************************
 Funcion V_Principal
***********************************************************************************/
void V_Principal(void)
{
    Read_RTC();
    lee_temperatura();
    Lcd_Out(1, 0, "                    ");
    char* cte = Convert_diasem(diasem);
    
    Lcd_Out(2, 0, cte);
   
    
    sprintf(buffer1,"%02u/%02u/%02u",dia,mes,anio);
    Lcd_Out(3,0,buffer1);
    sprintf(buffer1,"%02u:%02u:%02u",hora,minuto,segundo);
    Lcd_Out(4,0,buffer1);
    //lcd_comand(0x14);              //Cursor a la Derecha
    //lcd_putrs("TA 22,5");
}


/**********************************************************************************
 Funcion V_ProgReloj
***********************************************************************************/
void V_ProgReloj(void)
{

}

//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
// Funcion Write_RTC
// Permite capturar los datos del RTC y cargarlos en los registros correspondientes
// (diasem, anio, dia, hora, etc)
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[

    void Write_RTC(void)
    {
       RtccWrOn();                     //write enable the rtcc registers
       //mRtccSetClockOe(1);            //saca hacia afuera el pulso(ver patita)RTCC output pin
       PIE3bits.RTCCIE=1;
       segundo_d=segundo/10;
       segundo_u=segundo-segundo_d*10;
       RtccTime.f.sec =segundo_d*16+segundo_u;  //guarda segundo en rtcc
       minuto_d=minuto/10;
       minuto_u=minuto-minuto_d*10;
       RtccTime.f.min =minuto_d*16+minuto_u;    //guarda minuto en rtcc
       hora_d=hora/10;
       hora_u=hora-hora_d*10;
       RtccTime.f.hour=hora_d*16+hora_u;        //guarda hora en rtcc
       anio_d=anio/10;
       anio_u=anio-anio_d*10;
       RtccDate.f.year=anio_d*16+anio_u;        //guarda año en rtcc
       mes_d=mes/10;
       mes_u=mes-mes_d*10;
       RtccDate.f.mon=mes_d*16+mes_u;         //guarda mes en rtcc
       fecha_d=dia/10;
       fecha_u=dia-fecha_d*10;
       RtccDate.f.mday=fecha_d*16+fecha_u;      //guarda dia (numero)en rtcc
       dia_semana=diasem;
       RtccDate.f.wday =diasem;                 //guarda dia (domingo ... sabado)

       RtccWriteTime(&RtccTime,1);      //write into registers
       RtccWriteDate(&RtccDate,1);      //write into registers
       mRtccOn();                       //habilita rtcc
        }


//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
// Funcion Read_RTC
// Permite capturar los datos del RTC y cargarlos en los registros correspondientes
// (diasem, anio, dia, hora, etc)
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[

void Read_RTC(void)//los digitos del RTCC estan en formato BCD hay que particionar
                    //la lectura de la estructura RtccTime (cada elemento en 2 nibles)
   {
    RtccReadTime(&RtccTime);                //Lee estructura de la hora del RTCC
    segundo_u=(RtccTime.f.sec & 0x0F);      //lee la parte baja en BCD
    segundo_d=((RtccTime.f.sec & 0xF0)>>4); //lee la parte alta en BCD
    minuto_u=(RtccTime.f.min & 0x0F);
    minuto_d=((RtccTime.f.min & 0xF0)>>4);
    hora_u=(RtccTime.f.hour & 0x0F);
    hora_d=((RtccTime.f.hour & 0xF0)>>4);
    RtccReadDate(&RtccDate);                //Lee estructura del la Fecha del RTCC
    fecha_u=(RtccDate.f.mday & 0x0F);
    fecha_d=((RtccDate.f.mday & 0xF0)>>4);
    mes_u=(RtccDate.f.mon & 0x0F);
    mes_d=((RtccDate.f.mon & 0xF0)>>4);
    dia_semana=RtccDate.f.wday;
    anio_u=(RtccDate.f.year & 0x0F);
    anio_d=((RtccDate.f.year & 0xF0)>>4);
    segundo=segundo_d*10+segundo_u;
    minuto=minuto_d*10+minuto_u;
    hora=hora_d*10+hora_u;
    dia=fecha_d*10+fecha_u;
    diasem=dia_semana;
    mes=mes_d*10+mes_u;
    anio=anio_d*10+anio_u;
    }

//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/// Funcion lee_temperatura
/// Mide la tensión de la termocupla y transfiere un valor a la variable lectura
/// intervalo y duracion
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[

void lee_temperatura(void){
    
    ADCON0bits.ADON = 1;
    ADCON0bits.GO = 1;
    while(ADCON0bits.GO);
    __delay_ms(1);
    lectura = ADRESH;
    ADCON0bits.ADON = 0;     
}

void Delay_6seg(void){
    
    for(i=0 ;i<30;i++){
           __delay_ms(90);
        }
}

char* Convert_diasem(int dia){
    
    unsigned char buffer_dia[20];
    
    switch(dia){
        case 0: {
                sprintf(buffer_dia,"%s", "Domingo             ");
                break;
                }
        case 1: {
                sprintf(buffer_dia,"%s", "Lunes               ");
                break;
                }
        case 2: {
                sprintf(buffer_dia,"%s", "Martes              ");
                break;
                }
        case 3: {
                sprintf(buffer_dia,"%s", "Miercoles           ");
                break;
                }
        case 4: {
                sprintf(buffer_dia,"%s", "Jueves              ");
                break;
                }
        case 5: {
                sprintf(buffer_dia,"%s", "Viernes             ");
                break;
                }
        case 6: {
                sprintf(buffer_dia,"%s", "Sabado              ");
                break;
                }
    }
    return buffer_dia;
}