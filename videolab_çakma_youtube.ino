// VideoLab yatay arayuz - Arduino Uno + MCUFRIEND ILI9341 + TouchScreen
// Kutuphaneler: Adafruit GFX, MCUFRIEND_kbv, TouchScreen
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <string.h>
MCUFRIEND_kbv tft;
#define YP A3
#define XM A2
#define YM 9
#define XP 8
#define MINPRESSURE 100
#define MAXPRESSURE 1000
TouchScreen ts(XP, YP, XM, YM, 300);
#define BLACK 0x0000
#define WHITE 0xFFFF
#define RED 0xF800
#define GREY 0x7BEF
#define DARK 0x18E3
#define BLUE 0x041F
#define GREEN 0x07E0
#define PURPLE 0x781F
#define ORANGE 0xFD20
#define YELLOW 0xFFE0
#define CYAN 0x07FF
#define NAVY 0x0008

enum Screen : uint8_t { HOME, PLAYER, SHORTS, SUBSCRIPTIONS, ACCOUNT, SEARCH };
Screen screen = HOME;
uint8_t currentVideo=0, scrollOffset=0, videoCount=12;
char searchText[16]="";
uint8_t searchLen=0;
bool playing=true;
uint16_t elapsed=0;
unsigned long lastTouch=0,lastFrame=0,lastSec=0;
int touchStartY=-1;
const uint16_t durationSec[12]={600,1260,3600,305,780,945,520,430,690,515,800,360};
const uint16_t thumbColors[12]={BLUE,GREEN,ORANGE,PURPLE,CYAN,RED,YELLOW,0xFBE0,0x7D7C,0x2D7F,0xFD20,0x5DFF};
const char* titles[12]={"Raspberry Pi masaustu kurma","Minecraft hayatta kalma","Raspberry Pi OS yukleme","Minecraft'ta ev yaptim","Roblox obby meydan okumasi","Eski oyun konsolu yapimi","Ucretsiz oyunlar listesi","Komik kedi anlari","En komik fail videolari","Evde mini bilim deneyleri","Arduino ile isik oyunu","Komik oyun anlari"};
const char* channels[12]={"Pi Atolyesi","Block World","Tekno Rehber","Block World","Roblox Rehberi","Retro Oyunlar","Oyun Kesfi","Komik Patiler","Gulus Garantili","Mini Bilim","Devre Kulubu","Oyun Komedisi"};

bool rawTouch(int &rx,int &ry){ TSPoint p=ts.getPoint(); pinMode(XM,OUTPUT); pinMode(YP,OUTPUT); if(p.z>MINPRESSURE && p.z<MAXPRESSURE){rx=p.x;ry=p.y;return true;} return false; }
// Common 2.8in resistive shield raw range; if your panel differs, touch calibration may be needed.
bool touchXY(int &x,int &y){int a,b;if(!rawTouch(a,b))return false; x=map(b,120,900,0,319); y=map( a,120,900,0,239); x=constrain(x,0,319); y=constrain(y,0,239); return true;}
void header(){tft.fillRect(0,0,320,36,BLACK);tft.fillRoundRect(8,7,28,22,5,RED);tft.fillTriangle(18,11,18,25,28,18,WHITE);tft.setTextColor(WHITE);tft.setTextSize(2);tft.setCursor(44,10);tft.print(F("VideoLab"));tft.drawRoundRect(207,6,105,25,5,GREY);tft.drawCircle(219,17,5,WHITE);tft.drawLine(223,21,228,26,WHITE);tft.setTextSize(1);tft.setTextColor(searchLen?WHITE:GREY);tft.setCursor(233,15);if(searchLen)tft.print(searchText);else tft.print(F("Ara..."));}
void sidebar(){tft.fillRect(0,36,58,204,0x1082);const char* labs[]={"Ev","Shorts","Abone","Hesap"};for(int i=0;i<4;i++){int yy=46+i*48;bool active=(i==0&&screen==HOME)||(i==1&&screen==SHORTS)||(i==2&&screen==SUBSCRIPTIONS)||(i==3&&screen==ACCOUNT);tft.fillRoundRect(5,yy,48,39,6,active?RED:0x2104);tft.setTextSize(1);tft.setTextColor(WHITE);tft.setCursor(14,yy+15);tft.print(labs[i]);}}
void thumb(uint8_t i,int x,int y){
  uint16_t bg=thumbColors[i%12]; tft.fillRoundRect(x,y,100,55,5,bg);
  // bright, high-contrast illustrated mini thumbnails
  if(i==0||i==2||i==9||i==10){
    tft.fillRect(x+4,y+4,92,47,0x0820); tft.fillCircle(x+80,y+12,8,YELLOW);
    tft.fillRoundRect(x+10,y+11,46,32,3,(i==9)?GREEN:BLUE);
    tft.drawRect(x+14,y+15,38,24,CYAN); tft.fillRect(x+18,y+20,8,7,RED); tft.fillRect(x+29,y+20,8,7,YELLOW);tft.fillRect(x+40,y+20,8,7,GREEN);
    tft.fillRoundRect(x+63,y+20,25,22,3,GREY);tft.fillRect(x+68,y+25,15,12,BLACK);tft.fillCircle(x+75,y+31,3,GREEN);
    if(i==9){tft.fillCircle(x+73,y+14,4,WHITE);tft.fillCircle(x+82,y+14,4,WHITE);tft.fillCircle(x+73,y+14,2,BLACK);tft.fillCircle(x+82,y+14,2,BLACK);}
  }else if(i==1||i==3||i==5||i==6||i==11){
    tft.fillRect(x+3,y+3,94,49,(i==11)?PURPLE:CYAN);tft.fillCircle(x+82,y+12,7,YELLOW);tft.fillRect(x+3,y+33,94,19,GREEN);
    for(int k=0;k<5;k++)tft.fillRect(x+5+k*18,y+40+(k%2)*5,17,12,(k%2)?0x5A20:0x3666);
    tft.fillRect(x+42,y+19,15,14,0x8C40);tft.fillRect(x+42,y+33,6,12,BLUE);tft.fillRect(x+51,y+33,6,12,BLUE);tft.fillRect(x+39,y+23,4,9,ORANGE);tft.fillRect(x+57,y+23,4,9,ORANGE);
    if(i==3){tft.fillRect(x+69,y+29,20,19,ORANGE);tft.fillTriangle(x+65,y+30,x+79,y+17,x+93,y+30,RED);} if(i==11){tft.fillCircle(x+18,y+16,8,WHITE);tft.fillCircle(x+15,y+14,2,BLACK);tft.fillCircle(x+21,y+14,2,BLACK);tft.drawLine(x+14,y+20,x+22,y+20,RED);}
  }else if(i==4){
    tft.fillRect(x+3,y+3,94,49,NAVY);for(int k=0;k<4;k++)tft.fillRect(x+6+k*23,y+39-k*7,20,8,(k%2)?GREEN:YELLOW);
    tft.fillRoundRect(x+43,y+14,15,15,3,ORANGE);tft.fillRect(x+43,y+29,15,13,BLUE);tft.fillRect(x+39,y+30,4,11,RED);tft.fillRect(x+58,y+30,4,11,RED);
  }else if(i==7||i==8){
    tft.fillRect(x+3,y+3,94,49,0xFD20);tft.fillCircle(x+50,y+28,21,WHITE);tft.fillCircle(x+42,y+22,4,BLACK);tft.fillCircle(x+58,y+22,4,BLACK);tft.fillRoundRect(x+39,y+30,22,9,4,RED);tft.fillRect(x+43,y+30,14,3,WHITE);
    if(i==7){tft.fillTriangle(x+29,y+14,x+32,y+2,x+43,y+13,0x7BEF);tft.fillTriangle(x+57,y+13,x+69,y+2,x+72,y+14,0x7BEF);}else{tft.drawCircle(x+50,y+28,25,YELLOW);tft.fillCircle(x+78,y+12,5,RED);}
  }else{
    tft.fillRect(x+3,y+3,94,49,BLACK);tft.fillRoundRect(x+14,y+8,72,38,5,GREY);tft.fillRoundRect(x+20,y+13,60,25,3,NAVY);for(int k=0;k<5;k++)tft.fillCircle(x+27+k*11,y+25,4,(k%2)?RED:CYAN);tft.fillRect(x+37,y+40,24,3,WHITE);
  }
  // play badge
  tft.fillRoundRect(x+78,y+36,17,14,3,BLACK);tft.fillTriangle(x+84,y+39,x+84,y+47,x+91,y+43,WHITE);
}
bool matches(uint8_t i){if(!searchLen)return true;char q[16];strncpy(q,searchText,15);q[15]=0;for(int j=0;q[j];j++)if(q[j]>='A'&&q[j]<='Z')q[j]+=32;char title[40];strncpy(title,titles[i],39);title[39]=0;for(int j=0;title[j];j++)if(title[j]>='A'&&title[j]<='Z')title[j]+=32;return strstr(title,q)!=NULL || (strstr(q,"pi")&&(i==0||i==2)) || (strstr(q,"oyun")&&(i==1||i==3||i==4||i==5||i==6||i==11)) || (strstr(q,"komik")&&(i==7||i==8||i==11));}
void drawCard(uint8_t i,int y){tft.fillRect(64,y,246,42,BLACK);thumb(i,67,y+2);tft.setTextSize(1);tft.setTextColor(WHITE);tft.setCursor(173,y+7);tft.print(titles[i]);tft.setTextColor(GREY);tft.setCursor(173,y+23);tft.print(channels[i]);tft.drawFastHLine(66,y+41,243,DARK);}
void drawScrollbar(int total,int visible){int trackX=313,top=53,trackH=181;tft.fillRoundRect(trackX,top,5,trackH,2,0x3186);int thumbH=(total<=visible)?trackH:max(22,(trackH*visible)/total);int maxScroll=max(0,total-visible);int thumbY=top+(maxScroll?((trackH-thumbH)*scrollOffset/maxScroll):0);tft.fillRoundRect(trackX,thumbY,5,thumbH,2,CYAN);tft.drawFastHLine(310,53,8,GREY);tft.drawFastHLine(310,234,8,GREY);}
void drawHome(){screen=HOME;tft.fillScreen(BLACK);header();sidebar();tft.setTextSize(1);tft.setTextColor(CYAN);tft.setCursor(68,39);tft.print(F("Senin icin onerilenler"));int total=0;for(uint8_t i=0;i<videoCount;i++)if(matches(i))total++;int row=0,shown=0;for(uint8_t i=0;i<videoCount;i++)if(matches(i)){if(row>=scrollOffset&&shown<4){drawCard(i,53+shown*45);shown++;}row++;}if(!total){tft.setTextColor(WHITE);tft.setCursor(80,100);tft.print(F("Sonuc bulunamadi"));}drawScrollbar(total,4);}
void drawSplash(){for(int f=0;f<16;f++){tft.fillScreen(BLACK);int w=30+(f%8)*8;tft.fillRoundRect(160-w/2,65,w,52,12,RED);tft.fillTriangle(151,75,151,107,174,91,WHITE);tft.setTextColor(WHITE);tft.setTextSize(3);tft.setCursor(92,132);tft.print(F("VideoLab"));tft.setTextSize(1);tft.setTextColor(GREY);tft.setCursor(111,166);tft.print(F("Teknoloji ve oyun"));delay(65);}}
void simplePage(Screen s){screen=s;tft.fillScreen(BLACK);header();sidebar();tft.setTextColor(WHITE);tft.setTextSize(2);tft.setCursor(78,55);if(s==SHORTS)tft.print(F("Shorts"));else if(s==SUBSCRIPTIONS)tft.print(F("Abonelikler"));else tft.print(F("Hesap"));tft.setTextSize(1);tft.setTextColor(GREY);tft.setCursor(78,84);if(s==SHORTS)tft.print(F("Kisa video akisi"));else if(s==SUBSCRIPTIONS)tft.print(F("Takip ettigin kanallar"));else tft.print(F("VideoLab profili"));if(s==SHORTS){thumb(4,82,110);thumb(1,198,110);}}
void keyboard(){screen=SEARCH;tft.fillScreen(BLACK);header();tft.setTextColor(WHITE);tft.setTextSize(1);tft.setCursor(8,41);tft.print(F("Aramak icin harfe dokun"));const char keys[3][10]={{'a','b','c','d','e','f','g','h','i','j'},{'k','l','m','n','o','p','r','s','t','u'},{'v','w','x','y','z','0','1','2','3','4'}};for(int r=0;r<3;r++)for(int c=0;c<10;c++){int xx=7+c*31,yy=60+r*34;tft.drawRoundRect(xx,yy,27,28,4,WHITE);tft.setCursor(xx+10,yy+10);tft.print(keys[r][c]);}tft.drawRoundRect(7,166,70,27,4,GREEN);tft.setCursor(28,176);tft.print(F("Sil"));tft.drawRoundRect(84,166,78,27,4,CYAN);tft.setCursor(96,176);tft.print(F("Bosluk"));tft.fillRoundRect(169,166,143,27,4,RED);tft.setTextColor(WHITE);tft.setCursor(218,176);tft.print(F("Ara"));tft.setTextColor(GREY);tft.setCursor(8,207);tft.print(F("Arama: "));tft.print(searchText);tft.setCursor(8,225);tft.print(F("Ustteki VideoLab: geri"));}
void player(uint8_t i){screen=PLAYER;currentVideo=i;elapsed=0;playing=true;tft.fillScreen(BLACK);tft.setTextColor(WHITE);tft.setTextSize(1);tft.setCursor(8,8);tft.print(F("< Geri"));thumb(i,10,35);tft.setTextSize(1);tft.setCursor(120,42);tft.print(titles[i]);tft.setTextColor(GREY);tft.setCursor(120,59);tft.print(channels[i]);tft.fillRect(10,100,300,91,0x0841);tft.drawRoundRect(10,100,300,91,4,GREY);tft.setTextColor(CYAN);tft.setCursor(20,108);tft.print(F("SIMULATED VIDEO"));tft.setTextColor(WHITE);tft.setCursor(20,173);tft.print(F("Animasyonlu sahne oynuyor"));tft.drawRoundRect(10,198,300,8,3,WHITE);tft.fillRect(12,200,2,4,RED);tft.fillRoundRect(123,207,74,27,6,DARK);tft.setTextSize(1);tft.setCursor(151,216);tft.print(playing?"PAUSE":"> PLAY");}
void animateVideo(){static uint8_t phase=0;phase++;tft.fillRect(14,121,292,45,0x0841);int x=24+(phase*7)%265;uint8_t i=currentVideo;if(i==7||i==8||i==11){tft.fillCircle(x,142,13,YELLOW);tft.fillCircle(x-4,138,2,BLACK);tft.fillCircle(x+4,138,2,BLACK);tft.drawLine(x-5,148,x+5,148,RED);tft.fillCircle(280-x%40,130,3,WHITE);tft.fillCircle(50+(phase*3)%220,158,2,CYAN);}else if(i==0||i==2||i==10){tft.fillRoundRect(x,130,28,24,3,GREEN);tft.fillRect(x+5,135,18,12,BLACK);tft.fillCircle(x+14,141,3,CYAN);for(int k=0;k<5;k++)tft.drawFastVLine(35+k*50,126+(phase+k)%8,20,YELLOW);}else{tft.fillCircle(x,143,10,(phase%2)?ORANGE:CYAN);tft.fillRect(x-6,153,12,9,BLUE);tft.fillRect(x-13,161,8,5,RED);tft.fillRect(x+5,161,8,5,GREEN);tft.fillCircle(35+(phase*5)%250,128,3,YELLOW);}int px=12+((millis()/180)%294);tft.fillRect(12,200,296,4,BLACK);tft.fillRect(12,200,px,4,RED);}
void setup(){uint16_t id=tft.readID();if(id==0xD3D3)id=0x9341;tft.begin(id);tft.setRotation(1);randomSeed(analogRead(A5));drawSplash();drawHome();}
void loop(){int x,y;if(touchXY(x,y)&&millis()-lastTouch>180){lastTouch=millis();
 if(screen==HOME||screen==SHORTS||screen==SUBSCRIPTIONS||screen==ACCOUNT){
  if(y<36&&x>195){keyboard();}
  else if(x<60&&y>=36){if(y<84){scrollOffset=0;drawHome();}else if(y<132)simplePage(SHORTS);else if(y<180)simplePage(SUBSCRIPTIONS);else simplePage(ACCOUNT);}
  else if(screen==HOME&&x>=310&&y>=53){int total=0;for(uint8_t i=0;i<videoCount;i++)if(matches(i))total++;int maxScroll=max(0,total-4);if(y<145)scrollOffset=(scrollOffset>0)?scrollOffset-1:0;else scrollOffset=min(maxScroll,(int)scrollOffset+1);drawHome();}
  else if(screen==HOME&&x>=65&&y>=53){int row=(y-53)/45;if(row>=0&&row<4){int seen=0,vis=0;for(uint8_t i=0;i<videoCount;i++)if(matches(i)){if(seen>=scrollOffset){if(vis==row){player(i);break;}vis++;}seen++;}}}
  else if(screen==SHORTS)player(11);
 }else if(screen==SEARCH){if(y<36&&x<200){drawHome();}else if(y>=60&&y<162){int r=(y-60)/34,c=(x-7)/31;const char keys[3][10]={{'a','b','c','d','e','f','g','h','i','j'},{'k','l','m','n','o','p','r','s','t','u'},{'v','w','x','y','z','0','1','2','3','4'}};if(r<3&&c>=0&&c<10&&searchLen<14){searchText[searchLen++]=keys[r][c];searchText[searchLen]=0;keyboard();}}else if(y>=166&&y<198){if(x<80&&searchLen){searchText[--searchLen]=0;keyboard();}else if(x<165&&searchLen<14){searchText[searchLen++]=' ';searchText[searchLen]=0;keyboard();}else{scrollOffset=0;drawHome();}}}
 else if(screen==PLAYER){if(y<30&&x<100)drawHome();else if(y>=205&&y<239){playing=!playing;tft.fillRoundRect(123,207,74,27,6,DARK);tft.setTextColor(WHITE);tft.setTextSize(1);tft.setCursor(135,216);tft.print(playing?"PAUSE":"PLAY");}}
 }
 if(screen==PLAYER&&playing&&millis()-lastFrame>120){lastFrame=millis();animateVideo();if(millis()-lastSec>1000){lastSec=millis();elapsed++;if(elapsed>=durationSec[currentVideo]){uint8_t next=(currentVideo+1)%videoCount;player(next);}}}
}
