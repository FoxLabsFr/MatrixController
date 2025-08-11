#ifndef MATRIXCONTROLER_H
#define MATRIXCONTROLER_H

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include "matrix_mappings.h"

// Helper functions for color manipulation
inline uint8_t getR(uint32_t color) { return (color >> 16) & 0xFF; }
inline uint8_t getG(uint32_t color) { return (color >> 8) & 0xFF; }
inline uint8_t getB(uint32_t color) { return color & 0xFF; }

// Forward declaration for blender
class MatrixBlender;

// Lightweight base matrix controller - no templates, no blending
class MatrixController {
public:
  // Matrix type enumeration
  enum MatrixType {
    ROUND,
    HEXAGONAL,
    TRIANGULAR,
    CUSTOM
  };

  // Constructor
  MatrixController();
  
  // Destructor
  ~MatrixController();
  
  // Initialize with matrix type and pin
  void init(MatrixType type, int pin);
  
  // Initialize with custom matrix and pin
  void init(const uint8_t* customMatrix, int matrixRows, int matrixCols, int pin);
  
  // Basic immediate operations (no blending)
  void setColor(uint8_t r, uint8_t g, uint8_t b);
  void setMaxBrightness(uint8_t brightness);
  void show();
  void clear();
  
  // Image display (immediate, no blending)
  void setImage(const bool* image, int imageRows, int imageCols);
  void setImage(const bool* image, int imageRows, int imageCols, uint8_t r, uint8_t g, uint8_t b);
  
  // Template overloads for 2D arrays
  template<int IMG_ROWS, int IMG_COLS>
  void setImage(const bool (&image)[IMG_ROWS][IMG_COLS]) {
    setImage(&image[0][0], IMG_ROWS, IMG_COLS);
  }
  
  template<int IMG_ROWS, int IMG_COLS>
  void setImage(const bool (&image)[IMG_ROWS][IMG_COLS], uint8_t r, uint8_t g, uint8_t b) {
    setImage(&image[0][0], IMG_ROWS, IMG_COLS, r, g, b);
  }
  
  // Getters
  uint8_t* getColor();
  int getWidth() const { return actualWidth; }
  int getHeight() const { return actualHeight; }
  
  // JSON methods (optional - can be disabled with MATRIX_DISABLE_JSON)
#ifndef MATRIX_DISABLE_JSON
  String getJson();
#endif

  // Per-pixel helpers (logical coordinates)
  void setPixelXY(int x, int y, uint8_t r, uint8_t g, uint8_t b);
  
  // Friend class for blender access
  friend class MatrixBlender;

private:
  void displayImage(const bool* image, int imageRows, int imageCols);
  void initStrip(int pin);
  void computeMatrixDimensions();
  void convertToSparse(const uint8_t* sourceMatrix, int sourceRows, int sourceCols);
  int8_t getLedId(uint8_t x, uint8_t y) const;

  Adafruit_NeoPixel strip;
  uint8_t currentColor[3];
  uint8_t maxBrightness;
  
  // Computed matrix dimensions
  int actualWidth;
  int actualHeight;
  
  // Sparse LED mapping structure
  struct LedMapping {
    uint8_t x, y;
    uint16_t ledId;
  };
  
#ifdef __AVR__
  // AVR platforms (Arduino Nano/Uno): Disable sparse mapping to save RAM
  // Use direct matrix lookup instead (slower but saves ~576 bytes)
  const uint8_t* matrixData;
  uint8_t matrixRows, matrixCols;
#else
  // Non-AVR platforms (ESP32/ESP8266): Use sparse mapping for better performance
  LedMapping* ledMappings;
  uint8_t numActiveLeds;
#endif
  

};

// Separate blending class for smooth transitions
class MatrixBlender {
public:
  MatrixBlender(MatrixController* controller);
  ~MatrixBlender();
  
  // Blending operations
  void blendToColor(uint8_t r, uint8_t g, uint8_t b, unsigned long duration);
  void blendToImage(const bool* image, int imageRows, int imageCols, unsigned long duration);
  void blendToImage(const bool* image, int imageRows, int imageCols, uint8_t r, uint8_t g, uint8_t b, unsigned long duration);
  
  // Template overloads for 2D arrays
  template<int IMG_ROWS, int IMG_COLS>
  void blendToImage(const bool (&image)[IMG_ROWS][IMG_COLS], unsigned long duration) {
    blendToImage(&image[0][0], IMG_ROWS, IMG_COLS, duration);
  }
  
  template<int IMG_ROWS, int IMG_COLS>
  void blendToImage(const bool (&image)[IMG_ROWS][IMG_COLS], uint8_t r, uint8_t g, uint8_t b, unsigned long duration) {
    blendToImage(&image[0][0], IMG_ROWS, IMG_COLS, r, g, b, duration);
  }
  
  void update();  // Call this in loop when blending
  bool isBlending() const { return blending; }

private:
  void blendUnified(uint8_t blendFactor);
  
  MatrixController* matrix;
  
  // Dynamic blending arrays (allocated based on actual matrix size)
  uint8_t* currentImage;
  uint8_t* targetImage;
  uint8_t currentColor[3];
  uint8_t targetColor[3];
  
  // Blending state
  unsigned long blendTime;
  unsigned long blendStartTime;
  bool blending;
  bool blendingImage;
  bool blendingColor;
  
  int imageSize;  // width * height
};

// Namespace for matrix types
namespace MatrixTypes {
  using MatrixType = MatrixController::MatrixType;
  constexpr auto ROUND = MatrixController::ROUND;
  constexpr auto HEXAGONAL = MatrixController::HEXAGONAL;
  constexpr auto TRIANGULAR = MatrixController::TRIANGULAR;
  constexpr auto CUSTOM = MatrixController::CUSTOM;
}

// Type alias for simple usage
using Matrix = MatrixController;

#endif