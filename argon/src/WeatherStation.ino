// BELJ2434 - FONH3001
SYSTEM_MODE(SEMI_AUTOMATIC);
#include <math.h>
#include "../lib/google-maps-device-locator/src/google-maps-device-locator.h"
#include "../lib/JsonParserGeneratorRK/src/JsonParserGeneratorRK.h"
// JSON DATA FIELDS
float _lat = 0; // float
float _lon = 0; // float
float _acc = 0; // float
int _speed = 0; // Km/h
float _angle = 0; // Degree
float _pluie = 0; // mm/s
int _lumiere = 0; // Lux
int _humid = 0; // Pourcentage
int _tempInt = 0; // Nombre
int _tempDec = 0; // Nombre
int _pression = 0; // Kilo pascal

// Argon pins
const int LightPin = A0;
const int PluviometrePin = A5;
const int AnemometreVitessePin = A3;
const int AnemometreDirectionPin = A2;
const int TempHumidityPin_1 = D2;

// Google map locator
GoogleMapsDeviceLocator locator;
bool locationPosted = false;

// TCP client
TCPClient client;										// Create TCP Client object
byte server[] = {192, 168, 0, 103}; // Server IP
byte dataBuffer[1024];
String receivedData;

// Last time the sensor values were sent to the server
time_t lastTimePosted = 0;

// Pressure sensor values
int kTFactor = 7864320;
int kPFactor = 7864320;
int32_t c0,c1,c00,c10,c01,c11,c20,c21,c30 = {0};
uint8_t slave = 0x77;

void locationCallback(float lat, float lon, float acc)
{
	// Handle the returned location data for the device. This method is passed three arguments:
	// - Latitude
	// - Longitude
	// - Accuracy of estimated location (in meters)
	_lat = lat;
	_lon = lon;
	_acc = acc;
	locationPosted = true;
}

// Converts a number to it's two's complement value
void getTwosComplement(int32_t *raw, uint8_t length)
{
    if (*raw & ((uint32_t)1 << (length - 1)))
    {
        *raw -= (uint32_t)1 << length;
    }
}

// Read a byte in I2C from the slave device
uint8_t readByte(uint8_t slaveDevice ,uint8_t regAddress)
{
    Wire.beginTransmission(slaveDevice);
    Wire.write(regAddress);
    Wire.endTransmission(false);
		Wire.requestFrom(slaveDevice,1);
    //request 1 byte from slave
    return Wire.read(); //return this byte on success
}

// Write a byte in I2C to the slave device
void writeByte(uint8_t slaveDevice ,uint8_t regAddress, uint8_t value, uint8_t mask = 0xFF)
{
		int16_t old = readByte(slaveDevice,regAddress);
		uint8_t newData = ((uint8_t)old & ~mask) | (value & mask);
    Wire.beginTransmission(slaveDevice);
    Wire.write(regAddress);
		Wire.write(newData);
    Wire.endTransmission();
}

void setup()
{
	Serial.begin(9600); // Starts serial monitor and baud rate
	Wire.begin(); // Starts the I2C bus
	lastTimePosted = Time.now(); // Sets the first time
	Particle.connect(); // Connect to particle cloud services
	// Google map locator API
	locator.withSubscribe(locationCallback).withLocateOnce();
	// Sets pin modes
	pinMode(LightPin,INPUT);
	pinMode(AnemometreVitessePin,INPUT);
	pinMode(AnemometreDirectionPin,INPUT);
	pinMode(PluviometrePin,INPUT);

	// Wait for pressure coefs to be ready
	bool coefReady = false;
	bool sensorReady = false;
	while(!coefReady || !sensorReady){
		uint8_t flags = readByte(slave,0x08);
		if(((flags & 0x80) >> 7) == 1){
			coefReady = true;
		}
		if(((flags & 0x40) >> 6) == 1){
			sensorReady = true;
		}
		delay(100);
	}
	// Pressure sensor coefficients
	c0 = (readByte(slave,0x10) << 4) | ((readByte(slave,0x11) & 0xF0) >> 4);
	c1 = ((readByte(slave,0x11) & 0x0F) << 8 ) | (readByte(slave,0x12));
	getTwosComplement(&c0,12);
	getTwosComplement(&c1,12);

	c00 = ((readByte(slave,0x13) << 16) | (readByte(slave,0x14) << 8) | (readByte(slave,0x15) & 0b11110000)) >> 4;
	getTwosComplement(&c00,20);

	c10 = ((readByte(slave,0x15) & 0b00001111 ) << 16) | (readByte(slave,0x16) << 8) | (readByte(slave,0x17) & 0b11110000);
	getTwosComplement(&c10,20);
	
	c01 = (readByte(slave,0x18)<< 8) | readByte(slave,0x19);
	getTwosComplement(&c01,16);

	c11 = (readByte(slave,0x1A) << 8) | readByte(slave,0x1B);
	getTwosComplement(&c11,16);

	c20 = (readByte(slave,0x1C) << 8) | readByte(slave,0x1D);
	getTwosComplement(&c20,16);

	c21 = (readByte(slave,0x1E) << 8) | readByte(slave,0x1F);
	getTwosComplement(&c21,16);

	c30 = (readByte(slave,0x20) << 8) | readByte(slave,0x21);
	getTwosComplement(&c30,16);

	// Setup registers for I2C pressure sensor
	writeByte(slave,0x07, 0x20, 0x70);  // Set pressure config
	writeByte(slave,0x07, 0x03, 0x07);
	writeByte(slave,0x06, 0x20, 0x70);  // Set temp config
	writeByte(slave,0x06, 0x03, 0x07);
	delay(50);
	// Serial.printlnf(" c0 : %d",(int)c0);
	// Serial.printlnf(" c1 : %d",(int)c1);
	// Serial.printlnf(" c00 : %d",(int)c00);
	// Serial.printlnf(" c10 : %d",(int)c10);
	// Serial.printlnf(" c01 : %d",(int)c01);
	// Serial.printlnf(" c11 : %d",(int)c11);
	// Serial.printlnf(" c20 : %d",(int)c20);
	// Serial.printlnf(" c21 : %d",(int)c21);
	// Serial.printlnf(" c30 : %d",(int)c30);
}

void loop()
{
	// Test all sensors one by one
	TestBarometre();
	delay(150);
	testLightSensor();
	delay(150);
	testAnemometre();
	delay(150);
	testPluviometre();
	delay(150);
	TestBarometre();
	delay(150);
	testTemperatureAndHumidity();
	time_t now = Time.now();
	if(!locationPosted){
		// Google map locator API
		locator.loop();
	} else {
		if(now - lastTimePosted > 5){ // Post a new result every X seconds
			Particle.connect();
			WiFi.on();
			Serial.println("Connecting");
			if(WiFi.ready()){
				Serial.println("Connected");
				connectAndSendAllToServer();
				Particle.disconnect(); // Disconnect and turn off wifi to save power
				WiFi.off();
				Serial.println("Disconnected");
			}
		}
	}
}

void testAnemometre()
{
	// Test vitesse
	bool previouslyOn = false;
	int speed = 0;
	int impulse = 0;
	// Check the number of impulse in 1 seconds
	for(int i = 0; i < 1000; i++)
	{
		speed = analogRead(AnemometreVitessePin);
		if(speed > 1000 && !previouslyOn){
			impulse ++;
			previouslyOn = true;
		}
		if(speed < 1000 && previouslyOn){
			previouslyOn = false;
		}
		delay(1);
	}
	speed = 2.4 * impulse; // We know 1 impulse per second is equal to 2.4 km/h
	int direction = analogRead(AnemometreDirectionPin);
	float voltageChart[16] = {3.84,1.98,2.25,0.41,0.45,0.32,0.90,0.62,1.40,1.19,3.08,2.93,4.62,4.04,4.33,3.43};
	const float angleChart[16] = {0,22.5,45,67.5,90,112.5,135,157.5,180,202.5,225,247.5,270,292.5,315,337.5};
	for(int i = 0; i < 16;i++){
			voltageChart[i] = voltageChart[i] * (4096.0/5.0);
	}
	float angle = -1;
	for(int i = 0; i < 16;i++){
		if((direction > voltageChart[i] - 50) && (direction < voltageChart[i] + 50)){
			angle = angleChart[i]; // Find the angle that best fits the voltage we get
		}
	}
	_speed = speed;
	_angle = angle;
}

void testPluviometre()
{
	bool previouslyOn = true;
	int impulse = 0;
	float pluie;
	for(int i = 0; i < 30; i++) // Since this sensor is very bad, give it 3 seconds to calculate the amount of rain
	{
		pluie = analogRead(PluviometrePin);
		if(pluie > 1000 && !previouslyOn){
			impulse ++;
			previouslyOn = true;
		}
		if(pluie < 1000 && previouslyOn){
			previouslyOn = false;
		}
		delay(100);
	}
	pluie = ((0.2794 * impulse) / 3) * 60; // Calculate rain per min
	_pluie = pluie;
}

void testLightSensor()
{
	float analogIn = analogRead(LightPin);
	//Serial.printlnf(" Analog in : %f", analogIn);
	float saturation = 2550; // Saturation
	float lux = analogIn/saturation * 100; // Mise a l echelle sur 100 lux maximum
	_lumiere = lux;
	Serial.printlnf("Lux : %f",lux);
}

void testTemperatureAndHumidity()
{
	uint8_t data[40] = {0};
	noInterrupts(); // Pause interupts since this code is extremely time sensitive
	pinMode(TempHumidityPin_1, OUTPUT);
	// pull the pin high and wait 250 milliseconds
	digitalWrite(TempHumidityPin_1, HIGH);
	delay(250);
	// now pull it low for ~20 milliseconds
	digitalWrite(TempHumidityPin_1, LOW);
	delay(20);
	digitalWrite(TempHumidityPin_1, HIGH);
	delayMicroseconds(40);
	pinMode(TempHumidityPin_1, INPUT_PULLUP);
	pulseIn(TempHumidityPin_1,HIGH);
	// Record the 40 bit message
	for(int i = 0; i < 40; i++){
		data[i] = pulseIn(TempHumidityPin_1,HIGH);
	}
	interrupts();
	byte bytes[5] = {0};
	// Organise the data
	for(int i = 0; i < 5; i++)
	{
		for(int j = 0; j < 8; j++){
			bytes[i] <<= 1;
			if(data[i*8+j] > 50){
				bytes[i] |= 1;
			}
		}
	}
	// Validate the checksum
	if (bytes[4] != ((bytes[0] + bytes[1] + bytes[2] + bytes[3]) & 0xFF)) {
			Serial.println("Checksum invalid");
			return;
	}
	Serial.println("Checksum valid");
	// Fetch values
	int humidityInteger = bytes[0];
	int humidityDecimal = bytes[1];
	int tempInteger = bytes[2];
	int tempDecimal = bytes[3];

	Serial.printf("Temp : %d",tempInteger);
	Serial.printlnf("Humid : %d",humidityInteger);
	_humid = humidityInteger;
	_tempInt = tempInteger;
	_tempDec = tempDecimal;
}

void connectAndSendAllToServer()
{

	JsonWriterStatic<512> jw;
	{
		JsonWriterAutoObject obj(&jw);
		jw.insertKeyValue("Vitesse anemometre(Km/h)", _speed);
		jw.insertKeyValue("Angle anemometre(Degree)", _angle);
		jw.insertKeyValue("Pluie(mm/m)",_pluie);
		jw.insertKeyValue("Lumiere(Lux)",_lumiere);
		jw.insertKeyValue("Humidite(%)",_humid);
		jw.insertKeyValue("Temperature(Capteur humid, Celcius)",String(_tempInt) + "." + String(_tempDec));
		//jw.insertKeyValue("TempDecimal(Capteur humid)",_tempDec);
		jw.insertKeyValue("Pression (KPascal)",_pression);
	}
	String jsonObj(jw.getBuffer());
	sendToServer("/json",jsonObj);

	JsonWriterStatic<256> jw2;
	{
		JsonWriterAutoObject obj(&jw2);
		jw2.insertKeyValue("lat",_lat);
		jw2.insertKeyValue("lon",_lon);
		jw2.insertKeyValue("acc",_acc);
	}
	String jsonObj2(jw2.getBuffer());
	sendToServer("/location",jsonObj2);
}

void TestBarometre()
{
	uint8_t reg_temp[3]; // Registre temporaraire pour store nos valeurs
	Serial.println("Testing barometre");
	// Begin loop
	writeByte(slave,0x08,0x02); // Enable temp measurement
	delay(200);
	// Wait for values to be ready
	bool tmpReady = false;
	while(!tmpReady){
		uint8_t flags = readByte(slave,0x08);
		if(((flags & 0x20) >> 5) == 1){
			tmpReady = true;
		}
		delay(100);
	}

	// Read temperature
	reg_temp[0] = readByte(slave,0x03);
	reg_temp[1] = readByte(slave,0x04);
	reg_temp[2] = readByte(slave,0x05);

	int32_t rawTemp = (reg_temp[0] << 16) | (reg_temp[1] << 8) | reg_temp[2];
	//int32_t rawTemp = reg_temp[2] + (reg_temp[1] * 256) + (reg_temp[0] * 256 * 256);
	getTwosComplement(&rawTemp,24);
	// Scale down
	float temp = rawTemp;
  // Compensate
  temp /= kTFactor;
  rawTemp = temp;
	// Scale it
  temp = ( c0 / 2.0 ) + c1 * temp;

	writeByte(slave,0x08,0x01); // Enable pressure measurement
	delay(200);
	// Wait for values to be ready
	bool prsReady = false;
	while(!prsReady){
		uint8_t flags = readByte(slave,0x08);
		if(((flags & 0x10) >> 4) == 1){
			prsReady = true;
		}
		delay(100);
	}
	// Pressure
	reg_temp[0] = readByte(slave,0x00);
	reg_temp[1] = readByte(slave,0x01);
	reg_temp[2] = readByte(slave,0x02);
	int32_t rawPressure = (reg_temp[0] << 16) | (reg_temp[1] << 8) | (reg_temp[2]);
	getTwosComplement(&rawPressure,24);
	float pression = rawPressure;
	pression /= kPFactor;
	
	// Calculate compensated pressure value using temp and coefs
	pression = c00 + pression * (c10 + pression * (c20 + pression * c30)) + rawTemp * (c01 + pression * (c11 + pression * c21));
	Serial.print("Barometre (KiloPascal) : ");
	Serial.println(String(pression/1000));
	_pression = pression/1000;
}

void sendToServer(String endPoint, String body)
{
	// POST Message
	if (client.connect(server, 3000))
	{
		// Serial.println("Connection OK!");
		client.println("POST " + endPoint + " HTTP/1.0");
		client.println("Host: 192.168.0.103:3000"); // Should match server IP and port
		client.println("Content-Type: application/json");
		client.print("Content-Length: ");
		client.println(body.length());
		client.println();
		client.print(body);

		// // Print the string
		// Serial.println(receivedData);

		// Stop the current connection
		client.stop();
	}
	else
	{
		Serial.println("Server connection failed. Trying again...");
	}
}