# MIT License
# Copyright (c) 2025 Benb0jangles
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define I2C_ADDRESS 0x3C

// Pin definitions
#define SDA_PIN 5   // GPIO5 (D1)
#define SCL_PIN 4   // GPIO4 (D2)
#define BUTTON_FLAP 0   // GPIO0 (Flash button)

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Game Variables - Modified values marked with comments
float bird_y = 32;
float bird_velocity = 0;
const float gravity = 0.3;          // Reduced gravity for better control
const float flap_force = -2.0;      // 60% smaller jump force (originally -5.0)
const int bird_size = 3;            // 50% smaller bird (originally 6)

int pipe_width = 20;
int pipe_gap = 24;                  // Slightly smaller gap to match bird size
int pipe_x = SCREEN_WIDTH;
int pipe_height = 0;
int pipe_gap_position = 0;

int score = 0;
bool game_over = false;
unsigned long last_update = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_FLAP, INPUT_PULLUP);
  
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(I2C_ADDRESS, true);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  randomSeed(analogRead(A0));
  generateNewPipe();
}

void generateNewPipe() {
  pipe_gap_position = random(16, SCREEN_HEIGHT - 16 - pipe_gap);
  pipe_x = SCREEN_WIDTH;
}

void drawBird() {
  // Draw smaller bird using fillRect instead of circle for precision
  display.fillRect(12, bird_y - bird_size, 6, bird_size * 2, SH110X_WHITE);
}

void drawPipes() {
  // Upper pipe
  display.fillRect(pipe_x, 0, pipe_width, pipe_gap_position, SH110X_WHITE);
  // Lower pipe
  display.fillRect(pipe_x, pipe_gap_position + pipe_gap, pipe_width, 
                 SCREEN_HEIGHT - (pipe_gap_position + pipe_gap), SH110X_WHITE);
}

bool checkCollision() {
  // Adjusted collision detection for smaller bird
  if (pipe_x < 18 && pipe_x + pipe_width > 12) {
    if (bird_y - bird_size < pipe_gap_position || 
        bird_y + bird_size > pipe_gap_position + pipe_gap) {
      return true;
    }
  }
  if (bird_y - bird_size < 2 || bird_y + bird_size > SCREEN_HEIGHT - 2) {
    return true;
  }
  return false;
}

void gameOverScreen() {
  display.clearDisplay();
  display.setCursor(32, 20);
  display.print("Game Over!");
  display.setCursor(32, 35);
  display.print("Score: ");
  display.print(score);
  display.setCursor(16, 50);
  display.print("Press to restart");
  display.display();
}

void resetGame() {
  bird_y = 32;
  bird_velocity = 0;
  score = 0;
  pipe_x = SCREEN_WIDTH;
  game_over = false;
  generateNewPipe();
}

void loop() {
  if (game_over) {
    gameOverScreen();
    if (digitalRead(BUTTON_FLAP) == LOW) {
      delay(200);
      resetGame();
    }
    return;
  }

  if (millis() - last_update > 33) {
    // Modified physics for tighter control
    bird_velocity += gravity * 0.75;    // Reduced gravity effect
    bird_y += bird_velocity * 0.8;       // Damped vertical movement
    
    pipe_x -= 2.5;  // Slightly faster pipe movement
    
    if (pipe_x + pipe_width < 0) {
      generateNewPipe();
      score++;
    }

    if (checkCollision()) {
      game_over = true;
    }

    if (digitalRead(BUTTON_FLAP) == LOW) {
      bird_velocity = flap_force;
      // Add small visual feedback
      display.fillRect(12, bird_y - bird_size, 6, bird_size * 2, SH110X_BLACK);
    }

    display.clearDisplay();
    drawPipes();
    drawBird();
    
    display.setCursor(2, 2);
    display.print("Score: ");
    display.print(score);
    
    display.display();
    
    last_update = millis();
  }
}
