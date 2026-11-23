
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <string.h>

// =====================================================
// 1. DISPLAY AND TOUCHSCREEN
// =====================================================

MCUFRIEND_kbv tft;

#define TFT_ID 0x9341

#define XP 6
#define XM A2
#define YP A1
#define YM 7

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Confirmed landscape calibration
#define TS_LEFT   935
#define TS_RIGHT  167
#define TS_TOP    147
#define TS_BOTTOM 907

#define SCREEN_W 320
#define SCREEN_H 240

#define MINPRESSURE 10
#define MAXPRESSURE 1000

// =====================================================
// 2. JOYSTICK
// =====================================================

#define JOYSTICK_X_PIN A5

// Activation thresholds
#define JOY_LOW_THRESHOLD  400
#define JOY_HIGH_THRESHOLD 620

// Release thresholds (hysteresis)
#define JOY_LOW_RELEASE  450
#define JOY_HIGH_RELEASE 570

// Time between repeated volume commands
#define VOLUME_REPEAT_MS 160

int joystickDirection = 0;
int lastSentDirection = 0;

unsigned long lastVolumeCommand = 0;

// =====================================================
// 3. BUTTON GRID
// =====================================================

#define GRID_X 8
#define GRID_Y 7

#define BUTTON_W 96
#define BUTTON_H 70

#define GAP_X 8
#define GAP_Y 8

const char* buttonLabels[9] = {
  "TERMINAL",
  "VS CODE",
  "BROWSER",
  "FILES",
  "CALCULATOR",
  "CAPTURE",
  "TASK MGR",
  "LOCK",
  "NOTEPAD"
};

// =====================================================
// 4. COLOURS (RGB565)
// =====================================================

#define BLACK       0x0000
#define WHITE       0xFFFF

#define BACKGROUND  0x0841
#define BUTTON_BG   0x18E3
#define BUTTON_EDGE 0x39C7

#define ACCENT      0x07FF
#define PRESSED     0x1D5F

// =====================================================
// 5. TOUCHSCREEN STATE
// =====================================================

bool touchLatched = false;

unsigned long lastTouchTime = 0;

#define RELEASE_DEBOUNCE 120

// Non-blocking button highlight
int highlightedButton = -1;

unsigned long highlightUntil = 0;

#define HIGHLIGHT_DURATION 130

// =====================================================
// 6. DRAW BUTTON
// =====================================================

void drawButton(int index, bool pressed = false) {

  if (index < 0 || index >= 9)
    return;

  int row = index / 3;
  int col = index % 3;

  int x = GRID_X + col * (BUTTON_W + GAP_X);
  int y = GRID_Y + row * (BUTTON_H + GAP_Y);

  uint16_t fillColor = pressed ? PRESSED : BUTTON_BG;
  uint16_t borderColor = pressed ? ACCENT : BUTTON_EDGE;

  // Button background
  tft.fillRoundRect(
    x, y,
    BUTTON_W, BUTTON_H,
    9, fillColor
  );

  // Button outline
  tft.drawRoundRect(
    x, y,
    BUTTON_W, BUTTON_H,
    9, borderColor
  );

  // Label
  const char* label = buttonLabels[index];

  int textSize = 2;

  int textWidth = strlen(label) * 6 * textSize;

  // Reduce font size if necessary
  if (textWidth > BUTTON_W - 12) {
    textSize = 1;
  }

  textWidth = strlen(label) * 6 * textSize;

  int textHeight = 8 * textSize;

  int textX = x + (BUTTON_W - textWidth) / 2;
  int textY = y + (BUTTON_H - textHeight) / 2;

  tft.setTextSize(textSize);
  tft.setTextColor(WHITE);
  tft.setCursor(textX, textY);
  tft.print(label);
}

// =====================================================
// 7. DRAW INTERFACE
// =====================================================

void drawInterface() {

  tft.fillScreen(BACKGROUND);

  for (int i = 0; i < 9; i++) {
    drawButton(i, false);
  }
}

// =====================================================
// 8. BUTTON DETECTION
// =====================================================

int getButtonAt(int x, int y) {

  int localX = x - GRID_X;
  int localY = y - GRID_Y;

  if (localX < 0 || localY < 0)
    return -1;

  int col = localX / (BUTTON_W + GAP_X);
  int row = localY / (BUTTON_H + GAP_Y);

  if (row >= 3 || col >= 3)
    return -1;

  // Ignore horizontal gaps
  if ((localX % (BUTTON_W + GAP_X)) >= BUTTON_W)
    return -1;

  // Ignore vertical gaps
  if ((localY % (BUTTON_H + GAP_Y)) >= BUTTON_H)
    return -1;

  // Normal row-major numbering
  return (row * 3) + col;
}

// =====================================================
// 9. SEND BUTTON COMMAND
// =====================================================

void sendButtonCommand(int index) {

  if (index < 0 || index >= 9)
    return;

  Serial.print("BTN:");
  Serial.println(index + 1);

  // Visual feedback
  drawButton(index, true);

  highlightedButton = index;
  highlightUntil = millis() + HIGHLIGHT_DURATION;
}

// =====================================================
// 10. UPDATE BUTTON HIGHLIGHT
// =====================================================

void updateButtonHighlight() {

  if (highlightedButton == -1)
    return;

  if ((long)(millis() - highlightUntil) >= 0) {

    drawButton(highlightedButton, false);

    highlightedButton = -1;
  }
}

// =====================================================
// 11. READ TOUCHSCREEN
// =====================================================

void readTouch() {

  TSPoint p = ts.getPoint();

  // Restore touchscreen pins
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  bool validTouch =
    p.z > MINPRESSURE &&
    p.z < MAXPRESSURE;

  if (validTouch) {

    lastTouchTime = millis();

    // Trigger only once per press
    if (!touchLatched) {

      touchLatched = true;

      // Corrected coordinate mapping
      int screenX = map(
        p.x,
        TS_TOP,
        TS_BOTTOM,
        0,
        SCREEN_W - 1
      );

      int screenY = map(
        p.y,
        TS_LEFT,
        TS_RIGHT,
        0,
        SCREEN_H - 1
      );

      screenX = constrain(screenX, 0, SCREEN_W - 1);
      screenY = constrain(screenY, 0, SCREEN_H - 1);

      int button = getButtonAt(screenX, screenY);

      if (button != -1) {
        sendButtonCommand(button);
      }
    }
  }

  // Release detection
  else if (
    touchLatched &&
    millis() - lastTouchTime >= RELEASE_DEBOUNCE
  ) {

    touchLatched = false;
  }
}

// =====================================================
// 12. READ JOYSTICK AND CONTROL VOLUME
// =====================================================

void readJoystick() {

  int xValue = analogRead(JOYSTICK_X_PIN);

  unsigned long now = millis();

  // Determine joystick direction with hysteresis
  if (joystickDirection == 0) {

    if (xValue < JOY_LOW_THRESHOLD) {
      joystickDirection = -1;
    }
    else if (xValue > JOY_HIGH_THRESHOLD) {
      joystickDirection = 1;
    }
  }

  else if (joystickDirection == -1) {

    if (xValue > JOY_LOW_RELEASE) {
      joystickDirection = 0;
    }
  }

  else if (joystickDirection == 1) {

    if (xValue < JOY_HIGH_RELEASE) {
      joystickDirection = 0;
    }
  }

  // Joystick is centred
  if (joystickDirection == 0) {

    lastSentDirection = 0;
    return;
  }

  // Send immediately on a new direction.
  // Repeat while the joystick remains held.
  if (
    joystickDirection != lastSentDirection ||
    now - lastVolumeCommand >= VOLUME_REPEAT_MS
  ) {

    if (joystickDirection == 1) {
      Serial.println("VOL_DOWN");
    }
    else {
      Serial.println("VOL_UP");
    }

    lastVolumeCommand = now;
    lastSentDirection = joystickDirection;
  }
}

// =====================================================
// 13. SETUP
// =====================================================

void setup() {

  Serial.begin(9600);

  pinMode(JOYSTICK_X_PIN, INPUT);

  // Initialize confirmed ILI9341 display
  tft.begin(TFT_ID);

  // Landscape orientation: 320 x 240
  tft.setRotation(1);

  drawInterface();

  Serial.println("MINI STREAM DECK ONLINE");
  Serial.println("TOUCHSCREEN READY");
  Serial.println("JOYSTICK READY");
}

// =====================================================
// 14. MAIN LOOP
// =====================================================

void loop() {

  readTouch();

  readJoystick();

  updateButtonHighlight();

}
