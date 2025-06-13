# Saturn Smart Reset Button

Smart region/frequency/BIOS selector for Sega Saturn  
Originally based on the [**Saturn Switchless Mod**](https://github.com/sebknzl/saturnmod) (2004), this version expands functionality with full bankswitching support for multi-BIOS systems and RGB LED.

---

## Features

- ✅ Switchless region selector (JAP / USA / EUR)
- ✅ 50Hz / 60Hz frequency toggle
- ✅ Manage up to 4 BIOS banks (using A18/A19 lines)
- ✅ EEPROM save for last selected region/frequency
- ✅ RGB LED feedback (Common Cathode)
- ✅ Compatible with reprogrammable ROM ICs for BIOS replacement:
  - 29F800 (SOP44, 8Mbit, 2 banks)
  - 29F1610 (SOP44, 16Mbit, 4 banks)
  - 27C800 (DIP42, 8Mbit, 2 banks)
  - 27C160 (DIP42, 16Mbit, 4 banks)
- ✅ Reset button control (short/medium/long press)
- ✅ Works on VA0, VA1, VA SD, VA SG, VA9 and VA13 Sega Saturn boards
- ✅ LED colors and BIOS type fully configurable via `#define` macros

---

## Button Usage

| Action                  | Description                       |
|------------------------|-----------------------------------|
| Short press (<250ms)   | Perform RESET                     |
| Medium press (<1250ms) | Toggle 50Hz / 60Hz                |
| Long press (>1250ms)   | Cycle through region/BIOS presets|

> 💡 LED will flash to indicate 50Hz (slow) or 60Hz (fast)

---

> 🔴🟢🔵 LED colors for each region can be customized:

```c
// ** LED COLOR ASSIGNMENT **
#define COLOR_JAP   LED_BLUE
#define COLOR_USA   LED_GREEN
#define COLOR_JAP2  LED_CYAN
#define COLOR_EUR   LED_ORANGE
#define COLOR_JAP3  LED_VIOLET
```

## BIOS Bankswitch

> ℹ️ The BIOS chip to be used must be selected in code:
```c
// ** SELECT BIOS IC **
#define BIOS    IC_8M   // IC_16M (4 banks) | IC_8M (2 banks)
```

To support multiple BIOS variants, the system allows mapping specific images to each bank:

### Bank Map for 8Mbit (2 x 512KB banks)

| Region | Bank | A18 | 
|--------|------|-----| 
| JAP    | 0    | LO  | 
| USA    | 1 🔁 | HI  | 
| EUR    | 1 🔁 | HI  | 

### Bank Map for 16Mbit (4 x 512KB banks)

| Region | Bank | A19 | A18 | 
|--------|------|-----|-----| 
| JAP    | 0    | LO  | LO  | 
| USA    | 1 🔁 | LO  | HI  |
| JAP2   | 2    | HI  | LO  | 
| EUR    | 1 🔁 | LO  | HI  | 
| JAP3   | 3    | HI  | HI  | 

> 🔁 Same bank shared by USA / EUR

### Supported BIOS examples:
> These BIOS images are 512KB each and suitable for use in 8Mbit or 16Mbit chips, with 512KB (4Mbit) banks.

- **JAP:** Sega Saturn (Sega)
- **JAP2:** V-Saturn (Victor)
- **JAP3:** Hi-Saturn (Hitachi)
- **USA / EUR:** Sega Saturn - World-Wide (Sega)

> ℹ️ You may use either **retail BIOS dumps** or **Region-Free (RF)** versions. RF versions allow booting CDs from any region and do **not** require DIP switch connections. However, if you choose retail BIOS dumps, the DIP switch lines **must be connected** (JP6/JP10/JP12) for proper region functionality.

---

## Installation Notes

- Designed for Sega Saturn mainboards:
  - VA0, VA1, VA SD, VA SG, VA9, VA13
- DIP switch assignment via RC0–RC2 (JP6, JP10, JP12) -> **Only needed if using retail BIOS**
- RGB LED used in this project is a **high-brightness** type. If using **diffused or opaque LEDs**, resistor values can be reduced accordingly to achieve the desired brightness.

> ⚠️ Uses a common cathode RGB LED. Recommended resistor values:  
> - 🔴 Red = 220Ω, 🟢 Green = 2kΩ, 🔵 Blue = 1.2kΩ  

### PIC16F630 / PIC16F676 Pinout

| Pin | Name | Function                        |
|-----|------|---------------------------------|
| 1   | VCC  | +5V Power Supply                |
| 2   | RA5  | A18 BIOS Bankswitch             |
| 3   | RA4  | A19 BIOS Bankswitch (16M only)  |
| 4   | RA3  | ICSP MCLR / VPP                 |
| 5   | RC5  | Red LED (Anode)                 |
| 6   | RC4  | Green LED (Anode)               |
| 7   | RC3  | Blue LED (Anode)                |
| 8   | RC2  | JP12 Region DIP Switch          |
| 9   | RC1  | JP10 Region DIP Switch          |
| 10  | RC0  | JP6 Region DIP Switch           |
| 11  | RA2  | RESET OUT to console            |
| 12  | RA1  | 50/60Hz Toggle / ICSP CLK       |
| 13  | RA0  | Reset Button Input / ICSP DAT   |
| 14  | VSS  | Ground                          |

---

## Notes on Region DIP Switches (JP6, JP10, JP12) and Frequency Selector (JP1, JP2 / R29)

> ⚠️ The region configuration DIP switches (JP6, JP10, JP12) and the frequency selector (JP1, JP2 or R29) are implemented as matched pairs and must be carefully verified on each mainboard revision before use.

Each DIP switch has a corresponding paired terminal:

- **JP6 ↔ JP7**
- **JP10 ↔ JP11**
- **JP12 ↔ JP13**
- **JP1 ↔ JP2** (or **R29**, depending on board revision)

These pairs are wired such that:
- One side (e.g. JP6, JP10, JP12, JP1) typically routes to **VCC**
- The paired side (e.g. JP7, JP11, JP13, JP2/R29) routes to **GND**

Only one terminal of each pair should ever be active.  
For example, if **JP6** is used to connect to VCC, then **JP7** (its pair) must remain **disconnected**.  
The same rule applies for all DIP switch pairs — **both sides must never be active simultaneously**.

> ℹ️ Note: JP8–JP9 also form a physical pair, but they are never used by the region or frequency configuration logic.

---

### PCB Considerations

On some mainboard revisions, one of the DIP pair terminals may be **permanently connected** to GND or VCC through:

- Traces (e.g. direct PCB connections)
- Soldered jumpers or zero-ohm resistors

These connections must be carefully **identified and removed** before using the DIP switch inputs, or before driving these lines via the PIC.  
For example:
- A common case is **JP13** being permanently tied to GND when **JP12** is unused
- On some boards, **R29** replaces **JP2** as the GND-side of the frequency selector pair

> 🔍 Always inspect for:
> - Existing solder bridges
> - 0Ω resistors connecting DIP lines to GND or VCC
> - Traces between pads that force a logic level

### DIP Switch Layout Reference

The image below illustrates the typical layout of the region DIP switch pairs on Sega Saturn mainboards, including signal routing and important modification points.

![DIP switch layout](img/dipswitch.png)

- **Yellow highlights** indicate the **common terminals** that are used for region detection by the system.
- **Red highlights** mark **0Ω resistors** or **permanent traces** that must be **removed or cut** to safely use the DIP signals with the PIC microcontroller.
- **Green line** represents the signal line connected to **JP6**
- **Purple line** represents the signal line connected to **JP10**
- **Blue line** represents the signal line connected to **JP12**

> ⚠️ Only one terminal of each DIP pair (e.g. JP6 ↔ JP7) should ever be connected to either VCC or GND, not both.  
> The common terminal (yellow) must be **isolated from fixed power/ground** before being controlled via PIC.  
> Failing to do so may cause **permanent hardware damage**.

---

### ⚠️ Important Warning

Before connecting JP6, JP10, JP12, or JP1 to the PIC (for region or frequency control), make absolutely sure that:
- No passive components (resistors, jumpers) are bridging them to power or ground
- Their paired terminals (JP7, JP11, JP13, JP2/R29) are **not physically connected to GND or VCC**

🚫 Failing to disconnect pre-existing connections may cause:
- **Short circuits** between the PIC output (HI/LO) and fixed power/ground lines
- **Permanent damage** to the console mainboard or the PIC itself

Always verify with a multimeter and inspect the board layout before enabling region or frequency control via DIP switches.

---

## Building & Flashing

### Source Code (Optional Compilation)
To build from source:
- Use [MPLAB X IDE](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide) and [XC8 Compiler](https://www.microchip.com/en-us/tools-resources/develop/mplab-xc-compilers)
- Target microcontroller: **PIC16F630** or **PIC16F676**
- Clock: `4MHz` internal
- `MCLR` disabled (set as input)

### ⚡ Precompiled `.hex`
For convenience, a precompiled **`.hex` file** is included in the [Releases](../../releases) section.  
This allows quick flashing using:

- **MPLAB IPE 6.20 or newer**
- **PICKit 3** or compatible programmer

No source compilation is required if using the `.hex`.

---

## Preparing BIOS Images (Byte-Swapping and Merging)

1. **Endianness:**  
   BIOS binaries must be in **big-endian** format (byte-swapped).  
   This matches the 68000-based architecture of the Saturn.

> 💡 Tip: Most BIOS dumps are in little-endian and must be byte-swapped before merging.

You can inspect BIOS byte order using a hex editor such as [HxD](https://mh-nexus.de/en/hxd/), or through programmer software like **XGPro (XGecu Pro)** with the **T48 (TL866-3G)** EEPROM programmer, which provides an option for **byte-swapping**.

Here's an example showing a dump in little-endian and how it should appear in big-endian:

### Example: BIOS header region (addresses 0x9C0–0x9FF)

**Little-endian view (raw dump):**

    000009C0  43 4F 50 59 52 49 47 48 54 28 43 29 20 53 45 47  COPYRIGHT(C) SEG
    000009D0  41 20 45 4E 54 45 52 50 52 49 53 45 53 2C 4C 54  A ENTERPRISES,LT
    000009E0  44 2E 20 31 39 39 34 20 41 4C 4C 20 52 49 47 48  D. 1994 ALL RIGH
    000009F0  54 53 20 52 45 53 45 52 56 45 44 20 20 20 20 20  TS RESERVED     

**Big-endian (byte-swapped, correct for 68000 systems):**

    000009C0  4F 43 59 50 49 52 48 47 28 54 29 43 53 20 47 45  OCYPIRHG(T)CS GE
    000009D0  20 41 4E 45 45 54 50 52 49 52 45 53 2C 53 54 4C   ANEETPRIRE,S STL
    000009E0  2E 44 31 20 39 39 20 34 4C 41 20 4C 49 52 48 47  .D1 99 4LA LIRHG
    000009F0  54 53 20 52 45 53 45 52 56 45 44 20 20 20 20 20  TS RESERVED     

> 🔁 Use tools or scripts that swap bytes **pairwise (16-bit)** to convert from little- to big-endian format.

2. **Combining BIOS Images:**  
   Concatenate the BIOS files in the correct order:

   **For 29F800 (2 banks):**
   ```cmd
   copy /b JAP.BIN + USA.BIN 29F800.BIN
   ```

   **For 29F1610 (4 banks):**
   ```cmd
   copy /b JAP.BIN + USA.BIN + JAP2.BIN + JAP3.BIN 29F1610.BIN
   ```

---

## Flashing BIOS to IC (Programmers and Adapters)  
> ⚠️ Ensure the merged binary (e.g., `29F800.BIN`) is **byte-swapped** before flashing.

3. **Flashing:**  
   Use a **T48 (TL866-3G)** programmer with:

   - **ADP_S44-EX-1** SOP44 adapter for 29F800 and 29F1610  
   - **ADP_D42-EX-A** DIP42 adapter for 27C800 and 27C160
  
     ![Adapters](img/adapter.jpg)
   
   Or use any other programmer capable of writing to these ICs in their respective packages.

---

### ℹ️ Note on physical installation:  

> ⚠️ The flash chips used in this mod (29F800 and 29F1610) have **44 pins**, while the original Sega Saturn Mask ROM (IC7) has **40 pins**.
> Therefore, when installing the flash IC onto the board, correct alignment and pin handling is critical.

### BIOS IC7 alignment on the board (SOP44)

- Align pin **3** (A17) and **42** (A8) of the EEPROM to match the location of pins **1** (A17) and **40** (A8) of the original SOP40 Mask ROM (IC7).
- This places pins **1–2** and **43–44** of the flash IC **outside** the footprint of the original ROM and must be **lifted** (not soldered to the board).

  ![BIOS alignment](img/ic7.jpg)

### Wiring for lifted pins:

#### For **29F800**:
- Pin **1** (RY/BY#): **Not connected** or **VSS (GND)**
- Pin **2** (A18): Connect to **RA5 (Pin 2)** on the PIC
- Pin **43** (WE#): Connect to **VCC (+5V)**
- Pin **44** (RESET#): Connect to **VCC (+5V)**

#### For **29F1610**:
- Pin **1** (WE#): Connect to **VCC (+5V)**
- Pin **2** (A18): Connect to **RA5 (Pin 2)** on the PIC
- Pin **43** (A19): Connect to **RA4 (Pin 3)** on the PIC
- Pin **44** (WP#): Connect to **VSS (GND)**
---
> ⚠️In the VA0 revision of the Sega Saturn, the original Mask ROM (IC7) is in a DIP40 package.  
> In this case, the replacement BIOS must be in a compatible DIP42 format, such as the **27C800** (8Mbit) and **27C160** (16Mbit) UV EPROM.

### BIOS IC7 alignment on the board (DIP42)

- Align pin **2** (A17) and **41** (A8) of the EPROM to match the location of pins **1** (A17) and **40** (A8) of the original DIP40 Mask ROM (IC7).
- Although the board includes holes for DIP42, they serve different functions, and pins **1** and **42** of the EPROM **must be lifted**.

  ![DIP42 alignment](img/dip42.jpg)

### Wiring for lifted pins:

#### For **27C800**:
- Pin **1** (A18): Connect to **RA5 (Pin 2)** on the PIC
- Pin **42** (NC): **Not connected** 

#### For **27C160**:
- Pin **1** (A18): Connect to **RA5 (Pin 2)** on the PIC  
- Pin **42** (A19): Connect to **RA4 (Pin 3)** on the PIC

---

## License

This is a derivative work based on the original [Saturn Switchless Mod](https://github.com/sebknzl/saturnmod)
licensed under the [GNU General Public License v2 or later](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html)

---

## Credits

> Based on *Saturn Switchless Mod* by **Sebastian Kienzl (2004)**  
> Enhanced version, XC8 port and RGB LED / bankswitching logic by **Electroanalog (2025)**

## Topics / Tags

`sega-saturn` `saturn` `reset-mod` `pic16f630` `pic16f676` `modchip` `led-feedback` `multi-bios` `sega-hardware` `diy-console-mod` `retro-gaming`

