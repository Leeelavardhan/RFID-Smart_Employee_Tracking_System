# 📡 ATTENDIFY-RFID – Smart Employee Tracking & Reporting System

> RFID-based Employee Attendance and Tracking System using LPC2148 ARM7, Embedded C, UART, SPI, RTC, EEPROM, and Linux.

## 📖 Overview

ATTENDIFY-RFID is an embedded system developed using the LPC2148 ARM7 microcontroller to automate employee attendance. The system uses RFID cards for employee identification, records IN/OUT times using an RTC, calculates working hours, and stores employee records in a Linux-based CSV database.

## ✨ Features

- RFID-based Employee Authentication
- Admin Authentication
- Employee Attendance Logging
- Automatic IN/OUT Detection
- Working Hours Calculation
- Employee Add / Edit / Delete
- RTC Time Management
- EEPROM Storage for Admin Card
- UART Communication with Linux
- SPI Communication with EEPROM
- LCD Display Interface
- Keypad-based Admin Menu

---

## 🛠 Hardware Used

- LPC2148 ARM7 Microcontroller
- EM-18 RFID Reader
- AT25LC512 SPI EEPROM
- 16×2 LCD Display
- 4×4 Matrix Keypad
- RTC
- USB-to-TTL Converter
- Personal Computer

---

## 💻 Software Used

- Embedded C
- Keil uVision
- Flash Magic
- Proteus
- Ubuntu Linux
- GCC Compiler

---

## 📡 Communication Protocols

| Protocol | Purpose |
|----------|---------|
| UART0 | LPC2148 ↔ Linux PC |
| UART1 | LPC2148 ↔ RFID Reader |
| SPI | LPC2148 ↔ AT25LC512 EEPROM |

---

## 📂 Project Structure

```text
ATTENDIFY-RFID
│
├── Embedded_Code
│   ├── main.c
│   ├── uart0.c
│   ├── uart1.c
│   ├── spi.c
│   ├── rtc.c
│   ├── lcd.c
│   ├── keypad.c
│   ├── interrupt.c
│   └── headerfiles.h
│
├── Linux_Application
│   ├── ReadUART.c
│   └── employee_details.csv
│
├── Proteus
│   └── Circuit.png
│
├── Images
│
└── README.md
```

---

## 🔄 System Workflow

```text
RFID Card
    │
    ▼
RFID Reader (UART1)
    │
    ▼
LPC2148 ARM7
    │
    ├── RTC
    ├── EEPROM
    ├── LCD
    ├── Keypad
    │
    ▼
UART0
    │
    ▼
Linux Application
    │
    ▼
CSV Employee Database
    │
    ▼
Response to LPC2148
    │
    ▼
LCD Display
```

---

## 📦 Data Frame Format

```text
[RFID_ID](OPERATION){TIME}
```

Example:

```text
[12345678](ADD){10:15:22}

[12345678](DEL){14:22:51}

[12345678](EDT){09:30:11}

[12345678](LOG){08:59:30}
```

---

## 🚀 Project Highlights

- Real-time RFID attendance system
- Secure Admin authentication using EEPROM
- Linux-based employee database
- Automatic working-hour calculation
- Embedded C firmware with interrupt-driven UART communication
- SPI EEPROM interface
- RTC-based time stamping

---

## 🔮 Future Improvements

- Cloud Database Integration
- Wi-Fi Connectivity
- Web Dashboard
- Mobile Application
- Fingerprint + RFID Authentication
- Face Recognition
## Proteus image
<img width="1536" height="1024" alt="Proteus_image" src="https://github.com/user-attachments/assets/f6ba3afb-2b73-47e3-ad0e-96c0904fe8df" />
# Hardware Image
<img width="1600" height="1200" alt="Hardware_photo" src="https://github.com/user-attachments/assets/45b4da02-b12d-4a1b-9b3f-4af43f6deef8" />


---
##

## 👨‍💻 Author

**CHITTEM LEELA VARDHAN REDDY**

- B.Tech – Electrical & Electronics Engineering
- Embedded Systems Developer

