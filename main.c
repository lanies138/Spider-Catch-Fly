#include <msp430x14x.h>
#include "lcd.h"
#include "portyLcd.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#define BUTTON1 (P4IN & BIT4)
#define BUTTON2 (P4IN & BIT5)
#define BUTTON4 (P4IN & BIT7)

char screen[2][32];
int jump;
int points;
int time1;
int lifes;

bool gameEnded = false;
bool showedEndScreen = false;

bool playerPosFlag = false;
bool titleScreenFlag = false;

bool buttonPressed = false;
bool buttonPressed4 = false;

void initClock();

void initButtons() {
    P4DIR &= ~(BIT4 | BIT5 | BIT7);
}

void printText(char text[]) {
     for(int i = 0; i < strlen(text); i++){
          SEND_CHAR(text[i]);
     }
}

void initObjects(void) {
    SEND_CMD(CG_RAM_ADDR);
    unsigned char spider[]= {16, 9, 10, 30, 30, 10, 9, 16};
    unsigned char obj1[]= {0, 0, 6, 14, 6, 0, 0, 0};
    unsigned char obj2[]= {0, 0, 0, 6, 14, 6, 0, 0};
    unsigned char spidersilk[4][8] = {{1, 1, 19, 9, 5, 11, 9, 31},
                                      {31, 9, 11, 5, 9, 19, 1, 1},
                                      {1, 1, 25, 18, 20, 26, 18, 31},
                                      {31, 18, 26, 20, 18, 25, 1, 1}};
    for(int i=0; i<8; i++) {
        SEND_CHAR(spider[i]);
    }
    for(int i=0; i<8; i++) {
        SEND_CHAR(obj1[i]);
    }
    for(int i=0; i<8; i++) {
        SEND_CHAR(obj2[i]);
    }
    for(int i=0; i<4; i++) {
        for(int j=0; j<8; j++) {
            SEND_CHAR(spidersilk[i][j]);
        }
    }
}

void initScreen() {
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < 17; j++) {
            screen[i][j] = ' ';
        }
    }
}

void initTitleScreen(){
    char firstRow[] = "CATCH THE FLY";
    char secondRow[] = "PRESS: 2";

    SEND_CMD(DD_RAM_ADDR);
    SEND_CHAR(11);
    SEND_CHAR(13);
    SEND_CHAR(' ');
    printText(firstRow);

    SEND_CMD(DD_RAM_ADDR2);
    SEND_CHAR(12);
    SEND_CHAR(14);
    SEND_CHAR(' ');
    printText(secondRow);

    while(!titleScreenFlag) {
        if(!BUTTON2) {
            titleScreenFlag = true;
        }
    }
}

void generateObject() {
    int choice = 0;
    choice = rand() % 20;
    switch(choice) {
        case 1:
            screen[0][16] = 9;
            screen[1][16] = ' ';
            break;
        case 2:
            screen[0][16] = 10;
            screen[1][16] = ' ';
            break;
        case 3:
            screen[0][16] = ' ';
            screen[1][16] = 9;
            break;
        case 4:
            screen[0][16] = ' ';
            screen[1][16] = 10;
            break;
        default:
            screen[0][16] = ' ';
            screen[1][16] = ' ';
            break;
    }
}

void moveDisplayRight() {
    generateObject();
    for(int i = 0; i < 16; i++) {
        screen[0][i] = screen[0][i+1];
        screen[1][i] = screen[1][i+1];
    }
}

void display() {
    clearDisplay();
    SEND_CMD(DD_RAM_ADDR);
    for(int i = 0; i < 16; i++) {
        if(screen[0][i] == 'x') {
            SEND_CHAR(8);
        }
        else {
            SEND_CHAR(screen[0][i]);
        }
    }
    SEND_CMD(DD_RAM_ADDR2);
    for(int i = 0; i < 16; i++) {
        if(screen[1][i] == 'x') {
            SEND_CHAR(8);
        }
        else {
            SEND_CHAR(screen[1][i]);
        }
    }
}

void startGame() {
    while(1) {
        if(!playerPosFlag) {
            jump = 0;
        }
        else {
            jump = 1;
        }
    }
}

void getNumber(unsigned int num) {
    char charVal;
    if(num < 10) {
        charVal = num + '0';
        SEND_CHAR(charVal);
    }
    else {
        getNumber(num / 10);
        charVal = (num%10) + '0';
        SEND_CHAR(charVal);
    }
}

void initEndScreen(unsigned int result) {
    clearDisplay();
    SEND_CMD(DD_RAM_ADDR);
    char resultLabel[] = "Your score: ";
    printText(resultLabel);
    getNumber(result);

    SEND_CMD(DD_RAM_ADDR2);
    char secondRow[] = "RESTART: 4";
    printText(secondRow);
}


void updateDiodes(int lifes) {
    switch(lifes) {
    case 2:
        P2OUT |= BIT1;
        break;
    case 1:
        P1OUT &= ~BIT5;
        break;
    case 0:
        P1OUT &= ~BIT6;
        gameEnded = true;
        break;
    }
}

void initDiodes() {
    P1DIR |= BIT5;
    P1DIR |= BIT6;
    P2DIR |= BIT1;
}

void lightUpDiodes() {
    P1OUT |= BIT5;
    P1OUT |= BIT6;
    P2OUT &= ~BIT1;
}

void initAllComponents(){
    clearDisplay();

    playerPosFlag = false;
    jump = 0;
    lifes = 3;
    points = 0;

    initScreen();
    lightUpDiodes();
}

int main() {
    WDTCTL=WDTPW+ WDTHOLD;
    InitPortsLcd();
    InitLCD();
    clearDisplay();
    initButtons();
    initClock();
    initObjects();
    initDiodes();

    srand(time(NULL));

    initTitleScreen();
    initAllComponents();
    startGame();
}

#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void) {
    if(!gameEnded && titleScreenFlag) {

      if(!BUTTON1 && !buttonPressed) {
          playerPosFlag = !playerPosFlag;
          buttonPressed = true;
      }
      else if(BUTTON1) {
          buttonPressed = false;
      }

      time1++;
      if(time1 % 10 == 0) {
          clearDisplay();
          moveDisplayRight();
          if(jump == 0) {
              if(screen[0][0] != ' ' && screen[1][0] == ' ') {
                  points++;
              }
              else if(screen[0][0] == ' ' && screen[1][0] != ' ') {
                  lifes--;
                  updateDiodes(lifes);
              }
              screen[0][0] = 'x';
              screen[1][0] = ' ';
          }
          if(jump == 1) {
              if(screen[1][0] != ' ' && screen[0][0] == ' ') {
                  points++;
              }
              else if(screen[1][0] == ' ' && screen[0][0] != ' ') {
                  lifes--;
                  updateDiodes(lifes);
              }
              screen[0][0] = ' ';
              screen[1][0] = 'x';
          }

          display();

          time1 = 0;
      }
   }

   else if(titleScreenFlag) {
      if(!showedEndScreen) {
          initEndScreen(points);
          showedEndScreen = true;
      }

      if(!BUTTON4 && !buttonPressed4) {
          gameEnded = false;
          initAllComponents();
          showedEndScreen = false;
          buttonPressed4 = true;
      }
      else if (BUTTON4){
        buttonPressed4 = false;
      }
   }
}

void initClock() {
    BCSCTL1 |= XTS;
    do {
        IFG1 &= ~OFIFG;
        for (int i = 0xFF; i > 0; i--);
    }
    while ((IFG1 & OFIFG) == OFIFG);
    BCSCTL1 |= DIVA_1;
    BCSCTL2 |= SELM0 | SELM1;

    TACTL = TASSEL_1 + MC_1 +ID_3;
    CCTL0 = CCIE;
    CCR0=5000;
    _EINT();
}
