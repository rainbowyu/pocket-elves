//改为miniQ平台
#include <Adafruit_NeoPixel.h>
#include <Metro.h> 						//Include Metro library
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>


//#define DEBUG
//#define DEBUGLEDSTRIP
//#define DEBUGIR

//Standard PWM DC control
#define E1 6     //左侧电机使能
#define E2 5     //右侧电机使能
#define M1 7     //左侧电机方向
#define M2 4     //右侧电机方向

#define CARID 0

#define LED_NUM 6

//小车控制宏定义
#define END 255
#define STOP 0								//停止
#define FORWARD 1						//前进+200	速度200
#define BACKWARD 2					//后退+200	速度200
#define TURN_LEFT 3						//左转+200	速度200
#define TURN_RIGHT 4					//右转+200	速度200
#define ACC_FORWARD 5				//加速前进+200	速度200
#define DEC_FORWARD 6				//减速前进+200	速度200
#define ACC_BACKWARD 7			//加速后退+200	速度200
#define DEC_BACKWARD 8			//减速后退+200	速度200
#define ACC_LEFT 9						//加速左传+200	速度200
#define DEC_LEFT 10						//减速左转+200	速度200
#define ACC_RIGHT 11					//加速右转+200	速度200
#define DEC_RIGHT 12					//减速右转+200	速度200
#define LINE 13 							//巡线
#define SET_IR_THRESHOLD 14		//100

//MP3播放： 061 + 1  播放第一首歌
#define MP3_MUSIC 61
#define MP3_VALUE 62
//LED灯：
#define LEDCONTROL 71 
//(0xff代表控制所有8个灯)
#define LEDCOLOR 72
#define LEDTIME 73
//(00关,01开)

//小车定时：
#define TIMER_SET 90
//定时器报警音效：例子+01代表第一首歌
#define TIMER_SOUND 91
//取消定时
#define TIMER_CANCEL 92
//获取小车运动状态：
#define GET_STATE 100
//获取电池电量：
#define GET_ENERGY 101

//小车状态宏定义
#define TIMER_CHANGE 151
#define LED_CHANGE 152
#define SOUND_CHANGE 153

//传感器位置宏定义  5个红外传感器
#define MID A2
#define LEFT A1
#define LLEFT A0
#define RIGHT A3
#define RRIGHT A7

#define ERR 0

//小车运动参数宏定义

#define SPEED_MAX 200			//accmetro decmetro 时间间隔
#define ACC_TIME 2000 			//accmetro  时间间隔
#define DEC_TIME 2000 			//decmetro 时间间隔

//小车返回宏定义

#define CRASH_HIGH 1 			//碰撞传感器高
#define CRASH_LOW 2				//碰撞传感器低
#define IR1_HIGH 11				//巡线传感器1高
#define IR1_LOW 12					//巡线传感器1低
#define IR2_HIGH 13				//巡线传感器2高
#define IR2_LOW 14					//巡线传感器2低
#define IR3_HIGH 15				//巡线传感器3高
#define IR3_LOW 16					//巡线传感器3低
#define IR4_HIGH 17				//巡线传感器4高
#define IR4_LOW 18					//巡线传感器4低
#define IR5_HIGH 19				//巡线传感器5高
#define IR5_LOW 20					//巡线传感器5低
#define STATE_FORWARD  31	//运动状态 前进
#define STATE_BACKWARD 32	//运动状态 后退
#define STATE_LEFT 33				//运动状态 左转
#define STATE_RIGHT  34			//运动状态 右转
#define STATE_STOP 35				//运动状态 停止

#define PIN 13

//小车控制标志位
unsigned char DEC_FLAG = 0;						//减速开启 标志位
unsigned char ACC_FLAG = 0;						//减速开启 标志位
unsigned char TIMER_FLAG = 0;						//定时开启 标志位
unsigned char CMD_FLAG=0;							//接收到串口控制标志位
unsigned char LINE_FLAG=0;							//巡线开启 标志位

typedef unsigned int uint_16;
typedef unsigned char uint_8;
typedef char int_8;
typedef int int_16;

unsigned char serialCmd[2];							//串口命令缓存

//各种metro声明
Metro timerMetro = Metro(250);									
Metro accMetro =Metro(ACC_TIME/SPEED_MAX);
Metro decMetro =Metro(DEC_TIME/SPEED_MAX);
Metro sensorMetro = Metro(20);
Metro ledMetro[LED_NUM]=Metro(20);

unsigned char CMD=0;


unsigned char carSpeed=0;
unsigned char irCount=0;
unsigned char accMaxSpeed=0;
unsigned char decMaxSpeed=0;
unsigned char ledColor[LED_NUM]={};
unsigned char ledControl=0;

unsigned char blackValue=300;					//默认黑色阈值 30*10

unsigned char timerMusic=1;						//默认闹钟音乐
SoftwareSerial mySerial(2, 11);					// RX, TX


// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_NUM , PIN, NEO_GRB + NEO_KHZ800);

//小车状态结构体
struct carCon
{
	unsigned char carID;
	unsigned int carState;
	unsigned char speed;
} ;
struct carCon car;

//传感器状态结构体
struct sensor
{
	unsigned char btnPre;
	unsigned char btn;
	unsigned char color[5];
	unsigned char colorPre[5];
};
struct sensor sensorState;

//led颜色结构体 存储小车颜色
struct color
{
	int red;
	int green;
	int blue;
} ;

struct colorChange
{
	int redChange;
	int greenChange;
	int blueChange;
} ;

const struct color constColor[8] =
{
	{0,0,0},									//关
	{25500,0,0},								//赤
	{25500,165,0},							//橙
	{25500,25500,0},						//黄
	{0,25500,0},								//绿
	{0,127,25500},							//青
	{0,0,25500},								//蓝
	{13900,0,25500}						//紫
};
struct color LED_Color[LED_NUM];
struct colorChange ledColorChange[LED_NUM];

//前进
void forward(unsigned char a,unsigned char b)
{
	analogWrite (E1,a);
	digitalWrite(M1,HIGH);
	analogWrite (E2,b);
	digitalWrite(M2,HIGH);
}

//停止
void stop(void)
{
	forward(0,0);
}

//后退
void backOff (unsigned char a,unsigned char b)
{
	analogWrite (E1,a);
	digitalWrite(M1,LOW);
	analogWrite (E2,b);
	digitalWrite(M2,LOW);
}

//左转
void turnL (unsigned char a,unsigned char b)
{
	analogWrite (E1,a);
	digitalWrite(M1,HIGH);
	analogWrite (E2,b);
	digitalWrite(M2,LOW);
}

//右转
void turnR (unsigned char a,unsigned char b)
{
	analogWrite (E1,a);
	digitalWrite(M1,LOW);
	analogWrite (E2,b);
	digitalWrite(M2,HIGH);
}

//led控制函数

void ledLoop()
{
	for (unsigned char i=0;i<LED_NUM;i++)
	{
		strip.setPixelColor(i, strip.Color(LED_Color[i].red/100, LED_Color[i].green/100, LED_Color[i].blue/100));					//set every led‘s color
	}
	strip.show();
}

void ledChangeColor(unsigned char time)
{
	for (int num=0;num<LED_NUM;num++)
	{
		if (((ledControl>>num)&(0x01))==1)
		{
			if (time ==0 ) time=1;
			ledColorChange[num].redChange=(int)((constColor[ledColor[num]].red-LED_Color[num].red)/time);                //dont used the float to decrease the time 
			ledColorChange[num].greenChange=(int)((constColor[ledColor[num]].green-LED_Color[num].green)/time);
			ledColorChange[num].blueChange=(int)((constColor[ledColor[num]].blue-LED_Color[num].blue)/time);
#ifdef DEBUGLEDSTRIP
			Serial.print("LED");
			Serial.print(num);
			Serial.print(" ");
			Serial.println(ledColorChange[num].redChange);
			Serial.println(ledColorChange[num].greenChange);
			Serial.println(ledColorChange[num].blueChange);
#endif
		}
	}	
}

void turnDownLed(void)
{
	for (int x=0;x<LED_NUM;x++)
	{
		ledColorChange[x].blueChange=0;
		LED_Color[x].blue=0;
	}
}

//MP3播放函数

void changeMusic(int i)
{
	mp3_play_mp3 (i);
#ifdef DEBUG
	Serial.write(i);
#endif
}

void changeMusicValue(unsigned char i)
{
	mp3_set_volume (i);
}

//Timer设置函数
void timerSet(int minutes)
{
	timerMetro.interval((unsigned long)minutes*60*1000);
	TIMER_FLAG=1;
}
void timerChangeMusic(unsigned char music)
{
	timerMusic=music;
}


int carInit(struct carCon *car1)
{
	for (uint_8 i=4;i<8;i++)
	{
		pinMode(i,OUTPUT);
	}
	for (uint_8 i=9;i<11;i++)   //IR
	{
		pinMode(i,OUTPUT);
	}
	for (uint_8 i=12;i<14;i++) //LED
	{
		pinMode(i,OUTPUT);
	}
	pinMode(8,INPUT);			//IR接收口
	Serial.begin(115200);		//设置串口波特率
	strip.begin();
	strip.show(); 					// Initialize all pixels to 'off'
	PCICR=0x01;
	PCMSK0=0x01;               	//使能第0组引脚变化中断
#ifdef DEBUG
	Serial.println("start");
#endif
	car1->carID=CARID;
	car1->carState=0;
	car1->speed=0;
	sei();
	return 0;
}

unsigned char getCmd(unsigned char *serialcmd)
{
#ifdef DEBUG
	Serial.print("cmd0:  ");
	Serial.println(serialcmd[0],HEX);
	Serial.print("cmd1:  ");
	Serial.println(serialcmd[1],HEX);
#endif
	CMD_FLAG=1;
	switch (serialcmd[0])
	{
		case STOP:return STOP;break;
		case TURN_LEFT:return TURN_LEFT;break;
		case ACC_LEFT:return ACC_LEFT;break;
		case DEC_LEFT:return DEC_LEFT;break;
		case TURN_RIGHT:return TURN_RIGHT;break;
		case ACC_RIGHT:return ACC_RIGHT;break;
		case DEC_RIGHT:return DEC_RIGHT;break;
		case FORWARD:return FORWARD;break;
		case ACC_FORWARD:return ACC_FORWARD;break;
		case DEC_FORWARD:return DEC_FORWARD;break;
		case BACKWARD:return BACKWARD;break;
		case ACC_BACKWARD:return ACC_BACKWARD;break;
		case DEC_BACKWARD:return DEC_BACKWARD;break;
		case END:return END;break;

		case MP3_MUSIC:changeMusic(serialCmd[1]);return SOUND_CHANGE;break;
		case MP3_VALUE:changeMusicValue((serialCmd[1]*30)/100);return SOUND_CHANGE;break;

		case LEDCONTROL:ledControl=serialCmd[1];return LEDCONTROL;break;
		case LEDCOLOR:
		for (int i=0;i<LED_NUM;i++)
		{
			if (((ledControl>>i)&(0x01))==1)
			{
				ledColor[i]=constrain(serialCmd[1], 0, 7);
			}
		}		
		return LEDCOLOR;break;
		case LEDTIME:ledChangeColor(serialCmd[1]);return LEDTIME;break;
		
		case TIMER_SET:timerSet(serialCmd[1]);return TIMER_CHANGE;break;
		case TIMER_CANCEL:timerChangeMusic(serialCmd[1]);return TIMER_CHANGE;break;

		case GET_STATE:return GET_STATE;break;
		case GET_ENERGY:return GET_ENERGY;break;
		
		case LINE:return LINE;break;

		case SET_IR_THRESHOLD:return SET_IR_THRESHOLD;break;

		default:return STOP;break;
	}
}


void returnCmd()
{
	switch (car.carState)
	{
		case FORWARD:
				Serial.write(STATE_FORWARD );
#ifdef DEBUG
				Serial.println("STATE_FORWARD");
#endif
				break;
		case TURN_RIGHT:
				Serial.write(STATE_RIGHT);
#ifdef DEBUG
				Serial.println("STATE_RIGHT");
#endif
				break;
		case TURN_LEFT:
				Serial.write(STATE_LEFT);
#ifdef DEBUG
				Serial.println("STATE_LEFT");
#endif
				break;
		case STOP:
				Serial.write(STATE_STOP);
#ifdef DEBUG
				Serial.println("STATE_STOP");
#endif
				break;
		case ACC_LEFT:
				Serial.write(STATE_LEFT);
#ifdef DEBUG
				Serial.println("STATE_LEFT");
#endif
				break;
		case DEC_LEFT:
				Serial.write(STATE_LEFT);
#ifdef DEBUG
				Serial.println("STATE_LEFT");
#endif
				break;
		case ACC_RIGHT:
				Serial.write(STATE_RIGHT);
#ifdef DEBUG
				Serial.println("STATE_RIGHT");
#endif
				break;
		case DEC_RIGHT:
				Serial.write(STATE_RIGHT);
#ifdef DEBUG
				Serial.println("STATE_RIGHT");
#endif
				break;
		case ACC_FORWARD:
				Serial.write(STATE_FORWARD);
#ifdef DEBUG
				Serial.println("STATE_FORWARD");
#endif
				break;
		case DEC_FORWARD:
				Serial.write(STATE_FORWARD);
#ifdef DEBUG
				Serial.println("STATE_FORWARD");
#endif
				break;
		case BACKWARD:
				Serial.write(STATE_BACKWARD);
#ifdef DEBUG
				Serial.println("STATE_BACKWARD");
#endif
				break;
		case ACC_BACKWARD:
				Serial.write(STATE_BACKWARD);
#ifdef DEBUG
				Serial.println("STATE_BACKWARD");
#endif
				break;
		case DEC_BACKWARD:
				Serial.write(STATE_BACKWARD);
#ifdef DEBUG
				Serial.println("STATE_BACKWARD");
#endif
				break;
		default:break;
	}
}

unsigned char getIR(unsigned char position)
{
	unsigned int irValune=0;
	irValune=analogRead(position);
	if (irValune>blackValue)
	{
#ifdef DEBUGIR
		Serial.print(position);Serial.print(":   ");
		Serial.println(irValune);
#endif
		return HIGH;
	}
    else
	{
#ifdef DEBUGIR
		Serial.print(position);Serial.print(":   ");
		Serial.println(irValune);
#endif
		return LOW;
	}
}

void send40KHz()
{
	for (int i=0;i<24;i++)
	{
		digitalWrite(9,LOW) ;      //L_IR ON
		digitalWrite(10,LOW) ;    //R_IR ON
		delayMicroseconds(8);
		digitalWrite(9,HIGH);
		digitalWrite(10,HIGH);
		delayMicroseconds(8);
	}
}

ISR(PCINT0_vect) //PB0引脚变化中断
{
	irCount++;
}

unsigned char getSwitch()
{
	irCount=0;
	for (unsigned char i=0;i<20;i++)
	{
		send40KHz();
		delayMicroseconds(200);
	}
	if (irCount>20)
	{
		return HIGH;
	}
	else return LOW;
}

void setup(void)
{
	mySerial.begin (9600);
	mp3_set_serial (&mySerial);	                               //set softwareSerial for DFPlayer-mini mp3 module
	carInit(&car);
}

void loop(void)
{
	int a[3];
	ledLoop();
	if(Serial.available()>1)
	{
		for (int i=0;i<2;i++)serialCmd[i]=Serial.read();
		CMD=getCmd(serialCmd);
	}
		switch (CMD)
		{
			case END:
			if (CMD_FLAG)
			{
				mp3_stop ();
				stop();
				turnDownLed();
			}
			case LEDTIME:
			break;
			case LINE:
			if (CMD_FLAG)
			{
				LINE_FLAG=1;
				CMD_FLAG=0;
			}
			case FORWARD :
			if (CMD_FLAG)
			{
				DEC_FLAG=0;ACC_FLAG=0;carSpeed=serialCmd[1];CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("FORWARD");
#endif
			}
			forward(carSpeed,carSpeed);
			car.carState=FORWARD;
			break;
			case TURN_RIGHT :
			if (CMD_FLAG)
			{
				DEC_FLAG=0;ACC_FLAG=0;carSpeed=serialCmd[1];CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("TURN_RIGHT");
#endif
			}
			turnR(carSpeed,carSpeed);
			car.carState=TURN_RIGHT;
			break;
			case TURN_LEFT :
			if (CMD_FLAG)
			{
				DEC_FLAG=0;ACC_FLAG=0;carSpeed=serialCmd[1];CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("TURN_LEFT");
#endif
			}
			turnL(carSpeed,carSpeed);
			car.carState=TURN_LEFT;
			break;
			case STOP :
			if (CMD_FLAG)
			{
				DEC_FLAG=0;ACC_FLAG=0;carSpeed=serialCmd[1];CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("STOP");
#endif
			}
			stop();
			car.carState=STOP;
			break;
			case ACC_LEFT   :
			if (CMD_FLAG)
			{
				DEC_FLAG=0;ACC_FLAG=1;carSpeed=0;accMaxSpeed=serialCmd[1];CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("ACC_LEFT ");
#endif
			}
			turnL(carSpeed,carSpeed);
			car.carState=ACC_LEFT;
			break;
			case DEC_LEFT   :
			if (CMD_FLAG)
			{
				DEC_FLAG=1;ACC_FLAG=0;decMaxSpeed=serialCmd[1];carSpeed=decMaxSpeed;CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("DEC_LEFT");
#endif
			}
			turnL(carSpeed,carSpeed);
			car.carState=DEC_LEFT;
			break;
			case ACC_RIGHT   : if (CMD_FLAG)
			{
				DEC_FLAG=0;ACC_FLAG=1;carSpeed=0;accMaxSpeed=serialCmd[1];CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("ACC_RIGHT");
#endif
			}
			turnR(carSpeed,carSpeed);
			car.carState=ACC_RIGHT;
			break;
			case DEC_RIGHT   :
			if (CMD_FLAG)
			{
				DEC_FLAG=1;ACC_FLAG=0;decMaxSpeed=serialCmd[1];carSpeed=decMaxSpeed;CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("DEC_RIGHT");
#endif
			}
			turnR(carSpeed,carSpeed);
			car.carState=DEC_RIGHT;
			break;
			case ACC_FORWARD :
			if (CMD_FLAG)
			{
				DEC_FLAG=0;ACC_FLAG=1;carSpeed=0;accMaxSpeed=serialCmd[1];CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("ACC_FORWARD");
#endif
			}
			forward (carSpeed,carSpeed);
			car.carState=ACC_FORWARD;
			break;
			case DEC_FORWARD :
			if (CMD_FLAG)
			{
				DEC_FLAG=1;ACC_FLAG=0;decMaxSpeed=serialCmd[1];carSpeed=decMaxSpeed;CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("DEC_FORWARD");
#endif
			}
			forward (carSpeed,carSpeed);
			car.carState=DEC_FORWARD;
			break;
			case BACKWARD       :
			if (CMD_FLAG)
			{
				DEC_FLAG=0;ACC_FLAG=0;carSpeed=serialCmd[1];CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("BACKWARD");
#endif
			}
			backOff(carSpeed,carSpeed);
			car.carState=BACKWARD;
			break;
			case ACC_BACKWARD    :
			if (CMD_FLAG)
			{
				DEC_FLAG=0;ACC_FLAG=1;carSpeed=0;accMaxSpeed=serialCmd[1];CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("ACC_BACKWARD");
#endif
			}
			backOff(carSpeed,carSpeed);
			car.carState=ACC_BACKWARD;
			break;
			case DEC_BACKWARD    :
			if (CMD_FLAG)
			{
				DEC_FLAG=1;ACC_FLAG=0;decMaxSpeed=serialCmd[1];carSpeed=decMaxSpeed;CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("DEC_BACKWARD");
#endif
			}
			backOff(carSpeed,carSpeed);
			car.carState=DEC_BACKWARD;
			break;
			case GET_STATE:
			if (CMD_FLAG)
			{
				returnCmd();CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("GET_STATE");
#endif
			}
			case SET_IR_THRESHOLD:
			if (CMD_FLAG)
			{
				blackValue=serialCmd[1]*10;CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("SET_IR_THRESHOLD");
#endif
			}
			case TIMER_CHANGE:
			if (CMD_FLAG)
			{
				CMD_FLAG=0;
#ifdef DEBUG
				Serial.println("TIMER_CHANGE");
#endif
			}

			default:break;
		}
	if (sensorMetro.check()== 1)
	{
		sensorState.btn=getSwitch();
		sensorState.color[0]=getIR(LLEFT);
		sensorState.color[1]=getIR(LEFT);
		sensorState.color[2]=getIR(MID);
		sensorState.color[3]=getIR(RIGHT);
		sensorState.color[4]=getIR(RRIGHT);
		if (sensorState.btnPre!=sensorState.btn)
		{
			switch (sensorState.btn)
			{
				case HIGH:

#ifdef DEBUG
				Serial.print("CRASHSTATE:  ");
				Serial.println(CRASH_HIGH);
#else
				Serial.write(CRASH_HIGH);
#endif
				break;
				case LOW:
#ifdef DEBUG
				Serial.print("CRASHSTATE:  ");
				Serial.println(CRASH_LOW);
#else
				Serial.write(CRASH_LOW);
#endif
				break;
				default:break;
			}
			sensorState.btnPre=sensorState.btn;
		}
		if (sensorState.colorPre[0]!=sensorState.color[0])
		{
			switch (sensorState.color[0])
			{
				case HIGH:
#ifdef DEBUG
				Serial.print("IR1STATE:  ");
				Serial.println(IR1_HIGH);
#else
				Serial.write(IR1_HIGH);
#endif
				break;
				case LOW:
#ifdef DEBUG
				Serial.print("IR1STATE:  ");
				Serial.println(IR1_LOW);
#else
				Serial.write(IR1_LOW);
#endif
				break;
				default:break;
			}
			sensorState.colorPre[0]=sensorState.color[0];
		}
		if (sensorState.colorPre[1]!=sensorState.color[1])
		{
			switch (sensorState.color[1])
			{
				case HIGH:
#ifdef DEBUG
				Serial.print("IR2STATE:  ");
				Serial.println(IR2_HIGH);
#else
				Serial.write(IR2_HIGH);
#endif
				break;
				case LOW:
#ifdef DEBUG
				Serial.print("IR2STATE:  ");
				Serial.println(IR2_LOW);
#else
				Serial.write(IR2_LOW);
#endif
				break;
				default:break;
			}
			sensorState.colorPre[1]=sensorState.color[1];
		}
		if (sensorState.colorPre[2]!=sensorState.color[2])
		{
			switch (sensorState.color[2])
			{
				case HIGH:
#ifdef DEBUG
				Serial.print("IR3STATE:  ");
				Serial.println(IR3_HIGH);
#else
				Serial.write(IR2_HIGH);
#endif
				break;
				case LOW:
#ifdef DEBUG
				Serial.print("IR3STATE:  ");
				Serial.println(IR3_LOW);
#else
				Serial.write(IR3_LOW);
#endif
				break;
				default:break;
			}
			sensorState.colorPre[2]=sensorState.color[2];
		}
		if (sensorState.colorPre[3]!=sensorState.color[3])
		{
			switch (sensorState.color[3])
			{
				case HIGH:
#ifdef DEBUG
				Serial.print("IR4STATE:  ");
				Serial.println(IR4_HIGH);
#else
				Serial.write(IR4_HIGH);
#endif
				break;
				case LOW:
#ifdef DEBUG
				Serial.print("IR4STATE:  ");
				Serial.println(IR4_LOW);
#else
				Serial.write(IR4_LOW);
#endif
				break;
				default:break;
			}
			sensorState.colorPre[3]=sensorState.color[3];
		}
		if (sensorState.colorPre[4]!=sensorState.color[4])
		{
			switch (sensorState.color[4])
			{
				case HIGH:
#ifdef DEBUG
				Serial.print("IR5STATE:  ");
				Serial.println(IR5_HIGH);
#else
				Serial.write(IR5_HIGH);
#endif
				break;
				case LOW:
#ifdef DEBUG
				Serial.print("IR5STATE:  ");
				Serial.println(IR5_LOW);
#else
				Serial.write(IR5_LOW);
#endif
				break;
				default:break;
			}
			sensorState.colorPre[4]=sensorState.color[4];
		}
	}
	if (timerMetro.check() == 1)
	{
		if (TIMER_FLAG)
		{
			mp3_play_mp3 (timerMusic);
			TIMER_FLAG=0;
		}
	}
	if (accMetro.check() == 1)
	{
		if (ACC_FLAG)
		{
			if (carSpeed<=accMaxSpeed)carSpeed++;
			else ACC_FLAG=0;
		}
	}
	if (decMetro.check() == 1)
	{
		if (DEC_FLAG)
		{
			if (carSpeed>0)carSpeed--;
			else DEC_FLAG=0;
		}
	}
	for (int x;x<LED_NUM;x++)
	{
		if (ledMetro[x].check()==1)
		{
			if (	(ledColorChange[x].redChange>0&&LED_Color[x].red<constColor[ledColor[x]].red)	||
					(ledColorChange[x].redChange<0&&LED_Color[x].red>constColor[ledColor[x]].red)	)
			{
				a[0] = LED_Color[x].red+ledColorChange[x].redChange;
				LED_Color[x].red = constrain(a[0], 0, 25500);
#ifdef DEBUGLEDSTRIP
			Serial.print("LED RED");
			Serial.print(x);
			Serial.print(" ");
			Serial.println(LED_Color[x].red);
#endif
			}
			if (	(ledColorChange[x].greenChange>0&&LED_Color[x].green<constColor[ledColor[x]].green)	||
					(ledColorChange[x].greenChange<0&&LED_Color[x].green>constColor[ledColor[x]].green)	)
			{					
				a[1]= LED_Color[x].green+ledColorChange[x].greenChange;
				LED_Color[x].green = constrain(a[1], 0, 25500);
#ifdef DEBUGLEDSTRIP
			Serial.print("LED green");
			Serial.print(x);
			Serial.print(" ");
			Serial.println(ledColorChange[x].greenChange);
			Serial.println(LED_Color[x].green);
#endif
			}
			if (	(ledColorChange[x].blueChange>0&&LED_Color[x].blue<constColor[ledColor[x]].blue)	||
					(ledColorChange[x].blueChange<0&&LED_Color[x].blue>constColor[ledColor[x]].blue)	)
			{
				a[2] = LED_Color[x].blue+ledColorChange[x].blueChange;
				LED_Color[x].blue = constrain(a[2], 0, 25500);
			}
			
		}
	}
}
