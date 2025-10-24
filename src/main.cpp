#include <Arduino.h>

// LEDを接続したピン
int DIN_H = 2;
int DIN_L = 3;
//接続したＬＥＤの数を指定
int LED_MAX = 1;

const int trigPin = 5;
const int echoPin = 4;

const int soundPin = 6;

const int SoundOKPin = 7;

//cm
const int MIN_Dist = 10;
const int MAX_Dist = 50;

const int MIN_Hue = 0;      // 赤
const int MAX_Hue = 220;    // 青

const int DO = 261;
const int RE = 294;
const int MI = 330;
const int FA = 349;
const int SO = 392;
const int LA = 440;
const int SI = 493;
const int DO2 = 523;

double duration = 0;
double distance = 0;
double smoothedDistance = 0.0; // ★ 修正: 滑らかにした距離を保持する変数
bool firstReading = true;      // ★ 修正: 最初の測定かを判定するフラグ

// ★ 修正: スムージングの割合 (0.0に近いほど滑らか, 1.0に近いほど素早く反応)
const float SMOOTHING_FACTOR = 0.08; // 0.1 〜 0.3 くらいがおすすめです

float ratio = 0.0;

// --- 関数プロトタイプ宣言 ---
void LED_Init();
void LED_Set();
void LED_Hi_Bit();
void LED_Low_Bit();
void LED_Color_RGB(byte led_r, byte led_g, byte led_b);
void HSV_to_RGB(int h, int s, int v, int &r, int &g, int &b);
//=============================================================================//

void setup() {
  Serial.begin(115200);                
  pinMode(DIN_H, OUTPUT);     
  pinMode(DIN_L, OUTPUT);     
  LED_Init();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(soundPin, OUTPUT);
  pinMode(SoundOKPin, INPUT);
  tone(soundPin, 349);
  // ★ 修正: スムージングの初期値を設定
  // センサーの最初の読み取りで初期化するために、ここでは何もしないか、
  // もしくは firstReading フラグを使います。
}

void loop() {
  int r, g, b; // ローカル変数として宣言

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  
  if (duration > 0) {
    // 生の距離を計算
    distance = (duration / 2.0) * 0.034; 
    
    // ★ 修正: スムージング処理
    if (firstReading) {
      // 最初の有効な測定値で初期化
      smoothedDistance = distance;
      firstReading = false;
    } else {
      // 指数移動平均 (ローパスフィルタ)
      // 新しい値 = (生の測定値 * 割合) + (古い値 * (1 - 割合))
      smoothedDistance = (distance * SMOOTHING_FACTOR) + (smoothedDistance * (1.0 - SMOOTHING_FACTOR));
    }
    
    Serial.print("Raw Dist:");
    Serial.print(distance);
    Serial.print(" cm, Smooth Dist:"); // 生の値と滑らかにした値を両方表示
    Serial.print(smoothedDistance);
    Serial.println(" cm");
  }

  // ★ 修正: 生の 'distance' ではなく 'smoothedDistance' を使って計算
  if (smoothedDistance <= MIN_Dist) {
    ratio = 0.0;
  }
  else if (smoothedDistance >= MAX_Dist) {
    ratio = 1.0;
  }
  else {
    ratio = (float)(smoothedDistance - MIN_Dist) / (MAX_Dist - MIN_Dist);
  }

  int hue = (int)(MIN_Hue + (MAX_Hue - MIN_Hue) * ratio);
  int saturation = 255; 
  int value = 255;      

  HSV_to_RGB(hue, saturation, value, r, g, b);

  for(int n = 0; n < LED_MAX; n++) {
    LED_Color_RGB(r, g, b);
  }
  LED_Set();
  
  int soundFreq;
  if(ratio < 0.0){
    soundFreq = DO;
  }
  else if(ratio > 1.0){
    soundFreq = DO2;
  }
  else if(ratio >= 0.0 && ratio <= (1.0)/8){
    soundFreq = DO;
  }
  else if(ratio > (1.0)/8 && ratio <= (2.0)/8){
    soundFreq = RE;
  }
  else if(ratio > (2.0)/8 && ratio <= (3.0)/8){
    soundFreq = MI;
  }
  else if(ratio > (3.0)/8 && ratio <= (4.0)/8){
    soundFreq = FA;
  }
  else if(ratio > (4.0)/8 && ratio <= (5.0)/8){
    soundFreq = SO;
  }
  else if(ratio > (5.0)/8 && ratio <= (6.0)/8){
    soundFreq = LA;
  }
  else if(ratio > (6.0)/8 && ratio <= (7.0)/8){
    soundFreq = SI;
  }
  else{
    soundFreq = DO2;
  }
  if (digitalRead(SoundOKPin) == HIGH) {
    tone(soundPin, soundFreq);
  }
  else {
    noTone(soundPin);
  }
  // ★ 修正: delayを短くして更新頻度を上げる (50Hz)
  delay(20); 
}




//=============================================================================//
// (以下、LED_Init, LED_Set, LED_Hi_Bit, LED_Low_Bit, 
//  LED_Color_RGB, HSV_to_RGB 関数は変更なし)
//=============================================================================//

// ... (変更のない関数は省略) ...

//=========================================================//
// 初期化信号送出
//=========================================================//
void LED_Init() {
  digitalWrite(DIN_H, LOW);
  digitalWrite(DIN_L, HIGH);
  delay(10); 
}
//=========================================================//
// 値固定(Set)信号送出
//=========================================================//
void LED_Set() {
  digitalWrite(DIN_H, LOW);
  digitalWrite(DIN_L, HIGH);
  delay(1); 
}

//=========================================================//
// H(1)信号送出
//=========================================================//
void LED_Hi_Bit() {
  digitalWrite(DIN_H, HIGH);
  digitalWrite(DIN_L, HIGH);
  delayMicroseconds(1);
  digitalWrite(DIN_H, LOW);
  digitalWrite(DIN_L, HIGH);
  delayMicroseconds(1);
}
//=========================================================//
// L(0)信号送出
//=========================================================//
void LED_Low_Bit() {
  digitalWrite(DIN_H, LOW);
  digitalWrite(DIN_L, LOW);
  delayMicroseconds(1);
  digitalWrite(DIN_H, LOW);
  digitalWrite(DIN_L, HIGH);
  delayMicroseconds(1);
}

//=========================================================//
// LED用にＲＧＢカラー値を変換して指定する関数 
//=========================================================//
void LED_Color_RGB(byte led_r, byte led_g, byte led_b) {
  for (int k = 0; k <= 7; k++) {  //青 (B)
    if ((bitRead(led_b, 7 - k)) == 1) { 
      LED_Hi_Bit();
    }
    else {
      LED_Low_Bit();
    }
  }
  for (int k = 0; k <= 7; k++) {  //緑 (G)
    if ((bitRead(led_g, 7 - k)) == 1) {
      LED_Hi_Bit();
    }
    else {
      LED_Low_Bit();
    }
  }
  for (int k = 0; k <= 7; k++) {  //赤 (R)
    if ((bitRead(led_r, 7 - k)) == 1) {
      LED_Hi_Bit();
    }
    else {
      LED_Low_Bit();
    }
  }
}

// HSVからRGBへの変換関数
void HSV_to_RGB(int h, int s, int v, int &r, int &g, int &b) {
  unsigned char region, p, q, t;
  unsigned int h_prime, remainder;

  if (s == 0) {
    r = v; g = v; b = v;
    return;
  }
  
  h_prime = (unsigned int)h * 6;
  region = h_prime / 256; 
  remainder = (h_prime % 256);

  p = (v * (255 - s)) / 255;
  q = (v * (65025L - (long)s * remainder)) / 65025L;
  t = (v * (65025L - (long)s * (255 - remainder))) / 65025L;

  switch (region) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: 
    default:
      r = v; g = p; b = q; break;
  }
}