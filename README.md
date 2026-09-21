# TraceX

### Smart Tracking and Safety System for People and Valuables in Low-Connectivity Environments

**TraceX** is an adaptive tracking system designed for environments where reliable internet connectivity cannot always be assumed.

Instead of depending on a single communication technology, TraceX combines **Bluetooth Low Energy (BLE), GPS, cellular communication, motion sensing, local storage, and cloud connectivity**.

The goal is simple:

> **Keep a tracker useful even when one communication method or network becomes unavailable.**


## 🎥 Demo

**[▶️ Watch the TraceX Demo on YouTube](https://youtu.be/xiwYHVMmqJw?si=rSOID5MraY7NTpp4)**

The demo shows the current working prototype, including:

* Web Bluetooth connection
* Live tracker data
* Motion detection
* GPS acquisition
* Cellular communication
* Location history
* Map visualization
* Battery monitoring
* Remote buzzer/recovery
* Offline data logging


## 📸 Project

<!-- Add your best prototype photo here -->

![TraceX Prototype](images/tracex-prototype.jpeg)

TraceX is being developed as a small wearable or attachable tracker for **people and valuables**.

The system is designed around the reality that connectivity is not always available or reliable.


# The Problem

Many existing consumer trackers depend heavily on surrounding smartphone networks or continuous internet connectivity.

For example, a Bluetooth-based tracker may be useful when compatible devices are nearby, but its usefulness can decrease significantly when those devices or network infrastructure are unavailable.

Long-range technologies such as LoRa can provide low-power communication over large distances, but a deployment may require accessible gateway infrastructure.

This creates an interesting engineering problem:

**Can a tracker adapt its communication method depending on the environment around it?**

That is the problem TraceX is exploring.


# The Idea

TraceX combines multiple communication and sensing technologies:

```text
                    ┌─────────────────────┐
                    │       User          │
                    │      Phone/Web      │
                    └──────────┬──────────┘
                               │
                              BLE
                               │
                               ▼
                    ┌─────────────────────┐
                    │      ESP32-C3       │
                    │                     │
                    │ BLE                 │
                    │ Motion Detection    │
                    │ Battery             │
                    │ Local Storage       │
                    │ Device Logic        │
                    └──────┬───────┬──────┘
                           │       │
                          I2C     UART
                           │       │
                           ▼       ▼
                     ┌─────────┐ ┌────────────┐
                     │ MPU6050 │ │ SIM7600G   │
                     │ Motion  │ │ GPS + GSM  │
                     └─────────┘ └─────┬──────┘
                                       │
                                  Cellular
                                       │
                                       ▼
                              ┌─────────────────┐
                              │ Firebase RTDB   │
                              │ Location Data   │
                              └─────────────────┘
```

The tracker does not have to use every system at the same time.

For example:

### 📱 Owner nearby

**BLE connected**

The tracker communicates with the owner's phone.

* BLE is used for communication
* Phone can provide location
* Tracker data is displayed locally
* Cellular/GPS activity can be minimized

### 📡 BLE connection lost

**GPS + Cellular**

The tracker can obtain its own location and transmit it through the cellular network.

### 📦 Cellular connection unavailable

**Local storage**

The tracker stores location records locally instead of immediately losing the data.

When connectivity becomes available again, the stored records can be synchronized.

### 🚨 SOS

The system can prioritize obtaining the tracker's location and communicating it for emergency assistance.



# Current Features

## 🔵 Bluetooth Low Energy

TraceX uses BLE for short-range communication between the tracker and the user's device.

The current prototype supports:

* BLE advertising
* BLE connection
* BLE read/write characteristics
* Device status
* Sensor data
* Battery data
* Web Bluetooth


## 🌐 Web Bluetooth

The dashboard communicates directly with the ESP32 using **Web Bluetooth**.

This is important because the dashboard does not necessarily need internet access to communicate with the tracker.

As long as the ESP32 is powered and the browser supports Web Bluetooth, the user can connect directly to the device.

The dashboard currently displays:

* GPS status
* Latitude
* Longitude
* Altitude
* GPS time
* Motion status
* Accelerometer data
* Gyroscope data
* Battery percentage


## 📍 GPS + Cellular Tracking

The current prototype uses a **SIM7600G** for GPS and cellular communication.

GPS data includes:

* Latitude
* Longitude
* Altitude
* Date
* Time
* Fix status

When cellular connectivity is available, location records can be sent to the cloud backend.

> The production hardware is planned around the smaller **SIM7080G**, but the current prototype uses the SIM7600G.

---

## 🏃 Motion Detection

The prototype currently uses an **MPU6050** accelerometer and gyroscope.

Motion detection combines:

* Linear acceleration
* Angular velocity

Current prototype thresholds:

```text
Acceleration threshold: 0.35
Gyroscope threshold:    0.80
```

The sensor is used to distinguish between movement and stationary states.

A **BMA400** is planned for the next hardware revision because of its lower-power operation and built-in motion/wakeup capabilities.


## 🔋 Battery Monitoring

The prototype can read and display the device's battery percentage.

A dedicated **MAX17048 fuel gauge** is planned for a future hardware revision.


## 🔊 Recovery / Play Sound

When the tracker is within BLE range, the user can trigger the device's buzzer from the dashboard.

This is intended to help locate a lost item.

The play-sound function is restricted to BLE-connected operation.


## 📌 Last Seen Location

TraceX maintains a last-seen record.

When BLE is connected, the system can use the phone's location and timestamp.

When BLE communication is lost, the latest known location can be retained as the tracker's last-seen location.

The last-seen information is persisted locally on the ESP32.



## 🗃️ Offline Data Logging

One of the core ideas behind TraceX is that **temporary connectivity loss should not automatically mean data loss**.

When cellular connectivity is unavailable, GPS records can be stored locally in the ESP32's flash storage.

When connectivity becomes available again, queued records can be synchronized to the cloud.

```text
GPS Record
    │
    ▼
Cellular Available?
    │
 ┌──┴───────┐
 │          │
YES         NO
 │          │
 ▼          ▼
Cloud     Local
Storage   Storage
 │          │
 │      Connectivity
 │         returns
 │          │
 └────┬─────┘
      ▼
 Cloud Synchronization
```

The ESP32's flash storage is used for the local queue, while persistent device state is handled separately.

---

# Location History

TraceX records historical location information such as:

* Latitude
* Longitude
* Timestamp
* GPS status
* Source/status information

The prototype uses **Firebase Realtime Database** as the cloud backend.


# 🗺️ Map

The dashboard uses **Leaflet** to visualize tracker locations.

![TraceX Dashboard](images/dashboard.png)

<!-- Add a screenshot of your dashboard here -->


# Hardware

## Current Prototype

| Component               | Purpose                      |
| ----------------------- | ---------------------------- |
| **ESP32-C3 Super Mini** | Main microcontroller         |
| **SIM7600G**            | GPS + cellular communication |
| **MPU6050**             | Accelerometer + gyroscope    |
| **Buzzer**              | Recovery/audio feedback      |
| **Battery**             | Prototype power              |
| **Antenna**             | Cellular/GPS communication   |

## Planned Hardware Revision

| Component                 | Planned Use                 |
| ------------------------- | --------------------------- |
| ESP32-C3 module           | Main MCU                    |
| SIM7080G                  | Compact GPS + cellular      |
| BMA400                    | Low-power motion sensing    |
| MAX17048                  | Battery fuel gauge          |
| TP4056 / charging circuit | Battery charging            |
| Custom PCB                | Smaller integrated hardware |
| Compact LiPo              | Improved battery capacity   |

The current prototype and the planned production hardware are intentionally different. The current prototype is being used to validate the system architecture before moving to a smaller custom PCB.


# Software Stack

### Embedded

* C++
* PlatformIO
* Arduino framework
* ESP32-C3
* NimBLE
* LittleFS
* Preferences

### Web Dashboard

* HTML
* CSS
* JavaScript
* Web Bluetooth
* Leaflet

### Cloud

* Firebase Realtime Database


# Firmware Architecture

The firmware is split into modules so that each subsystem can be developed and tested independently.

```text
src/
├── main.cpp
│
├── ble.cpp
├── ble.h
│
├── callbacks.cpp
├── callbacks.h
│
├── characteristics.cpp
├── characteristics.h
│
├── gps.cpp
├── gps.h
│
├── gsm.cpp
├── gsm.h
│
├── sim7600.cpp
├── sim7600.h
│
├── mpu6050.cpp
├── mpu6050.h
│
├── battery.cpp
├── battery.h
│
├── storage.cpp
├── storage.h
│
├── pins.h
└── uuids.h
```

This modular structure makes it easier to replace individual hardware components later.


# ESP32-C3 Connections

### UART

```text
ESP32-C3 GPIO20 → SIM7600 TX
ESP32-C3 GPIO21 → SIM7600 RX
```

### I2C

```text
ESP32-C3 GPIO8 → MPU6050 SDA
ESP32-C3 GPIO9 → MPU6050 SCL
```

The prototype uses I2C for the MPU6050.

---

# Development Progress

### Implemented

* [x] BLE initialization
* [x] BLE read/write
* [x] Web Bluetooth
* [x] Local dashboard communication
* [x] MPU6050 initialization
* [x] MPU6050 calibration
* [x] Motion detection
* [x] Last-seen location
* [x] Persistent last-seen storage
* [x] Battery percentage
* [x] Remote buzzer
* [x] SIM7600G integration
* [x] GPS acquisition
* [x] Cellular communication
* [x] Firebase integration
* [x] Leaflet map
* [x] Location history
* [x] Offline location logging
* [x] Offline synchronization logic
* [x] Modular firmware
* [x] Modular dashboard JavaScript

### In Progress / Planned

* [ ] Custom PCB
* [ ] SIM7080G integration
* [ ] BMA400 integration
* [ ] MAX17048 battery gauge
* [ ] Improved power management
* [ ] Compact enclosure
* [ ] Improved tamper detection
* [ ] eSIM support
* [ ] Solar-assisted charging



# Design Decisions

### Why GSM instead of LoRa?

TraceX is intended for environments where cellular infrastructure is more readily accessible than dedicated LoRa gateway infrastructure.

Using cellular communication allows the prototype to communicate over existing mobile networks without requiring TraceX to deploy its own gateway.

### Why BLE?

BLE provides short-range communication with relatively low power consumption and is supported directly by the ESP32-C3.

It also makes it possible to build a local browser-based interface using Web Bluetooth.

### Why local storage?

A tracker should not automatically lose information simply because the network temporarily disappears.

Local storage provides a buffer between data collection and cloud synchronization.

### Why ESP32-C3?

The ESP32-C3 provides the combination of:

* BLE
* Processing
* GPIO
* UART
* I2C
* Flash storage

in a small and inexpensive development platform.

---

# Limitations

The current prototype has limitations:

* GPS accuracy decreases indoors.
* Cellular tracking requires network availability.
* BLE recovery requires the tracker to be within Bluetooth range.
* Battery life depends on GPS/cellular update frequency.
* The current prototype uses development boards rather than a custom PCB.
* Some planned low-power hardware has not yet been integrated.

---

# Roadmap

### Phase 1 — Prototype

* BLE
* Web Bluetooth
* Motion sensing
* GPS
* Cellular communication
* Cloud storage
* Location history
* Offline logging

### Phase 2 — Hardware

* Custom PCB
* SIM7080G
* BMA400
* MAX17048
* Better power management
* Compact enclosure

### Phase 3 — Product

* Smaller wearable/attachable form factor
* Improved battery life
* eSIM
* Improved emergency communication
* Production-ready enclosure
* Field testing



# Project Goal

TraceX is not trying to assume that connectivity will always be available.

It is exploring a different approach:

> **Build the tracker around the reality of unreliable connectivity.**

BLE can handle nearby communication.

GPS can provide positioning.

Cellular networks can provide wider-area communication.

Local storage can preserve information when the network disappears.

Together, these systems create an adaptive tracking architecture that can continue operating across different connectivity conditions.



# Author

**Success Asokere**

Embedded Systems & IoT

Nigeria


