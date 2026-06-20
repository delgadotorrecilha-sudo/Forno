// Luís Carlos Delgado Torrecilha - 15472530
// Beatriz Fonseca Silva - 15653959

// configurando os pinos do display lcd
// aqui a gente liga cada pino do lcd em uma porta do microcontrolador
sbit lcd_rs at latb4_bit;
sbit lcd_en at latb5_bit;
sbit lcd_d4 at latb0_bit;
sbit lcd_d5 at latb1_bit;
sbit lcd_d6 at latb2_bit;
sbit lcd_d7 at latb3_bit;

// aqui a gente avisa pro chip se esses pinos vão ser entrada ou saída
sbit lcd_rs_direction at trisb4_bit;
sbit lcd_en_direction at trisb5_bit;
sbit lcd_d4_direction at trisb0_bit;
sbit lcd_d5_direction at trisb1_bit;
sbit lcd_d6_direction at trisb2_bit;
sbit lcd_d7_direction at trisb3_bit;

void main() {
    // essas variáveis vão controlar a lógica do nosso botão e o número da tela
    unsigned char contador = 0;      // começa no zero e vai até o nove
    unsigned char flag_bouncing = 0; // marca se rolou aquele mau contato mecânico do botão
    unsigned char estado_botao = 0;  // guarda se a gente tá esperando apertar ou soltar o botão

    // desliga as leituras analógicas, transformando tudo em digital
    adcon1 = 0x0f;

    // o registrador tris define a direção. colocando 1 no rd0, ele vira uma entrada
    trisd0_bit = 1;
    // zera tudo na porta d pra garantir que não vai ter lixo de memória atrapalhando
    portd = 0;

    // liga o display lcd, limpa a tela de qualquer sujeira e desliga o cursor piscando
    Lcd_Init();
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Cmd(_LCD_CURSOR_OFF);

    // escreve a mensagem inicial lá na primeira linha, primeira coluna
    Lcd_Out(1, 1, "hellowrld");

    // bota o número inicial (zero) na segunda linha.
    // a gente soma com '0' pra transformar o número solto num caractere de texto que o lcd entenda
    Lcd_Chr(2, 1, contador + '0');

    // loop principal: o cérebro do programa fica rodando aqui pra sempre
    while(1) {

        // essa máquina de estados (switch) organiza o que o programa deve fazer com o botão
        switch(estado_botao) {

            // estado 0: o programa tá aqui esperando alguém apertar
            case 0:
                // verificou que o botão foi apertado (nível lógico 1)?
                if (portd.rd0 == 1) {

                    // dá uma pequena pausa de 20ms pra ignorar o "bounce"
                    // (aquela trepidação rápida das molas de dentro do botão)
                    delay_ms(20);

                    // depois da pausa, o botão ainda tá apertado? se sim, foi um aperto real
                    if (portd.rd0 == 1) {
                        flag_bouncing = 1; // aciona a flag pra dizer que tá lendo

                        // aumenta o nosso número
                        contador++;

                        // se passar do nove, zera pra caber tudo num dígito só
                        if (contador > 9) {
                            contador = 0;
                        }

                        // atualiza o número novo lá na tela do display
                        lcd_chr(2, 1, contador + '0');

                        // joga o estado pro número 1. agora o programa para de somar
                        // e vai só esperar a pessoa tirar o dedo de lá
                        estado_botao = 1;
                    }
                }
                break;

            // estado 1: o programa tá só aguardando o botão ser solto
            case 1:
                // verificou que o botão foi solto (nível lógico 0)?
                if (portd.rd0 == 0) {

                    // espera 20ms de novo pra evitar falso contato na hora de soltar
                    delay_ms(20);

                    // soltou mesmo?
                    if (portd.rd0 == 0) {
                        flag_bouncing = 0; // abaixa a flag
                        estado_botao = 0;  // volta pro estado inicial pra aguardar o próximo aperto
                    }
                }
                break;
        }
    }
}