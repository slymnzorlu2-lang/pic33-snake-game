#ifndef FCY
#define FCY 60000000UL // 60 MIPS
#endif

#include <xc.h>z
#include <libpic30.h>
#include <stdint.h>
#include <stdlib.h>

// --- KONF?GÜRASYON AYARLARI ---
#pragma config FNOSC = FRCPLL    
#pragma config POSCMD = NONE     
#pragma config FWDTEN = OFF      
#pragma config JTAGEN = OFF      

#define OLED_ADDR 0x78

// --- GLOBAL OYUN DE???KENLER? ---
int snakeX[64];
int snakeY[64];
int snakeLen = 5;
int foodX = 12;
int foodY = 4;
int dir = 0; 
int last_dir = 0; // Çapraz? engelleyen kilit
int hiz = 150; 
int score = 0;  
int lives = 3; 

// --- TÜM Ç?Z?M D?Z?LER? ---
const uint8_t num_font[10][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}  // 9
};

const uint8_t heart_font[5] = {0x06, 0x0F, 0x1E, 0x0F, 0x06}; 
const uint8_t circle_food[8] = {0x00, 0x3C, 0x7E, 0x7E, 0x7E, 0x7E, 0x3C, 0x00};

const uint8_t go_bmp[52] = {
    0x3C,0x42,0x42,0x4A,0x3A,0x00, 
    0x3F,0x48,0x48,0x48,0x3F,0x00, 
    0x7F,0x02,0x0C,0x02,0x7F,0x00, 
    0x7F,0x49,0x49,0x49,0x41,0x00, 
    0x00,0x00,0x00,0x00,0x00,      
    0x3E,0x41,0x41,0x41,0x3E,0x00, 
    0x1F,0x20,0x40,0x20,0x1F,0x00, 
    0x7F,0x49,0x49,0x49,0x41,0x00, 
    0x7F,0x09,0x19,0x29,0x46       
};

const uint8_t alpha_font[26][5] = {
    {0x3F, 0x48, 0x48, 0x48, 0x3F}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x41, 0x3E}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x48, 0x48, 0x48, 0x40}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x3A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x01, 0x01, 0x01, 0x01}, // L
    {0x7F, 0x20, 0x10, 0x20, 0x7F}, // M
    {0x7F, 0x10, 0x08, 0x04, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x48, 0x48, 0x48, 0x30}, // P
    {0x3E, 0x41, 0x45, 0x42, 0x3D}, // Q
    {0x7F, 0x48, 0x4C, 0x4A, 0x31}, // R
    {0x32, 0x49, 0x49, 0x49, 0x26}, // S
    {0x40, 0x40, 0x7F, 0x40, 0x40}, // T
    {0x7E, 0x01, 0x01, 0x01, 0x7E}, // U
    {0x7C, 0x02, 0x01, 0x02, 0x7C}, // V
    {0x7E, 0x01, 0x06, 0x01, 0x7E}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x60, 0x10, 0x0F, 0x10, 0x60}, // Y
    {0x43, 0x45, 0x49, 0x51, 0x61}  // Z
};

// --- I2C VE OLED ALT YAPISI ---
void I2C2_Init(void) {
    I2C2BRG = 150; 
    I2C2CONbits.I2CEN = 1;
    __delay_ms(20);
}
void I2C2_Start(void) { I2C2CONbits.SEN = 1; while(I2C2CONbits.SEN); }
void I2C2_Stop(void) { I2C2CONbits.PEN = 1; while(I2C2CONbits.PEN); }
void I2C2_Write(uint8_t data) { I2C2TRN = data; while(I2C2STATbits.TRSTAT); }

void OLED_Cmd(uint8_t cmd) {
    I2C2_Start(); I2C2_Write(OLED_ADDR); I2C2_Write(0x00); I2C2_Write(cmd); I2C2_Stop();
}

void OLED_Init(void) {
    __delay_ms(200);
    OLED_Cmd(0xAE); 
    OLED_Cmd(0x8D); OLED_Cmd(0x14); 
    OLED_Cmd(0x20); OLED_Cmd(0x00); 
    
    // --- TERS (AYNALANMI?) YAZIYI DÜZELTEN KES?N KOMUT ---
    OLED_Cmd(0xA1); // Sa?dan sola taramay? düzeltir (Aynalamay? kald?r?r)
    OLED_Cmd(0xC8); // Yukar?dan a?a?? taramay? düzeltir
    
    OLED_Cmd(0xAF); 
    __delay_ms(100);
}

void OLED_Clear(void) {
    uint16_t i; 
    OLED_Cmd(0x21); OLED_Cmd(0); OLED_Cmd(127); 
    OLED_Cmd(0x22); OLED_Cmd(0); OLED_Cmd(7);   
    I2C2_Start(); I2C2_Write(OLED_ADDR); I2C2_Write(0x40);
    for(i=0; i<1024; i++) I2C2_Write(0x00);
    I2C2_Stop();
}

// --- ADC (JOYSTICK) OKUMA ---
void ADC_Init(void) {
    ANSELBbits.ANSB0 = 0; 
    TRISBbits.TRISB0 = 1; 
    AD1CON1 = 0x8000;     
    AD1CHS0 = 0;          
    AD1CON3 = 0x0002; 
    AD1CON2 = 0;
}

uint16_t Read_ADC(uint8_t ch) {
    AD1CHS0bits.CH0SA = ch;
    AD1CON1bits.SAMP = 1; __delay_us(10);
    AD1CON1bits.SAMP = 0; while(!AD1CON1bits.DONE);
    return ADC1BUF0;
}

// --- MET?N YAZDIRMA ---
void Print_Text(uint8_t page, uint8_t col, const char* str) {
    int p;
    OLED_Cmd(0xB0 + page); 
    OLED_Cmd(0x00 + (col & 0x0F)); 
    OLED_Cmd(0x10 + ((col >> 4) & 0x0F)); 
    
    I2C2_Start(); I2C2_Write(OLED_ADDR); I2C2_Write(0x40);
    while(*str) {
        if(*str >= 'A' && *str <= 'Z') {
            for(p=0; p<5; p++) I2C2_Write(alpha_font[*str - 'A'][p]);
        } 
        else if(*str >= '0' && *str <= '9') {
            for(p=0; p<5; p++) I2C2_Write(num_font[*str - '0'][p]);
        }
        else if(*str == '>') {
            I2C2_Write(0x00); I2C2_Write(0x41); I2C2_Write(0x22); I2C2_Write(0x14); I2C2_Write(0x08);
        }
        else if(*str == '-') {
            I2C2_Write(0x08); I2C2_Write(0x08); I2C2_Write(0x08); I2C2_Write(0x00); I2C2_Write(0x00);
        }
        else { 
            for(p=0; p<5; p++) I2C2_Write(0x00);
        }
        I2C2_Write(0x00); 
        str++;
    }
    I2C2_Stop();
}

// --- G?R?? EKRANI ---
void Show_Intro(void) {
    uint16_t t;
    OLED_Clear();
    Print_Text(0, 20, "EEM392 PROJESI");
    Print_Text(2, 0, "SULEYMAN ZORLU");
    Print_Text(3, 0, "NECATI YESILBAS");
    Print_Text(4, 0, "IBRAHIM YESILKAYA");
    Print_Text(5, 0, "EMIN KURAK");
    for(t=0; t<3000; t++) { __delay_ms(1); }
}

// --- ZORLUK SEÇ?M MENÜSÜ ---
void Select_Difficulty(void) {
    int diff = 1; 
    uint16_t jX, jY;
    OLED_Clear();
    while(1) {
        jX = Read_ADC(0);
        jY = Read_ADC(1);
        
        if(jY > 800) { diff++; if(diff>2) diff=2; __delay_ms(200); }
        if(jY < 200) { diff--; if(diff<0) diff=0; __delay_ms(200); }
        
        Print_Text(0, 10, "ZORLUK SECINIZ");
        
        if(diff == 0) Print_Text(2, 20, "> 1-KOLAY"); else Print_Text(2, 20, "  1-KOLAY");
        if(diff == 1) Print_Text(4, 20, "> 2-ORTA "); else Print_Text(4, 20, "  2-ORTA ");
        if(diff == 2) Print_Text(6, 20, "> 3-ZOR  "); else Print_Text(6, 20, "  3-ZOR  ");
        
        if(jX > 800) {
            LATBbits.LATB1 = 1; __delay_ms(100); LATBbits.LATB1 = 0; 
            break;
        }
        __delay_ms(50);
    }
    
    if(diff == 0) hiz = 200;      
    else if(diff == 1) hiz = 130; 
    else if(diff == 2) hiz = 70;  
    OLED_Clear();
}

// --- ÇAPRAZI KES?N OLARAK ENGELLEYEN YEN? MOTOR ---
void Wait_And_Read_Joy(uint16_t ms) {
    uint16_t i;
    uint16_t jX, jY;
    int input_registered = 0; // K?L?T: Tek harekette sadece tek yön
    
    for(i = 0; i < ms; i += 10) {
        if(!input_registered) {
            jX = Read_ADC(0); 
            jY = Read_ADC(1);
            
            // X Ekseni hareketi (Sadece daha önce Y'de hareket ediyorsa dönebilir)
            if(jX > 800 && last_dir != 0 && last_dir != 1) { 
                dir = 0; 
                input_registered = 1; 
            }
            else if(jX < 200 && last_dir != 0 && last_dir != 1) { 
                dir = 1; 
                input_registered = 1; 
            }
            // Y Ekseni hareketi (Sadece daha önce X'te hareket ediyorsa dönebilir)
            else if(jY > 800 && last_dir != 2 && last_dir != 3) { 
                dir = 2; 
                input_registered = 1; 
            }
            else if(jY < 200 && last_dir != 2 && last_dir != 3) { 
                dir = 3; 
                input_registered = 1; 
            }
        }
        __delay_ms(10);
    }
}

void Draw_Snake(int x, int y, uint8_t state) {
    int p; 
    OLED_Cmd(0xB0 + y); 
    OLED_Cmd(0x00 + ((x*8) & 0x0F)); 
    OLED_Cmd(0x10 + (((x*8) >> 4) & 0x0F)); 
    I2C2_Start(); I2C2_Write(OLED_ADDR); I2C2_Write(0x40);
    for(p=0; p<8; p++) I2C2_Write(state ? 0xFF : 0x00);
    I2C2_Stop();
}

void Draw_Food(int x, int y) {
    int p; 
    OLED_Cmd(0xB0 + y); 
    OLED_Cmd(0x00 + ((x*8) & 0x0F)); 
    OLED_Cmd(0x10 + (((x*8) >> 4) & 0x0F)); 
    I2C2_Start(); I2C2_Write(OLED_ADDR); I2C2_Write(0x40);
    for(p=0; p<8; p++) I2C2_Write(circle_food[p]);
    I2C2_Stop();
}

void Draw_UI(int s, int l) {
    int i, j, d1, d2;
    d1 = (s / 10) % 10; d2 = s % 10;        
    
    OLED_Cmd(0xB0); OLED_Cmd(0x00); OLED_Cmd(0x10); 
    I2C2_Start(); I2C2_Write(OLED_ADDR); I2C2_Write(0x40);
    for(i=0; i<5; i++) I2C2_Write(num_font[d1][i]);
    I2C2_Write(0x00); 
    for(i=0; i<5; i++) I2C2_Write(num_font[d2][i]);
    I2C2_Stop();

    OLED_Cmd(0xB0); OLED_Cmd(0x00 + (105 & 0x0F)); OLED_Cmd(0x10 + ((105 >> 4) & 0x0F)); 
    I2C2_Start(); I2C2_Write(OLED_ADDR); I2C2_Write(0x40);
    for(j=0; j<3; j++) {
        if(j < l) { for(i=0; i<5; i++) I2C2_Write(heart_font[i]); } 
        else { for(i=0; i<5; i++) I2C2_Write(0x00); }
        I2C2_Write(0x00); 
    }
    I2C2_Stop();
}

void Show_GameOver(void) {
    int i;
    OLED_Clear();
    OLED_Cmd(0xB3); OLED_Cmd(0x00 + (35 & 0x0F)); OLED_Cmd(0x10 + ((35 >> 4) & 0x0F)); 
    I2C2_Start(); I2C2_Write(OLED_ADDR); I2C2_Write(0x40);
    for(i=0; i<52; i++) I2C2_Write(go_bmp[i]); 
    I2C2_Stop();
}

// --- ANA DÖNGÜ ---
int main(void) {
    int i; 
    int game_over_flag;

    PLLFBD = 63; CLKDIVbits.PLLPOST = 0; CLKDIVbits.PLLPRE = 0;
    while (OSCCONbits.LOCK != 1);

    ANSELB = 0x0000; 
    
    TRISBbits.TRISB1 = 0; 
    LATBbits.LATB1 = 0;   
    
    ADC_Init(); 
    I2C2_Init(); 
    OLED_Init(); 
    
    Show_Intro();
    Select_Difficulty();

    for(i=0; i<snakeLen; i++) { snakeX[i] = 10-i; snakeY[i] = 4; }

    Draw_UI(score, lives);

    while(1) {
        game_over_flag = 0; 
        
        Draw_Snake(snakeX[snakeLen-1], snakeY[snakeLen-1], 0);

        for(i = snakeLen-1; i > 0; i--) {
            snakeX[i] = snakeX[i-1];
            snakeY[i] = snakeY[i-1];
        }

        if(dir == 0) snakeX[0]++;
        else if(dir == 1) snakeX[0]--;
        else if(dir == 2) snakeY[0]--;
        else if(dir == 3) snakeY[0]++;
        
        last_dir = dir; // Y?lan bir ad?m att?ktan SONRA yönü kilitlenir

        if(snakeX[0] > 15) snakeX[0] = 0; 
        else if(snakeX[0] < 0) snakeX[0] = 15;
        
        if(snakeY[0] > 7) snakeY[0] = 1; 
        else if(snakeY[0] < 1) snakeY[0] = 7;

        for(i = 1; i < snakeLen; i++) {
            if(snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
                game_over_flag = 1;
            }
        }

        if(game_over_flag == 1) {
            lives--; 
            Draw_UI(score, lives); 
            
            LATBbits.LATB1 = 1; 
            Wait_And_Read_Joy(800); 
            LATBbits.LATB1 = 0;
            
            if(lives <= 0) {
                Show_GameOver(); 
                Wait_And_Read_Joy(3000);  
                
                lives = 3;
                score = 0; 
                Select_Difficulty();
            } else {
                Wait_And_Read_Joy(1000); 
            }
            
            snakeLen = 5;
            dir = 0;
            last_dir = 0;
            for(i=0; i<snakeLen; i++) { snakeX[i] = 10-i; snakeY[i] = 4; }
            foodX = (rand() % 16); 
            foodY = (rand() % 7) + 1; 
            
            OLED_Clear();
            Draw_UI(score, lives);
            continue; 
        }

        Draw_Snake(snakeX[0], snakeY[0], 1); 
        Draw_Food(foodX, foodY);             
        
        if(snakeX[0] == foodX && snakeY[0] == foodY) {
            LATBbits.LATB1 = 1; 
            Wait_And_Read_Joy(30); 
            LATBbits.LATB1 = 0;
            
            if(snakeLen < 60) snakeLen++; 
            score++; 
            Draw_UI(score, lives); 
            
            foodX = (rand() % 16); 
            foodY = (rand() % 7) + 1; 
            
            if(hiz > 50) hiz -= 4; 
        }

        Wait_And_Read_Joy(hiz); 
    }
    return 0;
}