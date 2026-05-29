#include <WiFi.h>
#include <TFT_eSPI.h>
#include <time.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

// ── Animation State ───────────────────────────
float colonAlpha = 1.0;
float colonDirection = -0.1;
float glowPulse = 0.0;
float dayPulse = 0.0;
int lastSecond = -1;
int lastDay = -1;

// ── WiFi ──────────────────────────────────────
const char* ssid     = "AirFiber-Oodae6";
const char* password = "seod3joo1Oozae6m";

// ── NTP ───────────────────────────────────────
const char* ntpServer       = "pool.ntp.org";
const long  gmtOffset_sec   = 19800;
const int   daylightOffset  = 0;

// ── Modern Theme Colors ───────────────────────
// Deep blue/purple gradient background
#define BG          0x0000        // Pure black base
#define S_BG        0x0000        // Pure black base

// Neon cyan - primary accent
#define C_BRIGHT    tft.color565(0, 255, 200)

// Deep purple glow - background glow effect
#define C_GLOW      tft.color565(80, 20, 120)

// Soft white for secondary text
#define C_WHITE     tft.color565(220, 225, 230)

// Dim text for labels
#define C_DIM       tft.color565(100, 110, 120)

// Vibrant red for active day
#define C_RED       tft.color565(255, 100, 120)

// Warm orange for temperature
#define C_ORANGE    tft.color565(255, 180, 60)

// Subtle separator
#define C_SEP       0x0000

// Additional colors for depth
#define C_ACCENT    tft.color565(100, 200, 255)
#define C_PURPLE    tft.color565(150, 80, 200)

// ── Helper: Smooth easing function ───────────
float easeInOutQuad(float t) {
  if (t < 0.5) return 2 * t * t;
  return -1 + (4 - 2 * t) * t;
}

// ── Helper: Blend two colors ────────────────
uint16_t blendColors(uint16_t color1, uint16_t color2, float alpha) {
  uint8_t r1 = (color1 >> 11) & 0x1F;
  uint8_t g1 = (color1 >> 5) & 0x3F;
  uint8_t b1 = color1 & 0x1F;
  
  uint8_t r2 = (color2 >> 11) & 0x1F;
  uint8_t g2 = (color2 >> 5) & 0x3F;
  uint8_t b2 = color2 & 0x1F;
  
  uint8_t r = (uint8_t)(r1 * (1.0 - alpha) + r2 * alpha);
  uint8_t g = (uint8_t)(g1 * (1.0 - alpha) + g2 * alpha);
  uint8_t b = (uint8_t)(b1 * (1.0 - alpha) + b2 * alpha);
  
  return tft.color565(r * 8, g * 4, b * 8);
}

// ── Helper: Draw gradient background ────────
void drawGradientBackground(TFT_eSprite& s) {
  // Subtle gradient from dark blue to black
  for (int y = 0; y < 240; y++) {
    float ratio = (float)y / 240.0;
    uint16_t color = blendColors(
      tft.color565(10, 15, 40),
      tft.color565(5, 5, 15),
      ratio
    );
    s.drawFastHLine(0, y, 320, color);
  }
}

// ── Helper: Draw glowing box ────────────────
void drawGlowingBox(TFT_eSprite& s, int x, int y, int w, int h, uint16_t glowColor, float intensity) {
  // Outer glow (3 layers for depth)
  for (int i = 3; i > 0; i--) {
    uint16_t fadeColor = blendColors(glowColor, BG, 1.0 - (intensity * (0.3 / i)));
    s.drawRect(x - i, y - i, w + (i * 2), h + (i * 2), fadeColor);
  }
  // Main border
  s.drawRect(x, y, w, h, glowColor);
}

// ── Splash Screen ─────────────────────────────
void showSplash(const char* l1, const char* l2) {
  tft.fillScreen(BG);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(C_BRIGHT, BG);
  tft.drawString("DESK CLOCK", 160, 90, 4);
  tft.setTextColor(C_WHITE, BG);
  tft.drawString(l1, 160, 140, 2);
  tft.drawString(l2, 160, 165, 2);
}

// ── Draw Clock Face with Animations ─────────
void drawFace(struct tm& t) {
  // Draw gradient background
  drawGradientBackground(sprite);

  int LP = 60;
  int RP = 68;
  int RX = 320 - RP;
  int CX = LP + (RX - LP) / 2;
  int CY = 110;

  // ── Animated vertical dividers ──────────
  sprite.drawFastVLine(LP, 0, 240, C_PURPLE);
  sprite.drawFastVLine(RX, 0, 240, C_PURPLE);

  // ── Days with smooth animation ──────────
  const char* dayNames[] = {
    "MON","TUE","WED","THU","FRI","SAT","SUN"
  };

  int activeIdx;
  if (t.tm_wday == 0)
    activeIdx = 6;
  else
    activeIdx = t.tm_wday - 1;

  // Trigger day change animation
  if (lastDay != activeIdx) {
    lastDay = activeIdx;
    dayPulse = 1.0;
  }

  int slotH = 240 / 7;

  for (int i = 0; i < 7; i++) {
    int yTop = i * slotH;
    int yMid = yTop + slotH / 2;
    bool active = (i == activeIdx);

    if (active) {
      // Pulsing red for active day
      float pulseIntensity = 0.7 + 0.3 * sin(dayPulse * 3.14159 * 2);
      uint16_t pulseColor = blendColors(C_RED, BG, 1.0 - pulseIntensity);
      sprite.setTextColor(pulseColor, BG);
    } else {
      sprite.setTextColor(C_WHITE, BG);
    }

    sprite.setTextDatum(MC_DATUM);
    sprite.drawString(dayNames[i], LP / 2, yMid, 2);
  }

  // ── Right Section with Glow Effect ──────
  int RC = RX + RP / 2;

  // DATE
  sprite.setTextDatum(MC_DATUM);
  sprite.setTextColor(C_DIM, BG);
  sprite.drawString("DATE", RC, 13, 1);

  char dayNum[4];
  strftime(dayNum, sizeof(dayNum), "%d", &t);
  sprite.setTextColor(C_ACCENT, BG);
  sprite.drawString(dayNum, RC, 36, 4);

  char monBuf[8];
  strftime(monBuf, sizeof(monBuf), "%b", &t);
  sprite.setTextColor(C_BRIGHT, BG);
  sprite.drawString(monBuf, RC, 62, 2);

  char yrBuf[6];
  strftime(yrBuf, sizeof(yrBuf), "%Y", &t);
  sprite.setTextColor(C_DIM, BG);
  sprite.drawString(yrBuf, RC, 80, 1);

  // TEMP
  sprite.setTextColor(C_DIM, BG);
  sprite.drawString("TEMP", RC, 108, 1);
  sprite.setTextColor(C_ORANGE, BG);
  sprite.drawString("32", RC, 132, 4);
  sprite.setTextColor(C_DIM, BG);
  sprite.drawString("o C", RC, 115, 1);

  // Location
  sprite.setTextColor(C_DIM, BG);
  sprite.drawString("KERALA", RC, 174, 1);
  sprite.drawString("IST", RC, 190, 1);

  // ── Main Time with Smooth Colon Fade ────
  char timeBuf[8];
  
  // Smooth colon fade animation
  if (colonAlpha <= 0.1) {
    colonDirection = 0.1;
  } else if (colonAlpha >= 1.0) {
    colonDirection = -0.1;
  }
  colonAlpha += colonDirection;

  if (colonAlpha > 0.3) {
    strftime(timeBuf, sizeof(timeBuf), "%I:%M", &t);
  } else {
    strftime(timeBuf, sizeof(timeBuf), "%I %M", &t);
  }

  // Update glow pulse animation
  glowPulse += 0.05;
  if (glowPulse > 1.0) glowPulse = 0.0;
  float glowIntensity = 0.5 + 0.5 * sin(glowPulse * 3.14159);

  // Triple-layer glow effect
  sprite.setTextDatum(MC_DATUM);
  
  // Deep glow layer
  sprite.setTextColor(blendColors(C_GLOW, BG, 0.7), S_BG);
  sprite.drawString(timeBuf, CX - 2, CY + 2, 7);
  sprite.drawString(timeBuf, CX + 2, CY - 2, 7);

  // Mid glow layer
  sprite.setTextColor(blendColors(C_PURPLE, BG, 0.5), S_BG);
  sprite.drawString(timeBuf, CX - 1, CY + 1, 7);
  sprite.drawString(timeBuf, CX + 1, CY - 1, 7);

  // Main bright cyan
  sprite.setTextColor(C_BRIGHT, S_BG);
  sprite.drawString(timeBuf, CX, CY, 7);

  // ── AM PM with Glow ─────────────────────
  char ampm[4];
  strftime(ampm, sizeof(ampm), "%p", &t);
  sprite.setTextDatum(ML_DATUM);
  sprite.setTextColor(blendColors(C_BRIGHT, C_PURPLE, 0.3), S_BG);
  sprite.drawString(ampm, CX + 76, CY - 32, 2);

  // ── Seconds with Animation ──────────────
  char secBuf[4];
  strftime(secBuf, sizeof(secBuf), "%S", &t);
  sprite.setTextDatum(MC_DATUM);
  
  // Breathing effect for seconds
  float breathe = 0.7 + 0.3 * sin(glowPulse * 3.14159);
  sprite.setTextColor(blendColors(C_WHITE, C_DIM, 1.0 - breathe), S_BG);
  sprite.drawString(secBuf, CX, CY + 52, 4);
  
  sprite.setTextColor(C_DIM, S_BG);
  sprite.drawString("SEC", CX, CY + 72, 1);

  // ── Animated Progress Bar ───────────────
  int barX = LP + 8;
  int barW = RX - LP - 16;
  int barY = 224;
  int filled = (t.tm_sec * barW) / 60;

  // Background bar with glow
  sprite.fillRoundRect(barX, barY, barW, 6, 3, blendColors(C_DIM, BG, 0.5));
  
  // Animated filled bar with easing
  float easeProgress = easeInOutQuad((float)t.tm_sec / 60.0);
  int easedFilled = (easeProgress * barW);
  
  // Gradient fill effect
  for (int i = 0; i < easedFilled; i++) {
    float ratio = (float)i / easedFilled;
    uint16_t barColor = blendColors(C_ACCENT, C_BRIGHT, ratio);
    sprite.drawFastVLine(barX + i, barY, 6, barColor);
  }

  // ── Push Sprite ─────────────────────────
  sprite.pushSprite(0, 0);
}

// ── Setup ────────────────────────────────────
void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(3);
  tft.invertDisplay(true);
  tft.fillScreen(BG);

  sprite.setColorDepth(16);
  sprite.createSprite(320, 240);

  // WiFi
  showSplash("Connecting to WiFi...", ssid);
  WiFi.begin(ssid, password);

  int a = 0;
  while (WiFi.status() != WL_CONNECTED && a < 30) {
    delay(500);
    a++;
  }

  // Time Sync
  if (WiFi.status() == WL_CONNECTED) {
    showSplash("Syncing NTP time...", "pool.ntp.org");
    configTime(gmtOffset_sec, daylightOffset, ntpServer);

    struct tm ti;
    int tr = 0;
    while (!getLocalTime(&ti) && tr < 20) {
      delay(500);
      tr++;
    }
  } else {
    showSplash("WiFi Failed!", "Check credentials");
    delay(3000);
  }

  tft.fillScreen(BG);
}

// ── Loop ─────────────────────────────────────
void loop() {
  struct tm t;

  if (!getLocalTime(&t)) {
    delay(50);
    return;
  }

  drawFace(t);
  
  // Smooth animation at 20fps (50ms per frame)
  delay(50);
}