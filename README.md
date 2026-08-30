# Real-Time-Bio-Signal-Processing-Neural-Prosthetic-Controller

An embedded C++ real-time digital signal processing (DSP) pipeline and actuator control system that extracts muscle contraction intensity from noisy bio-potential signals (EMG) to drive a neural prosthetic finger interface. Simulated and validated on the ATmega328P architecture via Wokwi.

---

##  Technical Overview
Surface Electromyography (sEMG) signals reside in the microvolt range ($10\mu\text{V} - 5\text{mV}$) and are inherently corrupted by powerline interference ($50/60\text{ Hz}$) and stochastic physiological noise. 

This project implements an on-device digital processing architecture that ingests raw AC bio-potential data, eliminates DC voltage bias, extracts the **Root Mean Square (RMS) envelope** across a sliding temporal window, and maps the extracted muscle intent to a high-precision PWM servo actuator in real time.

---

##  Signal Processing Pipeline

┌─────────────────┐    ┌─────────────────┐    ┌──────────────────┐
│  Raw Noisy EMG  │───>│  DC Bias Offset │───>│ Full-Wave Square │
│  (60Hz + Noise) │    │   Subtraction   │    │  Rectification   │
└─────────────────┘    └─────────────────┘    └──────────────────┘
│
┌─────────────────┐    ┌─────────────────┐             ▼
│ Servo Actuator  │<───│  Linear Mapping │<───┌──────────────────┐
│ PWM Angle (0-180)│    │ & Constraining  │    │ 25-Sample Moving │
└─────────────────┘    └─────────────────┘    │   RMS Window     │
└──────────────────┘


### Mathematical Formulation
The temporal intensity of the muscle contraction is extracted using a discrete moving Root Mean Square (RMS) envelope over $N = 25$ samples:

$$x_{\text{RMS}}[n] = \sqrt{\frac{1}{N} \sum_{k=0}^{N-1} (x[n-k] - V_{\text{bias}})^2}$$

Where $V_{\text{bias}} = 512$ ADC units ($2.5\text{V}$ midpoint DC offset).

---

##  Circuit Architecture & Hardware Specs

| Component | Pin Connection | Functional Role |
| :--- | :--- | :--- |
| **Arduino Uno** | ATmega328P | Embedded System Microcontroller |
| **Potentiometer** | Analog `A0` | Muscle Effort Simulation Source |
| **Servo Motor** | Digital `Pin 9` (PWM) | Single-Arm Prosthetic Tendon Actuator |
| **Status LED** | Digital `Pin 13` | Threshold Contraction Alarm Indicator |
| **Shared Ground Rail**| `GND` | Common Ground Reference Plane |

---

##  Technical Challenges & Engineering Solutions

### 1. High-Frequency Powerline Interference & Signal Saturation
* **Challenge:** Raw bio-signals are severely degraded by ambient $60\text{ Hz}$ powerline hum and stochastic muscle fiber noise, causing severe jitter in robotic actuators if raw values are mapped directly.
* **Solution:** Designed a sliding 25-sample moving RMS window buffer in C++. This algorithm square-rectifies the signal and computes real-time energy density, attenuating high-frequency AC noise while preserving intentional muscle flex dynamics.

### 2. ADC Input Unipolar Voltage Constraints
* **Challenge:** Human EMG signals are bipolar AC waveforms ($\pm V$). Microcontroller Analog-to-Digital Converters (ADCs) can only read positive unipolar voltages ($0 - 5\text{V}$), risking negative voltage underflow and hardware clipping.
* **Solution:** Biased the simulated raw signal around a $2.5\text{V}$ DC offset ($512$ raw ADC units). The C++ DSP pipeline subtracts this offset prior to squaring, maintaining signal integrity without risking negative ADC values.

### 3. CPU Execution Delays in High-Frequency Control Loops
* **Challenge:** Standard UART baud rates (`9600` baud) introduce significant serial buffer blocking when streaming dual-trace real-time waveforms at $100\text{ Hz}$, inducing noticeable servo latency.
* **Solution:** Reconfigured serial communication to `115200` baud ($11.5\text{ KB/s}$ throughput), freeing up CPU clock cycles to execute the moving-window loop smoothly with negligible control loop latency.
