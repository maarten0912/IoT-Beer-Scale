#include "HX711.h"
#include "EEPROM.h"
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = D5;
const int LOADCELL_SCK_PIN = D6;


WiFiClientSecure client;

// Listening endpoint
int listenport = 6969;
ESP8266WebServer server(listenport);

HX711 scale;

// WiFi creds
String ssid;
String pass;

// API endpoint
String host;
int port;

String measure_endpoint;
String calibrate_endpoint;

int delaytime = 1000;

void setup() {
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  EEPROM.begin(512);
  
  // Value      | Address
  // API port   | 0 (length 2)
  // API host   | 2 (length 60)
  // measure    | 62 (length 60)
  // calibrate  | 122 (length 60)
  // wifi ssid  | 182 (length 60)
  // wifi pass  | 242 (length 60)
  // delay      | 302 (length 2)
  // tare offset| 304 (length 4)


  // WiFi creds
  ssid = readstring(182, 241);
  pass = readstring(242,301);

  // API endpoint
  host = readstring(2,61);
  port = readint(0);

  measure_endpoint = readstring(62,121);
  calibrate_endpoint = readstring(122,181);

  delaytime = readint(302);

  scale.set_offset(readlong(304));
  
  // Connecting to WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  server.on("/", HTTP_POST, handleRoot);
  server.begin();
}

void handleRoot() {
  String command = server.arg("plain");
  int delimiter = command.indexOf(' ');
  int delimiter2 = command.indexOf(' ', delimiter + 1);

  if (command != "") {
    if (command.startsWith("delay")) {
      if (delimiter >= 0) {
        delayfunction(command.substring(delimiter + 1).toInt());
        server.send(200, "text/plain", "OK");
      }
      else {
        server.send(400, "text/plain", "Could not parse argument. Use 'delay <miliseconds>'");
      }
    } else if (command.startsWith("api")) {
      if (delimiter >= 0 && delimiter2 >= delimiter) {
        apifunction(command.substring(delimiter + 1, delimiter2), command.substring(delimiter2 + 1).toInt());
        // Response is dealt with in apifunction
      }
      else {
        server.send(400, "text/plain", "Could not parse argument. Use 'api <newhost> <newport>'");
      }
    } else if (command.startsWith("wifi")) {
      if (delimiter >= 0 && delimiter2 >= delimiter) {
        server.send(200, "text/plain", "Trying to change WiFi... Will fall back on failure");
        delay(1000); // so that we can still send the response
        wififunction(command.substring(delimiter + 1, delimiter2), command.substring(delimiter2 + 1));
        // Response is dealt with in wififunction    
      }
      else {
        server.send(400, "text/plain", "Could not parse argument. Use 'wifi <newssid> <newpassword>'");
      }
    } else if (command.startsWith("kg")) {
      if (delimiter >= 0 && delimiter2 >= delimiter) {
        kgfunction(command.substring(delimiter + 1, delimiter2).toFloat(), command.substring(delimiter2 + 1).toInt());
        server.send(200, "text/plain", "OK");
      }
      else {
        server.send(400, "text/plain", "Could not parse argument. Use 'kg <weight> <number of measures>'");
      }
    } else if (command.startsWith("tare")) {
      tarefunction();
      // OK is sent in tarefunction
    } else if (command.startsWith("endpoint")) {
      if (delimiter >= 0 && delimiter2 >= delimiter) {
        endpointfunction(command.substring(delimiter + 1, delimiter2), command.substring(delimiter2 + 1));
        server.send(200, "text/plain", "OK");
      }
      else {
        server.send(400, "text/plain", "Could not parse argument. Use 'endpoint <measure> <calibrate>'");
      }
    } else {
      server.send(400, "text/plain", "Command '" + command + "' not recognized");
    }
  } else {
    server.send(400, "text/plain", "Could not parse command");
  }
}

void delayfunction(int miliseconds) {
  delaytime = miliseconds;
  writeint(302, miliseconds);
}

void apifunction(String newhost, int newport) {
      String backup_host = host;
      int backup_port = port;
      if (newhost != "" && newhost.length() <= 60) {
        host = newhost;
        port = newport;
        writestring(2,newhost);
        writeint(0,newport);
        client.stop();
        client.setInsecure();
        if (client.connect(host, port) != 1) {
          server.send(500, "text/plain", "Failed to connect to new API, falling back to previous API...");
          host = backup_host;
          port = backup_port;
          writestring(2,backup_host);
          writeint(0,backup_port);
          client.stop();
          client.setInsecure();
          client.connect(host, port);
        } else {
          server.send(200, "text/plain", "OK");
        }
      }
}

void wififunction(String newssid, String newpass) {
  String backup_ssid = ssid;
  String backup_pass = pass;
  if (newssid != "" && newpass != "" && newssid.length() <= 60 && newpass.length() <= 60) {
    ssid = newssid;
    pass = newpass;
    writestring(182, newssid);
    writestring(242,newpass);
    WiFi.disconnect();
    WiFi.begin(ssid, pass);
    int wificounter = 0;
    while (WiFi.status() != WL_CONNECTED && wificounter < 20) {
      delay(500);
      wificounter++;
    }
    if (WiFi.status() != WL_CONNECTED) {
      ssid = backup_ssid;
      pass = backup_pass;
      writestring(182, backup_ssid);
      writestring(242,backup_pass);
      WiFi.begin(ssid, pass);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
      }
    }
  }
}

void kgfunction(float weight, int measurements) {
  if (weight >= 0 && measurements >= 1) {
    server.send(200, "text/plain", "OK");
    long avg = 0;
    for (int i = 0; i < measurements; i++) {
      avg += scale.get_value(255);
    }
    avg = avg / measurements;
    String data = "{\"averagereading\": ";
    data += String(avg);
    data += ", \"weight\": ";
    data += String(weight);
    data += "}";
    
    if (!client.connected()) {
      client.setInsecure();
      client.connect(host, port);
    }
    
    // HTTP header
    client.print("POST ");
    client.print(calibrate_endpoint);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.print(host);
    client.print(":");
    client.println(port);
    client.println("User-Agent: ESP8266 scale");
    client.println("Connection: close");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
  
    // HTTP body
    client.println(data);
  
    client.stop();
  } else {
    server.send(400, "text/plain", "Weight or measurement has an invalid value");
  }

}

void tarefunction() {
  server.send(200, "text/plain", "OK");
  scale.tare(255);
  writelong(304,scale.get_offset());
}

void endpointfunction(String new_measure_endpoint, String new_calibrate_endpoint) {
  if (new_measure_endpoint.length() <= 60 && new_calibrate_endpoint.length() <= 60) {
    measure_endpoint = new_measure_endpoint;
    calibrate_endpoint = new_calibrate_endpoint;
    writestring(62,new_measure_endpoint);
    writestring(122,new_calibrate_endpoint);
  }
}

void writestring(int address, String data) {
  int stringSize = data.length();
  for(int i=0; i<stringSize;i++) {
    EEPROM.write(address+i, data[i]);
  }
  EEPROM.write(address + stringSize, '\0');
  EEPROM.commit();
}

void writeint(int address, int data) {
  EEPROM.write(address, data >> 8);
  EEPROM.write(address + 1, data & 0xFF);
  EEPROM.commit();
}

void writelong(int address,long data) {
  EEPROM.write(address, (data & 0xFF));
  EEPROM.write(address + 1, ((data >> 8) & 0xFF));
  EEPROM.write(address + 2, ((data >> 16) & 0xFF));
  EEPROM.write(address + 3, ((data >> 24) & 0xFF));
  EEPROM.commit();
}

String readstring(int startAddress, int endAddress) {
  int maxLength = endAddress - startAddress + 1;
  char data[maxLength];
  unsigned char k = EEPROM.read(startAddress);
  int len = 0;
  while(k != '\0' && len < maxLength) {
    k = EEPROM.read(startAddress + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  return String(data);
}

int readint(int startAddress) {
  return (EEPROM.read(startAddress) << 8) + EEPROM.read(startAddress + 1);
}

int readlong(int startAddress) {
  long four = EEPROM.read(startAddress);
  long three = EEPROM.read(startAddress + 1);
  long two = EEPROM.read(startAddress + 2);
  long one = EEPROM.read(startAddress + 3);
 
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void loop() {
  // Reconnect to WiFi if not connected
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
  }

  // Handle our client
  server.handleClient();

  // Measure
  String data = "{\"reading\": ";
  data += String((int) scale.get_value(100));
  data += "}";
    
  if (!client.connected()) {
    client.setInsecure();
    client.connect(host, port);
  }

  // HTTP header
  client.print("POST ");
  client.print(measure_endpoint);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.print(host);
  client.print(":");
  client.println(port);
  client.println("User-Agent: ESP8266 scale");
  client.println("Connection: close");
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(data.length());
  client.println();
  
  // HTTP body
  client.println(data);

  client.stop();

  //Save power by putting ADC in sleep mode
  if (delaytime > 1000) {
    scale.power_down(); 
  }
  if (delaytime > 10) {
    delay(delaytime);
  }
  if (delaytime > 1000) {
    scale.power_up();
  }
}
