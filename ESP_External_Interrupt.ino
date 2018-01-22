#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
const char* ssid = "xxxxxxxxxxxxx";
const char* password = "xxxxxxxxxxx";
IPAddress server(192, 168, 0, xxx); // MQTT Server IP

const char* outTopic = "home/espexternalinterrupt"; // MQTT topic where status is published
int holdPin = 0;  // defines GPIO 0 as the hold pin (will hold CH_PD high untill we power down).
int pirPin = 12;  // defines GPIO 12 as the PIR read pin (reads the state of the PIR output).
int pir = 1;      // sets the PIR record (pir) to 1 (it must have been we woke up).

WiFiClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("firealarm1")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void wifi_connect() {
  // config static IP
  IPAddress ip(192, 168, 1, yy); // where yy is the desired IP Address
  IPAddress gateway(192, 168, 1, 1); // set gateway to match your network
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
  IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network
  WiFi.config(ip, gateway, subnet);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  pinMode(holdPin, OUTPUT);     // sets GPIO 0 to output
  digitalWrite(holdPin, HIGH);  // sets GPIO 0 to high (this holds CH_PD high even if the PIR output goes low)
  pinMode(pirPin, INPUT);       // sets GPIO 12 to an input so we can read the PIR output state

  Serial.begin(115200);         // Start Serial connection
  client.setServer(server, 1883); // setup MQTT prameters
  wifi_connect();               // Setup WiFi Connection
  pinMode(LED_BUILTIN, OUTPUT); /////////////////////
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  digitalWrite(LED_BUILTIN, LOW); ///////////////
  
  if((pir) == 0){  // if (pir) == 0, which its not first time through as we set it to "1" above
    client.publish(outTopic,"OFF");
    Serial.println("OFF");
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH); ///////////////
    client.disconnect(); // disconnect from MQTT
    ethClient.stop();    // close WiFi client
    delay(1000);         // wait for client to close
    digitalWrite(holdPin, LOW);  // set GPIO 0 low this takes CH_PD & powers down the ESP
  }else{                 // if (pir) == 0 is not true
    client.publish(outTopic,"ON");
    Serial.println("ON");
    while(digitalRead(pirPin) == 1){  // read GPIO 12, while GPIO 12 = 1 is true, wait (delay below) & read again, when GPIO 2 = 1 is false skip delay & move on out of "while loop"
      delay(500);
      client.loop();
      Serial.print(".");
    }
    pir = 0;             // set the value of (pir) to 0
    delay(2000);        // wait 20 sec
  }
  // end of void loop, returns to start loop again at void loop
}
