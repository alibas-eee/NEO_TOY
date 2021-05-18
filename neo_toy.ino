#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <MPU6050.h>

#define PIN           2
#define NUMPIXELS     48
#define color         3

#define VAL_SUB 1.0f
#define VAL_GAIN 64.0f
#define VAL_DAMP 0.9f

#define G_WIDTH 20	//+- 10
#define MAX_LED_GLOW 10


Vector normAccel, prevAccel , dif, sum;

Adafruit_NeoPixel pixels  = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
MPU6050 mpu;


void setup() {
  pixels.begin();
  Serial.begin(9600);

  Serial.println("Initialize MPU6050");

  while (!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }

  // If you want, you can set accelerometer offsets
  // mpu.setAccelOffsetX();
  // mpu.setAccelOffsetY();
  // mpu.setAccelOffsetZ();

  checkSettings();
}

void loop() {

  ReadGyro();
  
  MotionSense();
  
  MotionGlow();
  
  //print_values();
 
  delay(10);
}


void ReadGyro(){
  normAccel = mpu.readNormalizeAccel();
}

//#define VAL_SUB 1.0f
//#define VAL_GAIN 64.0f
//#define VAL_DAMP 0.9f
void MotionSense(){
  dif.XAxis = normAccel.XAxis - prevAccel.XAxis;
  dif.YAxis = normAccel.YAxis - prevAccel.YAxis;
  dif.ZAxis = normAccel.ZAxis - prevAccel.ZAxis;

  sum.XAxis += abs(dif.XAxis) * VAL_GAIN;
  sum.YAxis += abs(dif.YAxis) * VAL_GAIN;
  sum.ZAxis += abs(dif.ZAxis) * VAL_GAIN;
  
  if (sum.XAxis > 0){
    sum.XAxis -= VAL_SUB;
    sum.XAxis *= VAL_DAMP;
  }
  if (sum.YAxis > 0){
    sum.YAxis -= VAL_SUB;
	sum.YAxis *= VAL_DAMP;
  }
  if (sum.ZAxis > 0){
    sum.ZAxis -= VAL_SUB;
	sum.ZAxis *= VAL_DAMP;
  }

  prevAccel = normAccel;
}

uint32_t MotionHue(){
	
	float g_y, y, t;
	uint32_t rgbcolor = 0;
	uint16_t  hue = 0;	//max 65535
	byte value = 128;	//max 255	
	byte saturation = 255;	//max 255
		
	g_y = normAccel.YAxis;
	y = ((g_y + (G_WIDTH /2)) / G_WIDTH );
	
	hue = y * 65535; 
	rgbcolor = pixels.ColorHSV(hue, saturation, value);	

  Serial.print("hue: ");
  Serial.print(hue);
	
	return rgbcolor;
}


int MotionPosition(){
	float g_x = normAccel.XAxis;
	return ((g_x + (G_WIDTH /2)) / G_WIDTH ) * NUMPIXELS;	
}


void MotionGlow(){
		
	int glow_led_cnt =	(sum.XAxis + sum.YAxis + sum.ZAxis) / 100;
	
	int pos = MotionPosition();
	uint32_t rgbcolor = MotionHue();
	
	if(glow_led_cnt > MAX_LED_GLOW)
  	glow_led_cnt = MAX_LED_GLOW;
	
	  Serial.print(" pos: ");
	  Serial.print(pos);
	  Serial.print(" cnt: ");
	  Serial.print(glow_led_cnt);
	  Serial.print(" clr: ");
	  Serial.print(rgbcolor);
	
	for (int i = 0; i < NUMPIXELS ; i++)
    {
		if ( i > (pos - (glow_led_cnt/2)) &&  i < (pos + (glow_led_cnt/2))){
			pixels.setPixelColor(i, rgbcolor); 
      Serial.print(".");
		}
		else
			pixels.setPixelColor(i, 0); 
	}

Serial.println(" ");
  pixels.show();
	
}



void print_values()
{
  Serial.print(" X: ");
  Serial.print(normAccel.XAxis);
  Serial.print(" Y: ");
  Serial.print(normAccel.YAxis);
  Serial.print(" Z: ");
  Serial.print(normAccel.YAxis);

  Serial.print("\tX: ");
  Serial.print(sum.XAxis);
  Serial.print(" Y: ");
  Serial.print(sum.YAxis);
  Serial.print(" Z: ");
  Serial.println(sum.ZAxis);

}


void checkSettings()
{
  Serial.println();

  Serial.print(" * Sleep Mode:            ");
  Serial.println(mpu.getSleepEnabled() ? "Enabled" : "Disabled");

  Serial.print(" * Clock Source:          ");
  switch (mpu.getClockSource())
  {
    case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
    case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.println("PLL with external 19.2MHz reference"); break;
    case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.println("PLL with external 32.768kHz reference"); break;
    case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
    case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.println("Internal 8MHz oscillator"); break;
  }

  Serial.print(" * Accelerometer:         ");
  switch (mpu.getRange())
  {
    case MPU6050_RANGE_16G:            Serial.println("+/- 16 g"); break;
    case MPU6050_RANGE_8G:             Serial.println("+/- 8 g"); break;
    case MPU6050_RANGE_4G:             Serial.println("+/- 4 g"); break;
    case MPU6050_RANGE_2G:             Serial.println("+/- 2 g"); break;
  }

  Serial.print(" * Accelerometer offsets: ");
  Serial.print(mpu.getAccelOffsetX());
  Serial.print(" / ");
  Serial.print(mpu.getAccelOffsetY());
  Serial.print(" / ");
  Serial.println(mpu.getAccelOffsetZ());
  Serial.println();
}
