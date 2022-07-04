#define PIN_PASS_IT A0
#define PIN_BOUNCE_IT 2
#define PIN_DUNK_ECHO 8
#define PIN_DUNK_TRIG 9
#define PIN_SPK 7

#define PASS_IT_THRESHOLD 5
#define DUNK_ECHO_MAX_WAIT_MS 4000
#define LOOP_WAIT_MS 100
#define DUNK_PROX_CM 4

int pass_it_prev;
int bounce_it_prev;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_PASS_IT, INPUT);
  pinMode(PIN_BOUNCE_IT, INPUT);
  pinMode(PIN_DUNK_ECHO, INPUT);
  pinMode(PIN_DUNK_TRIG, OUTPUT);

  pass_it_prev = readPassIt();
  bounce_it_prev = readBounceIt();

  tone(PIN_SPK, 440, 1);
}

int readPassIt() {
  return map(analogRead(PIN_PASS_IT), 91, 1022, 1, 100);
}

int readBounceIt() {
  return digitalRead(PIN_BOUNCE_IT);
}

void loop() {
  sendProxPulse();
  unsigned long duration_us = waitProxPulse();

  unsigned long waited_ms = 0;
  if(duration_us == 0) // Waited full time
    waited_ms = DUNK_ECHO_MAX_WAIT_MS;
  else
    waited_ms = duration_us / 1000.0;

  if(waited_ms < LOOP_WAIT_MS)
    delay(LOOP_WAIT_MS - waited_ms);
  
  Serial.println(duration_us * 0.017);
  
  int pass_it_current = readPassIt();
  int bounce_it_current = readBounceIt();

  if(didPassIt(pass_it_prev, pass_it_current))
    Serial.println("Passed it");

  if(didBounceIt(bounce_it_prev, bounce_it_current))
    Serial.println("Bounced it");

  if(didDunkIt(duration_us))
    Serial.println("Dunked it");

  Serial.flush();
  // ...

  pass_it_prev = pass_it_current;
  bounce_it_prev = bounce_it_current;
  delay(100);
}

inline void sendProxPulse() {
  digitalWrite(PIN_DUNK_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_DUNK_TRIG, LOW);
}

inline unsigned long waitProxPulse() {
  return pulseIn(PIN_DUNK_ECHO, HIGH, DUNK_ECHO_MAX_WAIT_MS);
}

bool didPassIt(int prev_value, int next_value) {
  return abs(next_value - prev_value) > PASS_IT_THRESHOLD;
}

bool didBounceIt(int prev_value, int next_value) {
  return (prev_value == 0) && (next_value == 1);
}

bool didDunkIt(int duration_us) {
  return (0.017 * duration_us) < DUNK_PROX_CM;
}
