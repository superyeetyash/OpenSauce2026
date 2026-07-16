#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>


// ================= LCD =================

LiquidCrystal_I2C lcd(0x27,16,2);


// ================= COLORS =================

enum Color {
  RED,
  PURPLE,
  GREEN,
  BLUE
};


String colorNames[4] = {
  "RED",
  "PURPLE",
  "GREEN",
  "BLUE"
};


// Fixed duck colors

Color duckColors[4] = {
  RED,
  PURPLE,
  GREEN,
  BLUE
};


// ================= RGB LED PINS =================

int redPins[4] = {
  2,5,8,11
};

int greenPins[4] = {
  3,6,9,12
};

int bluePins[4] = {
  4,7,10,13
};


// ================= BUTTON PINS =================

int buttonPins[4] = {
  22,23,24,25
};


// ================= SERVOS =================

Servo ducks[4];


int servoPins[4] = {
  26,27,28,29
};


int duckDown = 0;
int duckUp = 90;


// ================= BUTTON COLORS =================

Color buttonColors[4];


// ================= GAME VARIABLES =================

int activeDuck;

int score = 0;


bool roundActive = false;


unsigned long roundStart;


int roundTime = 10;


unsigned long lastPress[4];

int debounceDelay = 150;



// ================= SETUP =================


void setup(){


  Serial.begin(9600);


  randomSeed(
    analogRead(A0)
  );


  // LCD

  lcd.init();

  lcd.backlight();


  lcd.clear();

  lcd.setCursor(0,0);

  lcd.print("DUCK HUNT");


  lcd.setCursor(0,1);

  lcd.print("Starting...");


  delay(2000);



  // RGB

  for(int i=0;i<4;i++){

    pinMode(redPins[i],OUTPUT);
    pinMode(greenPins[i],OUTPUT);
    pinMode(bluePins[i],OUTPUT);

  }



  // Buttons

  for(int i=0;i<4;i++){

    pinMode(
      buttonPins[i],
      INPUT_PULLUP
    );


    lastPress[i]=0;

  }



  // Servos

  for(int i=0;i<4;i++){

    ducks[i].attach(
      servoPins[i]
    );


    ducks[i].write(
      duckDown
    );

  }



  startRound();


}




// ================= LOOP =================


void loop(){


  updateTimer();

  checkButtons();


}




// ================= START ROUND =================


void startRound(){


  clearLEDs();


  activeDuck =
  random(0,4);



  Serial.println("================");

  Serial.println("NEW ROUND");


  Serial.print("Duck Activated: ");

  Serial.println(
    activeDuck+1
  );


  Serial.print("Duck Color: ");

  Serial.println(
    colorNames[
      duckColors[activeDuck]
    ]
  );



  // Duck rises FIRST

  ducks[activeDuck].write(
    duckUp
  );



  delay(700);



  randomizeButtons();



  roundStart = millis();


  roundActive=true;



}



// ================= TIMER =================


void updateTimer(){


  if(!roundActive)
    return;



  int remaining =
  roundTime -
  ((millis()-roundStart)/1000);



  lcd.setCursor(0,0);

  lcd.print("Score:");

  lcd.print(score);

  lcd.print(" ");



  lcd.setCursor(9,0);

  lcd.print("T:");

  lcd.print(remaining);

  lcd.print(" ");



  lcd.setCursor(0,1);

  lcd.print("Duck:");

  lcd.print(
    colorNames[
      duckColors[activeDuck]
    ]
  );



  if(remaining<=0){

    loseRound();

  }
  

}
// ================= BUTTON INPUT =================


void checkButtons(){


  if(!roundActive)
    return;



  for(int i=0;i<4;i++){


    if(
      digitalRead(buttonPins[i])
      ==LOW
    ){


      if(
        millis()-lastPress[i]
        >debounceDelay
      ){


        lastPress[i]=millis();


        buttonPressed(i);


      }


    }


  }


}




void buttonPressed(int button){


  Serial.println();


  Serial.print("Pressed Button: ");

  Serial.println(button+1);



  Serial.print("Button Color: ");

  Serial.println(
    colorNames[
      buttonColors[button]
    ]
  );



  if(
    buttonColors[button]
    ==
    duckColors[activeDuck]
  ){

    correctRound();

  }

  else{


    wrongButton();


  }



}



// ================= CORRECT =================


void correctRound(){


  roundActive=false;


  Serial.println("CORRECT");


  Serial.print("Lowering Duck: ");

  Serial.println(
    activeDuck+1
  );



  ducks[activeDuck].write(
    duckDown
  );


  score++;



  lcd.clear();


  lcd.setCursor(0,0);

  lcd.print("CORRECT!");


  lcd.setCursor(0,1);

  lcd.print("Score:");

  lcd.print(score);



  flashColor(GREEN);



  delay(1200);



  startRound();


}




// ================= WRONG =================


void wrongButton(){


  Serial.println("WRONG BUTTON");


  lcd.clear();


  lcd.setCursor(0,0);

  lcd.print("WRONG");


  lcd.setCursor(0,1);

  lcd.print("TRY AGAIN");



  // Flash red feedback

  for(int i=0;i<4;i++){

    setLED(
      i,
      RED
    );

  }


  delay(300);



  // Restore original button colors

  for(int i=0;i<4;i++){

    setLED(
      i,
      buttonColors[i]
    );

  }


}





// ================= TIME LOSS =================


void loseRound(){


  roundActive=false;



  ducks[activeDuck].write(
    duckDown
  );



  Serial.println("TIME UP");



  lcd.clear();


  lcd.setCursor(0,0);

  lcd.print("TIME UP!");


  lcd.setCursor(0,1);

  lcd.print("Score:");

  lcd.print(score);



  flashColor(RED);



  delay(2000);



  startRound();


}




// ================= RANDOM RGB =================


void randomizeButtons(){


  Color temp[4]={

    RED,
    PURPLE,
    GREEN,
    BLUE

  };



  for(int i=0;i<4;i++){


    int r=random(0,4);


    Color swap=temp[i];

    temp[i]=temp[r];

    temp[r]=swap;


  }



  for(int i=0;i<4;i++){


    buttonColors[i]=temp[i];


    setLED(
      i,
      buttonColors[i]
    );



    Serial.print("Button ");

    Serial.print(i+1);

    Serial.print(": ");

    Serial.println(
      colorNames[
        buttonColors[i]
      ]
    );


  }


}



// ================= RGB CONTROL =================


void setLED(
int led,
Color c
){


  int r=0;

  int g=0;

  int b=0;



  switch(c){


    case RED:

      r=255;

      break;


    case PURPLE:

      r=255;

      b=255;

      break;


    case GREEN:

      g=255;

      break;


    case BLUE:

      b=255;

      break;


  }



  analogWrite(
    redPins[led],
    r
  );


  analogWrite(
    greenPins[led],
    g
  );


  analogWrite(
    bluePins[led],
    b
  );


}




void clearLEDs(){


  for(int i=0;i<4;i++){


    analogWrite(
      redPins[i],
      0
    );


    analogWrite(
      greenPins[i],
      0
    );


    analogWrite(
      bluePins[i],
      0
    );


  }


}




void flashColor(Color c){


  for(int i=0;i<4;i++){

    setLED(
      i,
      c
    );

  }


  delay(300);


  clearLEDs();


}