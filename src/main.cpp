#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "InternetTime.h"
//#include "parser.h"
// Update these with values suitable for your network.

const char *ssid = "WIBER_DI Paola";
const char *password = "18663323";
const char *mqtt_server = "10.0.0.111";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (500)
char msg[MSG_BUFFER_SIZE];
String message;
String frame = "";

void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    ESP.wdtFeed();
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1')
  {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "INIT");
      // ... and resubscribe
      client.subscribe("inTopic");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  ESP.wdtDisable();
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1234);
  client.setCallback(callback);
  setUpTime();
}

void loop()
{
  ESP.wdtFeed();
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  /**
   * Obtengo la trama desde el medidor
   */
  //frame = getFromTerminal();

  unsigned long now = millis();
  if (now - lastMsg > 120000)
  {
    lastMsg = now;
    String time = getTime();
    // Serial.println(time);
    String date = getDate();
    // Serial.println(date);
    if (frame == "")
    {
      frame = F("r,250.00,21.99,22.42,1.02,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,5.50,13.01");
    }
    message = frame + "," + date + "," + time;

    const char *c = message.c_str();
    snprintf(msg, MSG_BUFFER_SIZE, "%s", c);
    Serial.print("Publish message: ");
    Serial.println(*c);
    client.publish("outTopic", msg);
    memset(msg, 0, sizeof(msg));
  }
  frame = "";
  //Serial.print("-");
  Serial.flush();
  
}
