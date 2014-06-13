#include <Metro.h> //Include Metro library
//Standard PWM DC control
#define E1 5     //M1 Speed Control
#define E2 6     //M1 Speed Control
#define M1 4     //M1 Speed Control
#define M2 7     //M1 Speed Control

#define ADVANCE 20    
#define TURNR 21     
#define STOP 22     
#define CARID 0     

#define MID 10
#define LEFT 11
#define RIGHT 12
#define ERR 0

typedef unsigned int uint_16;
typedef unsigned char uint_8;
typedef char int_8;
typedef int int_16;
char serialCmd[10];
Metro sensorMetro = Metro(250); 
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
	car1->carID=CARID;
	car1->carState=0;
	return 0;
}
unsigned char getCmd(char *serialcmd)
{
	if (serialcmd[0]=='w')return ADVANCE;
	else if (serialcmd[0]=='t')return TURNR;
	else if (serialcmd[0]=='s')return STOP;
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
	while(Serial.available()>0)
	{
		int i=0;
		serialCmd[i++]=Serial.read();
	}
        switch (getCmd(serialCmd))
	{
	case ADVANCE : advance (200,200);car.carState=ADVANCE;break;
	case TURNR : turnR(100,100);car.carState=TURNR; break;
	case STOP :stop();car.carState=STOP;break;
	}
	if (sensorMetro.check() == 1)
	{
		Serial.println("IR:");
		Serial.print("LEFT:");
		Serial.println(getIR(LEFT));
		Serial.print("MID:");
		Serial.println(getIR(MID));
		Serial.print("RIGHT:");
		Serial.println(getIR(RIGHT));
		Serial.println("Switch:");
		Serial.print("RIGHT:");
		Serial.println(getSwitch(RIGHT));
		Serial.print("LEFT:");
		Serial.println(getSwitch(LEFT));
	}
	Serial.println();
	Serial.println();
}
