#include "MatrixController.h"

// ============================================================================
// MatrixController Implementation (Lightweight base class)
// ============================================================================

MatrixController::MatrixController()
    : strip(0, 0, NEO_GRB + NEO_KHZ800),
      currentColor{0, 0, 255},
      maxBrightness(255),
      actualWidth(0),
      actualHeight(0)
#ifdef __AVR__
      , matrixData(nullptr),
      matrixRows(0),
      matrixCols(0) {
#else
      , ledMappings(nullptr),
      numActiveLeds(0) {
#endif
}

MatrixController::~MatrixController() {
#ifndef __AVR__
  if (ledMappings) {
    delete[] ledMappings;
    ledMappings = nullptr;
  }
#endif
}

void MatrixController::init(MatrixType type, int pin) {
  const uint8_t* sourceMatrix = nullptr;
  int sourceWidth = 0;
  int sourceHeight = 0;
  
  switch (type) {
    case ROUND:
      sourceMatrix = &ROUND_MATRIX[0][0];
      sourceWidth = 12;
      sourceHeight = 12;
      break;
    case HEXAGONAL:
      sourceMatrix = &HEXAGONAL_MATRIX[0][0];
      sourceWidth = 14;
      sourceHeight = 14;
      break;
    case TRIANGULAR:
      sourceMatrix = &TRIANGULAR_MATRIX[0][0];
      sourceWidth = 16;
      sourceHeight = 16;
      break;
    case CUSTOM:
      return; // Custom matrix requires the other init method
  }
  
#ifdef __AVR__
  // AVR: Store matrix reference directly to save RAM
  matrixData = sourceMatrix;
  matrixRows = sourceHeight;
  matrixCols = sourceWidth;
  actualWidth = sourceWidth;
  actualHeight = sourceHeight;
  
  // Count LEDs for strip initialization
  int ledCount = 0;
  for (int y = 0; y < sourceHeight; y++) {
    for (int x = 0; x < sourceWidth; x++) {
      uint8_t ledId = sourceMatrix[y * sourceWidth + x];
      if (ledId != 255) {
        ledCount = max(ledCount, (int)ledId + 1);
      }
    }
  }
  strip.setPin(pin);
  strip.begin();
  strip.updateLength(ledCount);
  strip.show();
#else
  // Non-AVR: Use sparse mapping for better performance
  convertToSparse(sourceMatrix, sourceHeight, sourceWidth);
  computeMatrixDimensions();
  initStrip(pin);
#endif
}

void MatrixController::init(const uint8_t* customMatrix, int matrixRows, int matrixCols, int pin) {
  convertToSparse(customMatrix, matrixRows, matrixCols);
  computeMatrixDimensions();
  initStrip(pin);
}

void MatrixController::setColor(uint8_t r, uint8_t g, uint8_t b) {
  currentColor[0] = r;
  currentColor[1] = g;
  currentColor[2] = b;
  
  uint8_t finalR = (uint16_t)r * maxBrightness / 255;
  uint8_t finalG = (uint16_t)g * maxBrightness / 255;
  uint8_t finalB = (uint16_t)b * maxBrightness / 255;
  
#ifdef __AVR__
  // AVR: Direct matrix lookup to save RAM
  for (int y = 0; y < matrixRows; y++) {
    for (int x = 0; x < matrixCols; x++) {
      uint8_t ledId = matrixData[y * matrixCols + x];
      if (ledId != 255) {
        strip.setPixelColor(ledId, strip.Color(finalR, finalG, finalB));
      }
    }
  }
#else
  // Non-AVR: Use sparse mapping for better performance
  for (int i = 0; i < numActiveLeds; i++) {
    strip.setPixelColor(ledMappings[i].ledId, strip.Color(finalR, finalG, finalB));
  }
#endif
  strip.show();
}

void MatrixController::setMaxBrightness(uint8_t brightness) {
  maxBrightness = brightness;
}

void MatrixController::show() {
  strip.show();
}

void MatrixController::clear() {
  strip.clear();
}

void MatrixController::setImage(const bool* image, int imageRows, int imageCols) {
  displayImage(image, imageRows, imageCols);
}

void MatrixController::setImage(const bool* image, int imageRows, int imageCols, uint8_t r, uint8_t g, uint8_t b) {
  currentColor[0] = r;
  currentColor[1] = g;
  currentColor[2] = b;
  displayImage(image, imageRows, imageCols);
}

uint8_t* MatrixController::getColor() {
  return currentColor;
}

void MatrixController::setPixelXY(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  if (x < 0 || y < 0) return;
  int8_t ledId = getLedId(x, y);
  if (ledId > -1) {
    uint8_t finalR = (uint16_t)r * maxBrightness / 255;
    uint8_t finalG = (uint16_t)g * maxBrightness / 255;
    uint8_t finalB = (uint16_t)b * maxBrightness / 255;
    strip.setPixelColor(ledId, strip.Color(finalR, finalG, finalB));
  }
}

#ifndef MATRIX_DISABLE_JSON
String MatrixController::getJson() {
  String json = "{";
  json += "\"width\":";
  json += String(actualWidth);
  json += ",\"height\":";
  json += String(actualHeight);
  json += ",\"color\":[";
  json += String(currentColor[0]);
  json += ",";
  json += String(currentColor[1]);
  json += ",";
  json += String(currentColor[2]);
  json += "],\"maxBrightness\":";
  json += String(maxBrightness);
  json += "}";
  return json;
}
#endif

// Private methods
void MatrixController::displayImage(const bool* image, int imageRows, int imageCols) {
  // Center image within matrix
  int matrixCenterX = actualWidth / 2;
  int matrixCenterY = actualHeight / 2;
  int imageCenterX = imageCols / 2;
  int imageCenterY = imageRows / 2;
  int offsetX = matrixCenterX - imageCenterX;
  int offsetY = matrixCenterY - imageCenterY;
  
  // Clear all pixels first
  strip.clear();
  
  uint8_t finalR = (uint16_t)currentColor[0] * maxBrightness / 255;
  uint8_t finalG = (uint16_t)currentColor[1] * maxBrightness / 255;
  uint8_t finalB = (uint16_t)currentColor[2] * maxBrightness / 255;
  
#ifdef __AVR__
  // AVR: Direct matrix lookup
  for (int y = 0; y < matrixRows; y++) {
    for (int x = 0; x < matrixCols; x++) {
      uint8_t ledId = matrixData[y * matrixCols + x];
      if (ledId != 255) {
        // Calculate image position with centering
        int imageX = x - offsetX;
        int imageY = y - offsetY;
        
        // Check if this pixel should be lit
        bool shouldLight = false;
        if (imageX >= 0 && imageX < imageCols && imageY >= 0 && imageY < imageRows) {
          shouldLight = image[imageY * imageCols + imageX];
        }
        
        if (shouldLight) {
          strip.setPixelColor(ledId, strip.Color(finalR, finalG, finalB));
        }
      }
    }
  }
#else
  // Non-AVR: Use sparse mapping
  for (int i = 0; i < numActiveLeds; i++) {
    int x = ledMappings[i].x;
    int y = ledMappings[i].y;
    
    // Calculate image position with centering
    int imageX = x - offsetX;
    int imageY = y - offsetY;
    
    // Check if this pixel should be lit
    bool shouldLight = false;
    if (imageX >= 0 && imageX < imageCols && imageY >= 0 && imageY < imageRows) {
      shouldLight = image[imageY * imageCols + imageX];
    }
    
    if (shouldLight) {
      strip.setPixelColor(ledMappings[i].ledId, strip.Color(finalR, finalG, finalB));
    }
  }
#endif
  
  strip.show();
}

void MatrixController::initStrip(int pin) {
  strip.setPin(pin);
  strip.begin();
  
#ifndef __AVR__
  // Non-AVR: Count actual LEDs needed from sparse mapping
  int ledCount = 0;
  for (int i = 0; i < numActiveLeds; i++) {
    ledCount = max(ledCount, (int)ledMappings[i].ledId + 1);
  }
  
  strip.updateLength(ledCount);
#endif
  strip.show();
}

void MatrixController::computeMatrixDimensions() {
#ifndef __AVR__
  // Non-AVR: Compute from sparse mapping
  int minX = 255, maxX = 0, minY = 255, maxY = 0;
  
  for (int i = 0; i < numActiveLeds; i++) {
    int x = ledMappings[i].x;
    int y = ledMappings[i].y;
    minX = min(minX, x);
    maxX = max(maxX, x);
    minY = min(minY, y);
    maxY = max(maxY, y);
  }
  
  actualWidth = maxX - minX + 1;
  actualHeight = maxY - minY + 1;
#endif
  // AVR: dimensions already set in init()
}

void MatrixController::convertToSparse(const uint8_t* sourceMatrix, int sourceRows, int sourceCols) {
#ifndef __AVR__
  // Non-AVR only: Create sparse mapping for performance
  // First pass: count active LEDs
  numActiveLeds = 0;
  for (int y = 0; y < sourceRows; y++) {
    for (int x = 0; x < sourceCols; x++) {
      // Read from PROGMEM
      uint8_t ledId = sourceMatrix[y * sourceCols + x];
      if (ledId != 255) {
        numActiveLeds++;
      }
    }
  }
  
  // Allocate sparse array
  if (ledMappings) {
    delete[] ledMappings;
  }
  ledMappings = new LedMapping[numActiveLeds];
  
  // Second pass: populate sparse array
  int index = 0;
  for (int y = 0; y < sourceRows; y++) {
    for (int x = 0; x < sourceCols; x++) {
      // Read from PROGMEM
      uint8_t ledId = sourceMatrix[y * sourceCols + x];
      if (ledId != 255) {
        ledMappings[index].x = x;
        ledMappings[index].y = y;
        ledMappings[index].ledId = ledId;
        index++;
      }
    }
  }
#endif
  // AVR: Skip sparse mapping to save RAM
}

int8_t MatrixController::getLedId(uint8_t x, uint8_t y) const {
#ifdef __AVR__
  // AVR: Direct matrix lookup
  if (x >= matrixCols || y >= matrixRows) return -1;
  uint8_t ledId = matrixData[y * matrixCols + x];
  return (ledId != 255) ? (int8_t)min(127, (int)ledId) : -1;
#else
  // Non-AVR: Use sparse mapping
  for (int i = 0; i < numActiveLeds; i++) {
    if (ledMappings[i].x == x && ledMappings[i].y == y) {
      return (int8_t)min(127, (int)ledMappings[i].ledId);
    }
  }
  return -1;
#endif
}

// ============================================================================
// MatrixBlender Implementation (Smooth transitions)
// ============================================================================

MatrixBlender::MatrixBlender(MatrixController* controller)
    : matrix(controller),
      currentImage(nullptr),
      targetImage(nullptr),
      blendTime(0),
      blendStartTime(0),
      blending(false),
      blendingImage(false),
      blendingColor(false) {
  
  // Allocate image buffers based on matrix size
  imageSize = controller->getWidth() * controller->getHeight();
  currentImage = new uint8_t[imageSize];
  targetImage = new uint8_t[imageSize];
  
  // Initialize arrays
  for (int i = 0; i < imageSize; i++) {
    currentImage[i] = 0;
    targetImage[i] = 0;
  }
  
  // Copy current color
  uint8_t* color = matrix->getColor();
  currentColor[0] = color[0];
  currentColor[1] = color[1];
  currentColor[2] = color[2];
}

MatrixBlender::~MatrixBlender() {
  if (currentImage) delete[] currentImage;
  if (targetImage) delete[] targetImage;
}

void MatrixBlender::blendToColor(uint8_t r, uint8_t g, uint8_t b, unsigned long duration) {
  targetColor[0] = r;
  targetColor[1] = g;
  targetColor[2] = b;
  
  blendTime = duration;
  blendStartTime = millis();
  blending = true;
  blendingColor = true;
  
  // Prime first frame
  blendUnified(0);
}

void MatrixBlender::blendToImage(const bool* image, int imageRows, int imageCols, unsigned long duration) {
  // Center and copy target image
  int matrixWidth = matrix->getWidth();
  int matrixHeight = matrix->getHeight();
  int matrixCenterX = matrixWidth / 2;
  int matrixCenterY = matrixHeight / 2;
  int imageCenterX = imageCols / 2;
  int imageCenterY = imageRows / 2;
  int offsetX = matrixCenterX - imageCenterX;
  int offsetY = matrixCenterY - imageCenterY;
  
  for (int y = 0; y < matrixHeight; y++) {
    for (int x = 0; x < matrixWidth; x++) {
      int imageX = x - offsetX;
      int imageY = y - offsetY;
      int index = y * matrixWidth + x;
      
      if (imageX >= 0 && imageX < imageCols && imageY >= 0 && imageY < imageRows) {
        targetImage[index] = image[imageY * imageCols + imageX] ? 255 : 0;
      } else {
        targetImage[index] = 0;
      }
    }
  }
  
  blendTime = duration;
  blendStartTime = millis();
  blending = true;
  blendingImage = true;
  
  // Prime first frame
  blendUnified(0);
}

void MatrixBlender::blendToImage(const bool* image, int imageRows, int imageCols, uint8_t r, uint8_t g, uint8_t b, unsigned long duration) {
  // Set up image blending
  blendToImage(image, imageRows, imageCols, duration);
  
  // Also set up color blending
  targetColor[0] = r;
  targetColor[1] = g;
  targetColor[2] = b;
  blendingColor = true;
}

void MatrixBlender::update() {
  if (!blending) return;
  
  unsigned long now = millis();
  unsigned long elapsed = now - blendStartTime;
  if (elapsed > blendTime) elapsed = blendTime;
  
  uint8_t factor = (blendTime > 0) ? (uint8_t)min(255UL, (elapsed * 255UL) / blendTime) : 255;
  blendUnified(factor);
  
  if (elapsed >= blendTime) {
    // Snap to exact targets
    if (blendingImage) {
      for (int i = 0; i < imageSize; i++) {
        currentImage[i] = targetImage[i];
      }
    }
    if (blendingColor) {
      currentColor[0] = targetColor[0];
      currentColor[1] = targetColor[1];
      currentColor[2] = targetColor[2];
    }
    blending = false;
    blendingImage = false;
    blendingColor = false;
  }
}

void MatrixBlender::blendUnified(uint8_t factor) {
  // Linear interpolation with proper signed math
  uint8_t t = factor;
  
  // Blend colors
  if (blendingColor) {
    for (int i = 0; i < 3; i++) {
      int16_t difference = (int16_t)targetColor[i] - (int16_t)currentColor[i];
      currentColor[i] = currentColor[i] + (difference * t) / 255;
    }
  }
  
  // Blend image
  if (blendingImage) {
    for (int i = 0; i < imageSize; i++) {
      int16_t difference = (int16_t)targetImage[i] - (int16_t)currentImage[i];
      currentImage[i] = currentImage[i] + (difference * t) / 255;
    }
  }
  
  // Render blended result
  int matrixWidth = matrix->getWidth();
  
  matrix->strip.clear();
  
#ifdef __AVR__
  // AVR: Direct matrix lookup
  for (int y = 0; y < matrix->matrixRows; y++) {
    for (int x = 0; x < matrix->matrixCols; x++) {
      uint8_t ledId = matrix->matrixData[y * matrix->matrixCols + x];
      if (ledId != 255) {
        int index = y * matrixWidth + x;
        
        uint8_t mask = blendingImage ? currentImage[index] : 255;
        uint8_t r = (uint16_t)currentColor[0] * mask / 255;
        uint8_t g = (uint16_t)currentColor[1] * mask / 255;
        uint8_t b = (uint16_t)currentColor[2] * mask / 255;
        
        // Apply global brightness
        r = (uint16_t)r * matrix->maxBrightness / 255;
        g = (uint16_t)g * matrix->maxBrightness / 255;
        b = (uint16_t)b * matrix->maxBrightness / 255;
        
        matrix->strip.setPixelColor(ledId, matrix->strip.Color(r, g, b));
      }
    }
  }
#else
  // Non-AVR: Use sparse mapping
  for (int i = 0; i < matrix->numActiveLeds; i++) {
    int x = matrix->ledMappings[i].x;
    int y = matrix->ledMappings[i].y;
    int index = y * matrixWidth + x;
    
    uint8_t mask = blendingImage ? currentImage[index] : 255;
    uint8_t r = (uint16_t)currentColor[0] * mask / 255;
    uint8_t g = (uint16_t)currentColor[1] * mask / 255;
    uint8_t b = (uint16_t)currentColor[2] * mask / 255;
    
    // Apply global brightness
    r = (uint16_t)r * matrix->maxBrightness / 255;
    g = (uint16_t)g * matrix->maxBrightness / 255;
    b = (uint16_t)b * matrix->maxBrightness / 255;
    
    matrix->strip.setPixelColor(matrix->ledMappings[i].ledId, matrix->strip.Color(r, g, b));
  }
#endif
  
  matrix->strip.show();
}