#include <Max3421e.h>  //ADKを利用するための3つのライブラリを読み込む
#include <Usb.h>
#include <AndroidAccessory.h>

//外部インテント指定：Androidアプリ側のaccesory_filter.xml内の属性と一致させる(2)
AndroidAccessory acc("Luarida Works",    //第1引数:組織名（manufacturer属性と一致）
    "Sarida_Ard1",                       //第2引数:モデル名（model属性と一致）
    "Sarida Ard1 - Arduino USB Host",    //第3引数:ダイアログ表示メッセージ
    "1.0",                               //第4引数:バージョン（version属性と一致）
    "http://accessories.android.com/",   //第5引数:ジャンプ先URL
    "0000000000000001");                 //第6引数:シリアル番号

void setup()  //最初に一度だけ実行される部分
{
  Serial.begin(115200);
  Serial.println("\r\nStart");
  //初期化：デジタル・アナログ入出力のピンのポート指定(3)

  //0～6を入力ポートに初期化する
  // 7～13ピンは通信で使っています。
  for(int i=0; i<=6; i++){
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);  //内部プルアップ
  }

  //USBホスト機能を有効にする
  acc.powerOn();
  Serial.println("--setup done--");//シリアルモニターにsetup()終了を出力
}

void loop() //繰り返し実行される部分
{
  byte byteDat[16];  //Androidとやりとりするデータ

  //Androidとの接続処理(4)
  if (acc.isConnected()) {  //Androidを起動・接続する命令を送る
    //communicate with Android application

    int len = acc.read(byteDat, sizeof(byteDat), 16);  //ADK接続から読み込み 
    if (len > 0) {
      if(byteDat[3]==0x01){
        Serial.println("adk.pinMode\r\n");
        adkPinMode(byteDat);
      }
      else if(byteDat[3]==0x04){
        Serial.println("adk.analogWrite\r\n");
        adkAnalogWrite(byteDat);
      }      
      else if(byteDat[3]==0x02){
        Serial.println("adk.digitalWrite\r\n");
        adkDigitalWrite(byteDat);
      }
      else if(byteDat[3]==0x06){
        Serial.println("adk.analogReference\r\n");
        adkAnalogReference(byteDat);
      }
      else if(byteDat[3]==0x03){
        Serial.println("adk.digitalRead\r\n");
        adkDigitalRead(byteDat);
      }
      else if(byteDat[3]==0x05){
        Serial.println("adk.analogRead\r\n");
        adkAnalogRead(byteDat);
      }
    }
  } else {
    //set the accessory to its default state
    //Androidと接続されていないときの処理
    //何もしない
  }
  delay(10); //10ミリ秒処理を停止(10ミリ秒おきにloop()を繰り返す)
}

//**************************************************
// 2-B-5.アナログリード: adk.analogRead(0x0B05)
//		adk.analogRead(pin)
//		0x03 0x06 0x0B05 0xXX 0x06
//		0xXXの値
//			ピンの番号(アナログ0～5)
//
//		受信
//		0x03 0x08 0x0B05 0xXX 0xHHHH 0x06
//		0xXXの値
//			ピンの番号
//		0xHHHHの値
//			10ビットの値(0～1023)
//**************************************************
void adkAnalogRead(byte byteDat[])
{
byte result[] = { 0x03, 0x07, 0x0B, 0x03, 0x00, 0x00, 0x06 };
  int v = analogRead(byteDat[4]);
  result[4] = v>>8;
  result[5] = v & 0xFF;
  acc.write(result, 7);  //USBアクセサリ(Android)に書き込む
}

//**************************************************
// 2-B-3.デジタルリード: adk.digitalRead(0x0B03)
//  adk.digitalRead(pin)
//  0x03 0x06 0x0B03 0xXX 0x06
//  0xXXの値
//    ピンの番号(0～6)
//
//  受信
//  0x03 0x06 0x0B03 0xXX 0x06
//  0xXXの値
//    0:LOW
//    1:HIGH
//**************************************************
void adkDigitalRead(byte byteDat[])
{
byte result[] = { 0x03, 0x06, 0x0B, 0x03, 0x00, 0x06 };
  result[4] = digitalRead(byteDat[4]);
  acc.write(result, 6);  //USBアクセサリ(Android)に書き込む
}

//**************************************************
// 2-B-6.アナログリファレンス: adk.analogReference(0x0B06)
//  adk.analogReference(type)
//  0x03 0x06 0x0B06 0xXX 0x06
//  0xXXの値
//    0: DEFAULT
//    1: INTERNAL
//    2: EXTERNAL
//**************************************************
void adkAnalogReference(byte byteDat[])
{
  if(byteDat[4]==0){
    analogReference(DEFAULT);
  }
  else if(byteDat[4]==1){
    analogReference(INTERNAL);
  }
  else if(byteDat[4]==2){
    analogReference(EXTERNAL);
  }
}

//**************************************************
// 2-B-2.デジタルライト: adk.digitalWrite(0x0B02)
//  adk.digitalWrite(pin, value)
//  0x03 0x07 0x0B02 0xXX 0xYY 0x06
//  0xXXの値
//    ピンの番号(0～6)
//  0xYYの値
//    0: LOW
//    1: HIGH
//**************************************************
void adkDigitalWrite(byte byteDat[])
{
  if(byteDat[5]==0){
    digitalWrite(byteDat[4], LOW);
  }
  else{
    digitalWrite(byteDat[4], HIGH);
  }
}

//**************************************************
// 2-B-4.アナログライト: adk.analogWrite(0x0B04)
//  adk.analogWrite(pin, value)
//  0x03 0x07 0x0B04 0xXX 0xYY 0x06
//  0xXXの値
//    ピンの番号(PWM出力が出来るアナログ0～5)
//  0xYYの値
//    出力PWM比率(0～255)
//**************************************************
void adkAnalogWrite(byte byteDat[])
{
  unsigned char v = byteDat[5];
  analogWrite(byteDat[4], (int)v);
}

//**************************************************
// 2-B-1.PINのモード設定: adk.pinMode(0x0B01)
//  adk.pinMode(pin, mode)
//  0x03 0x07 0x0B01 0xXX 0xYY 0x06
//  0xXXの値
//    ピンの番号(0～6)
//  0xYYの値
//    0: INPUTモード
//    1: OUTPUTモード
//**************************************************
void adkPinMode(byte byteDat[])
{
  if(byteDat[5]==0){
    pinMode(byteDat[4], INPUT);
    digitalWrite(byteDat[4], HIGH);  //内部プルアップする
  }
  else{
    digitalWrite(byteDat[4], LOW);  //内部プルアップを切る
    pinMode(byteDat[4], OUTPUT);
  }
}

