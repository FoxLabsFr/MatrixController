# MatrixController Library

An Arduino library for controlling NeoPixel LED matrix displays, using simple images, optional blending.

See our available matrix:
| Product | Shape | Size | LEDs | Documentation | Store |
| ------------------------------------------------------- | ---------- | ----- | ---- | -------------------------------------------------- | ------------------------------------------------ |
| [Hexagonal Matrix](https://github.com/FoxLabsFr/Hexagonal_leds_matrix_WS2812B) | Hexagonal | 133mm | 144 | [📖 Docs](https://github.com/FoxLabsFr/Hexagonal_leds_matrix_WS2812B) | [🛒 Buy](https://www.tindie.com/products/37170/) |
| [Round Matrix](https://github.com/FoxLabsFr/Round_leds_matrix_95mm_12x12_WS2812B) | Circular | 95mm | 112 | [📖 Docs](https://github.com/FoxLabsFr/Round_leds_matrix_95mm_12x12_WS2812B) | [🛒 Buy](https://www.tindie.com/products/25337/) |
| [Triangle Matrix](https://github.com/FoxLabsFr/Triangle_leds_matrix_WS2812B) | Triangular | 133mm | 144 | [📖 Docs](https://github.com/FoxLabsFr/Triangle_leds_matrix_WS2812B) | [🛒 Buy](https://www.tindie.com/products/37169/) |

## 🚀 Features

- [x] **Lightweight Core**: `MatrixController` for immediate LED control (minimal RAM usage)
- [x] **Image Display**: Automatic centering for any image size
- [x] **Optional Blending**: `MatrixBlender` for smooth transitions
- [x] **Direct JSON status output** for easy integration with web interfaces (optional)
- [x] **Support Round matrix** Embedded conversion matrix for our round matrix
- [x] **Support Hexagonal matrix** Embedded conversion matrix for our hexagonal matrix
- [x] **Support Triangular matrix** Embedded conversion matrix for our triangular matrix
- [ ] **Support Custom matrix** Allow to use custom conversion matrix

## 🏗️ Hardware & Specifications

**Required Hardware:**

- **Arduino-compatible board** - ESP32, Arduino Uno, ESP8266, etc.
- **NeoPixel LED Matrix** - WS2812B based

## 🛠️ Getting Started

### PlatformIO

Add to your `platformio.ini`:

```ini
lib_deps =
  https://github.com/FoxLabsFr/MatrixController.git
```

## 🚀 Usage

### 💡 Basic Usage (Immediate Display)

```cpp
#include "MatrixController.h"

MatrixController matrix;

void setup() {
  // Initialize with matrix type and pin
  matrix.init(MatrixController::ROUND, D6);
  matrix.setMaxBrightness(50); // Set global brightness to 50%
  matrix.setColor(255, 0, 0); // Red color - displays immediately

  // Define your image (any size - automatically centered)
  const bool myImage[8][8] = {
      {0, 0, 0, 1, 1, 0, 0, 0},
      {0, 0, 1, 1, 1, 1, 0, 0},
      // ... rest of your image
  };

  matrix.setImage(myImage, 255, 0, 0); // Display image immediately in red
}

void loop() {
  // No update() needed for immediate display
}
```

### 🎨 Blending

```cpp
#include "MatrixController.h"

MatrixController matrix;
MatrixBlender blender(&matrix);

void setup() {
  matrix.init(MatrixController::ROUND, D6);
  matrix.setMaxBrightness(128);

    // Define your image (any size - automatically centered)
  const bool myImage[8][8] = {
      {0, 0, 0, 1, 1, 0, 0, 0},
      {0, 0, 1, 1, 1, 1, 0, 0},
      // ... rest of your image
  };

  // Use blender for smooth transitions
  blender.blendToImage(myImage, 255, 0, 0, 3000);   // Image in red with 3s transition
}

void loop() {
  blender.update(); // Required for smooth animations
}
```

### 🎨 Matrix Types

```cpp
// 12x12 Round Matrix (112 LEDs)
matrix.init(MatrixController::ROUND, D6);

// 14x14 Hexagonal Matrix (144 LEDs)
matrix.init(MatrixController::HEXAGONAL, D6);

// 16x16 Triangular Matrix (144 LEDs)
matrix.init(MatrixController::TRIANGULAR, D6);
```

### ⚡ Real-Time Updates

The update method must be called in real-time to continuously update blending result. Avoid using blocking instructions like delay in your code, as they will freeze the entire logic and prevent update from being called as required.

### ⏱️ Non-Blocking Waiting

To handle non-blocking waiting, you can use the millis() function to track elapsed time without freezing the program. Here's an example:

```cpp
unsigned long previousMillis = 0;
const unsigned long interval = 1000; // 1 second

void loop() {
  unsigned long currentMillis = millis();

  // Check if the interval has elapsed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Perform your periodic task here
    Serial.println("1 second has passed");
  }

  // Always call update in the loop
  blender.update();
}
```

This approach ensures that your program remains responsive while performing periodic tasks.

## 📚 API Reference

### 🔧 MatrixController Constructor

```cpp
MatrixController()
```

### ⚙️ MatrixController Methods (Immediate Display)

| Method               | Parameters                                                            | Description                          |
| -------------------- | --------------------------------------------------------------------- | ------------------------------------ |
| `init()`             | `MatrixType type, int pin`                                            | Initialize with preset matrix type   |
| `setMaxBrightness()` | `uint8_t brightness`                                                  | Set global brightness (0-255)        |
| `setColor()`         | `uint8_t r, uint8_t g, uint8_t b`                                     | Set RGB color immediately            |
| `setImage()`         | `bool image[][], int rows, int cols`                                  | Display image immediately            |
| `setImage()`         | `bool image[][], int rows, int cols, uint8_t r, uint8_t g, uint8_t b` | Display image with color immediately |
| `show()`             | None                                                                  | Update LED strip display             |
| `clear()`            | None                                                                  | Clear all LEDs                       |
| `getColor()`         | None                                                                  | Get current RGB color array          |
| `getWidth()`         | None                                                                  | Get matrix logical width             |
| `getHeight()`        | None                                                                  | Get matrix logical height            |
| `setPixelXY()`       | `int x, int y, uint8_t r, uint8_t g, uint8_t b`                       | Set individual pixel color           |
| `getJson()`          | None                                                                  | Get JSON status (if enabled)         |

### 🔧 MatrixBlender Constructor

```cpp
MatrixBlender(MatrixController* matrix)
```

### 🎨 MatrixBlender Methods (Smooth Transitions)

| Method           | Parameters                                                                                    | Description                         |
| ---------------- | --------------------------------------------------------------------------------------------- | ----------------------------------- |
| `blendToColor()` | `uint8_t r, uint8_t g, uint8_t b, unsigned long duration`                                     | Blend to RGB color over time        |
| `blendToImage()` | `bool image[][], int rows, int cols, unsigned long duration`                                  | Blend to image over time            |
| `blendToImage()` | `bool image[][], int rows, int cols, uint8_t r, uint8_t g, uint8_t b, unsigned long duration` | Blend to image with color over time |
| `update()`       | None                                                                                          | Update animations (call in loop)    |

### 🤖 Real-World Usage

Check out [HexaTile](https://github.com/FoxLabsFr/HexaTile) - A module with 3-DOF and led display that uses the MatrixController library.

## 📄 License

This project is licensed under Creative Commons Attribution-NonCommercial (CC BY-NC) with a special clause reserving all commercial rights to the original author. See LICENSE for details.

Built with ❤️ by FoxLabs

This README was generated with AI assistance.
