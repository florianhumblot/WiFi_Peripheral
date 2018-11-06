#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <string>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

WiFiClient client;
const char *SSID = "hotspot55";
const char *pwd = "hotspot55";
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 4, /* clock=*/ 14, /* data=*/ 2);
SoftwareSerial due(12, 13, false, 256);
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

		command = static_cast<CMD>(basic_string[colon + 1] - '0');
		if (command == R_PLAYER_NAME || command == R_KILLED_BY) {
			naam = basic_string.substr(comma + 1, semicolon-1);
		} else {
			waarde = static_cast<uint8_t>(basic_string[comma + 1] - '0');
		}
	}

	std::string serialize() {
		std::string out = "CMD:";
		out += ((uint8_t) command + '0');
		out += ",";
		if (command == R_PLAYER_NAME || command == R_KILLED_BY) {
			out += naam;
		} else {
			out += (waarde + '0');
			out += ';';
		}
		return out;
	}


};

void setup() {
	pinMode(12, INPUT);
	pinMode(13, OUTPUT);

	u8g2.begin();
	u8g2.setFont(u8g2_font_5x7_mf );
	u8g2.clearBuffer();
	u8g2.drawStr(0,20,"Connecting to network");
	u8g2.sendBuffer();



	due.begin(2400);
	delay(1000);
	WiFi.begin(SSID, pwd);
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		u8g2.clearBuffer();
		u8g2.drawStr(0, 20,"Connecting...");
		u8g2.sendBuffer();
	}
	while (client.connect(IPAddress(192, 168, 137, 1), 8080) != true) {
		delay(500);
	}
	u8g2.clearBuffer();
	u8g2.drawStr(0, 10, "Connected!");
	u8g2.sendBuffer();

}



std::string incomingDue;
std::string incomingWifi;

void loop() {
	while(WiFi.status() == WL_CONNECTED) {
		while (client.connected()) {
			bool due_message_received = false;
			bool wifi_message_received = false;
			msg OUT;
			msg IN;
			if (due.available()) {
				while (due.available()) {
					due_message_received = true;
					incomingDue.push_back((char) due.read());
				}
				u8g2.clearBuffer();
				u8g2.drawStr(0, 20, incomingDue.c_str());
				u8g2.sendBuffer();
				OUT = msg(incomingDue);
				incomingDue = "";
			}
			if (client.available()) {
				while (client.available()) {
					wifi_message_received = true;
					incomingWifi.push_back((char) client.read());
				}
				u8g2.clearBuffer();
				u8g2.drawStr(0, 30, incomingWifi.c_str());
				u8g2.sendBuffer();
				IN = msg(incomingWifi);
				incomingWifi = "";
			}

			if (due_message_received && client.availableForWrite()) {
				client.write(OUT.serialize().c_str());
				due_message_received = false;
				delay(500);
			}
			if (wifi_message_received) {
				due.write(IN.serialize().c_str());
				wifi_message_received = false;
				delay(500);
			}

		}
		client.connect(IPAddress(192, 168, 137, 1), 8080);
	}
	WiFi.begin(SSID, pwd);
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
	}
}

