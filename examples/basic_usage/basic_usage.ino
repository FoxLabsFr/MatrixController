#include "MatrixController.h"

// Simple 6x6 images that work on any matrix size
const bool SMILEY[6][6] = {
    {0, 1, 0, 0, 1, 0},
    {0, 1, 0, 0, 1, 0},
    {0, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 1},
    {0, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 0, 0}
};

const bool HEART[6][6] = {
    {0, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 0, 0},
    {0, 0, 0, 0, 0, 0}
};

const bool CROSS[6][6] = {
    {0, 0, 1, 1, 0, 0},
    {0, 0, 1, 1, 0, 0},
    {1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1},
    {0, 0, 1, 1, 0, 0},
    {0, 0, 1, 1, 0, 0}
};

// Create matrix controller
MatrixController matrix;

void setup() {
  Serial.begin(115200);
  Serial.println("LEDMatrix Basic Usage - Immediate Display");
  
  // Initialize matrix (change type as needed: ROUND, HEXAGONAL, TRIANGULAR)
  matrix.init(MatrixController::HEXAGONAL, 6);
  
  // Set global brightness
  matrix.setMaxBrightness(128);
  
  // Start with red color
  matrix.setColor(255, 0, 0);
  
  Serial.println("Matrix ready!");
}

void loop() {
  // No update() needed for immediate display
  
  static int step = 0;
  static unsigned long lastChange = 0;
  
  if (millis() - lastChange > 2000) { // Change every 2 seconds
    
    switch (step) {
      case 0:
        matrix.setImage(SMILEY);
        matrix.setColor(255, 255, 0); // Yellow smiley
        Serial.println("Smiley (Yellow)");
        break;
      case 1:
        matrix.setImage(HEART);
        matrix.setColor(255, 0, 255); // Magenta heart
        Serial.println("Heart (Magenta)");
        break;
      case 2:
        matrix.setImage(CROSS);
        matrix.setColor(0, 255, 0); // Green cross
        Serial.println("Cross (Green)");
        break;
      case 3:
        matrix.setColor(0, 0, 255); // Blue solid
        Serial.println("Solid Blue");
        break;
    }
    
    step = (step + 1) % 4;
    lastChange = millis();
  }
  
  delay(50);
}