## Projeto 1: Forno
**Disciplina:** SEL0433 - Aplicação de Microprocessadores  
**Instituição:** Escola de Engenharia de São Carlos (EESC-USP)  
**Alunos:** 
Luís Carlos Delgado Torrecilha - 15472530
Beatriz Fonseca Silva - 15653959
## 1. Descrição do Projeto
Este projeto tem como objetivo fornecer ao aluno conhecimento suficiente para
compreender a interação entre os elementos e circuitos internos dos microcontroladores da
família PIC18. Criada em 2000 pela fabricante Microchip, esta família de mais de 100
modelos é uma ótima escolha para a continuidade do estudo de microcontroladores. Enquanto
o 8051 possui uma arquitetura mais antiga, memória e instruções Assembly, o PIC18 oferece
recursos mais modernos, como conversor analógico-digital de melhor resolução, múltiplos
temporizadores, comunicação USB nativa, maior quantidade de memória e sistema de
interrupções mais robusto. Além disso, o PIC18 possui maior velocidade de processamento
devido à arquitetura RISC e à execução mais eficiente das instruções, tornando-o mais
adequado para aplicações embarcadas modernas com múltiplos periféricos simultâneos.
Ademais, o estudo do PIC18 facilitará o estudo de microcontroladores de 32 bits, como
STM32 e ESP32.
## 2. Requisitos do projeto
Você foi contratado por uma indústria metalúrgica para projetar um dispositivo de
aferição de temperatura e tempo de um forno industrial. Os fornos industriais são amplamente
utilizados em processos de fabricação metálica, pintura eletrostática e manipulação de
químicos. Para este projeto, deseja-se monitorar a cada intervalo de tempo a temperatura
interna de um forno industrial. Você deverá utilizar o PIC18F4550 e todos os periféricos
necessários para que o equipamento funcione, com implementação por meio do software
SimulIDE e tendo como base o Kit EasyPIC v7.
O funcionamento do seu produto consiste em medir a temperatura interna do forno
durante um tempo predeterminado pelo usuário. A medição de temperatura deverá ser feita
por meio do sensor LM35 e exibida em um display LCD continuamente, respeitando a faixa
de 0 °C a 100 °C. Já a seleção do tempo deverá ser feita por meio de um botão, em que seu
estado representa uma aferição de curta duração ou uma aferição de longa duração. A
contagem do tempo também deverá ser exibida regressivamente no display LCD. Por fim, um
segundo botão deverá ser utilizado para iniciar o processo de contagem regressiva e medição
de temperatura

## 3. Estrutura de Arquivos
- `Checkpoint1.c`: Código fonte em linguagem C do checkpoint 1.
- `Checkpoint2.c`: Código fonte em linguagem C do checkpoint 2.
- `EFINAL.C`: Código fonte em linguagem C do Projeto Finalizado.
- `SEL0433_projeto_2.pdf`: Roteiro com as especificações técnicas seguidas.

## 4. Ferramentas Utilizadas
- **SimulIDE** Para validação da lógica de hardware e periféricos.
- **Linguagem:** C.
- **MikroC PRO for PIC** Para criação do código e gerador do .hex

### Execução no SimulIDE
![Imagem do circuito e LCD a funcionar no SimulIDE](Simu.jpg)
