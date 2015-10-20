#include "defs.h"
#include "hd44780.h"
#include "pomosni.h"
#include "uart.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>


typedef enum { VNES, POSTAVUVANJE, SECENJE, EXTRA } sostojbi;
typedef enum { PRVRED, VTORRED, TRETRED } redovi;

sostojbi STATE;
redovi MOMENTALENRED;

int main(void) {

    // Privremeni promenlivi
    unsigned char voNovRed = 1;
    unsigned char pritisnatoKopce;
    unsigned char pozicijaZapirka = 0;
    unsigned char pozicijaNaKarakter = 0;
    unsigned char cekajPoslednaCifra = 0;
    unsigned char vnesenaPoslednaCifra = 0;

    // Promenlivi za podatoci od LCD
    unsigned int dolzina=0;
    unsigned int kodPili=0;
    unsigned int brojParcinja=0;

    // INICIJALIZACIJA NA PORTI, DISPLEJ, TASTATURA...
    init_uart();
    lcd_init();
    lcd_clrscr();
    initKeypadPins();
    initEncoder();
    initOutputs();
    initInputs();
    // END INICIJALIZACIJA

    //printf("Zdravo!\n");
    //PCINT0_vect();
    // Привремен код за тестирање
    //cli();
    sei();

    char niza[6];

    while(1) {
        _delay_ms(10);
        lcd_goto(0x0B);
        lcd_clearChars(9);
        lcd_goto(0x0B);
        sprintf(niza, "%lu", angularPosition);
        lcd_puts(niza);
    }


//    unsigned int oldState = 0;
//    while(1) {
//        unsigned int newState;
//        newState = getInputs();
//        if(newState != oldState) {
//            oldState = newState;
//            printf("Pins: %X\n", newState);
//        }
//    }



    // PROVERKA ZA BEZBEDNOST + START & STOP
    // Услови за фатална грешка - при фатална грешка, машината се гаси целосно. Порака на екран : што е грешката
    // 1. Активни два крајни (за движење) - истовремено
    // 2. Ако не се активни крајните на пилите - пробај да се вовлечат пилите - со активирање на вентилите. Timeout = 2..5 секунди. Ако во ова време не се вратат пилите - фатална грешка.
    // 3. Ако нема притисок / има поголем притисок од притисочните сензори - фатална грешка
    // END PROVERKA

    // POSTAVUVANJE NA MASINATA NA 0 - DVIZENJE NA MOTOROT NA NAJMALA DOLZINA
    //
    // 1. Читање на моментална позиција,
    // 2. Ако е различно од нулта - придвижија машината кон нулта
    // 3. Читај енкодер + тахо + гранични + безбедносни
    //
    // END POSTAVUVANJE


    // Дифолтни состојби на почетокот
    STATE = VNES;
    MOMENTALENRED = PRVRED;

    while(1) { // Glaven loop

        // PROVERKA ZA BEZBEDNOST + START & STOP

        // END PROVERKA

        switch(STATE) {
            case VNES:
                        switch(MOMENTALENRED) {
                            case PRVRED:
                                    if(voNovRed) { // Подеси се на нов ред - позиционирај го курсорот, избриши линија, исчисти соодветни променливи
                                        lcd_printStatus("VNESI DOLZINA!");
                                        lcd_goto(0x00);
                                        lcd_clearChars(10);
                                        lcd_goto(0x00);
                                        lcd_putc(0x2A);
                                        lcd_putc(0x10);
                                        voNovRed = 0;
                                        pozicijaZapirka = 0;
                                        pozicijaNaKarakter = 0;
                                        vnesenaPoslednaCifra = 0;
                                        cekajPoslednaCifra = 0;
                                        dolzina = 0;
                                    } else { // Значи треба да внесуваме должина :-)

                                        pritisnatoKopce = scanKeypad();

                                        if(pritisnatoKopce != 0xFF) {
                                            // Прво исхендлај ги специјалните копчиња (ентер, BS, стрелка нагоре, clear)
                                            //
                                            if(pritisnatoKopce == 0x0C) { // ЕНТЕР
                                                // Проверка дали должината е во дозволениот опсег !
                                                if((dolzina > MINL) && (dolzina < MAXL) && vnesenaPoslednaCifra) {
                                                    // Можеме да отидеме во другата состојба !
                                                    MOMENTALENRED = VTORRED;
                                                    voNovRed = 1;
                                                    lcd_goto(0x00);
                                                    lcd_clearChars(1);
                                                    break;
                                                } else {
                                                    MOMENTALENRED = PRVRED;
                                                    voNovRed = 1;
                                                    break;
                                                }
                                                break;
                                            } else if(pritisnatoKopce == 0x0B) { // CLEAR -> Goto PRVRED со voNovRed = 1;
                                                MOMENTALENRED = PRVRED;
                                                voNovRed = 1;
                                                break;
                                            } else if(pritisnatoKopce == 0x0D) { // UP - Во првиот ред прави исто како и CLEAR
                                                MOMENTALENRED = PRVRED;
                                                voNovRed = 1;
                                                break;
                                            } else if(pritisnatoKopce == 0x0E) { // BACKSPACE
                                                break;
                                            } else if(pritisnatoKopce == 0x0A) { // COMMA - прикажи запирка на дисплеј, _ништо_ во dolzina!
                                                if(!pozicijaZapirka && ((pozicijaNaKarakter == 3) || (pozicijaNaKarakter == 4))) {
                                                    lcd_putc(0x2E);
                                                    pozicijaZapirka = pozicijaNaKarakter;
                                                    pozicijaNaKarakter++;
                                                    cekajPoslednaCifra = 1;
                                                }
                                                break;
                                                // Запиши ја локацијата на запирката !
                                            } else { // Значи е внесена бројка !
                                                if(!vnesenaPoslednaCifra) {
                                                    if(cekajPoslednaCifra) { // Максимално може да се внесат 6 карактери !
                                                        lcd_putc(pritisnatoKopce + 48);
                                                        //pozicijaNaKarakter++;
                                                        vnesenaPoslednaCifra = 1;
                                                        dolzina = dolzina*10 + pritisnatoKopce;
                                                    } else if(pozicijaNaKarakter <= 3) {
                                                        lcd_putc(pritisnatoKopce + 48);
                                                        pozicijaNaKarakter++;
                                                        dolzina = dolzina*10 + pritisnatoKopce;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    break;
                            case VTORRED:
                                    if(voNovRed) { // Подеси се на нов ред - позиционирај го курсорот, избриши линија, исчисти соодветни променливи
                                        lcd_printStatus("VNESI KOD ZA PILI!");

                                        lcd_goto(0x40);
                                        lcd_clearChars(10);
                                        lcd_goto(0x40);

                                        lcd_putc(0x2A);
                                        lcd_putc(0x10);
                                        voNovRed = 0;

                                        kodPili = 0;

                                    } else {
                                        pritisnatoKopce = scanKeypad();

                                        if(pritisnatoKopce != 0xFF) {

                                            if(pritisnatoKopce == 0x0C) { // ЕНТЕР
                                                if((kodPili != 0) && (kodPili != 9)) {
                                                    MOMENTALENRED = TRETRED;
                                                    voNovRed = 1;
                                                    lcd_goto(0x40);
                                                    lcd_clearChars(1);
                                                    break;
                                                } else {
                                                    MOMENTALENRED = VTORRED;
                                                    voNovRed = 1;
                                                    break;
                                                }
                                                break;
                                            } else if(pritisnatoKopce == 0x0B) { // CLEAR -> Goto PRVRED со voNovRed = 1;
                                                voNovRed = 1;
                                                lcd_goto(0x40);
                                                lcd_clearChars(10);
                                                MOMENTALENRED = VTORRED;
                                                break;
                                            } else if(pritisnatoKopce == 0x0D) { // UP
                                                voNovRed = 1;
                                                lcd_goto(0x40);
                                                lcd_clearChars(10);
                                                MOMENTALENRED = PRVRED;
                                                break;
                                            } else if(pritisnatoKopce == 0x0E) { // BACKSPACE
                                                break;
                                            } else if(pritisnatoKopce == 0x0A) { // COMMA
                                                break;
                                            } else { // Значи е внесена бројка !
                                                if((pritisnatoKopce != 9) && (pritisnatoKopce != 0) && !kodPili) {
                                                    lcd_putc(pritisnatoKopce + 48);
                                                    kodPili = pritisnatoKopce;
                                                }
                                            }
                                        }
                                    }

                                    break;
                            case TRETRED:
                                    if(voNovRed) { // Подеси се на нов ред - позиционирај го курсорот, избриши линија, исчисти соодветни променливи
                                        lcd_printStatus("VNESI BROJ PARCINJA!");
                                        lcd_goto(0x14);
                                        lcd_clearChars(10);
                                        lcd_goto(0x14);

                                        lcd_putc(0x2A);
                                        lcd_putc(0x10);
                                        voNovRed = 0;

                                        pozicijaNaKarakter = 0;
                                        cekajPoslednaCifra = 0;
                                        brojParcinja = 0;
                                    } else { // Значи треба да внесуваме должина :-)

                                        pritisnatoKopce = scanKeypad();

                                        if(pritisnatoKopce != 0xFF) {
                                            // Прво исхендлај ги специјалните копчиња (ентер, BS, стрелка нагоре, clear)
                                            //
                                            if(pritisnatoKopce == 0x0C) { // ЕНТЕР
                                                if(brojParcinja <= 999) {
                                                    lcd_printStatus("PRITISNI START!");
                                                    voNovRed = 1;
                                                    lcd_goto(0x14);
                                                    lcd_clearChars(1);
                                                    MOMENTALENRED = PRVRED;
                                                    STATE = POSTAVUVANJE;
                                                    break;
                                                } else {
                                                    voNovRed = 1;
                                                    MOMENTALENRED = TRETRED;
                                                    break;
                                                }
                                                break;
                                            } else if(pritisnatoKopce == 0x0B) { // CLEAR
                                                voNovRed = 1;
                                                lcd_goto(0x14);
                                                lcd_clearChars(10);
                                                MOMENTALENRED = TRETRED;
                                                break;
                                            } else if(pritisnatoKopce == 0x0D) { // UP
                                                voNovRed = 1;
                                                lcd_goto(0x14);
                                                lcd_clearChars(10);
                                                MOMENTALENRED = VTORRED;
                                                break;
                                            } else if(pritisnatoKopce == 0x0E) { // BACKSPACE
                                                break;
                                            } else if(pritisnatoKopce == 0x0A) { // COMMA
                                                break;
                                            } else { // Значи е внесена бројка !
                                                if(pozicijaNaKarakter <= 2) {
                                                    lcd_putc(pritisnatoKopce + 48);
                                                    brojParcinja = brojParcinja*10 + pritisnatoKopce;
                                                    pozicijaNaKarakter++;
                                                }
                                            }
                                        }
                                    }
                                    break;
                            default:
                                    break;
                        }
                        break;
            case POSTAVUVANJE:
                        printf("Dolzina : %u\nKod : %d\nBroj na parcinja : %d\n", dolzina, kodPili, brojParcinja);
                        break;

            case SECENJE:
                        break;

            case EXTRA:
                        break;

            default:
                        break;
        }

    }


}
