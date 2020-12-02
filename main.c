
// PIC18F26J50
// XC8 1.34

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <delays.h>
#define _XTAL_FREQ 8000000
#include "lcd.h"
#include <rtcc.h>
#include <string.h>

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


rtccTimeDate RtccTimeDate, RtccAlrmTimeDate, Rtcc_read_TimeDate;

char segundo_u; //variables BCD donde se guradan o leen los
char segundo_d; //datos del RTCC
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
int anio = 20; //año 2020 -> 00001110
int mes = 11; //mes enero-> 00000001 (1) .... dici-> 00001100 (12)
int diasem = 0; //0 Dom , 1 Lun , 2 Mar......, 6 Sab
int dia = 6; //dia 01 -> 00000001.....31 -> 00011111
int hora = 22; //hora 00 -> 00000000.....24 -> 00011000
int minuto = 55; //variables de numeros enteros que permiten
int segundo = 0; //representar los valores en el LCD


rtccTime RtccTime; // Inicializa la estructura de tiempo
rtccTime RtccTimeVal;
rtccDate RtccDate; //Inicializa la estructura de Fecha

//Posiciones de los elementos para cambiar

int posicionesLeft[] = {2, 0, 3, 0, 3, 3, 3, 6, 4, 0, 4, 3};
int posicionesUp[] = {2, 5, 2, 8, 2, 15, 2, 18, 4, 5, 4, 8, 4, 15, 4, 18};

char Sw_Up = 0;
char Sw_Down = 0;
char Sw_Left = 0;
char Sw_Right = 0;
char Sw_Center = 0;
char Humedad = 0;
char Flag_1 = 0;
int i;
char CuentamSeg = 0; //Para lectura del RTCC
char lectura = 0; //Lectura del conversor analogico
float temp_prog= 25.5; //Temperatura programada
float grados = 20; //Temperatura leida
char inicio_hora_ilum = 20;
char inicio_min_ilum = 0;
char fin_hora_ilum = 22;
char fin_min_ilum = 0;
char inicio_hora_riego = 22;
char inicio_min_riego = 0;
char fin_hora_riego = 21;
char fin_min_riego = 55;


#define ILUM_on         LATBbits.LATB7 = 1
#define ILUM_off        LATBbits.LATB7 = 0
#define RIEGO_on        LATBbits.LATB6 = 1
#define RIEGO_off       LATBbits.LATB6 = 0
#define TEMP_on         LATAbits.LA1 = 1
#define TEMP_off        LATAbits.LA1 = 0
#define HUMEDAD         Humedad = PORTCbits.RC7
#define SW_Up           Sw_Up = PORTAbits.RA3
#define SW_Down         Sw_Down = PORTCbits.RC4
#define SW_Left         Sw_Left = PORTAbits.RA0
#define SW_Right        Sw_Right = PORTAbits.RA2
#define SW_Center       Sw_Center = PORTAbits.RA5

void V_Principal(void);
void V_ProgReloj(void);
void V_ProgRiegoIlum(void);
void V_ProgTempAgua(void);
void V_Reiniciar(void);
char* Convert_diasem(int dia);
void control_leds_iluminacion(void);
void control_leds_riego(void);
void control_temperatura_agua(void);
void lee_temperatura(void);
void Write_RTC(void);
void Read_RTC(void);
void escribir(char fila, char columna, char *formato, char variable);

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

void main() {
    OSCTUNEbits.INTSRC = 1; //setea el oscilador de 32768 para el RTC
    OSCTUNEbits.PLLEN = 0; //desactiva PLL
    OSCCONbits.IRCF0 = 1; //selecciona el clock en 8MHz
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;
    OSCCONbits.SCS0 = 0; //oscilator INTRC
    OSCCONbits.SCS1 = 0;

    RTCCFGbits.RTCEN = 1; //Seteos del RTCC
    RTCCFGbits.RTCWREN = 1;
    T1CONbits.T1OSCEN = 1;

    TRISA = 0b11111101; //A7...A2 entradas, A1 y A0 salidas
    TRISB = 0; //Todas salidas
    TRISC = 0b11111111; //Todas entradas
    ANCON0 = 0b11111111; //Entradas digitales
    ANCON1 = 0b11110111; //Entradas AN11 analógica, el resto digitales
    ADCON0 = 0b00101111; //Ref:0-3,3v Conversor apunta a A11
    ADCON1 = 0b00111001; //Justif izquierda, tiempo adquisicion 20TAD y FOSC/8

    Write_RTC(); //Escribe los valores por defecto en el RTCC
    Lcd_Init();
    Lcd_Cmd(LCD_CLEAR);
    Lcd_Cmd(LCD_CURSOR_OFF);


    __delay_ms(98);
    __delay_ms(98);
    __delay_ms(98);
    __delay_ms(98);
    __delay_ms(98);
    __delay_ms(98);
    __delay_ms(98);
    __delay_ms(98);
    Lcd_Cmd(LCD_CLEAR);
    
  
    while (1) {

        //CuentamSeg++;
        //if (CuentamSeg >= 26){CuentamSeg = 0;V_Principal();}
        V_Principal();
        
        control_leds_iluminacion();
        control_leds_riego();
        control_temperatura_agua();
      //  sprintf(buffer1, "%03u", lectura);
       // Lcd_Out(1, 0, buffer1);
       
        
        
        if (SW_Left == 0) {

            V_ProgReloj();
        }

        if (SW_Down == 0) {
            V_Reiniciar();
        }

        if (SW_Right == 0) {
            V_ProgTempAgua();
        }

        if (SW_Up == 0) {
            V_ProgRiegoIlum();
        }

    }
}

/**********************************************************************************
 Funcion V_Principal
 ***********************************************************************************/
void V_Principal(void) {
    Lcd_Out(1, 0, "Ventana principal   ");
    Read_RTC(); 
    char* cte = Convert_diasem(diasem);
    Lcd_Out(2, 0, cte);
    sprintf(buffer1, "%02u/%02u/%02u             ", dia, mes, anio);
    Lcd_Out(3, 0, buffer1);
    sprintf(buffer1, "%02u:%02u:%02u  ", hora, minuto, segundo);
    Lcd_Out(4, 0, buffer1);
    sprintf(buffer1, "TA: %.1f%cC  ", temp_prog, 223);
    Lcd_Out(4, 10, buffer1);

}

/**********************************************************************************
 Funcion V_ProgReloj
 ***********************************************************************************/
void V_ProgReloj(void) {
    
    Lcd_Cmd(LCD_CLEAR);
    Lcd_Out(1, 0, "Program del reloj   ");
    char* cte = Convert_diasem(diasem);
    Lcd_Out(2, 0, cte);
    sprintf(buffer1, "%02u/%02u/%02u             ", dia, mes, anio);
    Lcd_Out(3, 0, buffer1);
    sprintf(buffer1, "%02u:%02u       ", hora, minuto);
    Lcd_Out(4, 0, buffer1);
    char diasem_tr = diasem;
    char dia_tr = dia;
    char mes_tr = mes;
    char anio_tr = anio;
    char hora_tr = hora;
    char minuto_tr = minuto;
    int posicion = 0;
    Lcd_Cmd(LCD_BLINK_CURSOR_ON);
    Lcd_Out(2, 0, "");

    for (i = 0; i < 30; i++) {
        
        sprintf(buffer1, "%d", diasem_tr);
        //Lcd_Out(1, 0, buffer1);
        __delay_ms(90);
       

        if (SW_Up == 0) {
            
            i = 0;
            if (posicion == 0) {
                if (diasem_tr < 6) {
                    diasem_tr++;
                }
                Lcd_Cmd(LCD_CURSOR_OFF);
                Lcd_Out(2, 0, Convert_diasem(diasem_tr));
                Lcd_Out(2, 0, "");
                Lcd_Cmd(LCD_BLINK_CURSOR_ON);
            } else if (posicion == 2) {
                if (dia_tr < 31) {
                    dia_tr++;
                }
                escribir(3,0, "%02u", dia_tr);
            } else if (posicion == 4) {
                if (mes_tr < 12) {
                    mes_tr++;
                }
                escribir(3,3,"%02u", mes_tr );
            } else if (posicion == 6) {
                if (anio_tr < 99) {
                    anio_tr++;
                }
                escribir(3,6, "%02u", anio_tr);
            } else if (posicion == 8) {
                if (hora_tr < 23) {
                    hora_tr++;
                }
                escribir(4,0, "%02u", hora_tr);
            } else if (posicion == 10) {
                if (minuto_tr < 59) {
                    minuto_tr++;
                }
                escribir(4,3, "%02u", minuto_tr);
            }
        }

        if (SW_Right == 0) {
            if (posicion <= 9) {
                posicion += 2;
            }
            i = 0;
            Lcd_Out(posicionesLeft[posicion], posicionesLeft[posicion + 1], "");
        }

        if (SW_Left == 0) {
            if (posicion >= 2) {
                posicion -= 2;
            }
            i = 0;
            Lcd_Out(posicionesLeft[posicion], posicionesLeft[posicion + 1], "");
        }

        if (SW_Down == 0) {
            i = 0;
            if (posicion == 0) {
                if (diasem_tr > 0) {
                    diasem_tr--;
                }
                Lcd_Cmd(LCD_CURSOR_OFF);
                Lcd_Out(2, 0, Convert_diasem(diasem_tr));
                Lcd_Out(2, 0, "");
                Lcd_Cmd(LCD_BLINK_CURSOR_ON); 
           } else if (posicion == 2) {
                if (dia_tr > 1) {
                    dia_tr--;
                }
                escribir(3,0, "%02u", dia_tr);
            } else if (posicion == 4) {
                if (mes_tr > 1) {
                    mes_tr--;
                }
                escribir(3,3, "%02u", mes_tr);
            } else if (posicion == 6) {
                if (anio_tr > 20) {
                    anio_tr--;
                }
                escribir(3,6, "%02u", anio_tr);
            } else if (posicion == 8) {
                if (hora_tr > 0) {
                    hora_tr--;
                }
                escribir(4,0, "%02u", hora_tr);
            } else if (posicion == 10) {
                if (minuto_tr > 0) {
                    minuto_tr--;
                }
                escribir(4,3, "%02u", minuto_tr);
            }
        }

        if (SW_Center == 0) {
            diasem = diasem_tr;
            dia = dia_tr;
            mes = mes_tr;
            anio = anio_tr;
            hora = hora_tr;
            minuto = minuto_tr;
            Write_RTC();
            break;
        }
    }
 Lcd_Cmd(LCD_CURSOR_OFF);
}

/**********************************************************************************
 Funcion V_ProgRiegoIlum
 ***********************************************************************************/

void V_ProgRiegoIlum(void) {
    
    Lcd_Cmd(LCD_CLEAR);
    Lcd_Out(1, 0, "Riego               ");
    sprintf(buffer1, "Inic %02u:%02u", inicio_hora_riego, inicio_min_riego);
    Lcd_Out(2, 0, buffer1);
    sprintf(buffer1, "Fin %02u:%02u", fin_hora_riego, fin_min_riego);
    Lcd_Out(2, 11, buffer1);
    Lcd_Out(3, 0, "Iluminacion         ");
    sprintf(buffer1, "Inic %02u:%02u", inicio_hora_ilum, inicio_min_ilum);
    Lcd_Out(4, 0, buffer1);
    sprintf(buffer1, "Fin %02u:%02u", fin_hora_ilum, fin_min_ilum);
    Lcd_Out(4, 11, buffer1);

    char i_hora_ilum_tr = inicio_hora_ilum;
    char i_min_ilum_tr = inicio_min_ilum;
    char f_hora_ilum_tr = fin_hora_ilum;
    char f_min_ilum_tr = fin_min_ilum;
    char i_hora_riego_tr = inicio_hora_riego;
    char i_min_riego_tr = inicio_min_riego;
    char f_hora_riego_tr = fin_hora_riego;
    char f_min_riego_tr = fin_min_riego;
    int posicion = 0;
    Lcd_Cmd(LCD_BLINK_CURSOR_ON);
    Lcd_Out(2, 5, "");

    for (i = 0; i < 30; i++) {
        __delay_ms(90);

        if (SW_Up == 0) {
            i = 0;
            if (posicion == 0) {
                if (i_hora_riego_tr < 23) {
                    i_hora_riego_tr++;
                }
                escribir(2,5, "%02u", i_hora_riego_tr);
            } else if (posicion == 2) {
                if (i_min_riego_tr < 59) {
                    i_min_riego_tr++;
                }
                escribir(2,8, "%02u", i_min_riego_tr);
            } else if (posicion == 4) {
                if (f_hora_riego_tr < 23) {
                    f_hora_riego_tr++;
                }
                escribir(2,15, "%02u", f_hora_riego_tr);
            } else if (posicion == 6) {
                if (f_min_riego_tr < 59) {
                    f_min_riego_tr++;
                }
                escribir(2,18, "%02u", f_min_riego_tr);
            } else if (posicion == 8) {
                if (i_hora_ilum_tr < 23) {
                    i_hora_ilum_tr++;
                }
                escribir(4,5, "%02u", i_hora_ilum_tr);
            } else if (posicion == 10) {
                if (i_min_ilum_tr < 59) {
                    i_min_ilum_tr++;
                }
                escribir(4,8, "%02u", i_min_ilum_tr);
            } else if (posicion == 12) {
                if (f_hora_ilum_tr < 23) {
                    f_hora_ilum_tr++;
                }
                escribir(4,15, "%02u", f_hora_ilum_tr);
            } else if (posicion == 14) {
                if (f_min_ilum_tr < 59) {
                    f_min_ilum_tr++;
                }
                escribir(4,18, "%02u", f_min_ilum_tr);
            }
        }

        if (SW_Right == 0) {
            if (posicion <= 13) {
                posicion += 2;
            }
            i = 0;
            Lcd_Out(posicionesUp[posicion], posicionesUp[posicion + 1], "");
        }

        if (SW_Left == 0) {
            if (posicion >= 2) {
                posicion -= 2;
            }
            i = 0;
            Lcd_Out(posicionesUp[posicion], posicionesUp[posicion + 1], "");
        }

        if (SW_Down == 0) {
            i = 0;
            if (posicion == 0) {
                if (i_hora_riego_tr > 0) {
                    i_hora_riego_tr--;
                }
                escribir(2,5, "%02u", i_hora_riego_tr);
            } else if (posicion == 2) {
                if (i_min_riego_tr > 0) {
                    i_min_riego_tr--;
                }
                escribir(2,8, "%02u", i_min_riego_tr);
            } else if (posicion == 4) {
                if (f_hora_riego_tr > 0) {
                    f_hora_riego_tr--;
                }
                escribir(2,15, "%02u", f_hora_riego_tr);
            } else if (posicion == 6) {
                if (f_min_riego_tr > 0) {
                    f_min_riego_tr--;
                }
                escribir(2,18, "%02u", f_min_riego_tr);
            } else if (posicion == 8) {
                if (i_hora_ilum_tr > 0) {
                    i_hora_ilum_tr--;
                }
                escribir(4,5, "%02u", i_hora_ilum_tr);
            } else if (posicion == 10) {
                if (i_min_ilum_tr > 0) {
                    i_min_ilum_tr--;
                }
                escribir(4,8, "%02u", i_min_ilum_tr);
            } else if (posicion == 12) {
                if (f_hora_ilum_tr > 0) {
                    f_hora_ilum_tr--;
                }
                escribir(4,15, "%02u", f_hora_ilum_tr);
            } else if (posicion == 14) {
                if (f_min_ilum_tr > 0) {
                    f_min_ilum_tr--;
                }
                escribir(4,18, "%02u", f_min_ilum_tr);
            }
        }


        if (SW_Center == 0) {
            inicio_hora_ilum = i_hora_ilum_tr;
            inicio_min_ilum = i_min_ilum_tr;
            fin_hora_ilum = f_hora_ilum_tr;
            fin_min_ilum = f_min_ilum_tr;
            inicio_hora_riego = i_hora_riego_tr;
            inicio_min_riego = i_min_riego_tr;
            fin_hora_riego = f_hora_riego_tr;
            fin_min_riego = f_min_riego_tr;
          //  Write_RTC();
            break;
        }
    }
    Lcd_Cmd(LCD_CURSOR_OFF);
}

/**********************************************************************************
 Funcion V_ProgTempAgua(void);
 ***********************************************************************************/
void V_ProgTempAgua(void) {
    
    Lcd_Cmd(LCD_CLEAR);
    float temp_prog_tr = temp_prog; 
    Lcd_Out(1, 0, "Programacion de la  ");
    Lcd_Out(2, 0, "Temperatura del agua");
    Lcd_Out(3, 0, "                    ");
    sprintf(buffer1, "Temperatura: %.1f%cC", temp_prog, 223);
    Lcd_Out(4, 0, buffer1);
    
    Lcd_Out(4, 13, "");
    Lcd_Cmd(LCD_BLINK_CURSOR_ON);
    
    for (i = 0; i < 30; i++) {
        __delay_ms(90);
        
        if (SW_Up == 0) {
            i = 0;
            if (temp_prog_tr < 33.1) {temp_prog_tr+=0.1; }
                Lcd_Cmd(LCD_CURSOR_OFF);
                sprintf(buffer1, "%0.1f", temp_prog_tr);
                Lcd_Out(4, 13, buffer1);
                Lcd_Out(4, 13, "");
                Lcd_Cmd(LCD_BLINK_CURSOR_ON);
        }



        if (SW_Down == 0) {
            i = 0;
            if (temp_prog_tr > 0) {temp_prog_tr-=0.1; }
                Lcd_Cmd(LCD_CURSOR_OFF);
                sprintf(buffer1, "%.1f", temp_prog_tr);
                Lcd_Out(4, 13, buffer1);
                Lcd_Out(4, 13, "");
                Lcd_Cmd(LCD_BLINK_CURSOR_ON);
    
        }


        if (SW_Center == 0) {
          temp_prog = temp_prog_tr;
          break;
        }
    }
    Lcd_Cmd(LCD_CURSOR_OFF);
}

/**********************************************************************************
 Funcion V_Reiniciar(void);
 ***********************************************************************************/

void V_Reiniciar(void) {
    
    Lcd_Cmd(LCD_CLEAR);
    char borrar = 0;  
    Lcd_Out(1, 0, "¿Esta seguro de rein");
    Lcd_Out(2, 0, "iciar a los valores ");
    Lcd_Out(3, 0, "por defecto?        ");
    Lcd_Out(4, 0, "   No          Si   ");

    Lcd_Out(4, 3, "");
    Lcd_Cmd(LCD_BLINK_CURSOR_ON);
    
    for (i = 0; i < 30; i++) {
        __delay_ms(90);
        
        if (SW_Right == 0) {
            i=0;
            borrar = 1;
            Lcd_Out(4, 15, "");
        }


        if (SW_Left == 0) {
            i=0;
            borrar = 0;
            Lcd_Out(4, 3, "");
        }


        if (SW_Center == 0) {
            
            if(borrar == 1){
                anio = 20; 
                mes = 11; 
                diasem = 0; 
                dia = 6; 
                hora = 23; 
                minuto = 58; 
                segundo = 0; 
                lectura = 0; 
                temp_prog= 25.3; 
                grados = 20; 
                inicio_hora_ilum = 20;
                inicio_min_ilum = 0;
                fin_hora_ilum = 22;
                fin_min_ilum = 0;
                inicio_hora_riego = 23;
                inicio_min_riego = 0;
                fin_hora_riego = 23;
                fin_min_riego = 59;
          Write_RTC();
          break;
            }else{
                break;
            }
        }      
    }
    Lcd_Cmd(LCD_CURSOR_OFF);
}

//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
// Funcion Write_RTC
// Permite capturar los datos del RTC y cargarlos en los registros correspondientes
// (diasem, anio, dia, hora, etc)
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[

void Write_RTC(void) {
    RtccWrOn(); //write enable the rtcc registers
    //mRtccSetClockOe(1);            //saca hacia afuera el pulso(ver patita)RTCC output pin
    PIE3bits.RTCCIE = 1;
    segundo_d = segundo / 10;
    segundo_u = segundo - segundo_d * 10;
    RtccTime.f.sec = segundo_d * 16 + segundo_u; //guarda segundo en rtcc
    minuto_d = minuto / 10;
    minuto_u = minuto - minuto_d * 10;
    RtccTime.f.min = minuto_d * 16 + minuto_u; //guarda minuto en rtcc
    hora_d = hora / 10;
    hora_u = hora - hora_d * 10;
    RtccTime.f.hour = hora_d * 16 + hora_u; //guarda hora en rtcc
    anio_d = anio / 10;
    anio_u = anio - anio_d * 10;
    RtccDate.f.year = anio_d * 16 + anio_u; //guarda año en rtcc
    mes_d = mes / 10;
    mes_u = mes - mes_d * 10;
    RtccDate.f.mon = mes_d * 16 + mes_u; //guarda mes en rtcc
    fecha_d = dia / 10;
    fecha_u = dia - fecha_d * 10;
    RtccDate.f.mday = fecha_d * 16 + fecha_u; //guarda dia (numero)en rtcc
    dia_semana = diasem;
    RtccDate.f.wday = diasem; //guarda dia (domingo ... sabado)

    RtccWriteTime(&RtccTime, 1); //write into registers
    RtccWriteDate(&RtccDate, 1); //write into registers
    mRtccOn(); //habilita rtcc
}


//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
// Funcion Read_RTC
// Permite capturar los datos del RTC y cargarlos en los registros correspondientes
// (diasem, anio, dia, hora, etc)
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[

void Read_RTC(void)//los digitos del RTCC estan en formato BCD hay que particionar
//la lectura de la estructura RtccTime (cada elemento en 2 nibles)
{
    RtccReadTime(&RtccTime); //Lee estructura de la hora del RTCC
    segundo_u = (RtccTime.f.sec & 0x0F); //lee la parte baja en BCD
    segundo_d = ((RtccTime.f.sec & 0xF0) >> 4); //lee la parte alta en BCD
    minuto_u = (RtccTime.f.min & 0x0F);
    minuto_d = ((RtccTime.f.min & 0xF0) >> 4);
    hora_u = (RtccTime.f.hour & 0x0F);
    hora_d = ((RtccTime.f.hour & 0xF0) >> 4);
    RtccReadDate(&RtccDate); //Lee estructura del la Fecha del RTCC
    fecha_u = (RtccDate.f.mday & 0x0F);
    fecha_d = ((RtccDate.f.mday & 0xF0) >> 4);
    mes_u = (RtccDate.f.mon & 0x0F);
    mes_d = ((RtccDate.f.mon & 0xF0) >> 4);
    dia_semana = RtccDate.f.wday;
    anio_u = (RtccDate.f.year & 0x0F);
    anio_d = ((RtccDate.f.year & 0xF0) >> 4);
    segundo = segundo_d * 10 + segundo_u;
    minuto = minuto_d * 10 + minuto_u;
    hora = hora_d * 10 + hora_u;
    dia = fecha_d * 10 + fecha_u;
    diasem = dia_semana;
    mes = mes_d * 10 + mes_u;
    anio = anio_d * 10 + anio_u;
}

//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/// Funcion lee_temperatura
/// Mide la tensión de la termocupla y transfiere un valor a la variable lectura
/// intervalo y duracion
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[

void lee_temperatura(void) {

    ADCON0bits.ADON = 1;
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO);
    __delay_ms(1);
    lectura = ADRESH;
    ADCON0bits.ADON = 0;
}

char* Convert_diasem(int dia) {

    unsigned char buffer_dia[20];

    switch (dia) {
        case 0:
        {
            sprintf(buffer_dia, "%s", "Domingo             ");
            break;
        }
        case 1:
        {
            sprintf(buffer_dia, "%s", "Lunes               ");
            break;
        }
        case 2:
        {
            sprintf(buffer_dia, "%s", "Martes              ");
            break;
        }
        case 3:
        {
            sprintf(buffer_dia, "%s", "Miercoles           ");
            break;
        }
        case 4:
        {
            sprintf(buffer_dia, "%s", "Jueves              ");
            break;
        }
        case 5:
        {
            sprintf(buffer_dia, "%s", "Viernes             ");
            break;
        }
        case 6:
        {
            sprintf(buffer_dia, "%s", "Sabado              ");
            break;
        }
    }
    return buffer_dia;
}

void control_leds_iluminacion() {
    
    if(inicio_hora_ilum == hora && inicio_min_ilum == minuto){
        ILUM_on;         
    }
    
    if(fin_hora_ilum == hora && fin_min_ilum == minuto){
        ILUM_off;
    }
}

void control_leds_riego(){
    
    if(inicio_hora_riego == hora && inicio_min_riego == minuto){
        if(HUMEDAD == 0){
            RIEGO_on;
        }
                
    }
    
    if(fin_hora_riego == hora && fin_min_riego == minuto){
        RIEGO_off;
    }
}

void control_temperatura_agua(){
    
    lee_temperatura();
    grados = lectura * 0.13;
    float grados_medidos = ((int)(grados*10)) / 10.0;
    float temp_min;
    float temp_max = temp_prog + 0.3;
    
    if(temp_prog > 0.3){ //en caso de que la temperatura sea 0.2 o menor y no quede negativa
        temp_min = temp_prog - 0.3;
    }else{
        temp_min = 0;
    }
    
    if(grados_medidos < temp_min) {
        TEMP_on;
    }else if(grados_medidos > temp_max){
        TEMP_off;
    }else{
        TEMP_off;
    }

}

void escribir(char fila, char columna, char *formato, char variable ){
    
    Lcd_Cmd(LCD_CURSOR_OFF);
    sprintf(buffer1, formato , variable);
    Lcd_Out(fila, columna, buffer1);
    Lcd_Out(fila, columna, "");
    Lcd_Cmd(LCD_BLINK_CURSOR_ON);
}
