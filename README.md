# ESP32 Network Clock

A DIY ESP32-based digital clock with NTP synchronization, dual 7-segment displays, brightness auto-control, and UPS backup.

## Features

- ESP32 WiFi time synchronization (NTP)
- Automatic fallback clock when internet is unavailable
- Dual TM1637 displays
  - Time display
  - Date display
- LDR auto-brightness adjustment
- AM/PM indicator LED
- Noon and midnight buzzer alerts
- Boot hardware self-test
- Manual adjustment buttons
- RTC-ready architecture
- UPS battery backup support

## Hardware

- ESP32
- 2x TM1637 4-digit displays
- LDR sensor
- buzzer
- 6 buttons
- UPS module + 18650 battery
- perfboard construction

## Wiring

| Component | ESP32 Pin |
|-----------|-----------|
| Time Display CLK | 18 |
| Time Display DIO | 19 |
| Date Display CLK | 16 |
| Date Display DIO | 17 |
| Buzzer | 33 |
| LDR | 34 |
| AM/PM LED | 32 |
| Mode Button | 25 |
| Adjust Button | 26 |
| Set Button | 27 |
| Left Button | 14 |
| Right Button | 12 |
| Source Button | 13 |

## Controls

Mode Button:
- Double click → show year
- Hold 2 seconds → return to time

Date is always shown on the second display.

## License

MIT License
