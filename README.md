# Pandora's Box Game

## Overview
Pandora's Box is a **multi-stage interactive game** combining hardware (Arduino) and software (IoT/Web). Players progress through 5 unique stages, with real-time data synced to Firebase and displayed on a web dashboard. 

## Features
### Hardware & Gameplay
- **Stage 1**: Pulse Sensor measures heart rate (BPM) to unlock the game.  
- **Stage 2**: Flappy Bird clone with physics-based movement on a 1.8" TFT display.  
- **Stage 3**: Reaction-time test requiring precise button presses under time limits.  
- **Stage 4**: Potentiometer-controlled dodging game with collision detection.  
- **Stage 5**: Color-matching puzzle with randomized levels and score tracking. 

### IoT Integration  
- **Firebase Realtime Database**: Stores scores, stage times, and total points.  
- **NodeMCU (ESP8266)**: Transmits data from Arduino to Firebase via Wi-Fi.  
- **Web Dashboard**: Built with HTML/CSS/JavaScript to display live stats, total time, and stage progression.  
#### Hardware
- Arduino Uno
- NodeMCU ESP8266
