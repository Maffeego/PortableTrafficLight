//=================================================
// Base - Portable Traffic Light
// Dev: Vinicius Maia, a.k.a maffeego
// Copyright 
// Part of Code from Fiercy-Rabbit - 1990-2018
//=================================================

// Version 1

#include <Adafruit_NeoPixel.h>

#define DBUG true

//Max number of traffic lights
#define NUM_SE_MAX 4

//Switch to choose action Blink or init the Traffic Light mode
#define PIN_INTER_ORDEM 3

//Switch to set minimum number of traffic light 
#define PIN_INTERRUPTOR1 4
#define PIN_INTERRUPTOR2 5

//Number of lights in a Square LED
#define PIXEL_COUNT 4

//LED PIN
#define PIN_LED 2

//Buzzer Piezo PIN
#define PIN_BUZZER 6

//The number of  lights and matching LED array
Adafruit_NeoPixel led_semaforo (PIXEL_COUNT, PIN_LED, NEO_RGB + NEO_KHZ800);

//Available match of colors
#define COLOR_WHITE 0
#define COLOR_GREEN 1
#define COLOR_YELLOW 2
#define COLOR_RED 3
#define COLOR_BLUE 4
#define COLOR_VIOLET 5
#define COLOR_OFF 6

//Waiting time between message and traffic light response
#define MAX_ESPERA 5000

//RGB colors
int tab_color[7][3]={
{255,255,255},
{0,255,0},
{255,210,0},
{255,0,0},
{0,0,255},
{153,0,153},
{0,0,0},
};


int tab_cor[] = {COLOR_GREEN,COLOR_YELLOW,COLOR_RED};
//Waiting time for colors to change
int tab_tempo[] = {500,100,600};

//traffic light stats
int tab_se[] = {COLOR_BLUE,COLOR_BLUE,COLOR_BLUE,COLOR_BLUE};

int num_se; //Traffic light number
int num_color; //Color number

boolean flag_update;
boolean flag_piscar;
int tempo_espera;
int index_cor;

//==================================================================
void setup()
{
	int num_se_ativos;
	int status_se;
	int i;
	int wait;
	int val_int;
	
	#if (DBUG)
	Serial.begin(38400);
	Serial.println("Dentro do setup Base - 2");
	#endif
	
	pinMode(PIN_INTER_ORDEM,INPUT_PULLUP);
	pinMode(PIN_INTERRUPTOR1,INPUT_PULLUP);
	pinMode(PIN_INTERRUPTOR2,INPUT_PULLUP);
	pinMode(PIN_BUZZER,OUTPUT);
	
	led_semaforo.begin();
	led_semaforo.show();
		
	//init base
	flag_update = true;	//Update stats LED
	
	//Traffic light num
	num_se = test_interruptor_se(); //Traffic light num in the switch
	num_se_ativos = 0; //Number of active traffic light
	
	while(num_se_ativos != num_se)
	{
		display_light_led();	//Change stats LED
	
		//Loop to test the stats of each traffic light
		for(i = 0; i < NUM_SE_MAX; i++)
		{
			status_se = tab_se[i];
			
			if (status_se == COLOR_BLUE)
			{
				#if(DBUG)
				Serial.print("Enviar msg para semaforo ");
				Serial.println(i);
				#endif
				byte msg = B00000000;
				msg = msg | (byte)i;
				Serial.print(msg,BIN);
				Serial.println();	
			}
		}
		//Waiting answer
		wait = 0; 
		
		while(wait<1000 & num_se_ativos != num_se)
		{
			
			if(Serial.available())
			{	
				byte tmp = serial_read();
				tmp = tmp & B00000011;
			
				if (tab_se[(int)tmp]== COLOR_BLUE)
				{
					num_se_ativos++;
					tab_se[(int)tmp]=COLOR_WHITE;
				}
			}
			delay(10);
			wait++;
		}
	}
	flag_piscar = true;
	
	for(i = 0; i < NUM_SE_MAX; i++)
	{
		status_se = tab_se[i];
		if (status_se == COLOR_BLUE)
		{
			tab_se[i] = COLOR_OFF; 
		}
	}
	display_light_led();
	

	
}
//==================================================================
// Main Loop
// 

void loop()
{	
int val_int;
int tmp_cor;
int status_se;
int x;

	if (flag_piscar == false)
	{
		//init Loop according to tempo_espera and index_cor
		
		if (tempo_espera == 0)
		{
			Serial.println("Aqui 1");
			Serial.print("Num_se: ");
			Serial.println(num_se);
			tempo_espera = tab_tempo[index_cor];
			tmp_cor = tab_cor[index_cor];
			status_se = tab_se[num_se];
			
			while(status_se == COLOR_OFF)
			{	
		
				num_se++;
				if(num_se >= NUM_SE_MAX)
				{
					num_se = 0;
				}
				status_se = tab_se[num_se];
			}
			Serial.print("Num_se depois de tratado: ");
			Serial.println(num_se);
			
			display_light_semaforo(num_se,tmp_cor);
			
			index_cor++;
			
			if (index_cor > 2)
			{
				
				index_cor = 0;
				num_se++;
				if(num_se >= NUM_SE_MAX)
				{
					num_se = 0;
				}
			}
		}	
		tempo_espera -= 1;	
	}
	
	delay(10);

	//Checking Switch
	val_int = digitalRead(PIN_INTER_ORDEM);	
	
	if (val_int == HIGH & flag_piscar == false)
	{
		//All traffic Lights blinking
		for(x = 0; x < NUM_SE_MAX; x++)
		{
			status_se = tab_se[x];
			if (status_se != COLOR_OFF)
			{
				display_light_semaforo(x,COLOR_WHITE);
			}
		}
		
		display_light_led();
		flag_piscar = true;
	}	
	
	//If all the Lights are blinking then all the lights turn red
	
	if (val_int == LOW & flag_piscar == true)
	{
		Serial.println("Aqui 0");
		for(x = 0; x < NUM_SE_MAX; x++)
		{
			status_se = tab_se[x];
			if (status_se != COLOR_OFF)
			{
				display_light_semaforo(x,COLOR_RED);
			}
		}
		display_light_led(); //update base LED
		Serial.println("Depois do Aqui 0");
		delay(10000); //Wait for all cars to leave
		num_se = 0;
		index_cor = 0;
		tempo_espera = 0;
		flag_piscar = false;
	}	

}
//------------------------------------------------------------------
// Function to turn on all the LED lights
void display_light_led()
{
	int red;
	int green;
	int blue;
	int status_se;

	for(int i = 0; i < NUM_SE_MAX; i++)
	{
		status_se = tab_se[i];
		red = tab_color[status_se][0];
		green = tab_color[status_se][1];
		blue = tab_color[status_se][2];
		led_semaforo.setPixelColor(i, led_semaforo.Color(red,green,blue));
		/*
		Serial.print("Semaforo: ");
		Serial.println(i);
		Serial.print("Red: ");
		Serial.println(red);
		Serial.print("Green: ");
		Serial.println(green);
		Serial.print("Blue: ");
		Serial.println(blue);
		Serial.println();
		*/
	}                          
	led_semaforo.show();
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
//--------------------------------------------------------------------
//Checking the switch to know how many traffic lights

int test_interruptor_se()
{
	int val_int1 = digitalRead(PIN_INTERRUPTOR1); //Orange wire
	int val_int2 = digitalRead(PIN_INTERRUPTOR2); //Blue wire
	
	
	if(val_int1 == LOW & val_int2 == HIGH)
	{
		return 2;
	}
	else if(val_int1 == HIGH & val_int2 == HIGH)
	{
		return 3;
	}
	else if(val_int1 == HIGH & val_int2 == LOW)
	{
		return 4;
	}
}
//-----------------------------------------------------------------------
// Function to send a message to the traffic lights and receive the answer

void display_light_semaforo(int num_se,int cor)
{	
	Serial.print("display_light_semaforo ");
	Serial.print(num_se);
	Serial.print("-");
	Serial.println(cor);
	
	byte msg = B00000000; //First message
	byte msg_se_base = B00000100; //Expected message when Traffic Light -> Base
	byte msg_se; //Traffic Light message
	
	int status_se;
	int tmp_cor;
	 
	msg = msg | num_se; //Merge initial message with traffic light(SE) number
	tmp_cor = cor << 3; //Sort the color in the bit message
	msg = msg | tmp_cor; //Initial message with color

	msg_se_base = msg_se_base | msg; //Testing expected message with received message
	
	Serial.println("Mensagem esperada");
	Serial.println(msg_se_base,BIN);
	
	
	
	int counter = 0;
	//Counter to wait traffic light message
	while(counter != MAX_ESPERA)
		{
		counter++;
		//if received 
		if (Serial.available())
		{
			msg_se = serial_read();

			//If the message is right
			if(msg_se_base == msg_se)
			{
				tab_se[num_se] = cor; //update traffic Light stats
				display_light_led(); //update all the base LED
				return;
			} 
		}
		delay(10);	
	}	
	
	//ALL RED
	for(int i = 0; i < NUM_SE_MAX; i++) 
	{	
		status_se = tab_se[i];
		//Check if traffic light its on
		if (status_se != COLOR_OFF)
		{
			//Send a message to traffic light
			#if(DBUG)
			Serial.print("Enviar msg, SEMÁFORO VERMELHO ");
			Serial.println(i);
			#endif
			byte msg = B00011000; //Action RED
			msg = msg | (byte)i; 
			Serial.print(msg,BIN);
			Serial.println();
			tab_se[i] = COLOR_VIOLET;	
		}
	}
	display_light_led(); //Update stats LED
	Serial.print("Ficamos aqui");
	int notes[10] = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440};
	int note_duration[10] = {200, 200, 100, 150 ,100 ,300 ,150 ,170 , 230 ,133}; 
	
	while(1)
	{
		for(int i = 0; i < 10; i++)
		{
    		tone(PIN_BUZZER, notes[i], note_duration[i]); 
    		delay(500);
    		noTone(PIN_BUZZER);
    	}
  	}
}

//----------------------------------------------------------------------

