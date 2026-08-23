# Solar-Powered IoT System for Real-Time Water Quality Monitoring in Aquaculture 🐟📡

This repository contains the hardware architecture and embedded software source code for an IoT-based remote monitoring system designed for aquaculture facilities. The project was developed during an academic research internship at the **Department of Electronics and Communication Engineering, Graphic Era University** (Dehradun, India) in collaboration with AIESEC.

📄 **[Click here to read the full Research Paper / Project Report](Solar-Powered_IoT_System_for_Real-Time_Water_Quality_Monitoring_in_Aquaculture.pdf)**

## 📌 Project Overview
The primary objective of this project is to eliminate the need for manual on-site water quality checks in fish farms. By utilizing an **ESP32 microcontroller** integrated with turbidity and temperature sensors, the system continuously reads water quality metrics and transmits them to the **Thinger.io** cloud platform via Wi-Fi. The entire node is powered by a custom solar energy infrastructure, ensuring sustainable, off-grid, and uninterrupted remote operation.

## ⚙️ Key Features & Technologies
* **Microcontroller:** ESP32 (Wi-Fi enabled)
* **Sensors:** Analog Turbidity Sensor, DS18B20 Digital Temperature Sensor
* **Cloud IoT Platform:** Thinger.io (Real-time dashboard and data logging)
* **Power Management:** Solar panel integrated with a custom voltage divider circuit
* **Programming:** Embedded C/C++ 

---

## 📸 System Architecture & Hardware Implementation
The hardware setup encompasses a custom-designed PCB and a solar panel infrastructure engineered for outdoor endurance.

<p align="center">
  <img src="assets/41.jpeg" width="45%" alt="Solar Panel Setup">
  <img src="assets/81.jpeg" width="45%" alt="Custom PCB Design">
</p>

## ☁️ Thinger.io Cloud Dashboard Integration
Sensor data is securely transmitted to the cloud in real-time, providing an intuitive dashboard for the remote monitoring of the aquatic environment.

<p align="center">
  <img src="assets/1.1.png" width="80%" alt="Thinger.io Dashboard">
</p>

## 📊 Data Analysis & Results
The collected sensor data is logged and visualized to systematically track water quality trends over time.

<p align="center">
  <img src="assets/6.6.png" width="45%" alt="Voltage / Sensor Data Graph">
  <img src="assets/93.png" width="45%" alt="Data Log Table">
</p>

---

## 📂 Repository Structure
* `/source`: Contains the main `.ino` / `.cpp` source files for the ESP32.
* `/assets`: Circuit schematics, PCB layout photos, and Thinger.io dashboard screenshots.
* `Solar-Powered IoT System...pdf`: The detailed methodology and official project documentation.


## ⚙️ Key Features & Technologies
* **Microcontroller:** ESP32 (Wi-Fi enabled)
* **Sensors:** Analog Turbidity Sensor, DS18B20 Digital Temperature Sensor
* **Cloud IoT Platform:** Thinger.io (Real-time dashboard and data logging)
* **Power Management:** Solar panel integrated with a custom voltage divider circuit
* **Programming Language:** C/C++ 


## 🚀 How to Use / Setup Instructions
1. Clone this repository to your local machine:
   ```bash
   git clone [https://github.com/yunus-kunduz/esp32-aquaculture-iot.git](https://github.com/yunus-kunduz/esp32-aquaculture-iot.git)

2. Open the `source` folder using VS Code or Arduino IDE.
3. Update the `WIFI_SSID`, `WIFI_PASSWORD`, and `THINGER_DEVICE_CREDENTIALS` in the main code.
4. Upload the code to your ESP32 board.