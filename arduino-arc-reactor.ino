#define BUTTON_PIN 2
#define CENTER_LED_PIN 3
#define TIER_1_LED_PIN 5
#define TIER_2_LED_PIN 6
#define TIER_3_LED_PIN 10
#define TIER_4_LED_PIN 11

#define MAX_LUM_LOW 15    // underShirt = 2
#define MAX_LUM 30        // underShirt = 1
#define MAX_LUM_HIGH 170  // underShirt = 0
#define MIN_LUM 1
#define RANDOM_INTERVAL 75 // average breathe cycle is 4 sec in underShirt = 1
                           // 1 interval is one complete breathe cycle
                           // 75 = avg 5 minutes


int displayMode = 0;
int underShirt = 1;
int randomCounter = 0;
int maxBrightness;      // how bright the LED is
int fadeAmount = 15;    // how many points to fade the LED by

int tier[] = {0, 0, 0, 0};
int tierPin[4] = {TIER_1_LED_PIN, TIER_2_LED_PIN, TIER_3_LED_PIN, TIER_4_LED_PIN};
int curTier = 0;
int zeDelay = 20;
int zeDelayMode1 = 20; // Tier by tier light up mode



void setup() {
  // button pin input with pullup (no add. resistor)
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // declare led pins to be an output:
  pinMode(CENTER_LED_PIN, OUTPUT);
  pinMode(TIER_1_LED_PIN, OUTPUT);
  pinMode(TIER_2_LED_PIN, OUTPUT);
  pinMode(TIER_3_LED_PIN, OUTPUT);
  pinMode(TIER_4_LED_PIN, OUTPUT);
   
  resetLeds(); 
}

void loop() {
  buttonDetect();
  if (underShirt == 0) {
    maxBrightness = MAX_LUM_HIGH;
    fadeAmount = 20;
    zeDelayMode1 = 20;
  } else if (underShirt == 1) {
    maxBrightness = MAX_LUM;
    fadeAmount = 10;
    zeDelayMode1 = 90;
  } else {
    maxBrightness = MAX_LUM_LOW;
    fadeAmount = 2;
    zeDelayMode1 = 90;
  }
  
  if (displayMode == 0) {
    fadeLoop(0);
  } else if (displayMode == 1) {
    fadeLoop(1);
  } else if (displayMode == 2) {
    flashLed();
  } else {
    breathe();
  }
}

void buttonDetect()
{
  static byte previous = HIGH;
  static unsigned long firstTime; // how long since the button was first pressed 
  long millis_held;    // How long the button was held (milliseconds)
  long secs_held;      // How long the button was held (seconds)
  static long prev_secs_held; // How long the button was held in the previous check
  int current;
  static bool ignore = false;
  
  current = digitalRead(BUTTON_PIN);
  
  // if the button state changes to pressed, remember the start time 
  if (current == LOW && previous == HIGH && (millis() - firstTime) > 200) {
    firstTime = millis();
    ignore = false;
  }

  millis_held = (millis() - firstTime);
  
  if (millis_held > 50 && !ignore) {
     if (current == LOW) {
        
        if (millis_held >= 300) {
          ignore = true;
          underShirt = underShirt + 1;
          if (underShirt > 2) {
            underShirt = 0;
          }
        }
     }
  }
  if (current == HIGH && previous == LOW && !ignore) {
     updateMode();
  }
  
  previous = current;
  prev_secs_held = secs_held;
}

void updateMode()
{
  displayMode = displayMode + 1;
  if (displayMode >= 4) {
    displayMode = 0;
  }
  
  resetLeds();
  delay(50);
}

void flashLed()
{
  static int flashCounter = 0;
  static long timer = millis();
  
  if (flashCounter % 2 == 1) {
    analogWrite(TIER_1_LED_PIN, 0);
    analogWrite(TIER_2_LED_PIN, 0);
    analogWrite(TIER_3_LED_PIN, 0);
    analogWrite(TIER_4_LED_PIN, 0);
  } else {
    analogWrite(TIER_1_LED_PIN, maxBrightness);
    analogWrite(TIER_2_LED_PIN, maxBrightness);
    analogWrite(TIER_3_LED_PIN, maxBrightness);
    analogWrite(TIER_4_LED_PIN, maxBrightness);
  }
  if (millis() - timer >= 300) {
    flashCounter++;
    timer = millis();  
  }
  
  if (flashCounter > 9) {
    flashCounter = 0;
    displayMode = 4;
  }
}
void resetLeds()
{
  analogWrite(CENTER_LED_PIN, 0);
  tier[0] = 0;
  analogWrite(TIER_1_LED_PIN, 0);
  tier[1] = 0;
  analogWrite(TIER_2_LED_PIN, 0);
  tier[2] = 0;
  analogWrite(TIER_3_LED_PIN, 0);
  tier[3] = 0;
  analogWrite(TIER_4_LED_PIN, 0);
  curTier = 0;
}

void fadeLoop(int mode)
{
  int fadeWay;
  if (tier[curTier] >= maxBrightness) {

    
    curTier++;
    if (curTier >= 4) {
      curTier = 0;    
      if (mode == 1) {
        displayMode = 4;
        return;
      }
      zeDelay = zeDelay - 4;

      if (zeDelay <= -30) {
        
        displayMode = 4;
        if (mode == 0) {
          resetLeds();
        }
        zeDelay = 20;
        return;
      }
    }
  }

  for (int i = 0; i < 4; i++) {
    if (mode == 0) {
      fadeWay = (i == curTier) ? 1 : -1;
    } else {
      fadeWay = (i == curTier) ? 1 : 0;
    }
    
    tier[i] = tier[i] + fadeAmount * fadeWay;
    if (tier[i] <= 0) {
      tier[i] = 0;
    }
    
    analogWrite(tierPin[i], tier[i]);
    
  }
  
  // wait for 30 milliseconds to see the dimming effect
  if (zeDelay > 0) {
    if (mode == 1) {
        delay(zeDelayMode1);
    } else {
      delay(zeDelay);
    }
  } else {
    delay(3);
  }
}

void breathe()
{
  static float in = 0;
  static int brightness = 0;
  in = in + 0.00035;
  
  if (in > 3.1419) {
    in = 0;
    
    randomCounter++;
    if (randomCounter >= RANDOM_INTERVAL) {
      randomCounter = 0;
      int randNumber = random(0, 51);
      if (randNumber < 25) {
        displayMode = 1;
      } else {
        displayMode = 0;
      }
      resetLeds();
      delay(50);
    }
    return;
  }
  brightness = sin(in) * maxBrightness + MIN_LUM;  
  
  analogWrite(CENTER_LED_PIN, brightness);
  analogWrite(TIER_1_LED_PIN, brightness);
  analogWrite(TIER_2_LED_PIN, brightness);
  analogWrite(TIER_3_LED_PIN, brightness);
  analogWrite(TIER_4_LED_PIN, brightness);
}

