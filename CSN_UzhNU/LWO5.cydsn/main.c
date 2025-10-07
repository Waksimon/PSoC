#include "project.h" 

/* Forward declarations */
static void FourDigit74HC595_sendData(uint8_t data);
static void FourDigit74HC595_sendOneDigit(uint8_t position, uint8_t digit, uint8_t dot);
static void initMatrix(void);
static void readMatrix(void);

/* Segment codes for 7-segment display */
static uint8_t LED_NUM[] = { 
    0xC0,   //0 
    0xF9,   //1 
    0xA4,   //2 
    0xB0,   //3 
    0x99,   //4 
    0x92,   //5 
    0x82,   //6 
    0xF8,   //7 
    0x80,   //8 
    0x90,   //9 
    0xBF,   //- 
    0x88,   //A 
    0x83,   //b 
    0xC6,   //C 
    0xA1,   //d 
    0x86,   //E 
    0x8E,   //F 
}; 

/* Глобальні змінні для відображення */
static uint8_t led_counter = 0;
static uint8_t display_data[8] = {7, 6, 5, 4, 3, 2, 1, 0};
static uint8_t display_dots[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* Обробник переривання таймера - динамічна індикація */
CY_ISR(Timer_Int_Handler2)
{
    /* Відображаємо поточний розряд */
    FourDigit74HC595_sendOneDigit(led_counter, display_data[led_counter], display_dots[led_counter]);
    
    /* Переходимо до наступного розряду */
    led_counter++;
    if(led_counter >= 8) {
        led_counter = 0;
    }
}

/* Send data to shift register */     
static void FourDigit74HC595_sendData(uint8_t data) {     
    for(uint8_t i = 0; i < 8; i++) {         
        if(data & (0x80 >> i)) { 
            Pin_DO_Write(1); 
        } else { 
            Pin_DO_Write(0); 
        } 
        Pin_CLK_Write(1); 
        Pin_CLK_Write(0); 
    } 
} 

/* Send one digit to display */ 
static void FourDigit74HC595_sendOneDigit(uint8_t position, uint8_t digit, uint8_t dot)  
{ 
    if(position >= 8) {
        FourDigit74HC595_sendData(0xFF); 
        FourDigit74HC595_sendData(0xFF); 
        return;
    } 
    
    /* Вибір позиції (активний низький рівень) */
    FourDigit74HC595_sendData(0xFF & ~(1 << position));     
    
    /* Відправка сегментів з урахуванням крапки */
    if(dot) { 
        FourDigit74HC595_sendData(LED_NUM[digit] & 0x7F); 
    } else { 
        FourDigit74HC595_sendData(LED_NUM[digit]); 
    } 
    
    /* Фіксація даних у регістрі */
    Pin_Latch_Write(1);
    Pin_Latch_Write(0); 
} 
 
/* Keypad matrix [ROW][COLUMN] */ 
static uint8_t keys[4][3] = { 
    {1, 2, 3}, 
    {4, 5, 6}, 
    {7, 8, 9}, 
    {10, 0, 11}, 
}; 

/* Arrays of function pointers for matrix scanning */
static void (*COLUMN_x_SetDriveMode[3])(uint8_t mode) = { 
    COLUMN_0_SetDriveMode, 
    COLUMN_1_SetDriveMode, 
    COLUMN_2_SetDriveMode 
};   

static void (*COLUMN_x_Write[3])(uint8_t value) = { 
    COLUMN_0_Write, 
    COLUMN_1_Write, 
    COLUMN_2_Write 
}; 

static uint8 (*ROW_x_Read[4])() = { 
    ROW_0_Read, 
    ROW_1_Read, 
    ROW_2_Read, 
    ROW_3_Read 
}; 

/* Initialize keypad matrix */ 
static void initMatrix(void) 
{     
    for(int column_index = 0; column_index < 3; column_index++) { 
        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_DIG_HIZ); 
    } 
} 

/* Read keypad matrix state */ 
static void readMatrix(void) 
{ 
    uint8_t row_counter = sizeof(ROW_x_Read)/sizeof(ROW_x_Read[0]); 
    uint8_t column_counter = sizeof(COLUMN_x_Write)/sizeof(COLUMN_x_Write[0]);     
    
    /* Scan each column */
    for(int column_index = 0; column_index < column_counter; column_index++) { 
        /* Activate column */
        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_STRONG); 
        COLUMN_x_Write[column_index](0);         
        
        /* Read all rows */
        for(int row_index = 0; row_index < row_counter; row_index++) { 
            keys[row_index][column_index] = ROW_x_Read[row_index](); 
        } 
        
        /* Deactivate column */
        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_DIG_HIZ); 
    } 
} 

/* Функція для оновлення відображення */
void updateDisplay(uint8_t key_pressed) {
    /* Відображаємо натиснуту клавішу на першому розряді */
    if(key_pressed <= 9) {
        display_data[0] = key_pressed;
        display_dots[0] = 0;
    } else if(key_pressed == 10) {
        /* Клавіша "*" */
        display_data[0] = 0;
        display_dots[0] = 1;
    } else if(key_pressed == 11) {
        /* Клавіша "#" */
        display_data[0] = 1;
        display_dots[0] = 1;
    }
}

int main(void) 
{ 
    CyGlobalIntEnable; /* Enable global interrupts */
    
    /* Запуск таймера та переривання */
    Timer_Start();
    Timer_Int_StartEx(Timer_Int_Handler2);
    
    /* Ініціалізація клавіатурної матриці */
    initMatrix(); 
    
    uint8_t last_state = 12; 
    
    for(;;) 
    { 
        /* Зчитування стану клавіатури */
        readMatrix(); 
        
        /* Обробка натискань клавіш */
        /* Перший ряд: 1, 2, 3 */
        for(int col = 0; col < 3; col++) {
            uint8_t key_value = col + 1;
            if(keys[0][col] == 0 && last_state != key_value) {
                last_state = key_value;
                updateDisplay(key_value);
            }
            if(keys[0][col] == 1 && last_state == key_value) {
                last_state = 12;
            }
        }
        
        /* Другий ряд: 4, 5, 6 */
        for(int col = 0; col < 3; col++) {
            uint8_t key_value = col + 4;
            if(keys[1][col] == 0 && last_state != key_value) {
                last_state = key_value;
                updateDisplay(key_value);
            }
            if(keys[1][col] == 1 && last_state == key_value) {
                last_state = 12;
            }
        }
        
        /* Третій ряд: 7, 8, 9 */
        for(int col = 0; col < 3; col++) {
            uint8_t key_value = col + 7;
            if(keys[2][col] == 0 && last_state != key_value) {
                last_state = key_value;
                updateDisplay(key_value);
            }
            if(keys[2][col] == 1 && last_state == key_value) {
                last_state = 12;
            }
        }
        
        /* Четвертий ряд: *, 0, # */
        if(keys[3][0] == 0 && last_state != 10) {
            last_state = 10;
            updateDisplay(10);
        }
        if(keys[3][0] == 1 && last_state == 10) {
            last_state = 12;
        }
        
        if(keys[3][1] == 0 && last_state != 0) {
            last_state = 0;
            updateDisplay(0);
        }
        if(keys[3][1] == 1 && last_state == 0) {
            last_state = 12;
        }
        
        if(keys[3][2] == 0 && last_state != 11) {
            last_state = 11;
            updateDisplay(11);
        }
        if(keys[3][2] == 1 && last_state == 11) {
            last_state = 12;
        }
        
        /* Невелика затримка для стабільності сканування */
        CyDelay(10);
    } 
}