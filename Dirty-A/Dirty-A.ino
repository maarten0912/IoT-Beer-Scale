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

const char* dirtya_root_ca = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIFTTCCBDWgAwIBAgISA0thyGN4KvIZjf3VrBgIjSOcMA0GCSqGSIb3DQEBCwUA\n" \
  "MDIxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQD\n" \
  "EwJSMzAeFw0yMDEyMjMwNTI5NTRaFw0yMTAzMjMwNTI5NTRaMC4xLDAqBgNVBAMT\n" \
  "I3Jlc2lkZW50aWVkaXJ0eWEuc3R1ZGVudC51dHdlbnRlLm5sMIIBIjANBgkqhkiG\n" \
  "9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtzmc2ihYCVsPrpGDOsnSQFQaLjl/RWlGKKPW\n" \
  "VnIbdw1NrET7l8fL/GfT6jFRo3N/EreY8XTh8TTeQ9vWwdjeNFonJYvITG5+BgZ8\n" \
  "+gw3pFFvGlFgYGkVfLYyccIilEeIRaX0mScD+e6ayxEeODk+yNNPZM9EUvDBEQcy\n" \
  "EBBeHXyCew/SdZuzh56HCWEcKpkoNmg5B5d0LO86AcLLgrYKa9ILWV3Z0WTGhOss\n" \
  "Jp4Rujd2Ar6mmrPXVYb7WrcIiYF/PKkkbdaDp1/WmyyqBweZjnPrz4yN76Ya2H2s\n" \
  "KMEHNuWy+NR7B19C/R4t7nGGCwgHLZrez8+hblXF7njvgRcMRQIDAQABo4ICXzCC\n" \
  "AlswDgYDVR0PAQH/BAQDAgWgMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcD\n" \
  "AjAMBgNVHRMBAf8EAjAAMB0GA1UdDgQWBBQ9sL3ZLwyuBJ4jpOHQfnwQv1AwbDAf\n" \
  "BgNVHSMEGDAWgBQULrMXt1hWy65QCUDmH6+dixTCxjBVBggrBgEFBQcBAQRJMEcw\n" \
  "IQYIKwYBBQUHMAGGFWh0dHA6Ly9yMy5vLmxlbmNyLm9yZzAiBggrBgEFBQcwAoYW\n" \
  "aHR0cDovL3IzLmkubGVuY3Iub3JnLzAuBgNVHREEJzAlgiNyZXNpZGVudGllZGly\n" \
  "dHlhLnN0dWRlbnQudXR3ZW50ZS5ubDBMBgNVHSAERTBDMAgGBmeBDAECATA3Bgsr\n" \
  "BgEEAYLfEwEBATAoMCYGCCsGAQUFBwIBFhpodHRwOi8vY3BzLmxldHNlbmNyeXB0\n" \
  "Lm9yZzCCAQUGCisGAQQB1nkCBAIEgfYEgfMA8QB2AESUZS6w7s6vxEAH2Kj+KMDa\n" \
  "5oK+2MsxtT/TM5a1toGoAAABdo5KK9kAAAQDAEcwRQIgNcdw4A9ziwaMUwPH6rYs\n" \
  "VW+7jO6Kn8xFONmlw8m8UdoCIQCfbVwF18pHqGkMqH9XxcTqfozmGGiCWnIncE6M\n" \
  "iQcZ/AB3APZclC/RdzAiFFQYCDCUVo7jTRMZM7/fDC8gC8xO8WTjAAABdo5KK8wA\n" \
  "AAQDAEgwRgIhAIBXU1DND2O50rO8AkZi9lJ21I7rgoVjEnDXpAUfruYtAiEAnEz1\n" \
  "brZDyIVAnmfJKjNXUm7/WzQflQmmM1DMVCuurggwDQYJKoZIhvcNAQELBQADggEB\n" \
  "AC6+R++mqwmw1OSDTGe+L4mh4pFsUR66winzz8Ij3+LLl8KToBzMy3lOxbjeyfrS\n" \
  "LvQ0nXAfWZco++BJJH3VqaxskwVBetwfHCQO11EsLrx+1itg3ZpxdheYzidvrK5h\n" \
  "t663YD6Jsy/86b4sbBP/foVgPjE99FZX9FGwHy9idJ6jL9ZMaLen5Xs02fmH3eth\n" \
  "dE4zGZg6/CjuGN+FLgixSDUXhMUYu616Cfr7HnWRMCbvLa0LM9p+8G0EzGyB0U8d\n" \
  "JENdiBnWg4EZUpznV6DxTnoFcGu4wUFoqr77/B6lXJBw7wsWLjYpfcXvxc0+n73v\n" \
  "V4fp2+41FdZ9EaMUScl3ooE=\n" \
  "-----END CERTIFICATE-----";


void setup() {
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  EEPROM.begin(256);
  
  // Value      | Address
  // API port   | 0 (length 2)
  // API host   | 2 (length 40)
  // measure    | 42 (length 30)
  // calibrate  | 72 (length 30)
  // wifi ssid  | 102 (length 30)
  // wifi pass  | 132 (length 30)
  // delay      | 162 (length 2)
  // tare offset| 164 (length 4)

  // WiFi creds
  ssid = readstring(102, 131);
  pass = readstring(132,161);

  // API endpoint
  host = readstring(2,41);
  port = readint(0);

  measure_endpoint = readstring(42,71);
  calibrate_endpoint = readstring(72,101);

  delaytime = readint(162);

  scale.set_offset(readlong(164));
  
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
  writeint(162, miliseconds);
}

void apifunction(String newhost, int newport) {
      String backup_host = host;
      int backup_port = port;
      if (newhost != "" && newhost.length() <= 40) {
        host = newhost;
        port = newport;
        writestring(2,newhost);
        writeint(0,newport);
        client.stop();
        if (port == 443) {
          client.setCACert(dirtya_root_ca);
        } else {
          client.setInsecure();
        }
        if (client.connect(host, port) != 1) {
          server.send(500, "text/plain", "Failed to connect to new API, falling back to previous API...");
          host = backup_host;
          port = backup_port;
          writestring(2,backup_host);
          writeint(0,backup_port);
          client.stop();
          if (port == 443) {
            client.setCACert(dirtya_root_ca);
          } else {
            client.setInsecure();
          }
          client.connect(host, port);
        } else {
          server.send(200, "text/plain", "OK");
        }
      }
}

void wififunction(String newssid, String newpass) {
  String backup_ssid = ssid;
  String backup_pass = pass;
  if (newssid != "" && newpass != "" && newssid.length() <= 30 && newpass.length() <= 30) {
    ssid = newssid;
    pass = newpass;
    writestring(102, newssid);
    writestring(132,newpass);
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
      writestring(102, backup_ssid);
      writestring(132,backup_pass);
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
      if (port == 443) {
        client.setCACert(dirtya_root_ca);
      } else {
        client.setInsecure();
      }
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
  writelong(164,scale.get_offset());
}

void endpointfunction(String new_measure_endpoint, String new_calibrate_endpoint) {
  if (new_measure_endpoint.length() <= 30 && new_calibrate_endpoint.length() <= 30) {
    measure_endpoint = new_measure_endpoint;
    calibrate_endpoint = new_calibrate_endpoint;
    writestring(42,new_measure_endpoint);
    writestring(72,new_calibrate_endpoint);
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
    if (port == 443) {
      client.setCACert(dirtya_root_ca);
    } else {
      client.setInsecure();
    }
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
