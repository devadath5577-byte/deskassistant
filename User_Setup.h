// ============================================================
// TFT_eSPI User Setup Configuration
// For ILI9341 Display with Touch (XPT2046)
// ============================================================

// Driver selection
#define ILI9341_DRIVER

// Display dimensions
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// ── SPI Pin Configuration ──────────────────
#define TFT_CS    10    // Chip Select
#define TFT_DC     2    // Data/Command
#define TFT_RST    4    // Reset
#define TFT_MOSI  11    // Master Out Slave In
#define TFT_SCLK  12    // Serial Clock
#define TFT_MISO  13    // Master In Slave Out

// ── Touch Screen Configuration ─────────────
#define TOUCH_CS   15    // Touch Chip Select (CRITICAL for touch to work)
#define TOUCH_CALIBRATION_DATA { 275, 3620, 264, 3532, 1 }

// ── SPI Port Configuration ─────────────────
#define USE_HSPI_PORT
#define SPI_PORT HSPI

// ── SPI Frequency ─────────────────────────
#define SPI_FREQUENCY      20000000   // Display SPI frequency
#define SPI_READ_FREQUENCY  4000000   // Read frequency
#define SPI_TOUCH_FREQUENCY 2500000   // Touch SPI frequency

// ── Font Loading ──────────────────────────
#define LOAD_GLCD       // Load GLC font
#define LOAD_FONT2      // Load font 2
#define LOAD_FONT4      // Load font 4
#define LOAD_FONT6      // Load font 6
#define LOAD_FONT7      // Load font 7
#define LOAD_FONT8      // Load font 8
#define LOAD_GFXFF      // Load GFX Free Fonts
#define SMOOTH_FONT     // Enable smooth font rendering

// ── Display Settings ──────────────────────
#define TFT_INVERSION_OFF           // Normal display mode
#define TFT_RGB_ORDER TFT_BGR       // RGB or BGR color order

// ============================================================
