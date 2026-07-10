// WPT TX Controller
// STM32F103C6 - Arduino IDE
//
// PIN CONNECTIONS:
// PA0 = PWM_OUT → IR2110#1 HIN+LIN
//               → 74HC14N → IR2110#2 HIN+LIN
// PA1 = ISEN    → ACS712 VIOUT (analog)
// PA2 = HB_EN   → IR2110 SD (LOW=enable!)
// PB0 = FAULT   → LM393 OUT (LOW=fault!)
// RST = Hardware reset only (no code needed)

#define PWM_OUT   PA0  // PWM output 85kHz
#define ISEN      PA1  // Current sense analog
#define HB_EN     PA2  // SD pin LOW=enable
#define FAULT     PB0  // Fault LOW=fault

HardwareTimer *pwmTimer;

void setup() {
  Serial.begin(9600);

  // Configure all pins
  pinMode(PWM_OUT, OUTPUT);       // PA0 PWM
  pinMode(HB_EN,   OUTPUT);       // PA2 SD
  pinMode(FAULT,   INPUT_PULLUP); // PB0 fault
  pinMode(ISEN,    INPUT_ANALOG); // PA1 ADC

  // Safe start
  // SD HIGH = H-Bridge DISABLED
  digitalWrite(HB_EN, HIGH);
  digitalWrite(PWM_OUT, LOW);

  delay(500);

  // Setup PWM on PA0 at 85kHz 50% duty
  pwmTimer = new HardwareTimer(TIM2);
  pwmTimer->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PWM_OUT);
  pwmTimer->setOverflow(85000, HERTZ_FORMAT);
  pwmTimer->setCaptureCompare(1, 50, PERCENT_COMPARE_FORMAT);
  pwmTimer->resume();

  Serial.println("========================");
  Serial.println("WPT TX System Starting");
  Serial.println("========================");
  Serial.println("PWM_OUT = PA0 = 85kHz");
  Serial.println("ISEN    = PA1 = ACS712");
  Serial.println("HB_EN   = PA2 = SD pin");
  Serial.println("FAULT   = PB0 = LM393");
  Serial.println("========================");

  // Check fault before enabling
  delay(100);

  if(digitalRead(FAULT) == HIGH) {
    // No fault → Enable H-Bridge
    // SD LOW = ENABLED!
    digitalWrite(HB_EN, LOW);
    Serial.println("No fault detected!");
    Serial.println("H-Bridge ENABLED!");
    Serial.println("PWM running at 85kHz!");
  }
  else {
    // Fault at startup!
    pwmTimer->pause();
    digitalWrite(PWM_OUT, LOW);
    Serial.println("FAULT at startup!");
    Serial.println("Check LM393!");
  }
}

void loop() {

  // ================================
  // CHECK FAULT PIN (PB0)
  // LOW = FAULT detected!
  // ================================
  if(digitalRead(FAULT) == LOW) {

    // Immediately disable H-Bridge!
    // SD HIGH = DISABLED!
    digitalWrite(HB_EN, HIGH);

    // Stop PWM
    pwmTimer->pause();
    digitalWrite(PWM_OUT, LOW);

    Serial.println("========================");
    Serial.println("!!!! FAULT DETECTED !!!!");
    Serial.println("H-Bridge DISABLED!");
    Serial.println("Reset STM32 to restart!");
    Serial.println("========================");

    // Stay in fault state forever
    while(true) {
      delay(1000);
      Serial.println("FAULT STATE - Reset required!");
    }
  }

  // ================================
  // READ CURRENT (PA1 = ISEN)
  // ACS712: 2.5V = 0A
  // 185mV per Amp
  // ================================
  int adc    = analogRead(ISEN);
  float volt = (adc * 3.3f) / 4096.0f;
  float curr = (volt - 2.5f) / 0.185f;

  // ================================
  // SOFTWARE OVERCURRENT BACKUP
  // LM393 = primary hardware protection
  // This = software backup only!
  // ================================
  if(curr > 4.5f) {
    Serial.println("Software overcurrent!");

    // Brief disable
    digitalWrite(HB_EN, HIGH); // SD HIGH = off
    delay(100);

    // Re-enable if no hardware fault
    if(digitalRead(FAULT) == HIGH) {
      digitalWrite(HB_EN, LOW); // SD LOW = on
      Serial.println("Re-enabled after overcurrent!");
    }
  }

  // ================================
  // PRINT STATUS
  // ================================
  Serial.print("PWM_OUT(PA0): 85kHz | ");
  Serial.print("ISEN(PA1): ");
  Serial.print(curr);
  Serial.print("A | ");
  Serial.print(volt);
  Serial.print("V | ");
  Serial.print("HB_EN(PA2): ");
  Serial.print(digitalRead(HB_EN) == LOW ? "ON" : "OFF");
  Serial.print(" | FAULT(PB0): ");
  Serial.println(digitalRead(FAULT) == HIGH ? "OK" : "FAULT!");

  delay(500);
}