#include <U8glib.h>
#include <SPI.h>

//Collegamenti (Arduino Uno):
//  - SS:    10
//  - MOSI:  11
//  - MISO:  12
//  - SCK:   13
//  - CE:    9

//Creatori:
// - Andrea Amadei
// - Matteo Rizzi
// - Andrea Signorelli
// - Marco Termini
// - Federico Dalmagioni

//Pin CE
#define CE 9

//Canali dati
#define CANALI 64
int canale[CANALI];

//Registri scheda nRF24L01P
#define _NRF24_CONFIG      0x00
#define _NRF24_EN_AA       0x01
#define _NRF24_RF_CH       0x05
#define _NRF24_RF_SETUP    0x06
#define _NRF24_RPD         0x09

//Scala di misura
#define SCALA 1.2

//Display
#define INTEST "C:12345678901234"
#define CAN "Canale: "

//Spettro di frequenze
int freq[14] = {6, 8, 11, 13, 16, 19, 21, 23, 26, 28, 31, 34, 37, 41};
int sto[16]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};       //Vettore potenza

float old = 0.0;

//Setup display OLED
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);

//INIZIALIZZAZIONE----------------------------------------------------------------------------------
//Inizializzazione registri scheda
//r = numero di registro
byte getRegister(byte r)
{
	byte c;

	PORTB &=~_BV(2);
	c = SPI.transfer(r&0x1F);
	c = SPI.transfer(0);  
	PORTB |= _BV(2);

	return(c);
}

//Setup registri scheda
//r = numero di registro
//v = valore del registro
void setRegister(byte r, byte v)
{
	PORTB &=~_BV(2);
	SPI.transfer((r&0x1F)|0x20);
	SPI.transfer(v);
	PORTB |= _BV(2);
}

//Reset vettore canale
void resetCanale(void)
{
	for( int i=0 ; i<CANALI ; i++)
		canale[i] = 0;
}

//ATTIVAZIONE SCHEDA--------------------------------------------------------------------------------
//Attivazione scheda
void powerUp(void)
{
	setRegister(_NRF24_CONFIG,getRegister(_NRF24_CONFIG)|0x02);
	delayMicroseconds(130);
}

//Disattivazione scheda
void powerDown(void)
{
	setRegister(_NRF24_CONFIG,getRegister(_NRF24_CONFIG)&~0x02);
}



//RX-MODE-------------------------------------------------------------------------------------------
//Abilita ricezione
void enable(void)
{
	PORTB |= _BV(1);
}

//Disabilita ricezione
void disable(void)
{
	PORTB &=~_BV(1);
}

//Setup ricezione
void setRX(void)
{
	setRegister(_NRF24_CONFIG,getRegister(_NRF24_CONFIG)|0x01);
	enable();
	delayMicroseconds(100);//Valore di default: 130
}



//SCANSIONE CANALI----------------------------------------------------------------------------------
void scanChannels(void)
{
	disable();
	
	for( int j=0 ; j<200  ; j++)
	{
		for(int i=0 ; i<CANALI ; i++)
		{
			//Apre un canale
			setRegister(_NRF24_RF_CH,(128*i)/CANALI);

			//Attiva RX
			setRX();

			//Aspetta attivazione RX
			delayMicroseconds(30);

			//Chiude canale
			disable();

			//Passa a canale successivo
			if(getRegister(_NRF24_RPD)>0)
				canale[i]++;
		}
	}
}



//OUTPUT CANALI-------------------------------------------------------------------------------------
void outputChannels(void)
{
	int norm = 0;	//Valore di default
	int i, j;	//Contatori
	int potenza;	//Potenza frequenza
    int pot[14];    //Vettore potenza
 
    //Trova valore massimo
    for(i=0 ; i<CANALI ; i++)
      	if(canale[i]>norm)
      		norm = canale[i];
           
    //Output dati
    for(j=0; j<14; j++)
    {
      	i=freq[j];
      		
      	//Calcola posizione vettore
      	if( norm!=0 )
      		potenza = (canale[i]*SCALA);
      	else
      		potenza = 0;
      		
      	//Incrementa valori bassi
      	if( potenza==0 && canale[i]>0 )
      		potenza++;
      		
      	//Decrementa valori alti
      	if( potenza>16 )
      		potenza = 16;

		pot[j]=potenza;
			  
		Serial.print(potenza);
			  
		if (potenza < 10)
			Serial.print("  ");
		else
			Serial.print(" ");
			   
		//Reset canale
		canale[i] = 0;
	}

	Serial.println("");
			  
	u8g.firstPage();  
	do
	{
		u8g.setRot180();
		u8g.setFont(u8g_font_unifont);
		u8g.drawStr(0, 64, INTEST);

		for (j=0; j<14; j++)
		{
		if (pot[j] > 0)
			u8g.drawBox(j*8 + 1 + 16, 48 - 2 - 3 * (pot[j] - 1), 6, 3 * pot[j]);
		else
			u8g.drawBox(j*8 + 1, 48, 0, 0);
		}

		u8g.undoRotation();
	}
	while(u8g.nextPage());
	
}

void outputChannel(int c)
{
	int norm = 0;	//Valore di default
	int i, j;	//Contatori
	int potenza;	//Potenza frequenza
  
	//Trova valore massimo
	for(i=0 ; i<CANALI ; i++)
		if(canale[i]>norm)
			norm = canale[i];
      
	i=freq[c];
  
	//Calcola posizione vettore
	if( norm!=0 )
		potenza = (canale[i]*SCALA);
	else
		potenza = 0;
      		
	//Incrementa valori bassi
	if( potenza==0 && canale[i]>0 )
		potenza++;
      		
	//Decrementa valori alti
	if( potenza>16 )
		potenza = 16;
      
	canale[i] = 0;
	sto[15]=potenza;
  
	u8g.firstPage();  
	do
	{
		u8g.setRot180();
		u8g.setFont(u8g_font_unifont);
		u8g.drawStr(0, 64, CAN);
		u8g.setPrintPos(64, 64);
		u8g.print(c+1);
      
		for (j=0; j<16; j++)
		{
			if (sto[j] > 0)
				u8g.drawBox(j*8 + 1, 48 - 2 - 3 * (sto[j] - 1), 6, 3 * sto[j]);
			else
				u8g.drawBox(j*8 + 1, 48, 6, 1);
		}
      
		u8g.undoRotation();
	}
	while(u8g.nextPage());
  
}

//OUTPUT DISPLAY------------------------------------------------------------------------------------
//Setup Display
void setDisplay(void)
{
	if (u8g.getMode() == U8G_MODE_R3G3B2)
		u8g.setColorIndex(255);     // white
	
	else if (u8g.getMode() == U8G_MODE_GRAY2BIT)
		u8g.setColorIndex(3);         // max intensity
	
	else if (u8g.getMode() == U8G_MODE_BW)
		u8g.setColorIndex(1);         // pixel on
	
	else if (u8g.getMode() == U8G_MODE_HICOLOR)
		u8g.setHiColorByRGB(255,255,255);
}



//MAIN----------------------------------------------------------------------------------------------
//Avvio
void setup()
{
	Serial.begin(57600);

	//Setup display
	setDisplay();
	u8g.firstPage();  
	do
	{
		u8g.setRot180();
        u8g.setFont(u8g_font_unifont);
		u8g.drawStr(8, 16, "CARICAMENTO...");
		u8g.drawStr(0, 16, "by MASK_team");
		u8g.drawStr(0, 64, "WiFi_DET   1.3.1");
        u8g.undoRotation();
	}
	while(u8g.nextPage());
	
	//Reset canale
	resetCanale();
	
	//Setup SPI
	SPI.begin();
	SPI.setDataMode(SPI_MODE0);
	SPI.setClockDivider(SPI_CLOCK_DIV2);
	SPI.setBitOrder(MSBFIRST);

	//Setup scheda
	pinMode(CE,OUTPUT);
	disable();

	//Attiva scheda
	powerUp();
 
	//Setup registri
	setRegister(_NRF24_EN_AA,0x0);
	setRegister(_NRF24_RF_SETUP,0x0F);
}

//Processo
void loop()
{
    int i;
    int t;
    float ten;
    
    //Scansione
    scanChannels();
    
    for (i=0; i<16; i++)
		sto[i]=sto[i+1];
    
	t = analogRead(0);
	ten = (float((t*5)/1023.00));
     
     
	if (ten <= old - 0.1 || ten >= old + 0.1)
		for (i=0; i<16; i++)
			sto[i] = 0;
	old = ten;
     
	if      (ten > 3.9 && ten < 4.2) outputChannel(13);
	else if (ten > 3.6 && ten < 4.2) outputChannel(12);
	else if (ten > 3.3 && ten < 4.2) outputChannel(11);
	else if (ten > 3.0 && ten < 4.2) outputChannel(10);
	else if (ten > 2.7 && ten < 4.2) outputChannel(9);
	else if (ten > 2.4 && ten < 4.2) outputChannel(8);
	else if (ten > 2.1 && ten < 4.2) outputChannel(7);
	else if (ten > 1.8 && ten < 4.2) outputChannel(6);
	else if (ten > 1.5 && ten < 4.2) outputChannel(5);
	else if (ten > 1.2 && ten < 4.2) outputChannel(4);
	else if (ten > 0.9 && ten < 4.2) outputChannel(3);
	else if (ten > 0.6 && ten < 4.2) outputChannel(2);
	else if (ten > 0.3 && ten < 4.2) outputChannel(1);
	else if (ten >= 0  && ten < 4.2)  outputChannel(0);
	else {Serial.println("TEST"); outputChannels();}
	
}
