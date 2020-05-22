//Global variable
#define BAUD9600 9600
#define BAUD38400 38400
#define BAUD115200 115200
#define PINEAPPLE_PIN  "E"  //鳳梨機碼

//TTP223
#define LOAD D4        // Connects to Load pin 1 
#define CLOCKIN D6     // Connects to the Clock pin 2
#define DATAIN D5      // Connects to the Data pin 9
#define CLOCKENABLE D3 // Connects to Clock Enable pin 15
#define TOUCH_ROW 4
#define TOUCH_COL 8

//DFPlayer
#define MUSIC_RX D8
#define MUSIC_TX D7
#define PlayerStop 512
#define PlayerPlay 513
#define LetGoMusic 9

//GY-521
#define SDA D2
#define SCL D1
#define MPU_addr 0x68

//ESP8266 WIFI
#define Port 80

/*
 * Ananas_LEVEL1_STOP     ->靜止階段，不能觸發任何狀態
 * Ananas_LEVEL2_RESETGY  ->校正階段，調整六軸陀螺儀數據
 * Ananas_LEVEL3_TOUCH    ->啟動階段，接收觸摸鳳梨訊號
 * Ananas_LEVEL4_HOLDUP   ->啟動階段，偵測是否拿起鳳梨
 * Ananas_LEVEL5_ROTATE   ->啟動階段，偵測是否旋轉完一圈
 * Ananas_LEVEL6_BACK     ->啟動階段，偵測是否放回原位
 * Ananas_LEVEL7_START    ->靜止階段，表示煙霧機正在啟動
 */
#define Ananas_LEVEL1_STOP 0
#define Ananas_LEVEL2_RESETGY 1
#define Ananas_LEVEL3_TOUCH 2
#define Ananas_LEVEL4_HOLDUP 3
#define Ananas_LEVEL5_ROTATE 4
#define Ananas_LEVEL6_BACK 5
#define Ananas_LEVEL7_START 6
#define Ananas_LEVEL8_WAITRESET 7
