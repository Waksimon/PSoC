/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h" 
int main(void) 
{ 
    CyGlobalIntEnable; /* Enable global interrupts. */    
    for(;;) 
    { 
        LED_R_Write(0); 
        CyDelay(500); 
        LED_R_Write(1); 
    CyDelay(500);          
        LED_B_Write(0); 
        CyDelay(500); 
        LED_B_Write(1); 
    CyDelay(500);          
        LED_R_Write(0); 
        CyDelay(500); 
        LED_R_Write(1); 
    CyDelay(500);          
        LED_G_Write(0); 
        CyDelay(500); 
        LED_G_Write(1); 
    CyDelay(500);  
    }
}


/* [] END OF FILE */
