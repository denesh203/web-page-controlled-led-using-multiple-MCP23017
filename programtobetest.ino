#include <ESP8266WiFi.h>
#include <MCP23017.h>
#define MCP23017_I2C_ADDRESS 0x20 
#define MCP123017_I2C_ADDRESS 0x21
MCP23017 mcp23017 = MCP23017(MCP23017_I2C_ADDRESS);
MCP23017 mcp123017 = MCP23017(MCP123017_I2C_ADDRESS);

const char* ssid = "IODATA-d08ee8-2G"; 
const char* password = "6938254582088";
// Set web server port number to 80
WiFiServer server(80);
// Variable to store the HTTP request
String header;
// Auxiliar variables to store the current output state
String output0State = "off";
String output1State = "off";
// Assign output variables to GPIO pins
const uint8_t led0 = 1;  
const uint8_t led1 = 0;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
void setup() {
    {
     Wire.begin(0x20); 
  mcp23017.init();
  Wire.begin(0x21); 
  mcp123017.init();
  configurePinsWithPinMode();
     mcp123017.writeRegister(MCP23017Register::GPIO_A, 0x00);
   mcp23017.writeRegister(MCP23017Register::GPIO_B, 0x00);
    }
  Serial.begin(115200);
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}
void configurePinsWithPinMode() 
{
   mcp23017.pinMode(led0, OUTPUT);
   mcp23017.digitalWrite(led0,LOW);
   mcp123017.pinMode(led1, OUTPUT);
     mcp123017.digitalWrite(led1,LOW);
}
void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            // turns the GPIOs on and off
            if (header.indexOf("GET /1/on") >= 0)
            { 
              output0State = "on";
              mcp23017.digitalWrite(led0, HIGH);
            } else if (header.indexOf("GET /1/off") >= 0) {
                Serial.println("GPIO 5 off");
              output0State = "off";
              mcp23017.digitalWrite(led0, LOW);
            }
            else if (header.indexOf("GET /0/on") >= 0) {
              output1State = "on";
              mcp123017.digitalWrite(led1, HIGH);
            } else if (header.indexOf("GET /0/off") >= 0) {
              Serial.println("GPIO 4 off");
              output1State = "off";
              mcp123017.digitalWrite(led1, LOW);
            }
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1><center>WELCOME TO CONTROL PANEL</center></h1>");
              client.println("<marquee>visit miraclebyece.blogspot.com for more details</marquee>");
            // Display current state, and ON/OFF buttons for GPIO 5  
            client.println("<p>PA0 " + output0State + "</p>");
            // If the output5State is off, it displays the ON button       
            if (output0State=="off") {
              client.println("<p><a href=\"/1/on\"><button class=\"button\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/1/off\"><button class=\"button button2\">ON</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 4  
            client.println("<p>PA1 " + output1State + "</p>");
            // If the output4State is off, it displays the ON button       
            if (output1State=="off") {
              client.println("<p><a href=\"/0/on\"><button class=\"button\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/0/off\"><button class=\"button button2\">ON</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
