//=================================================
// Portable Traffic Light
// Dev: Vinicius Maia, a.k.a maffeego
// Copyright 
// Part of Code from Fiercy-Rabbit
//=================================================

// Version 1



//PIN
// A0->A5 = 14->19

#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

#define DBUG true 

//Traffic lights PIN
#define PIN_SEMAFORO1 2
#define PIN_SEMAFORO2 3

//Square LED PIN
#define PIN_RED 6
#define PIN_YELLOW 7
#define PIN_GREEN 8

//Number of lights in a Square LED
#define PIXEL_COUNT 16 

//Available lights  
#define LIGHT_RED 0
#define LIGHT_YELLOW 1
#define LIGHT_GREEN 2

#define LIGHT_COUNT 3

//The number of  lights and matching LED array
Adafruit_NeoPixel tab_instance_light [] {
	{PIXEL_COUNT, PIN_RED, NEO_GRB + NEO_KHZ800},
	{PIXEL_COUNT, PIN_YELLOW, NEO_GRB + NEO_KHZ800},
	{PIXEL_COUNT, PIN_GREEN, NEO_GRB + NEO_KHZ800}
};

#define RX_HC12 4
#define TX_HC12 5
SoftwareSerial hc12(RX_HC12, TX_HC12);

//Available match of colors
#define COLOR_WHITE 0
#define COLOR_GREEN 1
#define COLOR_YELLOW 2
#define COLOR_RED 3
#define COLOR_OFF 4

//Colors 
int tab_color[5][3]={
{255,255,255},
{0,255,0},
{255,210,0},
{255,0,0},
{0,0,0}
};

byte action; //Store the action
int counter_piscar; //Stores blink action time
int num_semaforo;
//==================================================================
void setup()
{
	hc12.begin(2400);
	
	#if(DBUG)
	Serial.begin(38400);
	Serial.println("Dentro do setup 1");
	#endif
	
	//Sort traffic light 0 ou 1
	pinMode(PIN_SEMAFORO1,INPUT_PULLUP);
	pinMode(PIN_SEMAFORO2,INPUT_PULLUP);
	test_num_semaforo();
	
	//Starting traffic light
	// FOR to turn on all LED in tab_instance()
	for (int x = 0; x < LIGHT_COUNT; x++)
	{
		tab_instance_light[x].begin();
		tab_instance_light[x].show();
	}
	
	//Function to activate colors
	display_light(LIGHT_RED,COLOR_WHITE);
	display_light(LIGHT_YELLOW,COLOR_WHITE);
	display_light(LIGHT_GREEN,COLOR_WHITE);
	delay(2000);
	display_light_off();
	
	action = 0;	// Blink
	counter_piscar = 0;
	
}
//==================================================================
void loop()
{	
	byte ret;
	ret = test_base(); //Test to sort out which traffic light the message base is for
	
	if (ret != 100)
	{
		switch(ret)
		{
			case 0:
				display_light_off();
				action = 0;
				counter_piscar = 0;
				break;
			case 1:
				action = 1;
				display_light_off();
				display_light(LIGHT_GREEN,COLOR_GREEN);
				break;
			case 2:
				action = 1;
				display_light_off();
				display_light(LIGHT_YELLOW,COLOR_YELLOW);
				break;	
			case 3:
				action = 1;
				display_light_off();
				display_light(LIGHT_RED,COLOR_RED);
				break;
			default:
			break;
				
		}
	}
	
	//if Blink	
	if (action == 0)
	{
	//Turns the light on and off in an orderly fashion
		switch(counter_piscar)
		{
			case 0:
				display_light_off();
				display_light(LIGHT_RED,COLOR_WHITE);
				break;
			case 100:
				display_light_off();
				display_light(LIGHT_YELLOW,COLOR_WHITE);
				break;
			case 200:
				display_light_off();
				display_light(LIGHT_GREEN,COLOR_WHITE);
				break;	
		} 
		//Counts the delay time so does not interrupt the communication system 
		delay(10);
		counter_piscar++;
		if (counter_piscar > 300)
		{
			counter_piscar = 0;
		}
	}
}
//------------------------------------------------------------------
//Test function
//Identify message from base
int test_base()
{
	if (Serial.available())
	{
		byte data = serial_read();
		#if(DBUG)
		Serial.print("Data = ");
		Serial.println(data);
		#endif
		
		byte tmp = data & B00000011;
		Serial.print("tmp = ");
		Serial.println(tmp);
		
		if (num_semaforo == tmp) 
		{
			byte action = data & B00011000;
			action = action >> 3;
			#if(DBUG)
			Serial.print("action = ");
			Serial.println(action);
			#endif
			//Setting bit to indicate that message is Traffic Light -> Base
			data = data | B00000100;
			Serial.print(data);
			return action;
		} 
		else
		{
			#if(DBUG)
			Serial.println("Mensagem não é para mim");
			Serial.println(num_semaforo);
			#endif
			return 100;
		}
		return data;
	}
	else
	{
		return 100;
	}
}
//------------------------------------------------------------------
//Turn Colors on Function
// num_light is equal to which LED Square I want to turn on 0 1 2
// num_color is equal to which Color I want to turn on 0 1 2 3 4
void display_light(int num_light,int num_color)
{
	int red = tab_color[num_color][0];
	int green = tab_color[num_color][1];
	int blue = tab_color[num_color][2];

    for(int i=0; i<PIXEL_COUNT; i++) 
	{ 
	    tab_instance_light[num_light].setPixelColor(i, tab_instance_light[num_light].Color(red,green,blue));                          
    }
    tab_instance_light[num_light].show();
}
//-------------------------------------------------------------------
//Turn off all lights Function
void display_light_off()
{
	display_light(LIGHT_RED,COLOR_OFF);
	display_light(LIGHT_YELLOW,COLOR_OFF);
	display_light(LIGHT_GREEN,COLOR_OFF);
}
//--------------------------------------------------------------------
//Test to identify which Traffic light is 0 1 2 3
void test_num_semaforo()
{
	int val1 = digitalRead(PIN_SEMAFORO1);
	int val2 = digitalRead(PIN_SEMAFORO2);
	num_semaforo = 0; //Número que será retornado para confirmação
	if (val1 == LOW)
	{
		num_semaforo = num_semaforo | B00000001;
	}									
	if (val2 == LOW)
	{
		num_semaforo = num_semaforo | B00000010;
	}
	Serial.print("Sou o Semáforo ");
	Serial.println(num_semaforo);
}
//--------------------------------------------------------------------
//Bit reading Function
//Only for trial period
byte serial_read()
{
	byte ret;
	byte tmp1;
	byte tmp2;
	byte tmp3;
	byte tmp4;
	byte tmp5;
	byte tmp6;
	byte tmp7;
	byte tmp8;
	
	tmp1 = Serial.read()-'0';
	delay(50);	
	tmp2 = Serial.read()-'0';
	delay(50);
	tmp3 = Serial.read()-'0';
	delay(50);
	tmp4 = Serial.read()-'0';
	delay(50);
	tmp5 = Serial.read()-'0';
	delay(50);
	tmp6 = Serial.read()-'0';
	delay(50);
	tmp7 = Serial.read()-'0';
	delay(50);
	tmp8 = Serial.read()-'0';
	delay(50);
	
	ret = (tmp1*128)+(tmp2*64)+(tmp3*32)+(tmp4*16)+(tmp5*8)+(tmp6*4)+(tmp7*2)+tmp8;
	Serial.print("ret = ");
	Serial.println(ret, BIN);
	byte w = 0;
	
	for (int i = 0; i < 10; i++)
	{
		while (Serial.available() > 0)
		{
			char K = Serial.read();
			w++;
			delay(1);
		}
	delay(1);
	}
	return ret;
}