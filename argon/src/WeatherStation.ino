TCPClient client;										// Create TCP Client object
byte server[] = {192, 168, 0, 103}; // http://maker-io-iot.atwebpages.com/
byte dataBuffer[1024];

String receivedData;
int termo;
int barometre;

// Returns a json formatted key value object
String kv(String a, String b)
{
	String s("{\"");
	s += a;
	s += "\" : \"";
	s += b;
	s += "\"}";
	return s;
}

// String jsonObject(String[] keyValues){
// 	if(keyValues.length() % 2 != 0){
// 		return String("");
// 	}
// }

void setup()
{
	Serial.begin(9600);
	termo = 0;
	barometre = 0;
}

void loop()
{
	delay(1000);
	sendToServer(kv("termo", String(termo)));
	sendToServer(kv("barometre", String(barometre)));
	termo++;
	barometre += 2;
}

String getFromServer() {
	//GET Message
	if(client.connect(server, 3000))
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

	    while(receivedData.indexOf("\r\n\r\n") == -1)
	    {
	        memset(dataBuffer, 0x00, sizeof(dataBuffer));
	        client.read(dataBuffer, sizeof(dataBuffer));
	        receivedData += (const char*)dataBuffer;
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

void sendToServer(String body){
	// POST Message
	if (client.connect(server, 3000))
	{
		// Print some information that we have connected to the server
		// Serial.println("**********************************!");
		// Serial.println("New POST Connection!");
		// Serial.println("Connection OK!");
		// Send our HTTP data!
		client.println("POST /json HTTP/1.0");
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