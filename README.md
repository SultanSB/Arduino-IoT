# Pandora's Box Game

## Overview
Pandora's Box is an interactive puzzle game that combines Arduino with IoT capabilities using NodeMCU and Firebase. The game involves multiple stages, each featuring different sensor-based challenges. Players interact with the game through various input devices, and their progress is updated in real-time on a connected web platform via Firebase. 

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
### Project Structure
- **Arduino Code (Finalcode.ino)** - Controls game logic, interacts with sensors, and sends data to NodeMCU.
- ** Firebase Uploader (uploadToFirebase.ino) ** - Handles data transmission from NodeMCU to Firebase.
- ** Web Interface (HTML.ino) ** - Retrieves Firebase data and displays game results.
#### Hardware
- Arduino Uno
- NodeMCU ESP8266
