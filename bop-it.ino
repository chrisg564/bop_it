#define PIN_PASS_IT A0
#define PIN_BOUNCE_IT 2
#define PIN_DUNK_ECHO 8
#define PIN_DUNK_TRIG 9
#define PIN_SPK 7

#define PIN_7SEG_ONES 7
#define PIN_7SEG_TENS 4
#define PIN_7SEG_INHIBIT A1
#define PIN_RGB_R 3
#define PIN_RGB_G 5
#define PIN_RGB_B 6

// DUNK_ECHO_MAX_WAIT and LOOP_WAIT_MS should ideally be both prime and relatively prime to improve random number generation method
#define PASS_IT_THRESHOLD 5
#define DUNK_ECHO_MAX_WAIT_MS 4027
#define LOOP_WAIT_MS 103
#define DUNK_PROX_CM 4

#define ACTION_PASS_IT 0
#define ACTION_BOUNCE_IT 1
#define ACTION_DUNK_IT 2

#define NOTE_C7 2093
#define NOTE_E7 2637
#define NOTE_A6 1760

int pass_it_prev;
int bounce_it_prev;

int current_score = 0;
unsigned current_time_interval_ms = 10000;
unsigned total_attempts = 0;

void setup() {
  pinMode(PIN_PASS_IT, INPUT);
  pinMode(PIN_BOUNCE_IT, INPUT);
  pinMode(PIN_DUNK_ECHO, INPUT);
  pinMode(PIN_DUNK_TRIG, OUTPUT);

  pass_it_prev = readPassIt();
  bounce_it_prev = readBounceIt();

  digitalWrite(PIN_7SEG_INHIBIT, LOW);
}

int readPassIt() {
  return map(analogRead(PIN_PASS_IT), 91, 1022, 1, 100);
}

int readBounceIt() {
  return digitalRead(PIN_BOUNCE_IT);
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

inline unsigned getNextAction() {
  return millis() % 3;
}

inline int nextInterval(unsigned total_attempts) {
  return 10 - 0.09 * total_attempts;
}

enum GameState {
  WaitingToStart,
  GameLoopInitial,
  GameLoop,
  ShowingFinalScore,
  SuccessfulAttempt,
  UnsuccessfulAttempt
};

GameState game_state = WaitingToStart;

unsigned current_action = ACTION_BOUNCE_IT;

void game_waiting() {
  int bounce_it_current = readBounceIt();
  if(didBounceIt(bounce_it_prev, bounce_it_current)) {
    game_state = GameState::GameLoopInitial;
    bounce_it_prev = bounce_it_current;
  }
  bounce_it_prev = bounce_it_current;
}

void game_initialize() {
  current_action = getNextAction();
  game_state = GameState::GameLoop;
}

void game_main() {
  sendProxPulse();
  unsigned long duration_us = waitProxPulse();

  unsigned long waited_ms = 0;
  if(duration_us == 0) // Waited full time
    waited_ms = DUNK_ECHO_MAX_WAIT_MS;
  else
    waited_ms = duration_us / 1000.0;

  if(waited_ms < LOOP_WAIT_MS)
    delay(LOOP_WAIT_MS - waited_ms);

  int pass_it_current = readPassIt();
  int bounce_it_current = readBounceIt();

  if(didPassIt(pass_it_prev, pass_it_current)) {
    if(current_action != ACTION_PASS_IT)
      game_state = GameState::UnsuccessfulAttempt;
    else
      game_state = GameState::SuccessfulAttempt;
  }

  if(didBounceIt(bounce_it_prev, bounce_it_current)) {
    if(current_action != ACTION_BOUNCE_IT)
      game_state = GameState::UnsuccessfulAttempt;
    else
      game_state = GameState::SuccessfulAttempt;
  }

  if(didDunkIt(duration_us)) {
    if(current_action != ACTION_DUNK_IT)
      game_state = GameState::UnsuccessfulAttempt;
    else
      game_state = GameState::SuccessfulAttempt;
  }

  pass_it_prev = pass_it_current;
  bounce_it_prev = bounce_it_current;
  delay(LOOP_WAIT_MS);
}

void game_successful() {
  digitalWrite(PIN_RGB_G, HIGH);
  tone(PIN_SPK, NOTE_C7, 300);
  delay(300);
  tone(PIN_SPK, NOTE_E7, 500);
  delay(500);
  noTone(PIN_SPK);
  digitalWrite(PIN_RGB_G, LOW);

  if(current_score % 10 == 9) {
    digitalWrite(PIN_7SEG_TENS, HIGH);
    delay(100);
    digitalWrite(PIN_7SEG_TENS, LOW);
  }

  digitalWrite(PIN_7SEG_ONES, HIGH);
  delay(100);
  digitalWrite(PIN_7SEG_ONES, LOW);
}

void game_unsuccessful() {
  digitalWrite(PIN_RGB_R, HIGH);
  tone(PIN_SPK, NOTE_E7, 500);
  delay(300);
  tone(PIN_SPK, NOTE_C7, 300);
  delay(500);
  noTone(PIN_SPK);
  digitalWrite(PIN_RGB_R, LOW);

  game_state = GameState::ShowingFinalScore;
}

void show_score() {
  digitalWrite(PIN_RGB_R, HIGH);
  tone(PIN_SPK, NOTE_E7, 200);
  delay(200);
  tone(PIN_SPK, NOTE_C7, 200);
  delay(200);
  tone(PIN_SPK, NOTE_A6, 200);
  delay(200);
  noTone(PIN_SPK);
  digitalWrite(PIN_RGB_R, LOW);

  game_state = GameState::WaitingToStart;
}

void loop() {
  switch(game_state) {
    case GameState::WaitingToStart:
      game_waiting();
      break;
    case GameState::GameLoopInitial:
      game_initialize();
      break;
    case GameState::GameLoop:
      game_main();
      break;
    case GameState::SuccessfulAttempt:
      game_successful();
      break;
    case GameState::UnsuccessfulAttempt:
      game_unsuccessful();
      break;
    case GameState::ShowingFinalScore:
      show_score();
      break;
  }
}
