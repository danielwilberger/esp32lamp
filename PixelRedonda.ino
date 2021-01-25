#include "FastLED.h"

///////////////////////// LED WS2811
// Definitions for the LEDS
#define DATA_PIN    4  // Data output for LEDs
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    30
#define BRIGHTNESS  255
CRGB leds[NUM_LEDS];

///////////////////////// CAPACITIVE BUTTONS
#define btn_mode 14 // Button that changes the light mode
#define btn_brightness 13 // Button that changes the light brightness

bool flag_btn_mode = false;
bool flag_btn_brightness = false;

int light_mode = 1;
int num_light_modes = 6;

int brightness_multiplyer = 1;

int btn_mode_HIGH = 0;
int btn_brightness_HIGH = 0;
int btn_mode_LOW = 0;
int btn_brightness_LOW = 0;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime1 = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTime2 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 75;    // the debounce time; increase if the output flickers

int buttonState1;             // the current reading from the input pin
int lastButtonState1 = HIGH;   // the previous reading from the input pin
int buttonState2;             // the current reading from the input pin
int lastButtonState2 = HIGH;   // the previous reading from the input pin

////////////////////////////////////////////////////////////

///////////////////////// Variables for light mode fireplace()
#define FRAMES_PER_SECOND 60
bool gReverseDirection = false;
CRGBPalette16 gPal;
int COOLING = 40; //20   50
int SPARKING = 100; //70  100

unsigned long tempoUpdate = 0;

int brightness = 0;


void setup() {

	/*Serial.begin(115200);*/

	// tell FastLED about the LED strip configuration
	FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip).setDither(BRIGHTNESS < 255);

	// set master brightness control
	FastLED.setBrightness(BRIGHTNESS);

	brightness = BRIGHTNESS;

	light_mode = 1;

	initialize_lamp();

	rainbowCycle(5);

	

}

void loop() {

	EVERY_N_MILLISECONDS(50) {

		// Check if button was pressed
		if (readBtn_mode())
		{
			light_mode += 1;

			if (light_mode > num_light_modes)
			{
				light_mode = 1;
			}
		}

		// Check if button was pressed
		if (readBtn_brightness())
		{
			adjustBrightness();
		}
	}



	if (light_mode == 1) {
		moon(brightness);
	}
	else if (light_mode == 2) {
		fireplace(80, 40);

	}
	else if (light_mode == 3) {

		int speed = map(brightness, 50, 255, 2, 20);

		EVERY_N_MILLISECONDS(speed) {
			pacifica_loop();
			FastLED.show();
		}
	}
	else if (light_mode == 4) {

		int speed = map(brightness, 50, 255, 5, 40);
		rainbowCycle(speed);

	}
	else if (light_mode == 5) {

		int speed = map(brightness, 50, 255, 10, 200);

		TwinkleRandom(20, speed, false);
	}
	else if (light_mode == 6) {
		int speed = map(brightness, 50, 255, 1500, 10000);
		int gradient = map(brightness, 50, 255, 25, 3);

		jujuba(speed, gradient);
	}
	else {
		light_mode = 1;
	}
}

/////////////////////////////////////////////////////////////////////////
////////// MÉTODOS DE ILUMINAÇÃO
/////////////////////////////////////////////////////////////////////////

CRGB color = CHSV(random8(), 255, 255);

unsigned long next_color = 0;

void jujuba(int SpeedDelay, int gradient) {

	if (millis() >= next_color) {
		color = CHSV(random8(), 255, 255);
		next_color = millis() + SpeedDelay;
	}

	EVERY_N_MILLIS(100) {
		fadeTowardColor(leds, NUM_LEDS, color, gradient);
		FastLED.show();
	}
	// fade all existing pixels toward bgColor by "5" (out of 255)

}


void rainbowCycle(int SpeedDelay) {

	bool flagQuit = false;

	byte* c;
	uint16_t i, j;

	for (j = 0; j < 256; j++) { // 5 cycles of all colors on wheel
		for (i = 0; i < NUM_LEDS; i++) {

			EVERY_N_MILLIS(50) {
				if (readBtn_mode())
				{
					flagQuit = true;
					light_mode += 1;
					break;
				}
				if (readBtn_brightness())
				{
					flagQuit = true;
					adjustBrightness();
					break;
				}
			}

			c = Wheel(((i * 256 / NUM_LEDS) + j) & 255);
			setPixel(i, *c, *(c + 1), *(c + 2));
		}

		if (flagQuit)
		{
			break;
		}

		showStrip();
		delay(SpeedDelay);
	}
}

byte* Wheel(byte WheelPos) {
	static byte c[3];

	if (WheelPos < 85) {
		c[0] = WheelPos * 3;
		c[1] = 255 - WheelPos * 3;
		c[2] = 0;
	}
	else if (WheelPos < 170) {
		WheelPos -= 85;
		c[0] = 255 - WheelPos * 3;
		c[1] = 0;
		c[2] = WheelPos * 3;
	}
	else {
		WheelPos -= 170;
		c[0] = 0;
		c[1] = WheelPos * 3;
		c[2] = 255 - WheelPos * 3;
	}
	return c;
}

void TwinkleRandom(int Count, int SpeedDelay, boolean OnlyOne) {


	EVERY_N_MILLIS(SpeedDelay / 4) {
		setPixel(random(NUM_LEDS), 0, 0, 0);
	}

	if (meuDelay(SpeedDelay)) {

		setPixel(random(NUM_LEDS), random(0, 255), random(0, 255), random(0, 255));

		showStrip();

		if (OnlyOne) {
			setAll(0, 0, 0);
		}
	}
}

void moon(int brilho1) {

	for (int i = 0; i < NUM_LEDS; i++) {
		leds[i] = CRGB::White;
		FastLED.setBrightness(brilho1);
		FastLED.show();
	}
}

void fireplace(int SPARKING, int COOLING) {

	if (brightness > 1)
	{
		gPal = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
	}	
	if (brightness > 51) {
		gPal = CRGBPalette16(CRGB::Black, CRGB::Violet, CRGB::Coral, CRGB::White);
	}
	if (brightness > 101) {
		gPal = CRGBPalette16(CRGB::Black, CRGB::Aqua, CRGB::DarkCyan, CRGB::White);
	}
	if (brightness > 151) {
		gPal = CRGBPalette16(CRGB::Black, CRGB::Purple, CRGB::Coral, CRGB::White);
	}
	if (brightness > 201){
		gPal = CRGBPalette16(CRGB::Black, CRGB::ForestGreen, CRGB::Green, CRGB::White);
	}
	if (brightness > 251)
	{
		gPal = CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::OrangeRed, CRGB::Yellow);
	}


	// This first palette is the basic 'black body radiation' colors,
	// which run from black to red to bright yellow to white.
	//gPal = HeatColors_p;

	// These are other ways to set up the color palette for the 'fire'.
	// First, a gradient from black to red to yellow to white -- similar to HeatColors_p
	//   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);

	// Second, this palette is like the heat colors, but blue/aqua instead of red/yellow
	//   gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);

	// Third, here's a simpler, three-step gradient, from black to red to white
	//   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::White);

	// Add entropy to random number generator; we use a lot of it.
	random16_add_entropy(random(100));

	// Fourth, the most sophisticated: this one sets up a new palette every
	// time through the loop, based on a hue that changes every time.
	// The palette is a gradient from black, to a dark color based on the hue,
	// to a light color based on the hue, to white.
	//
	//   static uint8_t hue = 0;
	//   hue++;
	//   CRGB darkcolor  = CHSV(hue,255,192); // pure hue, three-quarters brightness
	//   CRGB lightcolor = CHSV(hue,128,255); // half 'whitened', full brightness
	//   gPal = CRGBPalette16( CRGB::Black, darkcolor, lightcolor, CRGB::White);

	// Array of temperature readings at each simulation cell
	static byte heat[NUM_LEDS];

	// Step 1.  Cool down every cell a little
	for (int i = 0; i < NUM_LEDS; i++) {
		heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
	}

	// Step 2.  Heat from each cell drifts 'up' and diffuses a little
	for (int k = NUM_LEDS - 1; k >= 2; k--) {
		heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
	}

	// Step 3.  Randomly ignite new 'sparks' of heat near the bottom
	if (random8() < SPARKING) {
		int y = random8(7);
		heat[y] = qadd8(heat[y], random8(160, 255));
	}

	// Step 4.  Map from heat cells to LED colors
	for (int j = 0; j < NUM_LEDS; j++) {
		// Scale the heat value from 0-255 down to 0-240
		// for best results with color palettes.
		byte colorindex = scale8(heat[j], 240);
		CRGB color = ColorFromPalette(gPal, colorindex);
		int pixelnumber;
		if (gReverseDirection) {
			pixelnumber = (NUM_LEDS - 1) - j;
		}
		else {
			pixelnumber = j;
		}
		leds[pixelnumber] = color;
	}

	FastLED.show(); // display this frame
	FastLED.delay(1000 / FRAMES_PER_SECOND);

}

//////////////////////////////// PACIFICA

CRGBPalette16 pacifica_palette_1 =
{ 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
  0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
CRGBPalette16 pacifica_palette_2 =
{ 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
  0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
CRGBPalette16 pacifica_palette_3 =
{ 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33,
  0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };
void pacifica_loop()
{
	// Increment the four "color index start" counters, one for each wave layer.
	// Each is incremented at a different speed, and the speeds vary over time.
	static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
	static uint32_t sLastms = 0;
	uint32_t ms = GET_MILLIS();
	uint32_t deltams = ms - sLastms;
	sLastms = ms;
	uint16_t speedfactor1 = beatsin16(3, 179, 269);
	uint16_t speedfactor2 = beatsin16(4, 179, 269);
	uint32_t deltams1 = (deltams * speedfactor1) / 256;
	uint32_t deltams2 = (deltams * speedfactor2) / 256;
	uint32_t deltams21 = (deltams1 + deltams2) / 2;
	sCIStart1 += (deltams1 * beatsin88(1011, 10, 13));
	sCIStart2 -= (deltams21 * beatsin88(777, 8, 11));
	sCIStart3 -= (deltams1 * beatsin88(501, 5, 7));
	sCIStart4 -= (deltams2 * beatsin88(257, 4, 6));

	// Clear out the LED array to a dim background blue-green
	fill_solid(leds, NUM_LEDS, CRGB(2, 6, 10));

	// Render each of four layers, with different scales and speeds, that vary over time
	pacifica_one_layer(pacifica_palette_1, sCIStart1, beatsin16(3, 11 * 256, 14 * 256), beatsin8(10, 70, 130), 0 - beat16(301));
	pacifica_one_layer(pacifica_palette_2, sCIStart2, beatsin16(4, 6 * 256, 9 * 256), beatsin8(17, 40, 80), beat16(401));
	pacifica_one_layer(pacifica_palette_3, sCIStart3, 6 * 256, beatsin8(9, 10, 38), 0 - beat16(503));
	pacifica_one_layer(pacifica_palette_3, sCIStart4, 5 * 256, beatsin8(8, 10, 28), beat16(601));

	// Add brighter 'whitecaps' where the waves lines up more
	pacifica_add_whitecaps();

	// Deepen the blues and greens a bit
	//pacifica_deepen_colors();
}

// Add one layer of waves into the led array
void pacifica_one_layer(CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
	uint16_t ci = cistart;
	uint16_t waveangle = ioff;
	uint16_t wavescale_half = (wavescale / 2) + 20;
	for (uint16_t i = 0; i < NUM_LEDS; i++) {
		waveangle += 250;
		uint16_t s16 = sin16(waveangle) + 32768;
		uint16_t cs = scale16(s16, wavescale_half) + wavescale_half;
		ci += cs;
		uint16_t sindex16 = sin16(ci) + 32768;
		uint8_t sindex8 = scale16(sindex16, 240);
		CRGB c = ColorFromPalette(p, sindex8, bri, LINEARBLEND);
		leds[i] += c;
	}
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
	uint8_t basethreshold = beatsin8(9, 55, 65);
	uint8_t wave = beat8(7);

	for (uint16_t i = 0; i < NUM_LEDS; i++) {
		uint8_t threshold = scale8(sin8(wave), 20) + basethreshold;
		wave += 7;
		uint8_t l = leds[i].getAverageLight();
		if (l > threshold) {
			uint8_t overage = l - threshold;
			uint8_t overage2 = qadd8(overage, overage);
			leds[i] += CRGB(overage, overage2, qadd8(overage2, overage2));
		}
	}
}

// Deepen the blues and greens
void pacifica_deepen_colors()
{
	for (uint16_t i = 0; i < NUM_LEDS; i++) {
		leds[i].blue = scale8(leds[i].blue, 145);
		leds[i].green = scale8(leds[i].green, 200);
		leds[i] |= CRGB(2, 5, 7);
	}
}



/////////////////////////////////////////////////////////////////////////
////////// MÉTODOS DE CONTROLE
/////////////////////////////////////////////////////////////////////////
void initialize_lamp() {

	int count = 0;

	for (int i = 1; i <= 100; i++) {
		btn_mode_HIGH += touchRead(btn_mode);
		btn_brightness_HIGH += touchRead(btn_brightness);
		count += 1;
	}

	btn_mode_HIGH = btn_mode_HIGH / count;
	btn_brightness_HIGH = btn_brightness_HIGH / count;

	btn_mode_LOW = btn_mode_HIGH / 3;
	btn_brightness_LOW = btn_brightness_HIGH / 3;
}

bool readBtn_mode() {

	int reading = touchRead(btn_mode);

	if (reading < 70) {
		reading = LOW;
	}
	else {
		reading = HIGH;
	}

	if (reading != lastButtonState1) {
		// reset the debouncing timer
		lastDebounceTime1 = millis();
	}

	if ((millis() - lastDebounceTime1) > debounceDelay) {

		if (reading != buttonState1) { //CHANGED STATUS

			buttonState1 = reading;

			if (lastButtonState1 == LOW) { // BUTTON PRESSED

				/*light_mode += 1;

				if (light_mode > num_light_modes) {
					light_mode = 1;
				}*/

				lastButtonState1 = reading;
				return true;

			}
		}
	}

	lastButtonState1 = reading;
	return false;
}

bool readBtn_brightness() {

	int reading = touchRead(btn_brightness);

	if (reading < 70) {
		reading = LOW;
	}
	else {
		reading = HIGH;
	}

	if (reading != lastButtonState2) {
		// reset the debouncing timer
		lastDebounceTime2 = millis();
	}

	if ((millis() - lastDebounceTime2) > debounceDelay) {

		if (reading != buttonState2) {

			buttonState2 = reading;

			if (lastButtonState2 == LOW) { // BUTTON PRESSED

				lastButtonState2 = reading;
				return true;
			}
		}
	}
	lastButtonState2 = reading;
	return false;
}

void turn_on_leds() {

	for (int i = 0; i < NUM_LEDS; i++) {
		leds[i] = CRGB::Red;
	}

	FastLED.show();
}

void turn_off_leds() {

	for (int i = 0; i < NUM_LEDS; i++) {
		leds[i] = CRGB::Black;
	}

	FastLED.show();

}

void adjustBrightness() {
	brightness += 50;

	if (brightness > 220 && brightness < 255)
	{
		brightness = 255;
	}

	if (brightness > 255)
	{
		brightness = 50;
	}
}

/////////////////////////////////////////////////////
///////////// MÉTODOS DE CONTROLE DE ILUMINAÇÃO

void showStrip() {
#ifdef ADAFRUIT_NEOPIXEL_H
	// NeoPixel
	strip.show();
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
	// FastLED
	FastLED.show();
#endif
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
#ifdef ADAFRUIT_NEOPIXEL_H
	// NeoPixel
	strip.setPixelColor(Pixel, strip.Color(red, green, blue));
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
	// FastLED
	leds[Pixel].r = red;
	leds[Pixel].g = green;
	leds[Pixel].b = blue;
#endif
}

void setAll(byte red, byte green, byte blue) {
	for (int i = 0; i < NUM_LEDS; i++) {
		setPixel(i, red, green, blue);
	}
	showStrip();
}

bool meuDelay(int delayVal) {

	unsigned long tempoAtual = millis();

	if (tempoAtual > tempoUpdate) {
		tempoUpdate = tempoAtual + delayVal;
		return true;
	}
	else {
		return false;
	}

}

// Helper function that blends one uint8_t toward another by a given amount
void nblendU8TowardU8(uint8_t& cur, const uint8_t target, uint8_t amount)
{
	if (cur == target) return;

	if (cur < target) {
		uint8_t delta = target - cur;
		delta = scale8_video(delta, amount);
		cur += delta;
	}
	else {
		uint8_t delta = cur - target;
		delta = scale8_video(delta, amount);
		cur -= delta;
	}
}

// Blend one CRGB color toward another CRGB color by a given amount.
// Blending is linear, and done in the RGB color space.
// This function modifies 'cur' in place.
CRGB fadeTowardColor(CRGB& cur, const CRGB& target, uint8_t amount)
{
	nblendU8TowardU8(cur.red, target.red, amount);
	nblendU8TowardU8(cur.green, target.green, amount);
	nblendU8TowardU8(cur.blue, target.blue, amount);
	return cur;
}

// Fade an entire array of CRGBs toward a given background color by a given amount
// This function modifies the pixel array in place.
void fadeTowardColor(CRGB* L, uint16_t N, const CRGB& bgColor, uint8_t fadeAmount)
{
	for (uint16_t i = 0; i < N; i++) {
		fadeTowardColor(L[i], bgColor, fadeAmount);
	}
}
