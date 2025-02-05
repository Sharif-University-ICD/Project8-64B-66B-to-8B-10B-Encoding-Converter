# **64B/66B to 8B/10B Encoding Converter with Error Detection and Correction**

This project implements a real-time encoding converter using an Arduino board to transform data from **64B/66B encoding** (used in high-speed communication protocols like 10 Gigabit Ethernet) to **8B/10B encoding** (common in interfaces such as USB, PCIe, and SATA). The system also includes robust **error detection and correction mechanisms** to ensure reliable data transmission under fault-injected scenarios.

---

## **Key Features**
- **Encoding Conversion**: Converts 64B/66B encoded data to 8B/10B encoding in real-time using an Arduino board.
- **Error Injection**: Simulates bit flips, transient errors, and permanent faults to test system resilience.
- **Error Detection**: Uses Hamming codes and encoding-specific rules to detect errors in transmitted data.
- **Error Correction**: Corrects transient errors by restoring original values and permanent errors using Hamming code decoding.
- **Fault Analysis**: Provides detailed logs of detected errors, corrected values, and suspected faults.
---

## **Team Members**
- **Atefeh Ghandehari**
- **Arian Afzalzadeh**
- **Nika Ghaderi**

---

## **Demonstration Video Link**
[![Watch the presentation on Aparat](https://img.shields.io/badge/Watch%20on-Aparat-FF0000?style=for-the-badge&logo=aparat&logoColor=white)](https://aparat.com/v/xkaw65c)
