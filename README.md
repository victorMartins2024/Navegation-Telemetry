# **Telemetry**
 
 
# *Project Description*
 
The goal is to develop a navigation system for equipment, with four distinct user classes. The Operator is responsible for performing equipment checklists, verifying its usage status. The Technician has the role of updating the equipment status during maintenance processes, assigning each one its corresponding state. The Administrator is in charge of registering and deleting cards in the system. Engineering is responsible for formatting the database of cards and, optionally, deleting specific card units. Additionally, the first three users have access to their respective functionalities through RFID card reading, while the last user has more restricted access, using a password system. The system is also capable of reading analog data, which is published to an MQTT broker in JSON format. These data can be exported to Prometheus and visualized on a Grafana dashboard.
 
# *Project Features*
 
1. Read each user's RFID cards.
2. Infortation on the display 
3. password of system to engineering
4. Publish all necessary information to the broker
5. Operator checklist
6. Change the machine status
 
# *Technologies Used*
 
1. ESP32
2. MQTT
3. WiFi
4. RTOS
5. Arduino component
6. Raspberry pi3
7. Display 
8. keypad 
9. RFID
 
# *General Information*
 
Compiler: VsCode 1.89.1  <br/>
MCU: ESP32  <br/>
Board: Dev module 38 pins <br/>
Date: 2024, May <br/>
Author: [@victorMartins2024](https://github.com/victorMartins2024)
 
# *Badges*
 
![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Espressif](https://img.shields.io/badge/espressif-E7352C.svg?style=for-the-badge&logo=espressif&logoColor=white)
![Mosquitto](https://img.shields.io/badge/mosquitto-%233C5280.svg?style=for-the-badge&logo=eclipsemosquitto&logoColor=white)
![Prometheus](https://img.shields.io/badge/Prometheus-E6522C?style=for-the-badge&logo=Prometheus&logoColor=white)
![Grafana](https://img.shields.io/badge/grafana-%23F46800.svg?style=for-the-badge&logo=grafana&logoColor=white)
![Raspberry Pi](https://img.shields.io/badge/-RaspberryPi-C51A4A?style=for-the-badge&logo=Raspberry-Pi)