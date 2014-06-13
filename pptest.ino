#include <Metro.h> //Include Metro library
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>


//#define DEBUG
//Standard PWM DC control
#define E1 5     //M1 Speed Control
#define E2 6     //M1 Speed Control
#define M1 4     //M1 Speed Control
#define M2 7     //M1 Speed Control

//小车动作宏定义
#define ADVANCE 20    
#define ADVANCEACC 21    
#define ADVANCEDEC 22    
#define TURNR 23     
#define TURNRACC 24     
#define TURNRDEC 25     
#define TURNL 26     
#define TURNLACC 27     
#define TURNLDEC 28     
#define BACK 29     
#define BACKACC 30     
#define BACKDEC 31     
#define STOP 32     
#define CARID 0 
#define WALK 33

//小车控制宏定义
#define ROBOT_NULL 1000
#define ROBOT_LEFT_TURN 1101
#define ROBOT_LEFT_ACC 1110
#define ROBOT_LEFT_DEC 1111
#define ROBOT_RIGHT_TURN 1201
#define ROBOT_RIGHT_ACC 1210
#define ROBOT_RIGHT_DEC 1211
#define ROBOT_GO_RUN 1301
#define ROBOT_GO_WALK 1302 
#define ROBOT_GO_ACC 1310
#define ROBOT_GO_DEC 1311
#define ROBOT_BACK_RUN 1401
#define ROBOT_BACK_ACC 1410
#define ROBOT_BACK_DEC 1411
#define ROBOT_LINE_START 1501
#define ROBOT_LINE_STOP 1500

//MP3播放：(以2开头的4为数，后面三位表示曲目)
#define SOUND_STOP 2000
#define SOUND_001 2001
#define SOUND_002 2002
#define SOUND_CHANGE 2100
//MP3音量:(以3开头的4为数，后面三位表示音量)
//vol 3100
//LED灯：
#define ROBOT_LED1_OPEN 4011
#define ROBOT_LED1_CLOSE 4010
#define ROBOT_LED2_OPEN 4021
#define ROBOT_LED2_CLOSE 4020
//小车定时：(以5开头的4为数，后面三位表示分钟)
#define ROBOT_TIMER_SET 5060
// 定时器报警音效：(以6开头的4为数，后面三位表示曲目)
#define ROBOT_TIMER_SOUND 6002
#define TIMER_SOUND_CHANGE 6000
// 获取小车运动状态：
#define ROBOT_GET_STATE 7000
// 获取电池电量：
#define ROBOT_GET_ENERGY 8000



//传感器位置宏定义
#define MID 10
#define LEFT 11
#define RIGHT 12
#define ERR 0

//小车运动参数宏定义
#define SPEED_MAX 200
#define RUN_SPEED_MAX 200 
#define BACK_SPEED_MAX 200 
#define ACC_TIME 2000 
#define DEC_TIME 2000 
#define TURNR_SPEED_MAX 50
#define TURNL_SPEED_MAX 50 
#define WALK_SPEED_MAX 50

//小车返回宏定义

#define ROBOT_BTN1_HIGH 1
#define ROBOT_BTN1_LOW 2
#define ROBOT_BTN2_HIGH 3
#define ROBOT_BTN2_LOW 4
#define ROBOT_COLOR1_HIGH 5
#define ROBOT_COLOR1_LOW 6
#define ROBOT_COLOR2_HIGH 7
#define ROBOT_COLOR2_LOW 8
#define ROBOT_COLOR3_HIGH 9
#define ROBOT_COLOR3_LOW 10
#define ROBOT_STATE_RUN 11
#define ROBOT_STATE_BACK 12
#define ROBOT_STATE_LEFT 13
#define ROBOT_STATE_RIGHT 14
#define ROBOT_STATE_STOP 15
#define ROBOT_STATE_WALK 16

//返回宏定义
//#define  0
//#define ERR 0
//#define ERR 0
//#define ERR 0
//#define ERR 0

//小车控制标志位
unsigned char DEC_FLAG = 0;
unsigned char ACC_FLAG = 0;
unsigned char TIMER_FLAG = 0;

typedef unsigned int uint_16;
typedef unsigned char uint_8;
typedef char int_8;
typedef int int_16;
char serialCmd[10];
Metro TimerMetro = Metro(250); 
Metro ACCMetro =Metro(ACC_TIME/SPEED_MAX);
Metro DECMetro =Metro(DEC_TIME/SPEED_MAX);
Metro sensorMetro = Metro(20); 
unsigned char CMD=0;
unsigned char carSpeed=0;

SoftwareSerial mySerial(10, 11); // RX, TX

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
    unsigned char BTN_pre[2];
	unsigned char BTN[2];
	unsigned char COLOR[3];
	unsigned char COLOR_pre[3];
};
struct sensor sensorState;

void stop(void)                    
{
	advance(0,0);
}   

void advance(unsigned char a,unsigned char b)         
{
	analogWrite (E1,a);      //PWM Speed Control
	digitalWrite(M1,HIGH);    
	analogWrite (E2,b);    
	digitalWrite(M2,LOW);
	
}  

void backOff (unsigned char a,unsigned char b)          
{
	analogWrite (E1,a);
	digitalWrite(M1,LOW);   
	analogWrite (E2,b);    
	digitalWrite(M2,HIGH);
	
}

void turnL (unsigned char a,unsigned char b)            
{
	analogWrite (E1,a);
	digitalWrite(M1,LOW);    
	analogWrite (E2,b);    
	digitalWrite(M2,LOW);
}


void turnR (unsigned char a,unsigned char b)             
{
	analogWrite (E1,a);
	digitalWrite(M1,HIGH);    
	analogWrite (E2,b);    
	digitalWrite(M2,HIGH);
}

//MP3播放函数

void changeMusic(int i)
{
	mp3_play_mp3 (i);
#ifdef DEBUG
	Serial.write(i);
#endif
}

void changeMusicValue(int i)
{
    mp3_set_volume (i);
}

//Timer设置函数
void timerSet(int minutes)
{

}
void timerChangeMusic(int music)
{
}


int carInit(struct carCon *car1)
{
	for (uint_8 i;i<7;i++)
	{
		pinMode(i,OUTPUT);
	}
#ifdef DEBUG
	Serial.begin(115200);      //Set Baud Rate
	Serial.println("start");
#endif
	Serial1.begin(115200);
	car1->carID=CARID;
	car1->carState=0;
	car1->speed=0;
	return 0;
}

unsigned char getCmd(char *serialcmd)
{
	int buttonState=0;
	int cmdNum=0;
#ifdef DEBUG
	Serial.write(serialcmd[0]);
	Serial.write(serialcmd[1]);
#endif
//	cmdNum=((unsigned int)serialcmd[1]<<8)+(unsigned int)serialcmd[0];
	cmdNum = *(unsigned int*)serialcmd;
	if ((cmdNum/1000)==1)
	{
#ifdef DEBUG
	Serial.print(cmdNum);
#endif	
		switch (cmdNum)
		{
			case ROBOT_NULL:return STOP;break;
			case ROBOT_LEFT_TURN:return TURNL;break;
			case ROBOT_GO_WALK:return WALK;break;
			case ROBOT_LEFT_ACC:return TURNLACC;break;
			case ROBOT_LEFT_DEC:return TURNLDEC;break;
			case ROBOT_RIGHT_TURN:return TURNR;break;
			case ROBOT_RIGHT_ACC:return TURNRACC;break;
			case ROBOT_RIGHT_DEC:return TURNRDEC;break;
			case ROBOT_GO_RUN:return ADVANCE;break;
			case ROBOT_GO_ACC:return ADVANCEACC;break;
			case ROBOT_GO_DEC:return ADVANCEDEC;break;
			case ROBOT_BACK_RUN:return BACK;break;
			case ROBOT_BACK_ACC:return BACKACC;break;
			case ROBOT_BACK_DEC:return BACKDEC;break;
			default:break;
		}
	}
	else if ((cmdNum/1000)==2)
	{
	    if(cmdNum==2000)mp3_stop ();
		else changeMusic(cmdNum-2000);
		return SOUND_CHANGE;
	}
	else if ((cmdNum/1000)==3)
	{
	    changeMusicValue(cmdNum-3000);
		return SOUND_CHANGE;
	}
	else if ((cmdNum/1000)==4)
	{
		switch (cmdNum)
		{
			case ROBOT_LED1_OPEN:break;
			case ROBOT_LED1_CLOSE:break;
			case ROBOT_LED2_CLOSE:break;
			case ROBOT_LED2_OPEN:break;
			default:break;
		}
	}
	else if ((cmdNum/1000)==5)
	{
		timerSet(cmdNum-5000);
		return ROBOT_TIMER_SET;
	}
	else if ((cmdNum/1000)==6)
	{
		timerChangeMusic(cmdNum-6000);
		return TIMER_SOUND_CHANGE;
	}
	else if ((cmdNum/1000)==7)
	{
		return ROBOT_GET_STATE;
	}
	else if ((cmdNum/1000)==8)
	{
		returnCmd();
		return ROBOT_GET_ENERGY;
	}
	else return STOP;
}

void returnCmd()
{
	switch (car.carState)
	{
		case ADVANCE:
				Serial1.write(ROBOT_STATE_RUN);
				break;
		case TURNR:
				Serial1.write(ROBOT_STATE_RIGHT);
				break;
		case TURNL:
				Serial1.write(ROBOT_STATE_LEFT);
				break;
		case STOP:
				Serial1.write(ROBOT_STATE_STOP);
				break;
		case TURNLACC:
				Serial1.write(ROBOT_STATE_LEFT);
				break;
		case TURNLDEC:
				Serial1.write(ROBOT_STATE_LEFT);
				break;
		case TURNRACC:
				Serial1.write(ROBOT_STATE_RIGHT);
				break;
		case TURNRDEC:
				Serial1.write(ROBOT_STATE_RIGHT);
				break;
		case ADVANCEACC:
				Serial1.write(ROBOT_STATE_RUN);
				break;
		case ADVANCEDEC:
				Serial1.write(ROBOT_STATE_RUN);
				break;
		case BACK:
				Serial1.write(ROBOT_STATE_BACK);
				break;
		case BACKACC:
				Serial1.write(ROBOT_STATE_BACK);
				break;
		case BACKDEC:
				Serial1.write(ROBOT_STATE_BACK);
				break;
		case WALK:
				Serial1.write(ROBOT_STATE_WALK);
				break;
		default:break;
	}
}

unsigned char getIR(unsigned char position)
{
	if (position==MID)
	{
		pinMode(3,INPUT);
		return digitalRead(3);
	}
	else if (position==LEFT)
	{
		pinMode(A1,INPUT);
		return digitalRead(A1);
	}
	else if (position==RIGHT)
	{
		pinMode(8,INPUT);
		return digitalRead(8);
	}
	else return ERR;
}

unsigned char getSwitch(unsigned char position)
{
	if (position==LEFT)
	{
		pinMode(A0,INPUT);
		return digitalRead(A0);
	}
	else if(position==RIGHT)
	{
		pinMode(2,INPUT);
		return digitalRead(2);
	}
	else return ERR;
}

void setup(void) 
{ 
	mySerial.begin (9600);
	mp3_set_serial (&mySerial);	//set softwareSerial for DFPlayer-mini mp3 module 
	carInit(&car);
}

void loop(void) 
{
#ifdef DEBUG
	if(Serial.available()>1)
	{
		for (int i=0;i<2;i++)serialCmd[i]=Serial.read();
		CMD=getCmd(serialCmd);
	}
#else
	if(Serial1.available()>1)
	{
		for (int i=0;i<2;i++)serialCmd[i]=Serial1.read();
		CMD=getCmd(serialCmd);
	}
#endif
	switch (CMD)
	{
		case ADVANCE    : DEC_FLAG=0;ACC_FLAG=0;
				  advance(RUN_SPEED_MAX,RUN_SPEED_MAX);
				  car.carState=ADVANCE;
				  break;
		case TURNR      : DEC_FLAG=0;ACC_FLAG=0;
				  turnR(TURNR_SPEED_MAX,TURNR_SPEED_MAX);
				  car.carState=TURNR; 
				  break;
		case TURNL      : DEC_FLAG=0;ACC_FLAG=0;
				  turnL(TURNL_SPEED_MAX,TURNL_SPEED_MAX);
				  car.carState=TURNL;
				  break;
		case STOP       : DEC_FLAG=0;ACC_FLAG=0;
				  stop();
				  car.carState=STOP;
				  break;
		case TURNLACC   : if (car.carState!=TURNLACC){DEC_FLAG=0;ACC_FLAG=1;carSpeed=0;}
					  turnL(carSpeed,carSpeed);
				  car.carState=TURNLACC;
				  break;
		case TURNLDEC   : if (car.carState!=TURNLDEC){DEC_FLAG=1;ACC_FLAG=0;carSpeed=TURNL_SPEED_MAX;}
					  turnL(carSpeed,carSpeed);
				  car.carState=TURNLDEC;
				  break;
		case TURNRACC   : if (car.carState!=TURNRACC){DEC_FLAG=0;ACC_FLAG=1;carSpeed=0;}
					  turnR(carSpeed,carSpeed);
				  car.carState=TURNRACC;
				  break;
		case TURNRDEC   : if (car.carState!=TURNRDEC){DEC_FLAG=1;ACC_FLAG=0;carSpeed=TURNR_SPEED_MAX;}
					  turnR(carSpeed,carSpeed);
				  car.carState=TURNRDEC;
				  break;
		case ADVANCEACC : if (car.carState!=ADVANCEACC){DEC_FLAG=0;ACC_FLAG=1;carSpeed=0;}
					  advance (carSpeed,carSpeed);
				  car.carState=ADVANCEACC;
				  break;
		case ADVANCEDEC : if (car.carState!=ADVANCEDEC){DEC_FLAG=1;ACC_FLAG=0;carSpeed=RUN_SPEED_MAX;}
					  advance (carSpeed,carSpeed);
				  car.carState=ADVANCEDEC;
				  break;
		case BACK       : DEC_FLAG=0;ACC_FLAG=0;
				  backOff(BACK_SPEED_MAX,BACK_SPEED_MAX);
				  car.carState=BACK;
				  break;
		case BACKACC    : if (car.carState!=BACKACC){DEC_FLAG=0;ACC_FLAG=1;carSpeed=0;}
					  backOff(carSpeed,carSpeed);
				  car.carState=BACKACC;
				  break;
		case BACKDEC    : if (car.carState!=BACKDEC){DEC_FLAG=1;ACC_FLAG=0;carSpeed=BACK_SPEED_MAX;}
					  backOff(carSpeed,carSpeed);
				  car.carState=BACKDEC;
		case WALK		:DEC_FLAG=0;ACC_FLAG=0;
				  advance(WALK_SPEED_MAX,WALK_SPEED_MAX);
				  car.carState=WALK;  
				  break;
	}
	if (sensorMetro.check()== 1)
	{
		sensorState.BTN[0]=getSwitch(LEFT);
		sensorState.BTN[1]=getSwitch(RIGHT);
		sensorState.COLOR[0]=getIR(LEFT);
		sensorState.COLOR[1]=getIR(MID);
		sensorState.COLOR[2]=getIR(RIGHT);
		if (sensorState.BTN_pre[0]!=sensorState.BTN[0])
		{
			switch (sensorState.BTN[0])
			{
				case HIGH:
				Serial1.write(ROBOT_BTN1_HIGH);
#ifdef DEBUG
				Serial.print(ROBOT_BTN1_HIGH);
#endif	
				break;
				case LOW:
#ifdef DEBUG
				Serial.println(ROBOT_BTN1_LOW);
#endif	
				Serial1.write(ROBOT_BTN1_LOW);
				break;
				default:break;
			}
			sensorState.BTN_pre[0]=sensorState.BTN[0];
		}
		if (sensorState.BTN_pre[1]!=sensorState.BTN[1])
		{
			switch (sensorState.BTN[1])
			{
				case HIGH:
				Serial1.write(ROBOT_BTN2_HIGH);
#ifdef DEBUG
				Serial.println(ROBOT_BTN2_HIGH);
#endif	
				break;
				case LOW:
				Serial1.write(ROBOT_BTN2_LOW);
#ifdef DEBUG
				Serial.println(ROBOT_BTN2_LOW);
#endif	
				break;
				default:break;
			}
			sensorState.BTN_pre[1]=sensorState.BTN[1];
		}
		if (sensorState.COLOR_pre[0]!=sensorState.COLOR[0])
		{
			switch (sensorState.COLOR[0])
			{
				case HIGH:
				Serial1.write(ROBOT_COLOR1_HIGH);
#ifdef DEBUG
				Serial.println(ROBOT_COLOR1_HIGH);
#endif	
				break;
				case LOW:
				Serial1.write(ROBOT_COLOR1_LOW);
#ifdef DEBUG
				Serial.println(ROBOT_COLOR1_LOW);
#endif	
				break;
				default:break;
			}
			sensorState.COLOR_pre[0]=sensorState.COLOR[0];
		}
		if (sensorState.COLOR_pre[1]!=sensorState.COLOR[1])
		{
			switch (sensorState.COLOR[1])
			{
				case HIGH:
				Serial1.write(ROBOT_COLOR2_HIGH);
#ifdef DEBUG
				Serial.println(ROBOT_COLOR2_HIGH);
#endif	
				break;
				case LOW:
				Serial1.write(ROBOT_COLOR2_LOW);
#ifdef DEBUG
				Serial.println(ROBOT_COLOR2_LOW);
#endif	
				break;
				default:break;
			}
			sensorState.COLOR_pre[1]=sensorState.COLOR[1];
		}
		if (sensorState.COLOR_pre[2]!=sensorState.COLOR[2])
		{
			switch (sensorState.COLOR[2])
			{
				case HIGH:
				Serial1.write(ROBOT_COLOR3_HIGH);
#ifdef DEBUG
				Serial.println(ROBOT_COLOR3_HIGH);
#endif	
				break;
				case LOW:
				Serial1.write(ROBOT_COLOR3_LOW);
#ifdef DEBUG
				Serial.println(ROBOT_COLOR3_LOW);
#endif	
				break;
				default:break;
			}
			sensorState.COLOR_pre[2]=sensorState.COLOR[2];
		}
	}
	if (TimerMetro.check() == 1)
	{
		if (TIMER_FLAG)
		{
		}
	}
	if (ACCMetro.check() == 1)
	{
		if (ACC_FLAG)
		{
			if (carSpeed<=200)carSpeed++;
		}
	}
	if (DECMetro.check() == 1)
	{
		if (DEC_FLAG)
		{
			if (carSpeed>0)carSpeed--;
		}
	}
}
