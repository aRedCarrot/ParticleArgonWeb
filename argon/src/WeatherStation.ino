// BELJ2434 - FONH3001
#include <math.h>
#include "../lib/google-maps-device-locator/src/google-maps-device-locator.h"
#include "../lib/JsonParserGeneratorRK/src/JsonParserGeneratorRK.h"

const int LightPin = A0;
const int PluviometrePin = A5;
const int AnemometreVitessePin = A3;
const int AnemometreDirectionPin = A2;
const int TempHumidityPin_1 = D2;
 const int TempHumidityPin_2 = D3;
GoogleMapsDeviceLocator locator;
TCPClient client;										// Create TCP Client object
byte server[] = {192, 168, 0, 103}; // http://maker-io-iot.atwebpages.com/
byte dataBuffer[1024];

String receivedData;

void locationCallback(float lat, float lon, float acc)
{
	// Handle the returned location data for the device. This method is passed three arguments:
	// - Latitude
	// - Longitude
	// - Accuracy of estimated location (in meters)
	JsonWriterStatic<256> jw;
	{
		JsonWriterAutoObject obj(&jw);
		jw.insertKeyValue("lat", lat);
		jw.insertKeyValue("lon", lon);
		jw.insertKeyValue("acc", acc);
	}
	String jsonObj(jw.getBuffer());
	sendToServer("/location",jsonObj);
}

void getTwosComplement(int32_t *raw, uint8_t length)
{
    if (*raw & ((uint32_t)1 << (length - 1)))
    {
        *raw -= (uint32_t)1 << length;
    }
}

// template<typename T>
// void printL(String a, T b){
// 	Serial.print(a);
// 	Serial.println(b);
// }

void setup()
{
	Serial.begin(9600);
	Wire.begin();
	// Google map locator API
	//locator.withSubscribe(locationCallback).withLocateOnce();
	pinMode(LightPin,INPUT);
	pinMode(AnemometreVitessePin,INPUT);
	pinMode(AnemometreDirectionPin,INPUT);
	pinMode(PluviometrePin,INPUT);
}

void loop()
{
	//testLightSensor();
	//delay(1000);
	//testAnemometre();
	//delay(1000);
	//testPluviometre();
	//delay(1000);
	//TestBarometre();
	//delay(1000);
	testTemperatureAndHumidity();
	delay(2500);
	// Google map locator API
	//locator.loop();
}

void testAnemometre()
{
	// Test vitesse
	bool previouslyOn = false;
	int32_t speed = 0;
	int impulse = 0;
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
	speed = 2.4 * impulse;
	int32_t direction = analogRead(AnemometreDirectionPin);
	float voltageChart[16] = {3.84,1.98,2.25,0.41,0.45,0.32,0.90,0.62,1.40,1.19,3.08,2.93,4.62,4.04,4.33,3.43};
	const float angleChart[16] = {0,22.5,45,67.5,90,112.5,135,157.5,180,202.5,225,247.5,270,292.5,315,337.5};
	for(int i = 0; i < 16;i++){
			voltageChart[i] = voltageChart[i] * (4096.0/5.0);
	}
	float angle = -1;
	for(int i = 0; i < 16;i++){
		if((direction > voltageChart[i] - 50) && (direction < voltageChart[i] + 50)){
			angle = angleChart[i];
		}
	}
	JsonWriterStatic<254> jw;
	{
		JsonWriterAutoObject obj(&jw);
		jw.insertKeyValue("Vitesse anemometre", String(speed) + String(" Km/h"));
		jw.insertKeyValue("Angle anemometre", String(angle) + String("°"));
	}
	String jsonObj(jw.getBuffer());
	sendToServer("/json",jsonObj);
}

void testPluviometre()
{
	bool previouslyOn = true;
	int impulse = 0;
	float pluie;
	for(int i = 0; i < 10; i++)
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
	pluie = 0.2794 * impulse;
	JsonWriterStatic<124> jw;
	{
		JsonWriterAutoObject obj(&jw);
		jw.insertKeyValue("Pluie ", String(pluie) + String(" mm/s"));
	}
	String jsonObj(jw.getBuffer());
	sendToServer("/json",jsonObj);
}

void testLightSensor()
{
	int32_t result = analogRead(LightPin);
	JsonWriterStatic<124> jw;
	{
		JsonWriterAutoObject obj(&jw);
		jw.insertKeyValue("Lumiere", result);
	}
	String jsonObj(jw.getBuffer());
	sendToServer("/json",jsonObj);
}



void testTemperatureAndHumidity()
{
	pinMode(TempHumidityPin_1,OUTPUT);
	digitalWrite(TempHumidityPin_1,1);
	delay(12);
	digitalWrite(TempHumidityPin_1,0);
	delay(12);
	pinMode(TempHumidityPin_1,INPUT);
	byte data[40] = {0};
	for(int i = 0; i < 40; i++){
		data[i] = digitalRead(TempHumidityPin_1);
		delay(4/40);
	}
	char integralRHData;
	char decimalRHData;
	char integralTData;
	char decimalTData;
	char checksum;
	memcpy(&integralRHData,&data[0],sizeof(char));
	memcpy(&decimalRHData,&data[8],sizeof(char));
	memcpy(&integralTData,&data[16],sizeof(char));
	memcpy(&decimalTData,&data[24],sizeof(char));
	memcpy(&checksum,&data[32],sizeof(char));

	Serial.print("Bytes : ");
	for(int i = 0; i < 40; i++)
	{
			if(i % 8 == 0){
				Serial.print(" ");
			}
			Serial.print(data[i]);
	}
	Serial.println(" ");
	Serial.printlnf("integralRHData : %s",integralRHData);
	Serial.printlnf("decimalRHData : %s",decimalRHData);
	Serial.printlnf("integralTData : %s",integralTData);
	Serial.printlnf("decimalTData : %s",decimalTData);
	Serial.printlnf("checksum : %s",checksum);

	int* checkSumValue = new int((int)integralRHData + (int)decimalRHData + (int)integralTData + (int)decimalTData);
	char checkSumValidate;
	memcpy(&checkSumValidate,&checkSumValue[24],sizeof(char));
	if(checkSumValidate == checksum){
		Serial.println("Check sum good");
	} else {
		Serial.println("Check sum bad");
	}
}

int8_t readByte(uint8_t slaveDevice ,uint8_t regAddress)
{
    Wire.beginTransmission(slaveDevice);
    Wire.write(regAddress);
    Wire.endTransmission(false);
    //request 1 byte from slave
    return Wire.read(); //return this byte on success
}

void writeByte(uint8_t slaveDevice ,uint8_t regAddress, uint8_t value)
{
    Wire.beginTransmission(slaveDevice);
    Wire.write(regAddress);
		Wire.write(value);
    Wire.endTransmission(false);
}

void TestBarometre()
{
	Serial.println("Testing barometre");
	uint8_t slave = 0x77;
	uint8_t reg_temp[3]; // Registre temporaraire pour store nos valeurs

	writeByte(slave,0x0C,0x89);

	// Compensated value
	int32_t c00_0 = readByte(slave,0x13);
	Serial.print(c00_0);
	int32_t c00_1 = readByte(slave,0x14);;
	int32_t c00_2 = readByte(slave,0x15); // Contains half of C00 and C10 [4:4]
	int32_t c00 = ((c00_0 << 16) | (c00_1 << 8) | (c00_2 & 0b11110000)) >> 4;
	getTwosComplement(&c00,20);

	int32_t c10_0 = (c00_2 & 0b00001111);
	int32_t c10_1 = readByte(slave,0x16);
	int32_t c10_2 = readByte(slave,0x17);
	int32_t c10 = (c10_0 << 16) | (c10_1 << 8) | (c10_2 & 0b11110000);
	getTwosComplement(&c10,20);
	
	int32_t c01_0 = readByte(slave,0x18);
	int32_t c01_1 = readByte(slave,0x19);
	int32_t c01 = (c01_0 << 8) | c01_1;
	getTwosComplement(&c01,16);

	int32_t c11_0 = readByte(slave,0x1A);
	int32_t c11_1 = readByte(slave,0x1B);
	int32_t c11 = (c11_0 << 8) | c11_1;
	getTwosComplement(&c11,16);

	int32_t c20_0 = readByte(slave,0x1C);
	int32_t c20_1 = readByte(slave,0x1D);
	int32_t c20 = (c20_0 << 8) | c20_1;
	getTwosComplement(&c20,16);

	int32_t c21_0 = readByte(slave,0x1E);
	int32_t c21_1 = readByte(slave,0x1F);
	int32_t c21 = (c21_0 << 8) | c21_1;
	getTwosComplement(&c21,16);

	int32_t c30_0 = readByte(slave,0x18);
	int32_t c30_1 = readByte(slave,0x19);
	int32_t c30 = (c30_0 << 8) | c30_1;
	getTwosComplement(&c30,16);

	int32_t kTFactor = 253952;
	int32_t kPFactor = 253952;

	writeByte(slave,0x06,0x01);

	writeByte(slave,0x07,0x80);

	// Begin loop
	writeByte(slave,0x08,0x02);

	// Temperature
	reg_temp[0] = readByte(slave,0x03);
	reg_temp[1] = readByte(slave,0x04);
	reg_temp[2] = readByte(slave,0x05);
	int32_t rawTemp = (reg_temp[0] << 16) | (reg_temp[1] << 8) | reg_temp[0];
	getTwosComplement(&rawTemp,24);

	writeByte(slave,0x08,0x01);

	// Pressure
	reg_temp[0] = readByte(slave,0x00);
	reg_temp[1] = readByte(slave,0x01);
	reg_temp[2] = readByte(slave,0x02);
	int32_t rawPressure = (reg_temp[0] << 16) | (reg_temp[1] << 8) | reg_temp[0];
	getTwosComplement(&rawPressure,24);


	float tempRaw_sc = rawTemp / (float)kTFactor;
	float rawPressure_sc = rawPressure / (float)kPFactor;
	float realPressure = c00 + rawPressure_sc*(c10 + rawPressure_sc *(c20+ rawPressure_sc *c30)) + tempRaw_sc *c01 + tempRaw_sc *rawPressure_sc *(c11+rawPressure_sc*c21);
	Serial.print("Barometre : ");
	Serial.println(rawPressure);
}

String getFromServer()
{
	//GET Message
	if (client.connect(server, 3000))
	{
		// Print some information that we have connected to the server
		Serial.println("**********************************!");
		Serial.println("New GET Connection!");
		Serial.println("Connection OK!");

		// Send our HTTP data!
		client.println("GET / HTTP/1.0");
		client.println("Host: 192.168.0.103:3000");
		client.println();

		receivedData = "";

		// Read data from the buffer

		while (receivedData.indexOf("\r\n\r\n") == -1)
		{
			memset(dataBuffer, 0x00, sizeof(dataBuffer));
			client.read(dataBuffer, sizeof(dataBuffer));
			receivedData += (const char *)dataBuffer;
		}
		// Print the string
		Serial.println(receivedData);

		// Stop the current connection
		client.stop();
	}
	else
	{
		Serial.println("Server connection failed. Trying again...");
	}
	return receivedData;
}

void sendToServer(String endPoint, String body)
{
	// POST Message
	if (client.connect(server, 3000))
	{
		// Print some information that we have connected to the server
		// Serial.println("**********************************!");
		// Serial.println("New POST Connection!");
		// Serial.println("Connection OK!");
		// Send our HTTP data!
		client.println("POST " + endPoint + " HTTP/1.0");
		client.println("Host: 192.168.0.103:3000");
		client.println("Content-Type: application/json");
		client.print("Content-Length: ");
		client.println(body.length());
		client.println();
		client.print(body);

		// receivedData = "";

		// // Read data from the buffer

		// while (receivedData.indexOf("\r\n\r\n") == -1)
		// {
		// 	memset(dataBuffer, 0x00, sizeof(dataBuffer));
		// 	client.read(dataBuffer, sizeof(dataBuffer));
		// 	receivedData += (const char *)dataBuffer;
		// }

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