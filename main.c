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

    char niza[6];

    // Globalni promenlivi
    unsigned char stegnato = 0; // Дали ги имаме активирано стегите или не
    unsigned char secenje = 0;
    unsigned char koiPili = 0; // Бинарно - поседните два бита 11 = двете пили сечат, 10 = левата пила сече, 01 = десната пила сече
    unsigned char lastStateVovleceniPili = 1; // TODO треба да се повторно провери при старт !!!

    unsigned char tmpNewStateVovleceniPili;
    unsigned char secEnd = 0; // Укажува на завршено _ЕДНО_ сечење - извлекување и целосно вовлекување на пилите. (еден процес) ... -----_________---- ...

    unsigned int iseceniParcinja = 0;

    unsigned char novaSostojba = 1;

    unsigned int tmpInputs = 0;

    // INICIJALIZACIJA NA PORTI, DISPLEJ, TASTATURA...
    init_uart();
    lcd_init();
    lcd_clrscr();
    initKeypadPins();
    initEncoder();
    initOutputs();
    initInputs();
    initADC();
    initPWM();

    // INICIJALIZACIJA NA DIFOLT VREDNOSTI NA DISPLEJ

    // Број на парчиња
    lcd_goto(0x1F);
    lcd_clearChars(6);
    lcd_goto(0x1F);
    sprintf(niza, "%u", iseceniParcinja);
    lcd_puts(niza);

    // Исчитај енкодер
    PCINT0_vect();
    lcd_goto(0x0B);
    lcd_clearChars(6);
    lcd_goto(0x0B);
    sprintf(niza, "%lu", angularPosition);
    lcd_puts(niza);
    // END INICIJALIZACIJA

    printf("Zdravo!\n");
    //PCINT0_vect();
    // Привремен код за тестирање
    //cli();
/*
    // Тестирање на излезите !
    while(1) {
        for(unsigned char i=0; i<8; i++) {
            printf("Palam : %d\n", i);
            setOutput(i, 1);
            printf("Sostojba : %d\n\n", getOutput());

            _delay_ms(500);

            printf("Gasam : %d\n", i);
            setOutput(i, 0);
            printf("Sostojba : %d\n\n", getOutput());

            _delay_ms(1000);
        }
    }
*/


    // Тестирање на PWM

    // Ќе вртиме само во една насока. За таа цел L1 треба да е високо, L2 да е ниско, заедно со H2. PWM оди на H1

    PORTE |= (1 << PE5);

    unsigned int tmp;
    while(1) {
        tmp = getrawADC(0);
        if(tmp > 1000)
            tmp = 1000;
        OCR3A = tmp;    // pin 5 на ардуино
        //_delay_ms(300);
    }

/*
    while(1) {
        printf("ADC vrednosti : ADC0 = %d\n", getrawADC(0));
        _delay_ms(500);
    }

    sei();



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

*/


    // PROVERKA ZA BEZBEDNOST + START & STOP
    // Услови за фатална грешка - при фатална грешка, машината се гаси целосно. Порака на екран : што е грешката
    // 1. Активни два крајни (за движење) - истовремено
    // 2. Ако не се активни крајните на пилите - пробај да се вовлечат пилите - со активирање на вентилите. Timeout = 2..5 секунди. Ако во ова време не се вратат пилите - фатална грешка.
    // 3. Ако нема притисок / има поголем притисок од притисочните сензори - фатална грешка // TODO ова неможе вака !
    // END PROVERKA

    // POSTAVUVANJE NA MASINATA NA 0 - DVIZENJE NA MOTOROT NA NAJMALA DOLZINA
    //
    // 1. Читање на моментална позиција,
    // 2. Ако е различно од нулта - придвижија машината кон нулта
    // 3. Читај енкодер + тахо + гранични + безбедносни
    //
    // END POSTAVUVANJE

    /* Keyboard layout :
     *
        +-----+-----+-----+
        |     |     |     |
        |  7  |  8  |  9  |
        |     |     |     |
        +-----------------+
        |     |     |     |
        |  4  |  5  |  6  |
        |     |     |     |
        +-----------------+
        |     |     |     |
        |  1  |  2  |  3  |
        |     |     |     |
        +-----------------+
        |     |     |     |
        | N/A |  0  |  .  |
        |     |     |     |
        +-----------------+
        |     |     |     |
        | CLR | U P |ENTER|
        |     |     |     |
        +-----+-----+-----+
    *
    */


    // Заштита.
    // /Заштита

    // Дифолтни состојби на почетокот
    STATE = VNES;
    MOMENTALENRED = PRVRED;

    while(1) { // Glaven loop

        // Читање на сите влезови при старт !
        inputs = getInputs();
        if(tmpInputs != inputs) {
            printf("Nova sostojba na vlez : %u\n", inputs);
            tmpInputs = inputs;
        }


        // PROVERKA ZA BEZBEDNOST + START & STOP

        // END PROVERKA


        // Глобално :
        // Ако се притисне STOP копчето, машината се „РЕСЕТИРА“ и состојбата се враќа на VNES и моменталниот ред е PRVRED
        //printf("inputs & (1 << STOPBTN) = %u\n", inputs & (1 << STOPBTN));
        unsigned char asdTMP = inputs & (1 << STOPBTN);
        if(asdTMP) {
            STATE = VNES;
            MOMENTALENRED = PRVRED;
            printf("STOP !\n");
            // Тука треба и без разлика каде сме биле СВЕ да се ресетира - променливи од втора и од трета состојба !
        }

        switch(STATE) {
            case VNES:
                        if(novaSostojba) {
                            novaSostojba = 0;
                            MOMENTALENRED = PRVRED;
                        }

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
                                                    } else if((pozicijaNaKarakter <= 3) && (pozicijaNaKarakter > 0)) {
                                                        lcd_putc(pritisnatoKopce + 48);
                                                        pozicijaNaKarakter++;
                                                        dolzina = dolzina*10 + pritisnatoKopce;
                                                    } else if ((pozicijaNaKarakter == 0) && (pritisnatoKopce != 0)) {
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

                                        vnesenaPoslednaCifra = 0;
                                        cekajPoslednaCifra = 0;

                                    } else {
                                        pritisnatoKopce = scanKeypad();

                                        if(pritisnatoKopce != 0xFF) {

                                            if(pritisnatoKopce == 0x0C) { // ЕНТЕР
                                                if((kodPili > 0) && (kodPili < 13)) {
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
                                            } else if(pritisnatoKopce == 0x0B) { // CLEAR -> Goto VTORRED со voNovRed = 1;
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

                                                if(!vnesenaPoslednaCifra) {
                                                    if(cekajPoslednaCifra) {
                                                        if(pritisnatoKopce <= 2) {
                                                            lcd_putc(pritisnatoKopce + 48);
                                                            kodPili = kodPili*10 + pritisnatoKopce;
                                                            vnesenaPoslednaCifra = 1;
                                                        }
                                                    } else if(pritisnatoKopce > 1) {
                                                        lcd_putc(pritisnatoKopce + 48);
                                                        vnesenaPoslednaCifra = 1;
                                                        kodPili = pritisnatoKopce;
                                                    } else if(pritisnatoKopce != 0) {
                                                        lcd_putc(pritisnatoKopce + 48);
                                                        cekajPoslednaCifra = 1;
                                                        kodPili = pritisnatoKopce;
                                                    }
                                                }




                                                //if((pritisnatoKopce != 9) && (pritisnatoKopce != 0) && !kodPili) {
                                                //    lcd_putc(pritisnatoKopce + 48);
                                                //    kodPili = pritisnatoKopce;
                                                //}
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
                                        lcd_putc(0x10); // SPACE ?
                                        voNovRed = 0;

                                        pozicijaNaKarakter = 0;
                                        cekajPoslednaCifra = 0;
                                        brojParcinja = 0;
                                        vnesenaPoslednaCifra = 0;
                                    } else {

                                        pritisnatoKopce = scanKeypad();

                                        if(pritisnatoKopce != 0xFF) {
                                            // Прво исхендлај ги специјалните копчиња (ентер, BS, стрелка нагоре, clear)
                                            //
                                            if(pritisnatoKopce == 0x0C) { // ЕНТЕР
                                                if((brojParcinja <= 999) && pozicijaNaKarakter > 0) {
                                                    lcd_printStatus("PRITISNI START!");
                                                    voNovRed = 1;
                                                    lcd_goto(0x14);
                                                    lcd_clearChars(1);

                                                    MOMENTALENRED = PRVRED;

                                                    //STATE = POSTAVUVANJE; TODO
                                                    STATE = SECENJE;

                                                    novaSostojba = 1;
                                                    printf("Dolzina : %u\nKod : %d\nBroj na parcinja : %d\n", dolzina, kodPili, brojParcinja);
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
                                                if(!vnesenaPoslednaCifra) {
                                                    if(pozicijaNaKarakter <= 2) {
                                                        lcd_putc(pritisnatoKopce + 48);
                                                        brojParcinja = brojParcinja*10 + pritisnatoKopce;

                                                        if((pozicijaNaKarakter == 0) && (pritisnatoKopce == 0)) {
                                                            vnesenaPoslednaCifra = 1;
                                                        }

                                                        pozicijaNaKarakter++;
                                                    }
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
                        if(novaSostojba) {
                            novaSostojba = 0;
                        }
                        //printf("Dolzina : %u\nKod : %d\nBroj na parcinja : %d\n", dolzina, kodPili, brojParcinja);



                        break;

            case SECENJE:
                        if(novaSostojba) {
                            novaSostojba = 0;
                            iseceniParcinja = 0;
                            secenje = 0;
                            stegnato = 0;
                            koiPili = 0;
                            tmpNewStateVovleceniPili = 0;
                            lcd_printStatus("SECHENJE !");
                        }

                        if(secenje && !secEnd) {
                            printf("koiPili = %d !\n", koiPili);
                            switch(koiPili) {
                                case 1 : // Гледаме десна пила
                                        tmpNewStateVovleceniPili = inputs & (1 << RSAWEND);
                                        if( (lastStateVovleceniPili == 0) && (tmpNewStateVovleceniPili == (1 << RSAWEND)) ) {
                                            printf("1\n");
                                            secEnd = 1;
                                        }
                                        lastStateVovleceniPili = tmpNewStateVovleceniPili;
                                        break;
                                case 2 : // Гледаме лева пила
                                        tmpNewStateVovleceniPili = inputs & (unsigned int)(1 << LSAWEND);
                                        printf("tmpNewStateVovleceniPili = %u\n", tmpNewStateVovleceniPili);
                                        if( (lastStateVovleceniPili == 0) && (tmpNewStateVovleceniPili == (1 << LSAWEND)) ) {
                                            printf("2\n");
                                            secEnd = 1;
                                        }
                                        lastStateVovleceniPili = tmpNewStateVovleceniPili;
                                        break;
                                case 3 : // Гледаме било која пила (во овој случај само десната)
                                        tmpNewStateVovleceniPili = inputs & (1 << RSAWEND);
                                        if( (lastStateVovleceniPili == 0) && (tmpNewStateVovleceniPili == (1 << RSAWEND)) ) {
                                            printf("3\n");
                                            secEnd = 1;
                                        }
                                        lastStateVovleceniPili = tmpNewStateVovleceniPili;
                                        break;
                                default :
                                        break;
                            }
                        } else if( !secenje && ((inputs & (1 << LBTN)) || (inputs & (1 << RBTN))) ) { // Тука проверуваме кој мотор е активен и ја овозможуваме соодветната пила да може да излегува

                            if(!stegnato) {
                                stegniStegi(kodPili);
                                stegnato = 1;
                                _delay_ms(1000); // Timeout за стегање на пилите
                            }

                            if( (inputs & (1 << LMOTOR)) ) {
                                setOutput(LSAW, 1);
                                koiPili = (koiPili | 0x02) & 0x0003;
                            }

                            if( (inputs & (1 << RMOTOR)) ) {
                                setOutput(RSAW, 1);
                                koiPili = (koiPili | 0x01) & 0x0003;
                            }
                            if((inputs & (1 << LBTN)) && (inputs & (1 << RBTN)) ) {
                                    secenje = 1;
                                    printf("SECENJE !\n");
                            }

                        } else if (secEnd) { // Сме завршиле еден процес на сечење.
                            iseceniParcinja++;

                            lcd_goto(0x1F);
                            lcd_clearChars(6);
                            lcd_goto(0x1F);
                            sprintf(niza, "%u", iseceniParcinja);
                            lcd_puts(niza);

                            otpustiStegi(); // Пушти стеги !
                            _delay_ms(1000);


                            stegnato = 0;
                            secEnd = 0;
                            secenje = 0;

                            if(iseceniParcinja == brojParcinja) { // Сме во мод за сечење на парчиња
                                // 1. Испиши на дисплеј "SECENJE ZAVRSHENO"
                                // 2. Подеси состојба EXTRA во која што ќе се чека кликнување на копчето STARTBTN за почеток на нов процес на сечење

                                secenje = 0;

                                lcd_printStatus(" SECENJE ZAVRSHENO");

                                STATE = EXTRA;

                                novaSostojba = 1;

                                break;
                            }
                        }

                        break;

            case EXTRA:
                        if(novaSostojba) {
                            novaSostojba = 0;
                        }

                        if(inputs & (1 << STARTBTN)) { // Започнува нов процес на сечење
                            novaSostojba = 1;
                            STATE = VNES;
                            break;
                        }

                        break;

            default:
                        break;
        }

    }


}
