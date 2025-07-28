/* ---------------------------------------------------------------
 * Saturn Smart Reset Button for PIC16F630/76
 * 16/8Mbit Bankswitching Extension for Multi-BIOS System
 * Electroanalog(c) 2025
 *
 * Based on original code for "Saturn Switchless Mod" 
 * by Sebastian Kienzl (2004)
 * ---------------------------------------------------------------
 * This is a derivative work licensed under the GNU General Public 
 * License, as published by the Free Software Foundation; either 
 * version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * This version features:
 * - RGB LED feedback with customizable color by cycle position
 * - Support 8Mbit 29F800 for 2-bank BIOS switching
 * - Support 16Mbit 29F1610 for 4-bank BIOS switching
 * - Ported to XC8 (v3.x), reorganized for clarity and maintainability
 * ----------------------------------------------------------------
 * 
 * PIC16F630/76 Pinout:
 *  1 - VCC = +5V
 *  2 - RA5 = A18 BIOS bankswitch
 *  3 - RA4 = A19 BIOS bankswitch (16M)
 *  4 - RA3 = ICSP MCLR/VPP
 *  5 - RC5 = Red LED | A[+] sourcing
 *  6 - RC4 = Green LED | A[+] sourcing
 *  7 - RC3 = Blue LED | A[+] sourcing
 *  8 - RC2 = JP12 Region dipswitch
 *  9 - RC1 = JP10 Region dipswitch
 * 10 - RC0 = JP6 Region dipswitch
 * 11 - RA2 = RESET OUT
 * 12 - RA1 = 50/60Hz Toggle | ICSP CLK
 * 13 - RA0 = RESET BUTTON | ICSP DAT
 * 14 - VSS = GND
 * 
 * For installation instructions, wiring options, and usage details,
 * please refer to the official README at:
 * https://www.electroanalog.com/sat-srb
 */

#include <xc.h>
#include <stdint.h>

// BASIC CONFIG
#pragma config MCLRE = OFF      // MCLR pin function is digital input
#pragma config BOREN = ON       // Brown-out Reset enabled
#pragma config PWRTE = ON       // Power-up Timer Enable bit
#pragma config WDTE = OFF       // Watchdog Timer Enable bit
#pragma config FOSC = INTRCIO   // Internal RC oscillator, I/O function on RA6/RA7
#pragma config CP = OFF         // Code Protection disabled
#define _XTAL_FREQ 4000000      // Clock frequency

// EEPROM DEFINITION
#define WRITE_EEPROM(addr, val)  eeprom_write((unsigned char)(addr), (unsigned char)(val))

// EEPROM INIT (Legacy)
__EEPROM_DATA( 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );
/* EEPROM Layout:
 * Byte 0: Current region index (0 = JAP, 1 = USA, 2 = JAP2, 3 = EUR, 4 = JAP3)
 */

// BIOS IC OPTIONS
#define IC_16M  1   // 29F1610
#define IC_8M   2   // 29F800

// ** SELECT BIOS IC **
#define BIOS    IC_8M   // IC_16M (4 banks) | IC_8M (2 banks)

#if BIOS == IC_16M
  #define COUNTRYNUM 5  // 0=JAP, 1=USA, 2=JAP2, 3=EUR, 4=JAP3
#elif BIOS == IC_8M
  #define COUNTRYNUM 3  // 0=JAP, 1=USA, 2=EUR
#else
  #error "Invalid chip selection."
#endif

// PIN DEFINITIONS
#define BUTTON  RA0
#define VFRQ    RA1
#define RST     RA2
#define RST_T   TRISA2
#define JP6     RC0
#define JP10    RC1
#define JP12    RC2
#define A18     RA5
#define A19     RA4
#define LED_R   RC5
#define LED_G   RC4
#define LED_B   RC3

// LED COLOR CODES (bit2=Red, bit1=Green, bit0=Blue)
#define LED_OFF     0b000
#define LED_RED     0b100
#define LED_GREEN   0b010
#define LED_BLUE    0b001
#define LED_YELLOW  0b110  // R+G
#define LED_CYAN    0b011  // G+B
#define LED_PURPLE  0b101  // R+B
#define LED_WHITE   0b111  // R+G+B

// ** LED COLOR ASSIGNMENT **
#define COLOR_JAP   LED_BLUE
#define COLOR_USA   LED_GREEN
#define COLOR_JAP2  LED_CYAN
#define COLOR_EUR   LED_YELLOW
#define COLOR_JAP3  LED_PURPLE

// Global variable
uint8_t currCountry;
__bit VF;   // Frequency flag: 0 = 50Hz, 1 = 60Hz

// Region configuration structure
typedef struct {
    uint8_t swBank;     // A18/A19 setting (bits 0-1)
    uint8_t ledCode;    // LED color code (bit2=R, bit1=G, bit0=B)
    uint8_t dipswitch;  // DIP switch region logic: 0=JAP, 1=USA, 2=EUR
    uint8_t freq;       // Frequency setting: 1 = 60Hz, 0 = 50Hz
} RegionConfig;

#if BIOS == IC_16M // 29F1610
const RegionConfig regions[COUNTRYNUM] = {
    { 0b00, COLOR_JAP,  0, 1 }, // JAP A18=LO,  A19=LO	Bank 0	60Hz
    { 0b01, COLOR_USA,  1, 1 }, // USA  A18=HI, A19=LO	Bank 1	60Hz
    { 0b10, COLOR_JAP2, 0, 1 }, // JAP2 A18=LO, A19=HI	Bank 2	60Hz
    { 0b01, COLOR_EUR,  2, 0 }, // EUR  A18=HI, A19=LO	Bank 1	50Hz
    { 0b11, COLOR_JAP3, 0, 1 }  // JAP3	A18=HI, A19=HI	Bank 3	60Hz
};
#elif BIOS == IC_8M // 29F800
const RegionConfig regions[COUNTRYNUM] = {
    { 0b00, COLOR_JAP,      0, 1 }, // JAP     A18=LO   Bank 0	60Hz
    { 0b01, COLOR_USA,      1, 1 }, // USA     A18=HI	Bank 1	60Hz
    { 0b01, COLOR_EUR,      2, 0 }  // EUR     A18=HI	Bank 1	50Hz
};
#else
  #error "Invalid BIOS IC selection!"
#endif
/* BIOS IC address table:
 * Bank0:	0x00000 ->  0x7FFFF     512 KB (4 Mbit) 29F1610 | 29F800
 * Bank1:	0x80000 ->  0xFFFFF     512 KB (4 Mbit) 29F1610 | 29F800
 * Bank2:	0x100000 -> 0x17FFFF	512 KB (4 Mbit) 29F1610
 * Bank3:	0x180000 -> 0x1FFFFF	512 KB (4 Mbit) 29F1610
 */

// Function Prototypes
void display5060(uint8_t dunkel);
void darkenLeds(uint16_t msec);
void delay(uint16_t t);
void setDipSwitches(int region);
void setLeds(void);
void setCountry(void);
void setDefaultVF(void);
void reset(void);
void _load(void);
void _save(void);

/* MAIN:
 * - Short press (< 250ms): Reset only.
 * - Medium press (250-1250ms): Toggle frequency (VF) and flash LED (no reset).
 * - Long press (>1250ms): Cycle regions; on release update bankswitch, save and reset.
 */
void main() {
    PCON = 0;           //Clear MCU reset flags 
    CMCON = 0b111;      // Disable analog comparators
    nRAPU = 0;          // Enable individual weak pull-ups (WPUA)
    WPUA = 0b00000001;  // Enable pull-up for RA0 (button)
    PORTA = 0;          // Clear RA outputs
    // RA2 [RESET] will only be configured to be an output on reset
    TRISA = 0x0D;       // RA0 [Button] = input | RA2 [RESET] = input (idle)
						// RA1 [VF] = out | RA5/RA4 [A18/A19] = out 
    PORTC = 0;          // Clear RC outputs
    TRISC = 0x00;       // All RC outputs
    RAIE = 1;           // Enable port change interrupt
    IOCA0 = 1;          // Enable change interrupt for RA0 (reset button)
    
    _load(); // Load current settings from EEPROM
    
    // Main loop:
    while(1) {
        RAIF = 0;
        SLEEP();
        delay(5);
        if (BUTTON != 0) continue;
        int pressTime = 0;
        while (BUTTON == 0) {
            delay(10);
            pressTime += 10;
            if (pressTime >= 1250) break;
        }
        if (pressTime < 250) {
            // Short press: reset
            reset();
        } else if (pressTime < 1250) {
            // Medium press: toggle 50/60Hz
            VF ^= 1;
            VFRQ = (VF != 0);  
            display5060(1);
            while (BUTTON == 0);
        } else {
            // Long press: cycle bank/regions
            while (BUTTON == 0) {
                delay(1000);
                if (BUTTON == 0) {
                    currCountry++;
                    if (currCountry >= COUNTRYNUM)
                        currCountry = 0;
                    setLeds();
                }
            }
            // Apply adjustments
            setCountry();
            setDefaultVF();
            _save();
            display5060(0);
            reset();
        }
    }   
}

/* Bank selection for each region
 * USA and EUR share Bank 1 | JAP uses bank 0-2-3
 *
 * Region dipswitch outputs:
 *      RC0  RC1  RC2
 *      JP6  JP10 JP12
 * USA  0    1    0
 * JAP  1    0    0
 * EUR  0    1    1
 */
void setDipSwitches(int dipswitch) {
    switch (dipswitch) {
        case 0: // JAP
            JP12 = 0; JP10 = 0; JP6 = 1;
            break;
        case 1: // USA
            JP12 = 0; JP10 = 1; JP6 = 0;
            break;
        case 2: // EUR
            JP12 = 1; JP10 = 1; JP6 = 0;
            break;
        default:
            // Fallback safe: JAP
            JP12 = 0; JP10 = 0; JP6 = 1;
            break;
    }
}

// Updates LED output based on current cycle position
void setLeds(void) {
    uint8_t color = regions[currCountry].ledCode;
    LED_R = (color & 0b100) ? 1 : 0;
    LED_G = (color & 0b010) ? 1 : 0;
    LED_B = (color & 0b001) ? 1 : 0;
}

// Temporarily turn off LED
void darkenLeds(uint16_t msec) {
    LED_R = 0; LED_G = 0; LED_B = 0;
    delay(msec);
    setLeds();
}

// Sets bank signals A18/A19, and configures dipswitches
void setCountry(void) {
    setDipSwitches(regions[currCountry].dipswitch); 
    // Ensure currCountry is within bounds
    uint8_t bank = regions[currCountry].swBank;
    A18 = (bank & 0b01) ? 1 : 0;
    #if BIOS == IC_16M
        A19 = (bank & 0b10) ? 1 : 0;
    #else
        A19 = 0;  // Not used for IC_8M
    #endif

}

// Sets VF pin state according to current region
void setDefaultVF(void) {
    VF = (regions[currCountry].freq != 0);  // Region/bank based default value
    VFRQ = (VF != 0);                       // Update RA1
}

// Visual LED feedback for region frequency
// Provides an LED feedback flash (slow for 50Hz, fast for 60Hz).
void display5060(uint8_t dunkel) {
    if (!dunkel) darkenLeds(200);
    if (VF == 0) {
        delay(200);
        darkenLeds(200);
    } else {
        for (int i = 0; i < 3; i++) {
            delay(75);
            darkenLeds(75);
        }
    }
}

// Delay helper
void delay(uint16_t t) {
    while (t--) __delay_ms(1);
}

// Generate RESET pulse
void reset() {
    RST = 0;
    RST_T = 0;
    delay(200);
    RST_T = 1;
    RST = 1;
}

// Load settings from EEPROM
void _load() {
    currCountry = eeprom_read(0);
    if (currCountry >= COUNTRYNUM) currCountry = 0;
    setCountry();
    setDefaultVF(); 
    setLeds();
    display5060(1);
}

// Save settings to EEPROM if values have changed
void _save(void) {
    if (eeprom_read(0) != currCountry) {
        WRITE_EEPROM(0, currCountry);
    }
}
