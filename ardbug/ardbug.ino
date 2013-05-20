// Ardbug by Andrew Gerrand <nf@wh3rd.net>
// D0 and D1 send random gates, whose maximum length is guided by A0 and A1.
// The DAC output sends a random voltage on each CLK in,
// its range is guided by A2 and A3.

// Random gates

const int inPin[] = {0, 1};
const int outPin[] = {3, 4};

int tickLen[] = {0, 0};
int drift[] = {0, 0};
unsigned long nextEdge[] = {0, 0};
boolean pinHigh[] = {false, false};

void gateStep(unsigned long now, int i) {
  if (now < nextEdge[i]) {
    return;
  }
  
  // Toggle edge.
  if (pinHigh[i]) {
    digitalWrite(outPin[i], LOW);
  } else {
    digitalWrite(outPin[i], HIGH);
  }
  pinHigh[i] = !pinHigh[i];
  
  // Compute next edge.
  int hi = analogRead(inPin[i]);
  int acc = random(0, hi/4) - hi/8;
  drift[i] = drift[i] + acc;
  tickLen[i] = tickLen[i] + drift[i];
  if (tickLen[i] < 0) {
    tickLen[i] = 0;
    drift[i] = -drift[i]/2;
  }
  if (tickLen[i] > hi && drift[i] > 0) {
    drift[i] = -drift[i]/2;
  }
  nextEdge[i] = now + (unsigned long)tickLen[i];;
}

void gateSetup() {
  for (int i = 0; i < 2; i++) {
    pinMode(outPin[i], OUTPUT);
    digitalWrite(outPin[i], LOW);
  }
}

// Random CV

const int loPin = 2;
const int hiPin = 3;

void outputRandomCV() {
  int lo = analogRead(loPin) / 4;
  int hi = analogRead(hiPin) / 4;
  int v = random(min(lo, hi), max(lo, hi));
  dacOutput(v);
}

// DAC

const int dacPinOffset = 5;

void dacOutputSetup() {
  for (int i=0; i<8; i++) {
    pinMode(dacPinOffset+i, OUTPUT);
    digitalWrite(dacPinOffset+i, LOW);
  }
}

void dacOutput(int v) {
  int t = v;
  for (int i=0; i<8; i++) {
    digitalWrite(dacPinOffset + i, t & 1);
    t = t >> 1;
  }
}

// Init, main loop, and interrupt handling.

boolean tick = false;

void tickInt() {
  tick = true;
}

void setup() {
  dacOutputSetup();
  gateSetup();
  attachInterrupt(0, tickInt, RISING);  
}

void loop() {
  if (tick) {
    outputRandomCV();
    tick = false;
  }
  unsigned long now = millis();
  gateStep(now, 0);
  gateStep(now, 1);
}
