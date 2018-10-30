#include <ESP8266WiFi.h>
#include <HardwareSerial.h>
#include <string>
WiFiClient client;
const char * SSID = "Arduino_WiFi";
const char * pwd = "cAdexdy%2";
enum CMD : uint8_t {
	R_PLAYER_NAME,
	R_SELECTED_DMG,
	R_PLAYER_ID,
	R_START_GAME,
	R_KILLED_BY,
	R_KILL_CONFIRM,
	R_LAST_MINUTE,
	R_GAME_OVER,
	T_KILLED_BY,
	N_DEFAULT_EMPTY
};

class msg {
public:
	CMD command;
	std::string naam;
	uint8_t waarde = 0;

	msg() {
		command = CMD::N_DEFAULT_EMPTY;
		naam = "";
		waarde = 0;
	}

	msg(std::string basic_string) {
		int colon = basic_string.find(':');
		int comma = basic_string.find(',');
		int semicolon = basic_string.find(';');

		command = static_cast<CMD>(basic_string[colon+1] - '0');
		if(command == R_PLAYER_NAME || command == R_KILLED_BY){
			naam = basic_string.substr(comma+1);
		}else{
			waarde = static_cast<uint8_t>(basic_string[comma+1] - '0');
		}
	}

	std::string serialize(){
		std::string out = "CMD:";
		out += ((uint8_t)command + '0');
		out += ",";
		if(command == R_PLAYER_NAME || command == R_KILLED_BY){
			out += naam;
		}else{
			out += (waarde + '0');
		}
		out += ';';
		return out;
	}


};
void setup(){
	Serial.begin(115200);
	Serial1.begin(115200);
	Serial.println();
	Serial.print("Connecting to: ");
	Serial.print(SSID);
	Serial.print(" , with password: ");
	Serial.println(pwd);
	WiFi.begin("Arduino_WiFi", "cAdexdy%2");
	while(WiFi.status() != WL_CONNECTED){
		Serial.print("Couldn't connect: " );
		Serial.println(WiFi.status());
		delay(1000);
		Serial.print(".");
	}
	Serial.println();
	Serial.print("Connected, IP address: ");
	Serial.println(WiFi.localIP());
	Serial.println("Trying to connect to 192.168.137.1:8080");
	while(client.connect(IPAddress(192,168,137,1), 8080) != true){
		Serial.println("Retrying...");
		delay(500);
	}
	Serial.println("Connected to host!");
	client.write("Hello World!");
}
std::string incomingDue;
std::string incomingWifi;

void loop(){
	while(client.connected()){
		bool due_message_received = false;
		bool wifi_message_received = false;
		msg OUT;
		msg IN;
		if(Serial.available()) {
			while (Serial.available()) {
				due_message_received = true;
				incomingDue.push_back((char)Serial.read());
			}
			OUT = msg(incomingDue);
		}
		if(client.available()){
			while(client.available()){
				wifi_message_received = true;
				incomingWifi.push_back((char)client.read());
			}
			IN = msg(incomingWifi);
		}

		if(due_message_received && client.availableForWrite()){
			client.write(OUT.serialize().c_str());
			due_message_received = false;
		}
		if(wifi_message_received && Serial.availableForWrite()){
			Serial.write(IN.serialize().c_str());
			wifi_message_received = false;
		}

	}
	client.connect(IPAddress(192,168,137,1), 8080);
}

