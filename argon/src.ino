const IPAddress IPaddr(192, 168, 0, 196); // server IP
const int port = 1337; // Port number

int led = D6; // DEL
int ptr = A0; // Phototransistor
int analogvalue = 0; // valeur lue au Phototransistor
int command = 0; // valeur lue au port serie
TCPClient client;

void setup() {
    Serial.begin(); // console USB
    
    pinMode(led, OUTPUT);
}

void loop() {
    
    
    if (Serial.available() > 0) {
        command = Serial.read();
    }
    
    if(command == 72){ // H
        // aucune action sur la LED donc LED figée
        
    }else if (command == 82){ // R
        analogvalue = analogRead(ptr); // lecture du phototransistor
        analogWrite(led, analogvalue/16); // affecter la valeur lue à la DEL
        
    }else if (command == 77){ // M
        analogWrite(led, 255); // LED au max
        
    }else if (command == 73){ // I
        analogWrite(led, 0); // LED éteinte
    }
  
   
    delay(10000); // délais de 10 sec
    analogvalue = analogRead(ptr); // lecture du phototransistor
    sendData(); // Envois analogvalue au serveur 
}

// Connect to server and send data buffer
void sendData() {

    int rc = client.connect(IPaddr, port);

    if (client.connected()) {
        
        // Format data to JSON object
        char buf[128];
        JSONBufferWriter writer(buf, sizeof(buf) - 1);
        
        writer.beginObject();
            writer.name("Val").value(analogvalue);
        writer.endObject();
        writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;

        // TCP write fully handles communication to server.
        client.write(buf);
        client.stop();
    } else {
        Serial.println("Connection failed!");           // Debug text
    }
}