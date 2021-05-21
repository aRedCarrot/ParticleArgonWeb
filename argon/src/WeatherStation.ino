#include "../lib/google-maps-device-locator/src/google-maps-device-locator.h"
#include "../lib/JsonParserGeneratorRK/src/JsonParserGeneratorRK.h"

GoogleMapsDeviceLocator locator;
TCPClient client;										// Create TCP Client object
byte server[] = {192, 168, 0, 103}; // http://maker-io-iot.atwebpages.com/
byte dataBuffer[1024];

String receivedData;
int termo;
int barometre;

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

void setup()
{
	Serial.begin(9600);
	termo = 0;
	barometre = 0;
	// Google map locator API
	locator.withSubscribe(locationCallback).withLocatePeriodic(30); // Every 5 minutes post the location
}

void loop()
{
	// Google map locator API
	locator.loop();
	//sendToServer(kv("termo", String(termo)));
	//sendToServer(kv("barometre", String(barometre)));
	termo++;
	barometre += 2;
	//delay(1000);
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
		//String body = kv("termo", String(termo));
		// Send our HTTP data!
		client.println("POST " + endPoint + " HTTP/1.0");
		client.println("Host: 192.168.0.103:3000");
		client.println("Content-Type: application/json");
		client.print("Content-Length: ");
		client.println(body.length());
		client.println();
		client.print(body);

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
}