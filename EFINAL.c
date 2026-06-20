// Luís Carlos Delgado Torrecilha - 15472530
// Beatriz Fonseca Silva - 15653959

// mapeamento do lcd movido para o portd
// mudamos pra liberar o portb para as interrupções dos botões!
// aqui a gente liga cada pino do lcd em uma porta do microcontrolador
sbit LCD_RS at LATD4_bit;
sbit LCD_EN at LATD5_bit;
sbit LCD_D4 at LATD0_bit;
sbit LCD_D5 at LATD1_bit;
sbit LCD_D6 at LATD2_bit;
sbit LCD_D7 at LATD3_bit;

// aqui a gente avisa pro chip se esses pinos vão ser entrada ou saída
sbit LCD_RS_Direction at TRISD4_bit;
sbit LCD_EN_Direction at TRISD5_bit;
sbit LCD_D4_Direction at TRISD0_bit;
sbit LCD_D5_Direction at TRISD1_bit;
sbit LCD_D6_Direction at TRISD2_bit;
sbit LCD_D7_Direction at TRISD3_bit;

// pino do led que finge ser a resistência do forno
sbit LED_FORNO at LATC0_bit;
sbit LED_FORNO_Direction at TRISC0_bit;

// definindo os estados da nossa máquina pra ficar mais fácil de ler
#define ESTADO_ESCOLHA 0
#define ESTADO_RODANDO 1
#define ESTADO_FIM     2

// variáveis globais (usamos volatile porque elas mudam dentro das interrupções)
volatile unsigned char estado_atual = ESTADO_ESCOLHA;
volatile unsigned char segundos = 0;
volatile unsigned char tempo_selecionado = 0; // guarda a escolha: 60 ou 15
volatile unsigned char ticks_tmr1 = 0; // conta as frações do timer
volatile unsigned char atualizar_tela = 1; // flag pra avisar o main que a tela mudou
volatile unsigned char sistema_pronto = 0; // trava pra segurar os fantasmas do simulide

void Interrupt() {
    // interrupção externa int0 (rb0) - botão do start
    if (INTCON.INT0IF) {
        // blindagem: o pic só aceita se o pino start tiver com 5v físicos e os outros zerados
        if (sistema_pronto && PORTB.F0 == 1 && PORTB.F1 == 0 && PORTB.F2 == 0) {
            // só deixa começar se a gente tiver no menu e já tiver escolhido um tempo
            if (estado_atual == ESTADO_ESCOLHA && tempo_selecionado > 0) {
                segundos = tempo_selecionado;
                estado_atual = ESTADO_RODANDO;
                T1CON.TMR1ON = 1; // liga o timer1
                T0CON.TMR0ON = 1; // liga o timer0
                atualizar_tela = 1;
            }
        }
        INTCON.INT0IF = 0; // limpa a flag pra matar o ruído fantasma
    }

    // interrupção externa int1 (rb1) - botão de 60 segundos
    if (INTCON3.INT1IF) {
        // blindagem pra garantir que a gente só apertou ele
        if (sistema_pronto && PORTB.F1 == 1 && PORTB.F0 == 0 && PORTB.F2 == 0) {
            if (estado_atual == ESTADO_ESCOLHA) {
                tempo_selecionado = 60;
                atualizar_tela = 1;
            }
        }
        INTCON3.INT1IF = 0;
    }

    // interrupção externa int2 (rb2) - botão de 15 segundos
    if (INTCON3.INT2IF) {
        // blindagem pra garantir que a gente só apertou ele
        if (sistema_pronto && PORTB.F2 == 1 && PORTB.F0 == 0 && PORTB.F1 == 0) {
            if (estado_atual == ESTADO_ESCOLHA) {
                tempo_selecionado = 15;
                atualizar_tela = 1;
            }
        }
        INTCON3.INT2IF = 0;
    }

    // interrupção do timer1 (vai abater os nossos segundos)
    if (PIR1.TMR1IF) {
        TMR1H = 0x0B;
        TMR1L = 0xDC;
        ticks_tmr1++;

        if (ticks_tmr1 >= 4) { // deu 1 segundo real
            ticks_tmr1 = 0;

            if (estado_atual == ESTADO_RODANDO) {
                if (segundos > 0) {
                    segundos--;
                    atualizar_tela = 1;
                }
                if (segundos == 0) {
                    T1CON.TMR1ON = 0;  // desliga o timer
                    LED_FORNO = 0;     // desliga a resistência por segurança
                    estado_atual = ESTADO_FIM; // joga a gente pra tela de fim
                    atualizar_tela = 1;
                }
            }
        }
        PIR1.TMR1IF = 0;
    }
}

// função pra calcular e mostrar a temperatura sem usar float (pra não torrar memória)
void exibe_temperatura(unsigned int adc_val) {
    char temp_str[9];
    unsigned long temp_x10;
    unsigned char dezenas, unidades, decimais;

    // continha pra transformar o valor do adc na temperatura real
    temp_x10 = ((unsigned long)adc_val * 1000) / 1023;

    // se bater 100 graus ou mais, a gente trava no 100.0 pra não bugar a formatação
    if (temp_x10 >= 1000) {
        temp_str[0] = '1'; temp_str[1] = '0'; temp_str[2] = '0'; temp_str[3] = '.';
        temp_str[4] = '0'; temp_str[5] = ' '; temp_str[6] = 223; temp_str[7] = 'C'; temp_str[8] = '\0';
    } else {
        // pega as dezenas, unidades e os decimais separadinhos
        dezenas = (temp_x10 / 100) % 10;
        unidades = (temp_x10 / 10) % 10;
        decimais = temp_x10 % 10;

        temp_str[0] = ' ';
        temp_str[1] = (dezenas > 0) ? (dezenas + '0') : ' '; // esconde o zero à esquerda
        temp_str[2] = unidades + '0';
        temp_str[3] = '.';
        temp_str[4] = decimais + '0';
        temp_str[5] = ' ';
        temp_str[6] = 223; // o 223 é o símbolo de grau '°' no lcd
        temp_str[7] = 'C';
        temp_str[8] = '\0';
    }

    Lcd_Out(2, 1, "Temp:");
    Lcd_Out(2, 7, temp_str);

    // acende o led do forno se a temperatura passar de 50 graus
    if ((temp_x10 / 10) > 50) {
        LED_FORNO = 1;
    } else {
        LED_FORNO = 0;
    }
}

void main() {
    unsigned int leitura_adc = 0;
    char tempo_str[7];

    // força a limpeza de tudo pra não puxar lixo de memória de simulações antigas
    estado_atual = ESTADO_ESCOLHA;
    segundos = 0;
    tempo_selecionado = 0;
    ticks_tmr1 = 0;
    atualizar_tela = 1;
    sistema_pronto = 0;

    // avisando quem é entrada (1) e quem é saída (0)
    TRISB.RB0 = 1; // botão start
    TRISB.RB1 = 1; // botão 60s
    TRISB.RB2 = 1; // botão 15s

    TRISA.RA0 = 1;
    TRISA.RA3 = 1;
    LED_FORNO_Direction = 0;
    LED_FORNO = 0;

    // adcon1: vref+ no an3, vref- no vss, e os pinos an0 até an4 como analógicos
    ADCON1 = 0b00011011;
    ADCON2 = 0b10001010; // joga o resultado pra direita
    ADCON0 = 0b00000001; // liga o módulo adc

    Lcd_Init();
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Cmd(_LCD_CURSOR_OFF);

    // dá aquele tempinho pro simulador eletrônico dar uma estabilizada
    Delay_ms(1000);

    // configura as interrupções pra disparar quando o sinal subir (borda de subida)
    INTCON2.INTEDG0 = 1;
    INTCON2.INTEDG1 = 1;
    INTCON2.INTEDG2 = 1;

    // limpa as flags pra garantir
    INTCON.INT0IF = 0;
    INTCON3.INT1IF = 0;
    INTCON3.INT2IF = 0;

    // liga a chave geral das interrupções
    INTCON.GIE = 1;
    INTCON.PEIE = 1;
    INTCON.INT0IE = 1;
    INTCON3.INT1IE = 1;
    INTCON3.INT2IE = 1;

    T1CON = 0x30;
    PIE1.TMR1IE = 1;
    T0CON = 0x07;

    // agora sim! tudo pronto, libera a trava pra a gente poder usar os botões
    sistema_pronto = 1;

    // loop infinito
    while(1) {
        // se a flag atualizar_tela for 1, a gente muda o que tá escrito
        if (atualizar_tela) {
            Lcd_Cmd(_LCD_CLEAR);

            if (estado_atual == ESTADO_FIM) {
                // avisa que terminou e segura 2 segundinhos pra dar tempo da pessoa ler
                Lcd_Out(1, 1, "PROCESSO FIM!");
                Delay_ms(2000);

                // auto-reset: zera tudo e volta pro menu sozinho
                estado_atual = ESTADO_ESCOLHA;
                tempo_selecionado = 0;
                atualizar_tela = 1;
                continue; // pula o resto e volta pro começo do loop pra redesenhar o menu

            } else if (estado_atual == ESTADO_ESCOLHA) {
                Lcd_Out(1, 1, "Selecione tempo:");

                if (tempo_selecionado == 60) {
                    Lcd_Out(2, 1, "> 60 Segundos");
                } else if (tempo_selecionado == 15) {
                    Lcd_Out(2, 1, "> 15 Segundos");
                } else {
                    Lcd_Out(2, 1, "Nenhum (0s)");
                }
            } else if (estado_atual == ESTADO_RODANDO) {
                // mostra a contagem
                tempo_str[0] = 'T'; tempo_str[1] = 'm'; tempo_str[2] = 'p'; tempo_str[3] = ':';
                tempo_str[4] = (segundos / 10) + '0';
                tempo_str[5] = (segundos % 10) + '0';
                tempo_str[6] = '\0';

                Lcd_Out(1, 1, tempo_str);
            }

            atualizar_tela = 0;
        }

        // se a máquina tiver rodando, a gente avisa o adc pra ler o potenciômetro
        if (estado_atual == ESTADO_RODANDO) {
            ADCON0.GO = 1; // manda a conversão rodar
            while(ADCON0.GO_NOT_DONE); // espera ficar pronto
            leitura_adc = (ADRESH << 8) + ADRESL; // junta as duas partes do resultado

            exibe_temperatura(leitura_adc);
            Delay_ms(200);
        }
    }
}