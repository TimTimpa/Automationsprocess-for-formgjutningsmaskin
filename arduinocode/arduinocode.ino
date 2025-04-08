#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Encoder.h>
#include <AccelStepper.h>
#include <PID_v1.h>
#include <Ethernet.h>
#include <EthernetServer.h>
#include <EthernetClient.h>

// Definiera skärmens dimensioner och återställningspinne för OLED-skärmen
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

// Initialisera OLED-skärmen
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Initialisera encodern med pinnar D18 och D19
Encoder myEnc(18, 19);
int lastPos = -1;  // Senaste positionen för encodern
int currentPos = 0;  // Nuvarande positionen för encodern
int buttonPin = 22;  // Pin för inbyggd brytare
int selectedOption = -1;  // Vald menyalternativ

// Definiera menyalternativ
const char* options[] = {"Starta uppvärmning", "Starta laddning", "Status", "Felmeddelande"};
const int numOptions = sizeof(options) / sizeof(options[0]);  // Antal menyalternativ

// Definiera skärmens tillstånd
enum ScreenState { MAIN_MENU, TEMP_MENU, LOAD_MENU, STATUS_MENU };
ScreenState screenState = MAIN_MENU;  // Initiellt tillstånd för skärmen

// Initialisera LM35 temperatursensorens pin
const int tempPin = A1;
float currentTemp = 0;  // Nuvarande temperatur
int selectedTemp = 20;  // Vald temperatur

// Initialisera stegmotorn med drivrutins-pinnar D8 och D9
AccelStepper stepper(AccelStepper::DRIVER, 8, 9);
int microSwitchPin = A2;  // Pin för mikrobrytaren
int currentHolder = 0;  // Nuvarande hållarens position

bool heatingStarted = false;  // Flagga för att indikera om uppvärmningen har startat

// PID kontrollvariabler
double Setpoint, Input, Output;
double Kp = 2, Ki = 5, Kd = 1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
int relayPin = 4;  // Pin för reläet

// Ethernet inställningar
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);
EthernetServer server(80);

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);  // Sätt knappens pin som ingång med pull-up resistor
  pinMode(microSwitchPin, INPUT_PULLUP);  // Sätt mikrobrytarens pin som ingång med pull-up resistor
  pinMode(relayPin, OUTPUT);  // Sätt reläets pin som utgång
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialisera OLED-skärmen
  display.clearDisplay();  // Rensa skärmen
  display.display();  // Uppdatera skärmen
  myEnc.write(0);  // Återställ encoderpositionen
  stepper.setMaxSpeed(1000);  // Sätt maximal hastighet för stegmotorn
  stepper.setAcceleration(500);  // Sätt acceleration för stegmotorn
  myPID.SetMode(AUTOMATIC);  // Sätt PID-kontrollern till automatiskt läge

  // Starta Ethernet och servern
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.begin(115200);
  Serial.println("HTTP server started");
}

void loop() {
  currentPos = myEnc.read() / 4;  // Läs encoderpositionen och justera för upplösning om nödvändigt

  if (screenState == MAIN_MENU) {
    if (currentPos != lastPos) {  // Om encoderpositionen har ändrats
      lastPos = currentPos;  // Uppdatera senaste positionen
      displayMenu(currentPos % numOptions);  // Visa menyn med det nya markerade alternativet
    }

    if (digitalRead(buttonPin) == LOW) {  // Om knappen är nedtryckt
      selectedOption = currentPos % numOptions;  // Uppdatera det valda alternativet
      if (selectedOption == 0) {
        screenState = TEMP_MENU;  // Byt till temperaturmenyn
        myEnc.write(selectedTemp * 4);  // Sätt encoderpositionen till den valda temperaturen
      } else if (selectedOption == 1) {
        screenState = LOAD_MENU;  // Byt till laddningsmenyn
        myEnc.write(currentHolder * 4);  // Sätt encoderpositionen till den nuvarande hållarens position
      } else if (selectedOption == 2) {
        screenState = STATUS_MENU;  // Byt till statusmenyn
      } else {
        displayMenu(selectedOption);  // Visa menyn med det valda alternativet markerat
      }
      delay(500);  // Debounce fördröjning för att förhindra flera val
    }
  } else if (screenState == TEMP_MENU) {
    currentTemp = analogRead(tempPin) * (5.0 / 1023.0) * 100;  // Läs nuvarande temperatur från LM35-sensorn
    selectedTemp = myEnc.read() / 4;  // Läs den valda temperaturen från encodern

    displayTempMenu(currentTemp, selectedTemp);  // Visa temperaturmenyn

    if (digitalRead(buttonPin) == LOW) {  // Om knappen är nedtryckt
      heatingStarted = true;  // Indikera att uppvärmningen har startat
      Setpoint = selectedTemp;  // Sätt den önskade temperaturen
      screenState = MAIN_MENU;  // Byt tillbaka till huvudmenyn
      delay(500);  // Debounce fördröjning för att förhindra flera val
    }
  } else if (screenState == LOAD_MENU) {
    currentHolder = myEnc.read() / 4;  // Läs nuvarande hållarens position från encodern

    displayLoadMenu(currentHolder);  // Visa laddningsmenyn

    if (digitalRead(buttonPin) == LOW) {  // Om knappen är nedtryckt
      // Vrid till den valda hållaren
      if (currentHolder % 2 == 0) {
        rotateLeft();  // Vrid stegmotorn till vänster
      } else {
        rotateRight();  // Vrid stegmotorn till höger
      }
      screenState = MAIN_MENU;  // Byt tillbaka till huvudmenyn
      delay(500);  // Debounce fördröjning för att förhindra flera val
    }
  } else if (screenState == STATUS_MENU) {
    currentTemp = analogRead(tempPin) * (5.0 / 1023.0) * 100;  // Läs nuvarande temperatur från LM35-sensorn

    displayStatusMenu(currentTemp, heatingStarted);  // Visa statusmenyn

    if (digitalRead(buttonPin) == LOW) {  // Om knappen är nedtryckt
      screenState = MAIN_MENU;  // Byt tillbaka till huvudmenyn
      delay(500);  // Debounce fördröjning för att förhindra flera val
    }
  }

  if (heatingStarted) {
    Input = analogRead(tempPin) * (5.0 / 1023.0) * 100;  // Läs nuvarande temperatur från LM35-sensorn
    myPID.Compute();  // Beräkna PID-utgången
    if (Output > 0) {
      digitalWrite(relayPin, HIGH);  // Slå på reläet
    } else {
      digitalWrite(relayPin, LOW);  // Stäng av reläet
    }
  }

  // Hantera klientförfrågningar
  EthernetClient client = server.available();
  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            handleClientRequest(client);
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    delay(1);
    client.stop();
  }
}

void displayMenu(int highlight) {
  display.clearDisplay();  // Rensa skärmen
  display.setTextSize(1);  // Sätt textstorleken
  display.setTextColor(SSD1306_WHITE);  // Sätt textfärgen till vit

  for (int i = 0; i < numOptions; i++) {  // Loop genom varje menyalternativ
    if (i == highlight) {  // Om det nuvarande alternativet är markerat
      display.fillRect(0, i * 16, SCREEN_WIDTH, 16, SSD1306_WHITE);  // Rita en fylld rektangel för att markera alternativet
      display.setTextColor(SSD1306_BLACK);  // Sätt textfärgen till svart för markerat alternativ
    } else {
      display.setTextColor(SSD1306_WHITE);  // Sätt textfärgen till vit för icke-markerade alternativ
    }
    display.setCursor(0, i * 16);  // Sätt markörens position för varje alternativ
    display.println(options[i]);  // Skriv ut alternativet
  }

  display.display();  // Uppdatera skärmen med det nya innehållet
}

void displayTempMenu(float currentTemp, int selectedTemp) {
  display.clearDisplay();  // Rensa skärmen
  display.setTextSize(1);  // Sätt textstorleken
  display.setTextColor(SSD1306_WHITE);  // Sätt textfärgen till vit

  display.setCursor(0, 0);  // Sätt markörens position
  display.print("Current Temp: ");  // Skriv ut etiketten för nuvarande temperatur
  display.println(currentTemp);  // Skriv ut nuvarande temperatur

  display.setCursor(0, 16);  // Sätt markörens position
  display.print("Set Temp: ");  // Skriv ut etiketten för inställd temperatur
  display.println(selectedTemp);  // Skriv ut vald temperatur

  display.display();  // Uppdatera skärmen med det nya innehållet
}

void displayLoadMenu(int currentHolder) {
  display.clearDisplay();  // Rensa skärmen
  display.setTextSize(1);  // Sätt textstorleken
  display.setTextColor(SSD1306_WHITE);  // Sätt textfärgen till vit

  display.setCursor(0, 0);  // Sätt markörens position
  display.print("Select Holder: ");  // Skriv ut etiketten för vald hållare
  display.println(currentHolder);  // Skriv ut nuvarande hållare

  display.setCursor(0, 16);  // Sätt markörens position
  display.print("Rotate Left");  // Skriv ut alternativet för att vrida till vänster

  display.setCursor(0, 32);  // Sätt markörens position
  display.print("Rotate Right");  // Skriv ut alternativet för att vrida till höger

  display.display();  // Uppdatera skärmen med det nya innehållet
}

void displayStatusMenu(float currentTemp, bool heatingStarted) {
  display.clearDisplay();  // Rensa skärmen
  display.setTextSize(1);  // Sätt textstorleken
  display.setTextColor(SSD1306_WHITE);  // Sätt textfärgen till vit

  display.setCursor(0, 0);  // Sätt markörens position
  display.print("Current Temp: ");  // Skriv ut etiketten för nuvarande temperatur
  display.println(currentTemp);  // Skriv ut nuvarande temperatur

  display.setCursor(0, 16);  // Sätt markörens position
  display.print("Heating: ");  // Skriv ut etiketten för uppvärmningsstatus
  display.println(heatingStarted ? "Started" : "Not Started");  // Skriv ut uppvärmningsstatus

  display.setCursor(0, 32);  // Sätt markörens position
  display.print("Tillbaka");  // Skriv ut alternativet för att gå tillbaka

  display.display();  // Uppdatera skärmen med det nya innehållet
}

void rotateLeft() {
  stepper.moveTo(stepper.currentPosition() - 60);  // Flytta stegmotorn 60 grader till vänster
  while (stepper.distanceToGo() != 0) {  // Medan stegmotorn inte har nått målpositionen
    stepper.run();  // Kör stegmotorn
    if (digitalRead(microSwitchPin) == LOW) {  // Om mikrobrytaren är aktiverad
      stepper.stop();  // Stanna stegmotorn
      break;  // Avsluta loopen
    }
  }
}

void rotateRight() {
  stepper.moveTo(stepper.currentPosition() + 60);  // Flytta stegmotorn 60 grader till höger
  while (stepper.distanceToGo() != 0) {  // Medan stegmotorn inte har nått målpositionen
    stepper.run();  // Kör stegmotorn
    if (digitalRead(microSwitchPin) == LOW) {  // Om mikrobrytaren är aktiverad
      stepper.stop();  // Stanna stegmotorn
      break;  // Avsluta loopen
    }
  }
}

void handleClientRequest(EthernetClient client) {
  String request = client.readStringUntil('\r');
  client.flush();
  
  if (request.indexOf("GET / ") >= 0) {
    handleRoot(client);
  } else if (request.indexOf("GET /start-upvarmning") >= 0) {
    handleStartUpvarmning(client);
  } else if (request.indexOf("GET /start-produktion") >= 0) {
    handleStartProduktion(client);
  } else if (request.indexOf("GET /fyll-farg") >= 0) {
    handleFyllFarg(client);
  } else if (request.indexOf("GET /felmeddelanden") >= 0) {
    handleFelmeddelanden(client);
  } else if (request.indexOf("GET /status") >= 0) {
    handleStatus(client);
  } else if (request.indexOf("GET /nodstopp") >= 0) {
    handleNodstopp(client);
  } else if (request.indexOf("POST /set-temperature") >= 0) {
    handleSetTemperature(client, request);
  } else if (request.indexOf("POST /start-heating") >= 0) {
    handleStartHeating(client);
  } else if (request.indexOf("POST /stop-heating") >= 0) {
    handleStopHeating(client);
  }
}

void handleRoot(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println(R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Fjärrstyrning formsprutning</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          text-align: center;
          margin-top: 50px;
        }
        .status-indicators {
          position: absolute;
          top: 20px;
          right: 20px;
          display: flex;
          gap: 10px;
        }
        .status-indicator {
          width: 40px;
          height: 40px;
          border-radius: 50%;
        }
        #status-green {
          background-color: green;
        }
        #status-yellow {
          background-color: yellow;
        }
        #status-red {
          background-color: red;
        }
        button {
          display: block;
          width: 200px;
          height: 50px;
          margin: 10px auto;
          font-size: 16px;
          color: #fff;
          background-color: #007BFF;
          border: none;
          border-radius: 5px;
          cursor: pointer;
        }
        button:hover {
          background-color: #0056b3;
        }
        #emergency {
          width: 250px;
          height: 70px;
          font-size: 20px;
          background-color: #FF0000;
        }
        #emergency:hover {
          background-color: #CC0000;
        }
      </style>
    </head>
    <body>
      <div class="status-indicators">
        <div id="status-green" class="status-indicator"></div>
        <div id="status-yellow" class="status-indicator"></div>
        <div id="status-red" class="status-indicator"></div>
      </div>
      <h1>Fjärrstyrning formsprutning</h1>
      <button onclick="window.location.href='/start-upvarmning'">Starta uppvärmning</button>
      <button onclick="window.location.href='/start-produktion'">Starta produktion</button>
      <button onclick="window.location.href='/fyll-farg'">Fyll färg</button>
      <button onclick="window.location.href='/felmeddelanden'">Felmeddelanden</button>
      <button onclick="window.location.href='/status'">Status</button>
      <button id="emergency" onclick="window.location.href='/nodstopp'">Nödstopp</button>
    </body>
    </html>
  )rawliteral");
}

void handleStartUpvarmning() {
  String page = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Starta uppvärmning</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          text-align: center;
          margin-top: 50px;
        }
        input {
          display: block;
          width: 200px;
          height: 30px;
          margin: 10px auto;
          font-size: 16px;
          padding: 5px;
        }
        button {
          display: block;
          width: 200px;
          height: 50px;
          margin: 10px auto;
          font-size: 16px;
          color: #fff;
          background-color: #007BFF;
          border: none;
          border-radius: 5px;
          cursor: pointer;
        }
        button:hover {
          background-color: #0056b3;
        }
        #emergency {
          width: 250px;
          height: 70px;
          font-size: 20px;
          background-color: #FF0000;
        }
        #emergency:hover {
          background-color: #CC0000;
        }
      </style>
    </head>
    <body>
      <h1>Starta uppvärmning</h1>
      <p>Aktuell Temperatur: )rawliteral";
  page += String(actualTemperature) + "°C</p>";
  page += R"rawliteral(
      <p>Inställd Temperatur: )rawliteral";
  page += String(setTemperature) + "°C</p>";
  page += R"rawliteral(
      <form action="/set-temperature" method="post">
        <input type="number" name="temperature" placeholder="Ange temperatur" required>
        <button type="submit">Ange temperatur</button>
      </form>
      <button onclick="startHeating()">Starta uppvärmning</button>
      <button onclick="stopHeating()">Stoppa uppvärmning</button>
      <button onclick="window.location.href='/'">Tillbaka</button>
      <button id="emergency" onclick="window.location.href='/nodstopp'">Nödstopp</button>

      <script>
        function startHeating() {
          fetch('/start-heating', { method: 'POST' });
        }
        function stopHeating() {
          fetch('/stop-heating', { method: 'POST' });
        }
      </script>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", page);
}

void handleStartProduktion() {
  server.send(200, "text/html", "<h1>Starta produktion</h1>");
}

void handleFyllFarg() {
  server.send(200, "text/html", "<h1>Fyll färg</h1>");
}

void handleFelmeddelanden() {
  server.send(200, "text/html", "<h1>Felmeddelanden</h1>");
}

void handleStatus() {
  server.send(200, "text/html", "<h1>Status</h1>");
}

void handleNodstopp() {
  server.send(200, "text/html", "<h1>Nödstopp</h1>");
}

void handleSetTemperature() {
  if (server.hasArg("temperature")) {
    setTemperature = server.arg("temperature").toFloat();
    server.send(200, "text/html", "<h1>Temperatur inställd</h1><a href='/start-upvarmning'>Tillbaka</a>");
  } else {
    server.send(400, "text/html", "<h1>Fel: Temperatur ej angiven</h1><a href='/start-upvarmning'>Tillbaka</a>");
  }
}

void handleStartHeating() {
  // Logic to start heating
  server.send(200, "text/html", "<h1>Uppvärmning startad</h1><a href='/start-upvarmning'>Tillbaka</a>");
}

void handleStopHeating() {
  // Logic to stop heating
  server.send(200, "text/html", "<h1>Uppvärmning stoppad</h1><a href='/start-upvarmning'>Tillbaka</a>");
}