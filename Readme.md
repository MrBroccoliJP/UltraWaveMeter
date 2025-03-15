 
# UltraWaveMeter 🌊📏

**UltraWaveMeter** is an Arduino-based ultrasonic measurement system designed to measure distances and detect waves using the **HC-SR04** ultrasonic sensor. It is built on the **ESP8266** microcontroller and supports real-time data logging, temperature-compensated calculations, and serial interface control.

## About the Project  

This project was developed in collaboration with the [**Department of Civil Engineering at the University of Aveiro**](https://www.ua.pt/pt/decivil/). It was designed as a measurement device to analyze wave behavior in an artificial canal. The collected wave data was used to study the effects of different materials and techniques for **wave energy dissipation**, specifically in structures like breakwaters and seawalls—such as the **Paredão de Aveiro**.  

The module was custom-built to be attached to the **artificial canal** used for the experiment. Waves were artificially generated using a mechanical system, allowing researchers to test various **frequencies and amplitudes**.  

Upon completion, the module was delivered to a faculty professor along with an **instruction manual**, ensuring that it could be reused and easily adapted for future experiments.  

A prototype version of the module featured **Wi-Fi connectivity**, enabling remote data transmission and control. However, due to time constraints and the complexity of **network configurations (IP addresses, Wi-Fi connections, etc.)**, this feature was not included in the final version.  


## Features
- Uses an **HC-SR04** ultrasonic sensor for accurate distance measurements.
- Integrated with an **ESP8266** microcontroller.
- Temperature compensation for precise readings.
- Real-time serial monitoring and logging.
- Adjustable sampling rate for flexible measurements.
- Supports button and serial commands for operation.

## Components
| Quantity | Component        | Model         |
|----------|-----------------|--------------|
| 1        | Microcontroller  | ESP8266      |
| 1        | Ultrasonic Sensor | HC-SR04     |
| 1        | Red LED         | Generic      |
| 1        | Resistor (330Ω) | Generic      |
| 3        | Push Button     | Generic      |
| 1        | USB Cable (Micro-USB)* | Generic |

###### *depends on the dev board used
## Wiring Diagram
The setup consists of connecting the **HC-SR04** sensor to the **ESP8266**, along with buttons for user input and an LED indicator. Ensure correct wiring to avoid malfunctioning.



## Usage Instructions
1. **Position the module** above the area to be measured.
2. **Connect** the USB cable to a computer.
3. **Open a Serial Monitor** (e.g., Arduino IDE) and set the baud rate to **9600**.
4. **Follow on-screen instructions:**
   - Enter the ambient temperature when prompted.
5. **Start measurement:**
   - Press **Button 1**, **Button 2**, or enter **'S'** in the serial monitor to begin.
   - The default sampling interval is **40ms**.
   - To change the sampling interval, enter **'N'** and specify a value **>40ms**.
6. **Stop measurement:**
   - Press **Button 1**, **Button 2**, or enter **'S'** in the serial monitor.
7. **Save the Data:**
   - Copy the serial monitor output and save it for further analysis.
8. **Reset the system:**
   - Enter **'R'** in the serial monitor or press the reset button on the ESP8266.

## How the Sensor Works
The **HC-SR04** ultrasonic sensor emits a **40 kHz** sound wave that reflects off objects. The sensor calculates the time taken for the wave to return and determines the distance using the formula:

$$
\text{Distance (cm)} = \frac{\text{Speed of Sound (cm/μs)} \times \text{Echo Signal Duration (μs)}}{2}
$$  

With temperature compensation:  

$$
\text{Distance} = 0.034 \times \frac{\text{Echo}}{2} \times \frac{1 + T}{273.15} \times 60.368
$$  

## Software
The system is programmed using **Arduino IDE** with the **NewPing** library by Tim Eckel. The library simplifies sensor interactions and improves accuracy.

### Useful Links
- [Arduino NewPing Library](https://www.arduino.cc/reference/en/libraries/newping/)
- [NewPing Documentation](https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home)


------
## 📜 License
Developed by **João Fernandes**  

This project is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License (CC BY-NC 4.0)

UltraWaveMeter © 2022-2025 by João Fernandes is licensed under Creative Commons Attribution-NonCommercial 4.0 International

To view a copy of this license, visit: http://creativecommons.org/licenses/by-nc/4.0/

or send a letter to:

Creative Commons
PO Box 1866  
Mountain View,    
CA 94042  
USA  

In summary (not a substitute for the full license):

 - You are free to share and adapt the material for non-commercial purposes
 - You must give appropriate credit and indicate if changes were made
 - You may not use the material for commercial purposes without specific permission
