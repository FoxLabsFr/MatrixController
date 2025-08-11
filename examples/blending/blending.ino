#include "MatrixController.h"

// Simple 6x6 patterns for smooth blending
const bool CIRCLE[6][6] = {
    {0, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1},
    {1, 1, 0, 0, 1, 1},
    {1, 1, 0, 0, 1, 1},
    {1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 0}
};

const bool DIAMOND[6][6] = {
    {0, 0, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 0, 0}
};

// Create matrix controller and blender
MatrixController matrix;
MatrixBlender blender(&matrix);

void setup() {
  Serial.begin(115200);
  Serial.println("LEDMatrix Advanced Blending - Smooth Transitions");
  
  // Initialize matrix (works with any type: ROUND, HEXAGONAL, TRIANGULAR)
  matrix.init(MatrixController::ROUND, 6);
  
  // Set brightness
  matrix.setMaxBrightness(192);
  
  // Start with smooth fade to blue
  blender.blendToColor(0, 100, 255, 2000);
  
  Serial.println("Starting smooth animations...");
}

void loop() {
  // REQUIRED: Update blender for smooth transitions
  // Call this as often as possible - NO delay()!
  blender.update();
  
  static int step = 0;
  static unsigned long lastChange = 0;
  
  if (millis() - lastChange > 3000) { // Change every 3 seconds
    
    switch (step) {
      case 0:
        // Blend to red color
        blender.blendToColor(255, 0, 0, 2000);
        Serial.println("Blending to Red");
        break;
      case 1:
        // Blend to circle pattern with green color
        blender.blendToImage(CIRCLE, 0, 255, 0, 2500);
        Serial.println("Blending to Green Circle");
        break;
      case 2:
        // Blend to yellow color
        blender.blendToColor(255, 255, 0, 2000);
        Serial.println("Blending to Yellow");
        break;
      case 3:
        // Blend to diamond pattern with magenta color
        blender.blendToImage(DIAMOND, 255, 0, 255, 2500);
        Serial.println("Blending to Magenta Diamond");
        break;
    }
    
    step = (step + 1) % 4;
    lastChange = millis();
  }
  
  // NO delay() - let blender.update() run as fast as possible for smooth animations
}
