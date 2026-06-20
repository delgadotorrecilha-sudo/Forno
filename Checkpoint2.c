// Luís Carlos Delgado Torrecilha - 15472530
// Beatriz Fonseca Silva - 15653959

// mapeamento do lcd movido para o portd
// mudamos pra liberar o portb para as interrupções dos botões!
sbit LCD_RS at LATD4_bit;
sbit LCD_EN at LATD5_bit;
sbit LCD_D4 at LATD0_bit;
sbit LCD_D5 at LATD1_bit;
sbit LCD_D6 at LATD2_bit;
sbit LCD_D7 at LATD3_bit;

sbit LCD_RS_Direction at TRISD4_bit;
sbit LCD_EN_Direction at TRISD5_bit;
sbit LCD_D4_Direction at TRISD0_bit;
sbit LCD_D5_Direction at TRISD1_bit;
sbit LCD_D6_Direction at TRISD2_bit;
sbit LCD_D7_Direction at TRISD3_bit;

// variáveis globais (usamos volatile porque elas mudam dentro das interrupções)
volatile unsigned char segundos = 0;
volatile unsigned char modo_ativo = 0; // 0 = parado, 1 = longa, 2 = curta
volatile unsigned char ticks_tmr1 = 0; // conta as frações de 250ms
volatile unsigned char atualizar_tela = 0; // flag pra avisar o main que algo mudou

void Interrupt() {

    // interrupção do botão 1 (longa duração - 60s) pino RB0
    if (INT0IF_bit) {
        Delay_ms(20); // debounce rápido de segurança
        if (PORTB.RB0 == 1) { // confere se foi um aperto real (borda de subida)
            modo_ativo = 1;
            segundos = 60;
            atualizar_tela = 1;

            // liga o timer 0 e garante que o timer 1 tá desligado
            T0CON.TMR0ON = 1;
            T1CON.TMR1ON = 0;
        }
        INT0IF_bit = 0; // abaixa a bandeira da interrupção
    }

    // interrupção do botão 2 (curta duração - 10s) pino RB1
    if (INT1IF_bit) {
        Delay_ms(20);
        if (PORTB.RB1 == 1) {
            modo_ativo = 2;
            segundos = 10;
            ticks_tmr1 = 0; // zera as frações de tempo
            atualizar_tela = 1;

            // liga o timer 1 e garante que o timer 0 tá desligado
            T1CON.TMR1ON = 1;
            T0CON.TMR0ON = 0;
        }
        INT1IF_bit = 0;
    }

    // interrupção do timer 0 (conta 1 segundo exato)
    if (TMR0IF_bit) {
        TMR0IF_bit = 0;
        TMR0H = 0xC2; // recarrega pra contar mais 1s
        TMR0L = 0xF7;

        if (segundos > 0) {
            segundos--;
            atualizar_tela = 1;
        } else {
            T0CON.TMR0ON = 0; // chegou no zero, desliga o cronômetro
        }
    }

    // interrupção do timer 1 (conta frações de 250ms)
    if (TMR1IF_bit) {
        TMR1IF_bit = 0;
        TMR1H = 0x0B; // recarrega pra contar mais 250ms
        TMR1L = 0xDC;

        ticks_tmr1++; // soma 1 na nossa fração

        // se bateu 4 frações de 250ms, passou 1 segundo inteiro
        if (ticks_tmr1 >= 4) {
            ticks_tmr1 = 0;

            if (segundos > 0) {
                segundos--;
                atualizar_tela = 1;
            } else {
                T1CON.TMR1ON = 0; // zerou o tempo total, desliga tudo
            }
        }
    }
}

void main() {
    // transforma tudo em porta digital pura
    ADCON1 = 0x0F;

    // bota os pinos de interrupção como entrada pra ler os botões
    TRISB0_bit = 1;
    TRISB1_bit = 1;

    // liga e limpa o display
    Lcd_Init();
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Cmd(_LCD_CURSOR_OFF);
    Lcd_Out(1, 1, "");

    // configura o timer 0 (1s) -> 16 bits, prescaler 1:128
    T0CON = 0x86; // 1000 0110 em binário
    TMR0H = 0xC2;
    TMR0L = 0xF7;
    T0CON.TMR0ON = 0; // deixa desligado até o botão mandar ligar

    // configura o timer 1 (250ms) -> 16 bits, prescaler 1:8
    T1CON = 0x30; // 0011 0000 em binário (prescaler 1:8)
    TMR1H = 0x0B;
    TMR1L = 0xDC;
    T1CON.TMR1ON = 0;

    // configura como os botões vão ser lidos
    INTCON2.INTEDG0 = 1; // aciona o int0 só na borda de subida (aperto)
    INTCON2.INTEDG1 = 1; // aciona o int1 só na borda de subida (aperto)

    // liga os disjuntores de interrupção um por um
    INT0IE_bit = 1; // libera interrupção do botao 1
    INT1IE_bit = 1; // libera interrupção do botao 2
    TMR0IE_bit = 1; // libera interrupção do timer 0
    TMR1IE_bit = 1; // libera interrupção do timer 1

    // liga os disjuntores gerais da casa
    PEIE_bit = 1;
    GIE_bit = 1;

    // nosso main loop agora fica praticamente vazio e leve
    while(1) {

        // ele só trabalha de verdade se alguma interrupção mexer na flag
        if (atualizar_tela) {
            atualizar_tela = 0; // reseta a flag

            Lcd_Cmd(_LCD_CLEAR);

            if (modo_ativo == 1) {
                Lcd_Out(1, 1, "Longa - 60s");
            } else if (modo_ativo == 2) {
                Lcd_Out(1, 1, "Curta - 10s");
            }

            // formata os digitos pra tela sem usar biblioteca float de matemática
            // pega a dezena (/) e pega a unidade (%)
            Lcd_Chr(2, 1, (segundos / 10) + '0');
            Lcd_Chr(2, 2, (segundos % 10) + '0');
            Lcd_Out(2, 3, "seg");
        }
    }
}