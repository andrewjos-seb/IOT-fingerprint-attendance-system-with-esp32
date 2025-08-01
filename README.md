# 🧠 IoT Attendance Logger with Fingerprint & Google Sheets

This project is an **ESP32-based IoT attendance system** that uses a fingerprint sensor and logs attendance data to **Google Sheets** via HTTPS. It also features a **TFT display** for real-time status and feedback.

---

## 🛠️ Features

- 📡 Connects ESP32 to WiFi
- 🔐 Fingerprint authentication with Adafruit sensor library
- 📈 Real-time data logging to Google Sheets
- 🖥️ Onboard TFT feedback (status, prompts, results)
- 🔊 Buzzer alert for access status

---

## 📷 Hardware Used

- ESP32 Dev Board
- R305 Fingerprint Sensor
- 1.8” TFT Display (TFT_eSPI)
- Buzzer
- USB Cable and Breadboard

---

## 🧩 Libraries Required

- `WiFi.h`, `HTTPClient.h`, `WiFiClientSecure.h`
- `Adafruit_Fingerprint`
- `TFT_eSPI`

Install libraries via Arduino Library Manager or manually.

---

## 🧾 Google Apps Script

The `attendance_script.gs` handles:

- Name mapping from fingerprint ID
- Logs to two sheets:
  - **Sheet1**: Detailed logs
  - **Summary**: IN/OUT logic with timestamp and duration

Deploy it as a Web App and enable anonymous access.

---

## 🔧 How It Works

1. Place your finger on the scanner.
2. ESP32 reads fingerprint and verifies.
3. On success:
    - Buzzes
    - Logs data to Google Sheets
    - Displays verification on TFT
4. On failure:
    - Shows error
    - Buzzes differently

---

## 📝 Setup Instructions

1. Clone this repo.
2. Upload the Arduino code to your ESP32.
3. Modify:
    - WiFi credentials
    - Google Script URL ('sheetid'+?name=)
4. Deploy `attendance_script.gs` in your Google Apps Script Editor and modify sheet id.
5. Enjoy automated attendance logging!

---

## 📄 License

[MIT License](LICENSE) — feel free to use and modify.

---

## 👨‍💻 Author

**Andrewjos Sebastian**  
B.Tech Robotics and Automation | Passionate about ROS2, IoT, Autonomous Systems  
GitHub: https://github.com/andrewjos-seb

---

## 🌐 Keywords

ESP32, IoT, Fingerprint, Google Sheets, Attendance, TFT Display, Firebase alternative, Adafruit, R305, Embedded System
