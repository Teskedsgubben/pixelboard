// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "pins.h"

#define MAX_SPEED 5

#define HID 0
#define NPC 1

#define X 12
#define Y 24

Adafruit_NeoPixel strip(X*Y, LED_PIN, NEO_GRB + NEO_KHZ800);

struct color_rgb rgb_off;

bool mute = true;

void setup() {
  Serial.begin(9600);

  pinMode(PIN_P1,INPUT);
  pinMode(PIN_P2,INPUT);
  pinMode(BUTTON_P1,INPUT);
  pinMode(BUTTON_P2,INPUT);
  pinMode(BUZZER_PIN,OUTPUT);
  pinMode(SIDE_BUTTON,OUTPUT);  
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(100); // Set BRIGHTNESS to about 1/5 (max = 255)
}


void loop() {
  
  int winner = bounce();
  // countDown(3);
  delay(3000);
}


void countDown( int count){
  struct vec pos[2], dir[2];
  pos[0].x=round(float(X)/2 - 2); pos[0].y=0+2+5+1;
  pos[1].x=round(float(X)/2); pos[1].y=Y-1-2-5-1;
  dir[0].x=0; dir[0].y=-1;
  dir[1].x=0; dir[1].y=1;

  struct color_rgb color;
  color.r=15;

  int beep=125;
  bool beeping = false;
  unsigned long timer;

  for(int i=count; i>0; i--){
    for(int p=0; p<2; p++){
      displayNumber(i,pos[p],dir[p],color,true);
    }
    strip.show();
    timer = millis();
    while(timer + 999 > millis()){
      // for(int p =0; p<2; p++){
      //   makeMove(&player[p], ball);
      // }
      // strip.show();
      if(!mute && !beeping && timer+beep > millis()){
        analogWrite(BUZZER_PIN,127);
        beeping=true;
      }
      else if(beeping && timer+beep < millis()){
        analogWrite(BUZZER_PIN,0);
        beeping=false;
      }
    }
  }
  for(int p=0; p<2; p++){
    displayNumber(-1,pos[p],dir[p],color,true);
  }
  for(int i = 255; i>=0;i--){
    analogWrite(BUZZER_PIN,i);
    delay(1);
  }
}

int bounce(){
  srand(millis());

  struct pongPlayer player[2];
  player[0].number = 0;
  player[0].color.b = 255;
  player[0].pos.x = -1; player[0].pos.y = 1;
  player[0].wheelPort=PIN_P1;
  player[0].buttonPort=BUTTON_P1;
  player[0].playerType=NPC;

  player[1].number = 1;
  player[1].color.g = 255;
  player[1].pos.x = -1; player[1].pos.y = Y-2;
  player[1].wheelPort=PIN_P2;
  player[1].buttonPort=BUTTON_P2;
  player[1].playerType=NPC;


  setupPongRound(player);

  int bestOf = 5;
  while(max(player[0].score, player[1].score)<=bestOf/2){
    displayScore(player,3000);
    if(playPongRound(player) == -1){
      return -1;
    }
  }
  flashWinner(player,5);
  return 1;

  
  
}

int playPongRound(struct pongPlayer player[]){
  struct pongBall ball;
  ball.color.r = 255;

  bool sideButtonPressed = false;
  long sideButtonTimer = millis();
  long currTime = millis();
  int frameDelay = 10;
  int bounces = 0;

  struct vec pix;

  setupPongBall(&ball);
  strip.clear();
  for(int p =0; p<2; p++){
    player[p].size = 4;
    drawPlayer(&player[p]);
  }
  lightPixel(ball.pos,ball.color);
  countDown(3);
  while(1){
    if(digitalRead(SIDE_BUTTON)){
      sideButtonPressed = true;
      if(millis()-sideButtonTimer > 2000){
        return -1;
      }
    }
    else{
      if(sideButtonPressed){
        mute = !mute; 
      }
      sideButtonPressed=false;
      sideButtonTimer=millis();
    }

    
    for(int p =0; p<2; p++){
      makeMove(&player[p], ball);
    }

    if(currTime+round((double)frameDelay/(ball.speed*ball.boost)) < millis()){
      currTime=millis();
      
      lightPixel(ball.pos,rgb_off);
      ball.pos.x+=ball.dir.x;
      ball.pos.y+=ball.dir.y;
      
      if(ball.pos.y < 0 || ball.pos.y >= Y){
        int winner = 1*(ball.pos.y < 0);
        ball.pos.x-=ball.dir.x;
        ball.pos.y-=ball.dir.y;
        ball.dir.y=-ball.dir.y;
        lossAnimation(ball);
        if(player[winner].playerType == NPC){
          displayBouncesCount(bounces, player[!winner]);
          strip.show();
          delay(4000);
        }
        if(player[!winner].size == 1){
          player[winner].score+=1;
          return winner;
        }
        else{
          player[!winner].size-=1;
          setupPongBall(&ball);
          strip.clear();
          for(int p =0; p<2; p++){
            drawPlayer(&player[p]);
          }
          lightPixel(ball.pos,ball.color);
          strip.show();
          delay(1500);
          bounceSound();
        }
      }

      // Side wall bounce
      bool collided = false;
      if(ball.pos.x >= X || ball.pos.x < 0){
        collided=true;
        ball.dir.x=-ball.dir.x;
        ball.pos.x+=2*ball.dir.x;
        if(ball.dir.y==0){
          ball.dir.y=rand()%3-1;
          ball.pos.y+=ball.dir.y;
        }
      }
      for(int p =0; p<2; p++){
        if(checkCollision(&player[p], &ball)){
          collided=true;
          bounces++;
          ball.speed=min(ball.speed*1.01, MAX_SPEED);
        }
      }
      if(collided && !mute){
        bounceSound();
      }
      
    }
    
    lightPixel(ball.pos,ball.color);
    strip.show();
      
  }
  
  delay(1);
}

void bounceSound(){
  digitalWrite(BUZZER_PIN,255);
  delay(1);
  digitalWrite(BUZZER_PIN,0);
}

void lossAnimation(struct pongBall ball){
  lightPixel(ball.pos,ball.color);
  strip.show();
  lossSound();
  for(int i =0;i<10;i++){
    lightPixel(ball.pos,rgb_off);
    strip.show();
    delay(120);
    lightPixel(ball.pos,ball.color);
    strip.show();
    delay(120);
  }
}

void lossSound(){
  playNote('A',4,150);
  delay(150);
  playNote('E',3,300);
}

void setupPongRound(struct pongPlayer player[]){
  
  struct color_rgb timer_color;
  timer_color.r=255;

  struct vec pix;
  int enterTime = round(5000.0/X);
  int progress = 0;
  for(int p=0; p<2; p++){
    player->score = 0;
  }
  strip.clear();
  long currTime = millis();
  while(currTime + enterTime*X > millis()){
    if(player[0].playerType==HID && player[1].playerType==HID){
      delay(400);
      break;
    }
    for(int p=0; p<2; p++){
      if(player[p].playerType==NPC && digitalRead(player[p].buttonPort)){
        player[p].playerType=HID;
        pix.y = player[p].pos.y;
        for(int x=0; x<X; x++){
          pix.x = x;
          lightPixel(pix,player[p].color);
        }
      }
    }
    if(currTime + progress*enterTime < millis()){
      for(int p=0; p<2; p++){
        if(player[p].playerType != HID){
          pix.x=!p*progress+p*(X-1-progress);
          pix.y=player[p].pos.y;

          lightPixel(pix,timer_color);
        }
      }
      progress+=1;
      strip.show();

    }
  }
  for(int p=0; p<2; p++){
    if(player->playerType==NPC){
      player->pos.x = round((X+player->size)/2);
    }
  }
}


void setupPongBall(struct pongBall* ball){
  ball->speed=1.0;
  ball->boost=1.0;

  ball->pos.x = 0+(rand()%2)*(X-1);
  ball->pos.y = round(Y/2)-rand()%2;
  ball->dir.x = ball->pos.x > 0 ? -1 : 1;
  ball->dir.y = 0;
  
}

bool checkCollision(struct pongPlayer* player, struct pongBall* ball){
  if(ball->pos.y != player->pos.y){
    return false;
  }
  for(int i=0; i<player->size; i++){
    if(ball->pos.x == player->pos.x+i){
      // Collided
      ball->dir.y=-ball->dir.y;
      ball->pos.y+=2*ball->dir.y;
      // Handle fired balls getting random direction
      if(ball->dir.x==0){
        ball->boost = 1.0;
        if(ball->pos.x == 0){
          ball->dir.x = 1;
        }
        else if(ball->pos.x == X-1){
          ball->dir.x = -1;
        }
        else {
          ball->dir.x= 2*(rand()%2)-1;
        }
        ball->pos.x+=ball->dir.x;
      }
      else if(ball->pos.x - ball->dir.x < player->pos.x || ball->pos.x - ball->dir.x > player->pos.x+player->size-1 ){
        ball->dir.x=-ball->dir.x;
        ball->pos.x+=2*ball->dir.x;
        if(ball->pos.x < 0 || ball->pos.x > X-1){
          ball->dir.x=-ball->dir.x;
          ball->pos.x+=2*ball->dir.x;
        }
        
      }
      if(readFire(player, *ball)){
        ball->boost = 1.8;
        ball->pos.x-=ball->dir.x;
        ball->dir.x=0;
      }
      return true;
    }
    else if(ball->pos.x-ball->dir.x == player->pos.x+i){
      ball->dir.y=-ball->dir.y;
      ball->pos.y+=2*ball->dir.y;
      if(readFire(player, *ball)){
        ball->boost = 1.8;
        ball->pos.x-=ball->dir.x;
        ball->dir.x=0;
      }
      return true;
    }
  }
  return false;
}


void makeMove(struct pongPlayer* player, struct pongBall ball){
  switch(player->playerType){
    case HID: return movePlayer(player, ball);
    case NPC: return moveNPC(player, ball); 
  }
}

bool readFire(struct pongPlayer* player, struct pongBall ball){
  switch(player->playerType){
    case HID: return firePlayer(player, ball);
    case NPC: return fireNPC(player, ball); 
  }
}

bool firePlayer(struct pongPlayer* player, struct pongBall ball){
  bool button = digitalRead(player->buttonPort);
  return button;
}

bool fireNPC(struct pongPlayer* player, struct pongBall ball){
  int prob = 5;
  return !(rand()%prob);
}

void movePlayer(struct pongPlayer* player, struct pongBall ball){
  int input = round(float(analogRead(player->wheelPort))*(X-player->size)/1023.0);
  
  input = input*(player->number==0) + (X-player->size-input)*(player->number==1);
  if(player->pos.x == input){
    return;
  }
  
  struct vec pix;
  for(int i=0; i<player->size; i++){
    pix.x = player->pos.x+i;
    pix.y = player->pos.y;
    lightPixel(pix, rgb_off);
  }
  player->pos.x = input;
  drawPlayer(player);
}

void drawPlayer(struct pongPlayer* player){
  struct vec pix;
  for(int i=0; i<player->size; i++){
    pix.x = player->pos.x+i;
    pix.y = player->pos.y;
    lightPixel(pix, player->color);
  }
}


void moveNPC(struct pongPlayer* player, struct pongBall ball){
  int width = player->size;
  int input;
  if( (ball.pos.y-player->pos.y > 0) == (ball.dir.y > 0) ){
    int step = 0;
    if(rand()%3 == 0){
      step = -1*(float(player->pos.x)>(X-1)/2);
      step = rand()%7 == 0 ? -step : step;
    }
    input = player->pos.x + step;
  }else{
    input = round(float(ball.pos.x+ball.dir.x)-float(width-1)/2);
  }
  
  input = min(max(input,0),X-width);
  if(player->pos.x == input){
    return;
  }
  input=player->pos.x + (input-player->pos.x)/abs(input-player->pos.x);
  
  struct vec pix;
  for(int i=0; i<player->size; i++){
    pix.x = player->pos.x+i;
    pix.y = player->pos.y;
    lightPixel(pix, rgb_off);
  }
  player->pos.x = input;
  for(int i=0; i<player->size; i++){
    pix.x = player->pos.x+i;
    pix.y = player->pos.y;
    lightPixel(pix, player->color);
  }
}

int pixel(struct vec pos){
  return pos.x*Y+pos.y*!(pos.x%2)+(Y-pos.y-1)*(pos.x%2);
}

void lightPixel(struct vec pos, struct color_rgb color){
  strip.setPixelColor(pixel(pos),strip.Color(color.r,color.g,color.b));
}

// struct vec {
//   int x;
//   int y;
// }

// struct pongBall {
//   vec dir;
//   ver pos;
// }


// int pong(){
//   ball = new pongBall()
//   ball.pos.x = 0
//   ball.pos.y = 0
//   ball.dir.x = 0
//   ball.dir.y = 1
//   playPong(ball)
// }


// int playPong(pongBall ball){
//   while(1){

//   }
// }

// void moveBall(pongBall ball, )

// SCORE DISPLAY FUNCTIONS

bool nums[10][5][3] =
{
  {
    {0, 1, 0},
    {1, 0, 1},
    {1, 0, 1},
    {1, 0, 1},
    {0, 1, 0},
  },
  {
    {0, 1, 0},
    {1, 1, 0},
    {0, 1, 0},
    {0, 1, 0},
    {1, 1, 1},
  },
  {
    {0, 1, 0},
    {1, 0, 1},
    {0, 0, 1},
    {0, 1, 0},
    {1, 1, 1},
  },
  {
    {1, 1, 0},
    {0, 0, 1},
    {0, 1, 0},
    {0, 0, 1},
    {1, 1, 0},
  },
  {
    {1, 0, 1},
    {1, 0, 1},
    {1, 1, 1},
    {0, 0, 1},
    {0, 0, 1},
  },
  {
    {1, 1, 1},
    {1, 0, 0},
    {1, 1, 0},
    {0, 0, 1},
    {1, 1, 0},
  },
  {
    {0, 1, 0},
    {1, 0, 0},
    {1, 1, 0},
    {1, 0, 1},
    {0, 1, 0},
  },
  {
    {1, 1, 1},
    {0, 0, 1},
    {0, 0, 1},
    {0, 1, 0},
    {0, 1, 0},
  },
  {
    {0, 1, 0},
    {1, 0, 1},
    {0, 1, 0},
    {1, 0, 1},
    {0, 1, 0},
  },
  {
    {0, 1, 0},
    {1, 0, 1},
    {0, 1, 1},
    {0, 0, 1},
    {0, 1, 0},
  }
};


void displayNumber(int number, struct vec pos, struct vec dir, struct color_rgb color, bool clear){
  struct vec pix;

  for(int r=0; r<5; r++){
    for(int c=0; c<3; c++){
      pix.x = pos.x + dir.x*r - dir.y*c;
      pix.y = pos.y + dir.x*c + dir.y*r;
      if(number==-1 ){
        lightPixel(pix, rgb_off);
      }
      else if(nums[number][r][c]){
        lightPixel(pix, color);
      }
      else if(clear){
        lightPixel(pix, rgb_off);
      }
    }
  }
}

void displayBouncesCount(int bounces, struct pongPlayer player){
  struct vec pos, dir;
  int base_x = round(float(X)/2 - 2);
  pos.y=player.number==1?Y-9:8;
  dir.x=0; 
  dir.y=player.number==1?1:-1;
  for(int i=0; i<3; i++){
    pos.x = base_x + (i-1)*4;
    int exp = pow(10,2-i);
    displayNumber(bounces/exp,pos,dir,player.color,true);
    bounces = bounces  % exp;
  }

}



void displayScore(struct pongPlayer player[], unsigned int wait){
  struct vec pix;
  struct color_rgb lineColor;

  strip.clear();
  lineColor.r = 255;
  pix.x=2;
  pix.y=round((Y-1)/2);
  lightPixel(pix, lineColor);
  pix.y=round((Y-1)/2)-1;
  lightPixel(pix, lineColor);

  for(int p = 0; p<2; p++){
    for(int x=0; x<5; x++){
      for(int y=0; y<3; y++){
        if(nums[player[p].score][x][y]){
          pix.x = 4-x;
          pix.y = 2*(player[p].pos.y == 1) + (Y-1)*(player[p].pos.y == Y-2) - y;
          lightPixel(pix, player[p].color);
        }
      }
    }
  }
  strip.show();
  delay(wait);
  strip.clear();
}


void flashWinner(struct pongPlayer player[], unsigned int flashes){
  struct vec pix;
  struct color_rgb lineColor;
  lineColor.r = 255; 
  pix.x=2;
  for(int i=0; i<flashes; i++){
    displayScore(player,700);
    pix.y=4;
    lightPixel(pix, lineColor);
    pix.y=5;
    lightPixel(pix, lineColor);
    strip.show();
    delay(300);
  }
  displayScore(player,1500);

}

// ----------------------------- SOUNDS --------------------------------- 

void playNote(char note, unsigned int octave, unsigned int duration){
  int freq;
  switch(note){
    case 'C': freq = 16.35;
    break;
    case 'D': freq = 18.35;
    break;
    case 'E': freq = 20.60;
    break;
    case 'F': freq = 21.83;
    break;
    case 'G': freq = 24.50;
    break;
    case 'A': freq = 27.50;
    break;
    case 'B': freq = 30.87;
  }
  freq*=pow(2,octave);
  // Serial.println(freq);
  tone(BUZZER_PIN, freq, duration);
}