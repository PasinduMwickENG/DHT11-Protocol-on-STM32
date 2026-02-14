# STM32 DHT11 Bare-Metal Driver (Low-Level Implementation)

A professional C-based firmware implementation for interfacing the **DHT11** temperature and humidity sensor with the **STM32F103C8T6 (Blue Pill)**. This project bypasses high-level abstraction libraries to demonstrate a deep understanding of bit-banging, timing-sensitive protocols, and **STM32CubeIDE** development workflows.

---

## 🎯 Project Overview
The objective was to develop a custom driver following the DHT11 technical datasheet. The firmware handles high-precision timing to decode the sensor's proprietary 40-bit single-wire serial signal and transmits the processed data to a PC via **USB CDC (Virtual COM Port)**.



### Key Features
* **Bare-Metal Implementation:** Custom driver developed from scratch without external Arduino/DHT libraries.
* **Microsecond Resolution:** Hardware Timer (**TIM2**) utilized for precision sampling.
* **Dynamic GPIO Reconfiguration:** Real-time switching between Output and Input modes on a single pin.
* **USB Telemetry:** Integrated USB Middleware for serial data visualization.
* **Data Integrity:** Checksum validation algorithm implemented to filter transmission noise.

---

## 🛠️ Technical Deep Dive

### 1. Timing & Clock Configuration
The DHT11 requires microsecond-level accuracy to distinguish between a logic '0' ($26$-$28\mu s$) and a logic '1' ($70\mu s$). Using **STM32CubeIDE's Clock Tree**, the internal system clock was verified at $72\text{ MHz}$. 

**TIM2** was configured with a Prescaler of **71**:
$$72\text{ MHz} / (71 + 1) = 1\text{ MHz} \implies 1\text{ tick} = 1\mu s$$

### 2. The Communication Handshake
1.  **MCU Host Start:** Pulls the bus **Low for $18\text{ ms}$**, then **High for $30\mu s$**.
2.  **Sensor Response:** The DHT11 pulls the bus **Low for $80\mu s$**, then **High for $80\mu s$**.
3.  **Data Transmission:** 40 bits are streamed, where the duration of the **High** pulse determines the bit value.



### 3. Software Architecture
* **`microDelay()`**: A blocking delay function using the TIM2 counter.
* **`DHT11_Start()`**: Manages the initialization handshake and GPIO mode switching.
* **`DHT11_Read()`**: Samples the data line and reconstructs bytes using bitwise shifting.

---

## 📂 Project Structure
```text
├── Core
│   ├── Inc
│   │   └── main.h             # Peripheral & Pin definitions
│   └── Src
│       ├── main.c             # Core DHT11 logic & Sensor polling loop
│       └── stm32f1xx_it.c     # Interrupt Service Routines
├── USB_DEVICE
│   └── App
│       └── usbd_cdc_if.c      # USB Virtual COM Port interface
└── DHT11_Project.ioc          # STM32CubeMX configuration file

```
![WhatsApp Image 2026-02-15 at 1 58 00 AM](https://github.com/user-attachments/assets/e3dad668-dbca-4abf-8ae7-36502f9623bb)
<img src = "https://github.com/user-attachments/assets/9597d933-b021-4da8-9938-6af49bda1e1c" width = "500">
<img src = "https://github.com/user-attachments/assets/68d82652-04c3-49ef-8038-1d2255dae37e" height = "500">




