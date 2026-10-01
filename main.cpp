#include <WiFi.h>
#include <WebServer.h>

const char* ap_ssid = "Mecanum-Bot";
const char* ap_password = "12345678password";

WebServer server(80);

#define FL_IN1 26
#define FL_IN2 27
#define FR_IN1 14
#define FR_IN2 12
#define RL_IN1 33
#define RL_IN2 32
#define RR_IN1 15
#define RR_IN2 4

#define PWM_FREQ 5000
#define PWM_RES 8

int motorSpeed = 200;

void setupMotors() {
  pinMode(FL_IN1, OUTPUT); pinMode(FL_IN2, OUTPUT);
  pinMode(FR_IN1, OUTPUT); pinMode(FR_IN2, OUTPUT);
  pinMode(RL_IN1, OUTPUT); pinMode(RL_IN2, OUTPUT);
  pinMode(RR_IN1, OUTPUT); pinMode(RR_IN2, OUTPUT);

  ledcAttachChannel(FL_IN1, PWM_FREQ, PWM_RES, 0);
  ledcAttachChannel(FL_IN2, PWM_FREQ, PWM_RES, 1);
  ledcAttachChannel(FR_IN1, PWM_FREQ, PWM_RES, 2);
  ledcAttachChannel(FR_IN2, PWM_FREQ, PWM_RES, 3);
  ledcAttachChannel(RL_IN1, PWM_FREQ, PWM_RES, 4);
  ledcAttachChannel(RL_IN2, PWM_FREQ, PWM_RES, 5);
  ledcAttachChannel(RR_IN1, PWM_FREQ, PWM_RES, 6);
  ledcAttachChannel(RR_IN2, PWM_FREQ, PWM_RES, 7);
  
  stopMotors();
}

void setMotor(int in1Pin, int in2Pin, int speed) {
  if (speed > 0) {
    ledcWrite(in1Pin, speed);
    ledcWrite(in2Pin, 0);
  } else if (speed < 0) {
    ledcWrite(in1Pin, 0);
    ledcWrite(in2Pin, -speed);
  } else {
    ledcWrite(in1Pin, 0);
    ledcWrite(in2Pin, 0);
  }
}

void stopMotors() {
  setMotor(FL_IN1, FL_IN2, 0);
  setMotor(FR_IN1, FR_IN2, 0);
  setMotor(RL_IN1, RL_IN2, 0);
  setMotor(RR_IN1, RR_IN2, 0);
}

void moveMecanum(int fl, int fr, int rl, int rr) {
  setMotor(FL_IN1, FL_IN2, fl);
  setMotor(FR_IN1, FR_IN2, fr);
  setMotor(RL_IN1, RL_IN2, rl);
  setMotor(RR_IN1, RR_IN2, rr);
}

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Mecanum ESP32 Controller</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #121212;
            color: #ffffff;
            text-align: center;
            margin: 0;
            padding: 20px;
            user-select: none;
        }
        h1 { margin-bottom: 5px; color: #00e676; }
        .container {
            max-width: 500px;
            margin: 0 auto;
        }
        .status {
            font-size: 14px;
            color: #aaa;
            margin-bottom: 20px;
        }
        .grid {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 10px;
            margin: 20px 0;
        }
        .btn {
            background: #1e1e1e;
            border: 2px solid #00e676;
            color: white;
            padding: 20px 0;
            font-size: 20px;
            font-weight: bold;
            border-radius: 12px;
            cursor: pointer;
            transition: 0.1s;
        }
        .btn:active, .btn.active {
            background: #00e676;
            color: black;
            box-shadow: 0 0 15px #00e676;
        }
        .slider-container {
            margin: 30px 0;
            background: #1e1e1e;
            padding: 15px;
            border-radius: 10px;
        }
        input[type=range] {
            width: 80%;
            accent-color: #00e676;
        }
        .key-hint {
            font-size: 12px;
            color: #888;
            margin-top: 15px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Mecanum Bot</h1>
        <div class="status" id="status">Status: Connected</div>

        <div class="grid">
            <div></div>
            <button class="btn" id="btn-W">W<br>&#8593;</button>
            <div></div>
            <button class="btn" id="btn-A">A<br>&#8592;</button>
            <button class="btn" id="btn-STOP" style="border-color:#ff5252">STOP</button>
            <button class="btn" id="btn-D">D<br>&#8594;</button>
            <div></div>
            <button class="btn" id="btn-S">S<br>&#8595;</button>
            <div></div>
        </div>

        <div class="grid" style="margin-top: 10px;">
            <button class="btn" id="btn-Q">Q<br>&#8630; Rotate L</button>
            <div></div>
            <button class="btn" id="btn-E">E<br>&#8631; Rotate R</button>
        </div>

        <div class="slider-container">
            <label for="speed">Speed: <span id="speed-val">200</span></label><br><br>
            <input type="range" id="speed" min="50" max="255" value="200" oninput="updateSpeed(this.value)">
        </div>

        <div class="key-hint">Use <b>W A S D</b> to move strafe/forward, <b>Q / E</b> to turn, and <b>SPACE</b> to stop.</div>
    </div>

    <script>
        let activeKey = null;

        function sendCommand(cmd) {
            fetch('/cmd?dir=' + cmd)
                .catch(err => console.error(err));
        }

        function updateSpeed(val) {
            document.getElementById('speed-val').innerText = val;
            fetch('/speed?val=' + val);
        }

        const keyMap = {
            'w': 'W', 'W': 'W', 'ArrowUp': 'W',
            's': 'S', 'S': 'S', 'ArrowDown': 'S',
            'a': 'A', 'A': 'A', 'ArrowLeft': 'A',
            'd': 'D', 'D': 'D', 'ArrowRight': 'D',
            'q': 'Q', 'Q': 'Q',
            'e': 'E', 'E': 'E',
            ' ': 'STOP'
        };

        window.addEventListener('keydown', (e) => {
            if (e.repeat) return;
            const dir = keyMap[e.key];
            if (dir && activeKey !== dir) {
                activeKey = dir;
                highlightButton(dir, true);
                sendCommand(dir);
            }
        });

        window.addEventListener('keyup', (e) => {
            const dir = keyMap[e.key];
            if (dir && activeKey === dir) {
                activeKey = null;
                highlightButton(dir, false);
                sendCommand('STOP');
            }
        });

        function highlightButton(dir, active) {
            const btn = document.getElementById('btn-' + dir);
            if (btn) {
                if (active) btn.classList.add('active');
                else btn.classList.remove('active');
            }
        }

        ['W', 'A', 'S', 'D', 'Q', 'E', 'STOP'].forEach(dir => {
            const btn = document.getElementById('btn-' + dir);
            if (!btn) return;
            
            const startHandler = (e) => {
                e.preventDefault();
                sendCommand(dir);
                btn.classList.add('active');
            };
            const endHandler = (e) => {
                e.preventDefault();
                sendCommand('STOP');
                btn.classList.remove('active');
            };

            btn.addEventListener('mousedown', startHandler);
            btn.addEventListener('mouseup', endHandler);
            btn.addEventListener('touchstart', startHandler);
            btn.addEventListener('touchend', endHandler);
        });
    </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", HTML_PAGE);
}

void handleCommand() {
  if (server.hasArg("dir")) {
    String dir = server.arg("dir");
    int s = motorSpeed;

    if (dir == "W") {
      moveMecanum(s, s, s, s);
    } else if (dir == "S") {
      moveMecanum(-s, -s, -s, -s);
    } else if (dir == "A") {
      moveMecanum(-s, s, s, -s);
    } else if (dir == "D") {
      moveMecanum(s, -s, -s, s);
    } else if (dir == "Q") {
      moveMecanum(-s, s, -s, s);
    } else if (dir == "E") {
      moveMecanum(s, -s, s, -s);
    } else {
      stopMotors();
    }
  }
  server.send(200, "text/plain", "OK");
}

void handleSpeed() {
  if (server.hasArg("val")) {
    motorSpeed = server.arg("val").toInt();
  }
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  setupMotors();

  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("Access Point Started!");
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/cmd", handleCommand);
  server.on("/speed", handleSpeed);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
