#include <Wire.h>
#include <U8g2lib.h>

enum GameState {
  WELCOME,
  DIFFICULTY_SELECT,
  GAME_INIT,
  GAME_PLAY,
  GAME_OVER,
  GAME_WIN
};


enum Difficulty {
  EASY,
  MEDIUM,
  HARD
};


GameState currentState;
Difficulty selectedDifficulty = EASY;
int print = 0;
const int NUM_COLORS = 3;
int seqLength = 0;
int currentHeart = 3;
int currentLevel = 1;
const int signal_pin0 = 0;
const int signal_pin1 = 1;
const int signal_pin2 = 10;
const int button_down = 4;
const int button_up = 3;
const int buzzer = 7;
int button_up_state;
int button_down_state;
int button_confirm_state;

unsigned long prevTime = millis();

long interval = 300;
int* randomSequence = NULL;
int* inputSequence = NULL;
int* sequence = NULL;

int inputLength = 0;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);


void setup() {
  Serial.begin(9600);

  Wire.begin(8, 9);
  u8g2.begin();

  currentState = WELCOME;
  pinMode(buzzer, OUTPUT);
  pinMode(button_up, INPUT);
  pinMode(button_down, INPUT);


  // u8g2_font_5x7_tr smaller font
}

void loop() {


  switch (currentState) {
    case WELCOME:
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(5, 10, "Welcome, Ubaldo");
      u8g2.setFont(u8g2_font_5x7_tr);
      u8g2.drawStr(5, 30, "Tap UP or DOWN");
      u8g2.drawStr(5, 45, "to proceed");
      u8g2.sendBuffer();


      if (digitalRead(button_down) == HIGH || digitalRead(button_up) == HIGH) {
        currentState = DIFFICULTY_SELECT;
        u8g2.clearBuffer();
        u8g2.sendBuffer();
        delay(300);
      }

      break;

    case DIFFICULTY_SELECT:
      button_up_state = digitalRead(button_up);
      button_down_state = digitalRead(button_down);
      button_confirm_state = button_up_state && button_down_state;

      if (button_up_state == HIGH) {
        selectedDifficulty = (selectedDifficulty == EASY) ? HARD : (Difficulty)(selectedDifficulty - 1);
        delay(200);
      }
      if (button_down_state == HIGH) {
        selectedDifficulty = (selectedDifficulty == HARD) ? EASY : (Difficulty)(selectedDifficulty + 1);
        delay(200);
      }

      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(5, 10, "Select Difficulty:");
      u8g2.setFont(u8g2_font_5x7_tr);
      if (selectedDifficulty == EASY) {
        u8g2.drawStr(15, 30, "> Easy");
      } else {
        u8g2.drawStr(20, 30, "Easy");
      }

      if (selectedDifficulty == MEDIUM) {
        u8g2.drawStr(15, 40, "> Medium");
      } else {
        u8g2.drawStr(20, 40, "Medium");
      }

      if (selectedDifficulty == HARD) {
        u8g2.drawStr(15, 50, "> Hard");
      } else {
        u8g2.drawStr(20, 50, "Hard");
      }

      u8g2.sendBuffer();

      if (button_confirm_state) {
        Serial.print("Selected Difficulty: ");
        if (selectedDifficulty == EASY) Serial.println("Selected Easy");
        if (selectedDifficulty == MEDIUM) Serial.println("Selected Medium");
        if (selectedDifficulty == HARD) Serial.println("Selected Hard");

        currentState = GAME_INIT;
      }
      break;

    case GAME_INIT:
      delay(1000);
      initializeGame();
      currentState = GAME_PLAY;
      break;

    case GAME_PLAY:
      delay(1000);
      playGame();
      break;

    case GAME_OVER:
      endGame();
      delay(1000);
      break;

    case GAME_WIN:
      winGame();
      break;
  }
}

void initializeGame() {
  currentLevel = 1;  
  currentHeart = 3; 
  seqLength = (selectedDifficulty == EASY) ? 4 : (selectedDifficulty == MEDIUM) ? 6  : 8;

  if (randomSequence != NULL) {
    free(randomSequence);
    randomSequence = NULL;
  }
  if (inputSequence != NULL) {
    free(inputSequence);
    inputSequence = NULL;
  }

  randomSequence = (int*)malloc(sizeof(int) * seqLength);
  inputSequence = (int*)malloc(sizeof(int) * seqLength);

  delay(1000);

  centeredText("Making a sequence...", 30);

  u8g2.clearBuffer();
  
  if (selectedDifficulty == EASY) {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Easy");
    u8g2.drawStr(35, 10, "I");

    int heartSize = 6;
    int heartSpacing = 18;
    int startX = 128 - heartSize - 2;

    for (int i = 0; i < 3; i++) {
      drawHeart(startX, 10, heartSize);
      startX -= heartSpacing;
    }

   generateRandomSequence(randomSequence, seqLength, 2000);
  } else if (selectedDifficulty == MEDIUM) {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Medium");
    u8g2.drawStr(55, 10, "I");

    int heartSize = 6;
    int heartSpacing = 18;
    int startX = 128 - heartSize - 2;

    for (int i = 0; i < 3; i++) {
      drawHeart(startX, 10, heartSize);
      startX -= heartSpacing;
    }

    generateRandomSequence(randomSequence, seqLength, 1300);
  } else if (selectedDifficulty == HARD) {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Hard");
    u8g2.drawStr(35, 10, "I");

    int heartSize = 6;
    int heartSpacing = 18;
    int startX = 128 - heartSize - 2;

    for (int i = 0; i < 3; i++) {
      drawHeart(startX, 10, heartSize);
      startX -= heartSpacing;
    }

    generateRandomSequence(randomSequence, seqLength, 800);
  }

  u8g2.sendBuffer();

  inputLength = 0;
}

void generateRandomSequence(int* existingSequence, int length, int ms) {

  pinMode(signal_pin0, OUTPUT);
  pinMode(signal_pin1, OUTPUT);
  pinMode(signal_pin2, OUTPUT);

  for (int i = 0; i < length; i++) {

    int randomNumber = random(NUM_COLORS);
    if (randomNumber == 2) {
      randomNumber = 10;
    }
    existingSequence[i] = randomNumber;
    Serial.print(existingSequence[i]);
    Serial.print(" ");

    delay(2000);
    if (existingSequence[i] == 0) {
      digitalWrite(signal_pin0, HIGH);
    }

    if (existingSequence[i] == 1) {
      digitalWrite(signal_pin1, HIGH);
    }

    if (existingSequence[i] == 10) {
      digitalWrite(signal_pin2, HIGH);
    }

    delay(ms);
    digitalWrite(signal_pin0, LOW);
    digitalWrite(signal_pin1, LOW);
    digitalWrite(signal_pin2, LOW);
    delay(200);
  }


  pinMode(signal_pin0, INPUT);
  pinMode(signal_pin1, INPUT);
  pinMode(signal_pin2, INPUT);
  delay(1000);
}

bool checkSequence(int seq0[], int seq1[], int length) {
  bool isTheSame = true;
  for (int i = 0; i < length; i++) {
    if (seq0[i] != seq1[i]) {
      isTheSame = false;
      break;
    }
  }

  if (isTheSame == false) {
    Serial.println("Uncool and abnormal");
    return false;
  }

  Serial.println("Cool and normal");

  return true;
}

void playGame() {
  unsigned long currentTime = millis();
  button_up_state = digitalRead(button_up);
  button_down_state = digitalRead(button_down);

  if (button_down_state == HIGH) {

    digitalWrite(buzzer, HIGH);
    delay(250);
    digitalWrite(buzzer, LOW);

    Serial.print("Before delete: ");
    for (int i = 0; i < inputLength; i++) {
      Serial.print(inputSequence[i]);
      Serial.print(" ");
    }

    if (inputLength > 0) {
      inputLength--;
      inputSequence[inputLength] = -1;
    }

    Serial.print("After delete: ");

    for (int i = 0; i < inputLength; i++) {
      Serial.print(inputSequence[i]);
      Serial.print(" ");
    }
    Serial.println();

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(0, 30, "Your inputs: ");
    displayInputs();
    displayLevelAndHearts();
    u8g2.sendBuffer();
  }


  if (button_up_state == HIGH) {
    digitalWrite(buzzer, HIGH);
    delay(250);
    digitalWrite(buzzer, LOW);
    if (inputLength == seqLength && checkSequence(inputSequence, randomSequence, inputLength)) {
      currentLevel++;
      Serial.println("Correct sequence!");
      Serial.println("\n--- DEBUG INFO ---");
      Serial.print("Current Level before increment: ");
      Serial.println(currentLevel);
      Serial.print("Sequence Length: ");
      Serial.println(seqLength);
      Serial.print("Current random sequence: ");
      centeredText("CORRECT!", 30);
      delay(1000);
      if (currentLevel == 4) {
        currentState = GAME_WIN; 
        return;
      }

      u8g2.clearBuffer();
      inputLength = 0;
     

      for (int i = 0; i < seqLength; i++) {
        randomSequence[i] = 0;
        inputSequence[i] = 0;
      }

      Serial.println("After reinitialization!");


      

      centeredText("Making a sequence...", 30);
      if (selectedDifficulty == EASY) {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        displayLevelAndHearts();
      
        if (currentLevel == 2) {
          
          generateRandomSequence(randomSequence, seqLength, 1800);
        }
        if (currentLevel == 3) {
         generateRandomSequence(randomSequence, seqLength, 1600);
        }


        u8g2.sendBuffer();
      }

      if (selectedDifficulty == MEDIUM) {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        displayLevelAndHearts();

        if (currentLevel == 2) {
          generateRandomSequence(randomSequence, seqLength, 1100);
        }

        if (currentLevel == 3) {
          generateRandomSequence(randomSequence, seqLength, 900);
        }

        u8g2.sendBuffer();  
      }


      if (selectedDifficulty == HARD) {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        displayLevelAndHearts();

        if (currentLevel == 2) {
          generateRandomSequence(randomSequence, seqLength, 500);
        }

        if (currentLevel == 3) {
          generateRandomSequence(randomSequence, seqLength, 200);
        }
     
        u8g2.sendBuffer();    
      }

     

    } else {
      currentHeart--;
      if (currentHeart == 0) {
        Serial.println("Out of hearts.");
        currentState = GAME_OVER;
      }

      if(currentHeart != 0){
        centeredText("INCORRECT!", 30);
        delay(1000);
        centeredText("TRY AGAIN!", 30);
        delay(1000);
      
      
        u8g2.clearBuffer();

        displayLevelAndHearts();
        u8g2.sendBuffer();
        Serial.println("Incorrect sequence. Try again.");

        u8g2.sendBuffer();
        inputLength = 0;
      }
      
    }
  }


  if (currentTime - prevTime > interval) {
    int touchedSensor = getTouchedSensor();
    if (touchedSensor != -1) {
      digitalWrite(buzzer, HIGH);
      digitalWrite(touchedSensor, HIGH);
      delay(250);
      digitalWrite(buzzer, LOW);
      digitalWrite(touchedSensor, LOW);
      if (inputLength < seqLength) {
        inputSequence[inputLength] = touchedSensor;
        inputLength++;
        Serial.print("Input: ");
        Serial.println(touchedSensor);
        u8g2.setFont(u8g2_font_5x7_tr);
        u8g2.drawStr(0, 30, "Your inputs: ");
        displayInputs();
      prevTime = currentTime;
    }
   }
  }
  u8g2.sendBuffer();
}

void endGame() {

  if (randomSequence != NULL) {
    free(randomSequence);
    randomSequence = NULL;
  }
  if (inputSequence != NULL) {
    free(inputSequence);
    inputSequence = NULL;
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(5, 10, "Game over");
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(5, 30, "Tap UP or DOWN");
  u8g2.drawStr(5, 45, "to proceed");
  u8g2.sendBuffer();
  if (digitalRead(button_down) == HIGH || digitalRead(button_up) == HIGH) {
      currentState = DIFFICULTY_SELECT;
      u8g2.clearBuffer();
      u8g2.sendBuffer();
      delay(300);
  }
}


void winGame() {

  if (randomSequence != NULL) {
    free(randomSequence);
    randomSequence = NULL;
  }
  if (inputSequence != NULL) {
    free(inputSequence);
    inputSequence = NULL;
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(5, 10, "You win!!!");
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(5, 30, "Tap UP or DOWN");
  u8g2.drawStr(5, 45, "to proceed");
  u8g2.sendBuffer();
  if (digitalRead(button_down) == HIGH || digitalRead(button_up) == HIGH) {
      currentState = DIFFICULTY_SELECT;
      u8g2.clearBuffer();
      u8g2.sendBuffer();
      delay(300);
  }
}

int getTouchedSensor() {
  for (int i = 0; i < 11; i++) {
    if (i >= 2 && i <= 9) {
      continue;
    }

    if (digitalRead(i) == HIGH) {
      int pin3 = digitalRead(signal_pin2);
      Serial.print("Pin 3 status: ");
      Serial.println(pin3);
      Serial.print("Pin that is high: ");
      Serial.println(i);
      return i;
    }
  }
  return -1;
}


void drawHeart(int x, int y, int size) {
  u8g2.drawLine(x, y - size / 2, x - size / 6, y - 5 * size / 6);
  u8g2.drawLine(x - size / 6, y - 5 * size / 6, x - size / 3, y - size);
  u8g2.drawLine(x - size / 3, y - size, x - size / 2, y - size);
  u8g2.drawLine(x - size / 2, y - size, x - 2 * size / 3, y - 5 * size / 6);
  u8g2.drawLine(x - 2 * size / 3, y - 5 * size / 6, x - 5 * size / 6, y - 2 * size / 3);
  u8g2.drawLine(x - 5 * size / 6, y - 2 * size / 3, x - size, y - size / 3);

  u8g2.drawLine(x, y - size / 2, x + size / 6, y - 5 * size / 6);
  u8g2.drawLine(x + size / 6, y - 5 * size / 6, x + size / 3, y - size);
  u8g2.drawLine(x + size / 3, y - size, x + size / 2, y - size);
  u8g2.drawLine(x + size / 2, y - size, x + 2 * size / 3, y - 5 * size / 6);
  u8g2.drawLine(x + 2 * size / 3, y - 5 * size / 6, x + 5 * size / 6, y - 2 * size / 3);
  u8g2.drawLine(x + 5 * size / 6, y - 2 * size / 3, x + size, y - size / 3);

  u8g2.drawLine(x - size, y - size / 3, x - 2 * size / 3, y + size / 3);
  u8g2.drawLine(x - 2 * size / 3, y + size / 3, x - size / 3, y + 2 * size / 3);
  u8g2.drawLine(x - size / 3, y + 2 * size / 3, x, y + size);

  u8g2.drawLine(x + size, y - size / 3, x + 2 * size / 3, y + size / 3);
  u8g2.drawLine(x + 2 * size / 3, y + size / 3, x + size / 3, y + 2 * size / 3);
  u8g2.drawLine(x + size / 3, y + 2 * size / 3, x, y + size);
}

void displayInputs() {
    for (int i = 0; i < inputLength; i++) {
        int space = i * 6; 
        if (inputSequence[i] == 0) {
            u8g2.drawStr(space, 40, "R");
        } else if (inputSequence[i] == 1) {
            u8g2.drawStr(space, 40, "G");
        } else if (inputSequence[i] == 10) {
            u8g2.drawStr(space, 40, "Y");
        }
    }
}

// for deleting
void displayLevelAndHearts() {
    
    u8g2.setFont(u8g2_font_ncenB08_tr);
    if (selectedDifficulty == EASY) {
        u8g2.drawStr(0, 10, "Easy");
    } else if (selectedDifficulty == MEDIUM) {
        u8g2.drawStr(0, 10, "Medium");
    } else if (selectedDifficulty == HARD) {
        u8g2.drawStr(0, 10, "Hard");
    }
    int levelX = (selectedDifficulty == MEDIUM) ? 55 : 35;

    if (currentLevel == 1) {
        u8g2.drawStr(levelX, 10, "I");
    } else if (currentLevel == 2) {
        u8g2.drawStr(levelX, 10, "II");
    } else if (currentLevel == 3) {
        u8g2.drawStr(levelX, 10, "III");
    }

    int heartSize = 6;
    int heartSpacing = 18;
    int startX = 128 - heartSize - 2;

    for (int i = 0; i < currentHeart; i++) {
        drawHeart(startX, 10, heartSize);
        startX -= heartSpacing;
    }

    
}

void centeredText(const char* text, int y) {
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_5x7_tr); 

    int displayWidth = 128; 
    int textWidth = u8g2.getStrWidth(text);
    
    int x = (displayWidth - textWidth) / 2;

    u8g2.drawStr(x, y, text);

    u8g2.sendBuffer();
}




