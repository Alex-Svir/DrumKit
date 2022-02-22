#define SERIAL_SPEED 115200
#define ADC_PRESCALER 0x04
#define N 4
#define DEFAULT_TRESHOLD 50
#define DEFAULT_DURATION 30
#define QUEUE 200

const unsigned long duration = DEFAULT_DURATION;
const uint8_t defaultTreshold = DEFAULT_TRESHOLD;
uint8_t admux_ref;
uint8_t active_pin = 0;

/*volatile*/ uint8_t a=0, b=0;

struct Queue {
  unsigned long moment;
  uint8_t val, code;
};

class Channel {
  boolean active = false;
  unsigned long end_time;
  uint8_t code;
  uint8_t magnitude;
  
void make_buf(uint8_t code, uint8_t velocity, unsigned long moment, byte* buf) {
  buf[0] = (byte)code;
  buf[1] = (byte)velocity;
  *(unsigned long *)(buf+2) = moment;
}

  void start(uint8_t value, unsigned long moment) {
    active = true;
    end_time = moment + duration;
    magnitude = value;
  }

  void m_round(uint8_t value, unsigned long moment) {
    if (value > magnitude) magnitude = value;
    if (moment > end_time) m_stop();
  }
  
  void m_stop() {
    active = false;
    m_send();
  }
  
  void m_send() {
    byte buf[6];
    make_buf(code, magnitude, end_time - duration, buf);
    Serial.write(buf, 6);
  }

public:
  void init(uint8_t channel) {
    code = channel;
  }
  
  void touch(unsigned long moment) {
    if (active) {
      if (moment > end_time) m_stop();
    }
  }
  
  void touch(uint8_t value, unsigned long moment) {
    if (active) {
      m_round(value, moment);
    } else {
      start(value, moment);
    }
  }
};

Queue queue[QUEUE];
Channel channels[N];

void circleTouch() {
  unsigned long moment = millis();
  for (int i=0; i<N; i++) {
    channels[i].touch(moment);
  }
}

void setup() {
  Serial.begin(SERIAL_SPEED);

  for (uint8_t i=0; i<N; i++) {
    channels[i].init(i);
  }
  
  ADCSRA = 0b00101000 | (ADC_PRESCALER & 0x07);
  ADCSRB = 0;
  ADMUX = 0b01100000;
  admux_ref = ADMUX;
  ADCSRA |= 0b10000000;
  ADCSRA |= 0b01000000;
}

void loop() {
  if (a == b) {
    circleTouch();
    return;
  }

  channels[queue[b].code].touch(queue[b].val, queue[b].moment);
  if (++b == QUEUE) b=0;
}

boolean trueValue = false;

ISR(ADC_vect) {
  if (trueValue) {
    uint8_t result = ADCH;

    if (result > defaultTreshold) {
        queue[a].val = result;
        queue[a].moment = millis();
        queue[a].code = active_pin;
        if (++a == QUEUE) a = 0;
    }
    
    if (++active_pin == N) active_pin = 0;
    ADMUX = admux_ref | (active_pin & 0x07);
    trueValue = false;
  } else {
    trueValue = true;
  }
}
