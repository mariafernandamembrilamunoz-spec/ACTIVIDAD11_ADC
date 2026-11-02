// ACTIVIDAD 11: Control de motores DC con PWM (ESP32 + L298N) por Monitor Serial

#include <Arduino.h>

constexpr uint8_t M1_PIN_A = 18;  
constexpr uint8_t M1_PIN_B = 19;  
constexpr uint8_t M2_PIN_A = 21;  
constexpr uint8_t M2_PIN_B = 22;  

constexpr uint32_t PWM_FREQ = 20000;  
constexpr uint8_t PWM_RESOLUTION = 8; 


constexpr uint8_t VEL_DETENIDO = 0;
constexpr uint8_t VEL_BAJA = 85;   
constexpr uint8_t VEL_MEDIA = 170; 
constexpr uint8_t VEL_ALTA = 255;  

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  #define PWM_ATTACH(pin, ch) ledcAttachChannel(pin, PWM_FREQ, PWM_RESOLUTION, ch)
  #define PWM_WRITE(pin, ch, duty) ledcWrite(pin, duty)
#else
  #define PWM_ATTACH(pin, ch) do { ledcSetup(ch, PWM_FREQ, PWM_RESOLUTION); ledcAttachPin(pin, ch); } while (0)
  #define PWM_WRITE(pin, ch, duty) ledcWrite(ch, duty)
#endif

class Motor {
  private:
    uint8_t pinA, pinB;
    uint8_t chA, chB;
    bool direction;  
    uint8_t speed;   

    void apply() {
      if (direction) {
        PWM_WRITE(pinA, chA, speed);
        PWM_WRITE(pinB, chB, 0);
      } else {
        PWM_WRITE(pinA, chA, 0);
        PWM_WRITE(pinB, chB, speed);
      }
    }

  public:
    Motor(uint8_t a, uint8_t b, uint8_t channelA, uint8_t channelB)
      : pinA(a), pinB(b), chA(channelA), chB(channelB),
        direction(true), speed(VEL_DETENIDO) {}

    void begin() {
      PWM_ATTACH(pinA, chA);
      PWM_ATTACH(pinB, chB);
      stop();
    }

    void setDirection(bool newDirection) {
      direction = newDirection;
      apply();  
    }

    void setSpeed(uint8_t newSpeed) {
      speed = newSpeed;
      apply();
    }

    void stop() { setSpeed(VEL_DETENIDO); }

    bool getDirection() const { return direction; }
    uint8_t getSpeed() const { return speed; }

    const char* getSpeedName() const {
      if (speed == VEL_DETENIDO) return "DETENIDO";
      if (speed == VEL_BAJA) return "BAJA (33%)";
      if (speed == VEL_MEDIA) return "MEDIA (66%)";
      if (speed == VEL_ALTA) return "ALTA (100%)";
      return "PERSONALIZADA";
    }
};


Motor motor1(M1_PIN_A, M1_PIN_B, 0, 1);
Motor motor2(M2_PIN_A, M2_PIN_B, 2, 3);

class SerialMenu {
  private:
    Motor* activeMotor;
    String inputBuffer;

    void prompt() { Serial.print("\nComando > "); }

  public:
    explicit SerialMenu(Motor* defaultMotor) : activeMotor(defaultMotor) {}

    void showMenu() {
      Serial.println("\n--------------------------------");
      Serial.println("  MENU DE CONTROL DE MOTORES    ");
      Serial.println("--------------------------------");
      Serial.println("1)  Seleccionar MOTOR 1");
      Serial.println("2)  Seleccionar MOTOR 2");
      Serial.println("di) Direccion IZQUIERDA");
      Serial.println("dr) Direccion DERECHA");
      Serial.println("v1) Velocidad BAJA  (33%)");
      Serial.println("v2) Velocidad MEDIA (66%)");
      Serial.println("v3) Velocidad ALTA  (100%)");
      Serial.println("s)  STOP / Detener motor");
      Serial.println("q)  Query / Ver estado");
      Serial.println("m)  Mostrar este menu");
      Serial.println("--------------------------------");
      Serial.print("Comando > ");
    }

   
    void update() {
      while (Serial.available() > 0) {
        char c = Serial.read();

        if (c == '\n' || c == '\r') {
          inputBuffer.trim();
          if (inputBuffer.length() > 0) {
            processCommand(inputBuffer);
            inputBuffer = "";
            prompt();
          }
        } else if (inputBuffer.length() < 16) { 
          inputBuffer += c;
        }
      }
    }

  private:
    void processCommand(String command) {
      command.toLowerCase();
      command.replace(" ", "");  

      if (command == "1") {
        activeMotor = &motor1;
        Serial.println(">> Motor 1 Seleccionado.");
      } else if (command == "2") {
        activeMotor = &motor2;
        Serial.println(">> Motor 2 Seleccionado.");
      } else if (command == "di") {
        activeMotor->setDirection(false);
        Serial.println(">> Direccion: IZQUIERDA");
      } else if (command == "dr") {
        activeMotor->setDirection(true);
        Serial.println(">> Direccion: DERECHA");
      } else if (command == "v1") {
        activeMotor->setSpeed(VEL_BAJA);
        Serial.println(">> Velocidad: BAJA (33%)");
      } else if (command == "v2") {
        activeMotor->setSpeed(VEL_MEDIA);
        Serial.println(">> Velocidad: MEDIA (66%)");
      } else if (command == "v3") {
        activeMotor->setSpeed(VEL_ALTA);
        Serial.println(">> Velocidad: ALTA (100%)");
      } else if (command == "s") {
        activeMotor->stop();
        Serial.println(">> Motor detenido.");
      } else if (command == "q") {
        showStatus();
      } else if (command == "m") {
        showMenu();
      } else {
        Serial.println("!! Comando no valido. Envia 'm' para ver el menu.");
      }
    }

    void showStatus() {
      Serial.println("\n--- ESTADO ACTUAL ---");
      Serial.print("Motor Activo: ");
      Serial.println(activeMotor == &motor1 ? "Motor 1" : "Motor 2");
      Serial.print("Direccion:    ");
      Serial.println(activeMotor->getDirection() ? "DERECHA" : "IZQUIERDA");
      Serial.print("Velocidad:    ");
      Serial.print(activeMotor->getSpeedName());
      Serial.print(" (PWM: ");
      Serial.print(activeMotor->getSpeed());
      Serial.println(" / 255)");
    }
};

SerialMenu menu(&motor1);


void setup() {
  Serial.begin(115200);

  motor1.begin();
  motor2.begin();

  menu.showMenu();
}

void loop() {
  menu.update();
}
