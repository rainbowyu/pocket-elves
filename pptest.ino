#include <Metro.h> //Include Metro library
//Standard PWM DC control
#define E1 5     //M1 Speed Control
#define E2 6     //M1 Speed Control
#define M1 4     //M1 Speed Control
#define M2 7     //M1 Speed Control

//小车动作宏定义
#define ADVANCE 20    
#define TURNR 21     
#define STOP 22     
#define TURNL 23     
#define CARID 0 

//发送命令宏定义
#define ROBOT_NULL 1000
#define ROBOT_LEFT_PRESS 1101
#define ROBOT_LEFT_RELEASE 1100
#define ROBOT_LEFT_ACC 1110
#define ROBOT_LEFT_DEC 1111
#define ROBOT_RIGHT_PRESS 1201
#define ROBOT_RIGHT_RELEASE 1200
#define ROBOT_RIGHT_ACC 1210
#define ROBOT_RIGHT_DEC 1211
#define ROBOT_GO_PRESS 1301
#define ROBOT_GO_RELEASE 1300
#define ROBOT_GO_ACC 1310
#define ROBOT_GO_DEC 1311
#define ROBOT_BACK_PRESS 1401
#define ROBOT_BACK_RELEASE 1400
#define ROBOT_BACK_ACC 1410
#define ROBOT_BACK_DEC 1411
#define ROBOT_LINE_START 1501
#define ROBOT_LINE_STOP 1500 

//传感器位置宏定义
#define MID 10
#define LEFT 11
#define RIGHT 12
#define ERR 0

//返回宏定义
//#define  0
//#define ERR 0
//#define ERR 0
//#define ERR 0
//#define ERR 0

typedef unsigned int uint_16;
typedef unsigned char uint_8;
typedef char int_8;
typedef int int_16;
char serialCmd[10];
Metro sensorMetro = Metro(250); 
unsigned char CMD=0;
struct carCon
{
	uint_8 carID;
	uint_8 carState;
} ;
struct carCon car;
void stop(void)                    //Stop
{
	digitalWrite(E1,LOW);   
	digitalWrite(E2,LOW);      
}   
void advance(unsigned char a,unsigned char b)          //Move forward
{
	analogWrite (E1,a);      //PWM Speed Control
	digitalWrite(M1,HIGH);    
	analogWrite (E2,b);    
	digitalWrite(M2,LOW);
}  
void backOff (unsigned char a,unsigned char b)          //Move backward
{
	analogWrite (E1,a);
	digitalWrite(M1,LOW);   
	analogWrite (E2,b);    
	digitalWrite(M2,HIGH);
}
void turnL (unsigned char a,unsigned char b)             //Turn Left
{
	analogWrite (E1,a);
	digitalWrite(M1,LOW);    
	analogWrite (E2,b);    
	digitalWrite(M2,LOW);
}
void turnR (unsigned char a,unsigned char b)             //Turn Right
{
	analogWrite (E1,a);
	digitalWrite(M1,HIGH);    
	analogWrite (E2,b);    
	digitalWrite(M2,HIGH);
}
int carInit(struct carCon *car1)
{
	for (uint_8 i;i<7;i++)
	{
		pinMode(i,OUTPUT);
	}
	Serial.begin(115200);      //Set Baud Rate
	Serial.println("start");
	Serial1.begin(115200);
	car1->carID=CARID;
	car1->carState=0;
	return 0;
}
unsigned char getCmd(char *serialcmd)
{
	int buttonState=0;
	int cmdNum=0;
	Serial.write(serialcmd[0]);
	Serial.write(serialcmd[1]);
	//cmdNum=serial
	if (serialcmd[0]=='l'){if(serialcmd[1]=='d')buttonState=1;}
	else if (serialcmd[0]=='l'){if(serialcmd[1]=='u')buttonState=2;}
	else if (serialcmd[0]=='r'){if(serialcmd[1]=='d')buttonState=3;}
	else if (serialcmd[0]=='r'){if(serialcmd[1]=='u')buttonState=4;}
	else if (serialcmd[0]=='g'){if(serialcmd[1]=='d')buttonState=5;}
	else if (serialcmd[0]=='g'){if(serialcmd[1]=='u')buttonState=6;}
	switch (buttonState)
	{
		case 1:return TURNL;break;
		case 2:return STOP;break;
		case 3:return TURNR;break;
		case 4:return STOP;break;
		case 5:return ADVANCE;break;
		case 6:return STOP;break;
		default:break;
	}
	return ADVANCE;
}
unsigned char returnCmd(unsigned char cmdReturn)
{
	switch (cmdReturn)
	{
		case 1:break;
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
	carInit(&car);
} 
void loop(void) 
{
	if(Serial1.available()>1)
	{
		for (int i=0;i<2;i++)serialCmd[i]=Serial1.read();
		CMD=getCmd(serialCmd);
	}
        switch (CMD)
	{
	case ADVANCE : advance (200,200);car.carState=ADVANCE;break;
	case TURNR : turnR(100,100);car.carState=TURNR; break;
	case TURNL : turnL(100,100);car.carState=TURNL; break;
	case STOP :stop();car.carState=STOP;break;
	}
	//if (sensorMetro.check() == 1)
	//{
	//Serial.println("IR:");
	//Serial.print("LEFT:");
	//Serial.println(getIR(LEFT));
	//Serial.print("MID:");
	//Serial.println(getIR(MID));
	//Serial.print("RIGHT:");
	//Serial.println(getIR(RIGHT));
	//Serial.println("Switch:");
	//Serial.print("RIGHT:");
	//Serial.println(getSwitch(RIGHT));
	//Serial.print("LEFT:");
	//Serial.println(getSwitch(LEFT));
	//Serial.println();
	//Serial.println();
	//}
}
