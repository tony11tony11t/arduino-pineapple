#include "main.h"
#include "define.h"

/*宣告音樂播放器變數*/
SoftwareSerial DFPlayer(MUSIC_RX,MUSIC_TX);
DFRobotDFPlayerMini player;

/*宣告六軸陀螺儀變數*/
int16_t ax, ay, az, gx, gy, gz,tmp;

int Ananas=Ananas_LEVEL1_STOP;
byte touchpoint[8];

/*宣告Wifi傳送資料變數*/
const char* ssid="My ASUS";
const char* password="0981882626";
const char* host="192.168.43.246";
String url = "/AnanasServer/php/client/Update_NewInfo.php";
const byte DataNum=4;
String DataName[DataNum][2]={
  {"number",PINEAPPLE_PIN},
  {"light","0"},
  {"alive","0"},
  {"level",(String)Ananas}};
String postRequest;
WiFiClient client;
int DoingTime=0;
const int SendDataTime=5000;
int DoMPUTime=0;
const int MPUTime=2000;
long OKTime=0;

void setup() {
  Serial.begin(BAUD115200);
  
  /*初始化ESP8266**/
  delay(10);
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  client.setNoDelay(true);

  /*初始化TTP223*/
  pinMode(LOAD, OUTPUT);
  pinMode(CLOCKENABLE, OUTPUT);
  pinMode(CLOCKIN, OUTPUT);

  /*初始化DFPlayer(音樂播放器)*/
  DFPlayer.begin(BAUD9600);
  while(player.begin(DFPlayer)==false){
    delay(500);
    Serial.print("-");
  }
  Serial.println(player.begin(DFPlayer)?"DFplayer:SetUp OK":"DFplayer:SetUp Error");
  player.volume(30);

  pinMode(D0,OUTPUT);
  digitalWrite(D0,HIGH);
  player.play(18);
  delay(3500);
  

  /*初始化GY-521(六軸傳感器)*/
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true); 

  ChangeAnanasLevel(Ananas_LEVEL2_RESETGY);
}

void PostData(){
  postRequest =(String)("GET ") + url +"?";
  //帶入參數
  for(int i=0;i<DataNum;i++){
    postRequest+=DataName[i][0];
    postRequest+="=";
    postRequest+=DataName[i][1];
    if(i!=DataNum-1)
      postRequest+="&";
  }
  postRequest +=" HTTP/1.1\r\n" ;
  postRequest +="Content-Type: text/html;charset=utf-8\r\n";
  postRequest +="Host: ";
  postRequest += host;
  postRequest +="\r\n";
  postRequest +="Connection: Keep Alive\r\n\r\n";
}

void ChangeData(byte DataPos,String val){
  DataName[DataPos][1]=val;
}

void ChangeAnanasLevel(int level){
  Ananas=level;
  ChangeData(3,(String)Ananas);
  secret_which=0;
}


boolean SendDataToDB(){
  if(!client.connect(host,Port)){
    Serial.print("connecting failed ");
    return false;
  } 
  delay(10);
  PostData();
  client.print(postRequest); // 發送HTTP請求
  return true;

}

void GetGYData(){
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true); // request a total of 14 registers
  ax=Wire.read()<<8|Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  ay=Wire.read()<<8|Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  az=Wire.read()<<8|Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  tmp=Wire.read()<<8|Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  gx=Wire.read()<<8|Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  gy=Wire.read()<<8|Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  gz=Wire.read()<<8|Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}

/*---------------------------------------------------*/
 
byte TTP;  //紀錄TTP223數據(0~255)

void GetTouchInfo(){
      /*透過腳位得知現在被觸發的模組*/
      digitalWrite(LOAD,LOW);
      delayMicroseconds(5);
      digitalWrite(LOAD,HIGH);
      delayMicroseconds(5);
      digitalWrite(CLOCKIN,HIGH);
      digitalWrite(CLOCKENABLE,LOW);
      TTP=shiftIn(DATAIN, CLOCKIN, LSBFIRST);
      digitalWrite(CLOCKENABLE,HIGH);
      
      /*重置陣列*/
      for(int i=0;i<TOUCH_COL;i++){
         touchpoint[i]=0;
      }
      
      byte TTP_temp=TTP;
      /*將0~255換算成8-bits*/
      int point=0;
      while(TTP_temp!=1&&TTP_temp!=0){
         touchpoint[point]=TTP_temp%2;
         TTP_temp/=2;
         point++;
      }
      touchpoint[point]=TTP_temp;
      
}
/*---------------------------------------------------*/
const long TouchSuccess=100000;
byte OldSoundPlay=-1;
long TouchTime=0;
int AsPlay=0;

boolean SetMusic(){
  /*決定音樂是要放哪一首*/
  /*這邊有一個規則就是我們會放編號的第一首*/
    for(int i=0;i<8;i++){ 
      if(touchpoint[i]==1){
         TouchTime++;
         /*檢查是否跟上次摸得一樣*/
         if(i+1!=OldSoundPlay&&AsPlay==0){
            player.play(i+1);
            AsPlay=1;
            OldSoundPlay=i+1;
          }
          break;
      }
      if(i==7&&OldSoundPlay!=LetGoMusic){
        AsPlay=0;
        player.play(LetGoMusic);
        OldSoundPlay=LetGoMusic;
      }
    } 
   if(TouchTime>=TouchSuccess){
      AsPlay=0;
      player.play(LetGoMusic);
      return true;
    }
  return false;
}
/*---------------------------------------------------*/

  /* Level    ->速度超過才算在旋轉
   * gyMAX    ->位移要累積到這個數據才算旋轉完成
   * levelMAX ->速度要超過Level多少次
   * stopMAX  ->停止超過多少次就重置
   * stopV    ->停止時的速度
   */
  const int Level=-3000;
  const long gyMAX=-270000;
  const byte levelMAX=25;
  const byte stopMAX=8;
  const byte stopV=100;

  byte NowLevel=0;
  byte oldLevel=0;
  byte stopCount=0;
  long gytotal=0;
  byte stage=0;
boolean DetectRotate(){
  if(stage==0){
    /*開始偵測是否旋轉*/
    if(gy<Level){
       NowLevel++;
       Serial.print(gytotal);
       Serial.print("/");
       Serial.print(gyMAX); 
       Serial.println();
    }
    gytotal+=gy;
    /*檢測是否旋轉完畢*/
    if(NowLevel>=levelMAX&&gytotal<=gyMAX){
      stage=1;
    }
    /*關卡停住stopMAX次就從頭*/
    if(oldLevel==NowLevel){
      stopCount++;
    }else{
      stopCount=0;
    }
    if(stopCount>=8){
      NowLevel=0;
      stopCount=0;
    }
    if(NowLevel==0){
      gytotal=0;
    }
    oldLevel=NowLevel;
    return false;
  }else if(stage==1){
      if(gy>-100)
        return true;
  }
}
/*---------------------------------------------------*/
long ZAvg=0,YAvg=0,XAvg=0,TotalAvg=0;
double Z_Standard=0,Y_Standard=0,X_Standard=0;
int AvgTime=0;
boolean ResetXYZ(){
  ZAvg+=az;
  YAvg+=ay;
  XAvg+=ax;
  AvgTime++;
   if(AvgTime==500){
     XAvg/=AvgTime;
     YAvg/=AvgTime;
     ZAvg/=AvgTime;
     TotalAvg=sqrt(XAvg*XAvg+YAvg*YAvg+ZAvg*ZAvg);
     AvgTime=0;
     X_Standard=(double)XAvg/(double)TotalAvg;
     Y_Standard=(double)YAvg/(double)TotalAvg;
     Z_Standard=(double)ZAvg/(double)TotalAvg;   
     return true;
  }
  return false;
}
/*---------------------------------------------------*/
long MoveTime=0,Continuous=0,Nocontinuous=0,Distance=0;
double Judge_Standard=0.02;
double Total_Standard=250;
byte MoveTime_Standard=5;
byte NoContinuous_Standard=3;
 
boolean DetectMove(){
  /*找出當下的數值*/
  long x=ax;
  long y=ay;
  long z=az;
  long total=sqrt(x*x+y*y+z*z);
  double Now_X=(double)x/(double)total;
  double Now_Y=(double)y/(double)total;
  double Now_Z=(double)z/(double)total;
  if((fabs(Now_X-X_Standard)>=Judge_Standard||
     fabs(Now_Y-Y_Standard)>=Judge_Standard||
     fabs(Now_Z-Z_Standard)>=Judge_Standard)&&
     abs(TotalAvg-total)>=Total_Standard){
     MoveTime++;
     if(MoveTime>=MoveTime_Standard){
      MoveTime=0;
        return true;
     }
  }else{
    Nocontinuous++;
    if(Nocontinuous>=NoContinuous_Standard)
      MoveTime=0;
  }
  return false;
}
/*---------------------------------------------------*/
 /* Stop_Standard       ->XYZ數據要小於這個標準
  * StopTotal_Standard  ->total數據要小於這個標準
  * StopLevel_Standard  ->要連續靜止這麼多次
  */
 const double Stop_Standard=0.02;
 const double StopTotal_Standard=250;
 const int StopLevel_Standard=10;

 double Old_x=0,Old_y=0,Old_z=0;
 long Old_total=0;
 byte StopLevel=0;
boolean DetectStop(){
  /*找出當下的數值*/
  long x=ax;
  long y=ay;
  long z=az;
  long total=sqrt(x*x+y*y+z*z);
  double Now_X=(double)x/(double)total;
  double Now_Y=(double)y/(double)total;
  double Now_Z=(double)z/(double)total;
  if((fabs(Old_x-Now_X)<=Stop_Standard&&
     fabs(Old_y-Now_Y)<=Stop_Standard&&
     fabs(Old_z-Now_Z)<=Stop_Standard)&&
     abs(Old_total-total)<=StopTotal_Standard){
     StopLevel++;
   }else{
     StopLevel=0;
   }
   if(StopLevel==StopLevel_Standard){
    StopLevel=0;
    return true;
   }
   Old_x=Now_X;
   Old_y=Now_Y;
   Old_z=Now_Z;
   Old_total=total;
   return false;
}

void initAnanas(){
  ChangeAnanasLevel(Ananas_LEVEL2_RESETGY);
  digitalWrite(D0,HIGH);
  OldSoundPlay=-1;
  TouchTime=0;
  OKTime=0;

  NowLevel=0;
  oldLevel=0;
  stopCount=0;
  gytotal=0;
  stage=0;

  ZAvg=0;
  YAvg=0;
  XAvg=0;
  TotalAvg=0;
  AvgTime=0;

  MoveTime=0;
  Continuous=0;
  Nocontinuous=0;
  Distance=0;

  Old_x=0;
  Old_y=0;
  Old_z=0;
  Old_total=0;
  StopLevel=0;

  DataName[0][1]=PINEAPPLE_PIN;
  DataName[1][1]="0";
  DataName[2][1]="0";
  DataName[3][1]=(String)Ananas_LEVEL2_RESETGY;
  postRequest="";

  DoingTime=0;
  DoMPUTime=0;
  
 }


void loop() { 
  /*取得目前觸控狀態*/
  GetTouchInfo();
  ChangeData(1,(String)TTP);

  /*透過switch選擇目前鳳梨需要做的事情，一共分為七個階段*/ 
  switch(Ananas){
   case Ananas_LEVEL2_RESETGY:
      GetGYData();
      if(ResetXYZ()){
        ChangeAnanasLevel(Ananas_LEVEL3_TOUCH);
      } 
      break;
    case Ananas_LEVEL3_TOUCH:
      if(SetMusic()){
        ChangeAnanasLevel(Ananas_LEVEL4_HOLDUP);
        digitalWrite(D0,LOW);
      }
      break;
    case Ananas_LEVEL4_HOLDUP:
      if(DoMPUTime>=MPUTime){
         GetGYData();
         if(DetectMove()){
            player.play(19);  
            ChangeAnanasLevel(Ananas_LEVEL5_ROTATE);
         }
         DoMPUTime=0;
      }else{
        DoMPUTime++;
      };break;
    case Ananas_LEVEL5_ROTATE:
      if(DoMPUTime>=MPUTime){
         GetGYData();
         if(DetectRotate()){
           ChangeAnanasLevel(Ananas_LEVEL6_BACK);
         }
         DoMPUTime=0;
      }else{
        DoMPUTime++;
      };break;
    case Ananas_LEVEL6_BACK:
      if(DoMPUTime>=MPUTime){
         GetGYData();
         if(DetectStop()){
           ChangeAnanasLevel(Ananas_LEVEL7_START);
         }
         DoMPUTime=0;
      }else{
        DoMPUTime++;
      };break;
    case Ananas_LEVEL7_START:
      digitalWrite(D0,HIGH);
      OKTime++;
      if(OKTime>=100000){
        ChangeAnanasLevel(Ananas_LEVEL8_WAITRESET);
     }
    case Ananas_LEVEL8_WAITRESET:
      if(TTP>14&&TTP<32){
         initAnanas();
         player.play(18);
         delay(1000);
      }
      break;
    default:break;
  }

  /* 透過此方式製作出arduino的多重執行緒功能
   * 在DoingTime累加到定值後才會觸發SendDataToDB的函式
   * 達成一秒更新一次資料庫的功能
   */
  if(DoingTime>=SendDataTime){
    SendDataToDB();
    DoingTime=0;
  }else{
    DoingTime++;
  }

}
