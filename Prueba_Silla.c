/*
 * main implementation: use this 'C' sample to create your own application
 *
 */





#include "derivative.h" /* include peripheral declarations */

unsigned char resultado_canales[3];
unsigned char secuencia_canales[]={(1<<6)+12,(1<<6)+11,(1<<6)+14,(1<<6)+13}; // PTB2,PTC2, PTC0, PTB3
unsigned short i;
unsigned short valor_mv;
unsigned char cont;
unsigned char autom;
unsigned short tv=0; //tiempo viejo
unsigned short frecuencia;
unsigned short velocidad_w;
unsigned short velocidad_max=0;
unsigned short mVout;
unsigned short periodo;
unsigned char ADC_selector=0;
unsigned char temperatura;
unsigned short corriente; 
unsigned char erase;
unsigned char aux;
unsigned char dato_disponible;
unsigned long velocidad_l=0;
unsigned char mensaje[]={"###\n"};
unsigned short digito;
unsigned char back=0;
unsigned short i_2;
unsigned short blue_vol;

unsigned char temperatura;
unsigned char cont_temp=0;
unsigned char duty_cycle;
unsigned char peri;
unsigned char itera;
unsigned char compare;

void DAC0_init(void) 
{
	SIM_SCGC6 |= (1<<31);   		// clock to DAC module SIM_SCG6=SIMSCG6|(1<<31)
	DAC0_C0 = (1<<7)+ (1<<5);     	// enable DAC and use software trigger
}	

void clk_init (void)
{
// FIRC = 4 MHz. BusClk = 4 MHz
// UART0: FIRC. UART1: BusClk. UART2: BusClk. TPM: FIRC. IIC: BusClk. PIT:
MCG_C1|=(1<<6) + (1<<1);	//MCGOUTCLK : IRCLK. CG: Clock gate, MCGIRCLK enable pag 116
MCG_C2|=1;					//Mux IRCLK : FIRC (4 MHz) pag 116
MCG_SC=0;					//Preescaler FIRC 1:1 pag 116

SIM_CLKDIV1=0;				//OUTDIV4=OUTDIV1= 1:1 pag 116. Busclk 4 MHz
SIM_SOPT2|=15<<24;			//Seleccion MCGIRCLK tanto para UART0 como para TPM
}

void tempWaring(temperatura)
{	
	if (temperatura>=40) // verifica que la temperatura
	{	
		//inicia conteo
		SIM_SCGC6|=(1<<23); //PIT
		PIT_MCR=1;
		PIT_LDVAL1=1000000;  //500 ms entre cada channel, clk : 4 MHz
		PIT_TCTRL1=3; //TEN=1, TIEN=1
		//NVIC_ISER=(1<<22); //Intr PIT
		
	}	
	else
	{
		if (back>=1)
		{
			//TPM1_SC=15; //clk 4 MHz, preescaler=128
			//TPM1_C0SC=(9<<2);
			//DAC0_init();
			back=0;
		}
	}
}

void check_battery(bateria)
{
	if (bateria<2280)
	{
		back++;
		i=0;
		DAC0_DAT0L = i & 0xff; 	//write low byte
		DAC0_DAT0H = (i >> 8);	// write high byte
		DAC0_C0 |= (0<<7);    	// Disable DAC 
		SIM_SCGC6 |= (0<<31);   		// clock to DAC module SIM_SCG6=SIMSCG6|(1<<31)
	}
	else 
	{
		back=0;
	}
}

void PIT_IRQHandler()
{
	if (PIT_TFLG0==1)
	{
	PIT_TFLG0=1; //Apaga bandera
	tempWaring(temperatura);
	ADC0_SC1A=secuencia_canales[ADC_selector];
	
	if (digito<10)
			mensaje[2]=digito + 0x30;
			mensaje[1]=0 + 0x30;
			mensaje[0]=0 + 0x30;
		if (digito>10)
		{
			mensaje[0]=0 + 0x30;
			mensaje[2]=digito%10 + 0x30;
			mensaje[1]=digito/10 + 0x30;
		}
		if (digito>=100)
		{
			mensaje[2]=digito%10 + 0x30;
			digito=digito/10;
			mensaje[1]=digito%10 + 0x30;
			mensaje[0]=digito/10 + 0x30;
		}
		do{
			do{} while((UART0_S1 &(1<<7))==0);
			UART0_D=mensaje[i_2++];
		}while(mensaje[i_2]!=0);
		i_2=0;
		//digito=0;
		//aux=0;
		//UART0_C2=(1<<3)+(1<<7); //Transmit Enable, TIE para transmision
		//TPM0_C2SC=(2<<2)+(1<<6);     //input capture, falling edge, CHIE=1
		//NVIC_ISER=(1<<12);
	
	}
	
	if (PIT_TFLG1==1)
	{
		PIT_TFLG1=1; //Apaga bandera
		cont_temp++;
		if (cont_temp%2==0)//
		{
			//GPIOA_PDOR=(1<<12); //prende el buzzer. 
			
			TPM1_MOD=peri;
			TPM1_C0SC=(9<<2); //PWM edge aligned, set on match
			//duty_cycle=75;
			duty_cycle=75;
			TPM1_C0V=(duty_cycle*peri)/100;
			
		}
		if (cont_temp%2!=0)
		{
			//GPIOA_PDOR=0; //apaga el buzzer.
			TPM1_MOD=peri;
			TPM1_C0SC=(9<<2); //PWM edge aligned, set on match
			//duty_cycle=75;
			duty_cycle=0;
			TPM1_C0V=(duty_cycle*peri)/100;
			
		}
		if (cont_temp>=100)
		{
			
			//GPIOA_PDOR=0; //apaga el buzzer.
			//secuencia_canales[0]=0;
			back++;
			i=0;
			DAC0_DAT0L = i & 0xff; 	//write low byte
			DAC0_DAT0H = (i >> 8);	// write high byte
			DAC0_C0 |= (0<<7);    	// Disable DAC 
			SIM_SCGC6 |= (0<<31);   		// clock to DAC module SIM_SCG6=SIMSCG6|(1<<31)
			
			TPM1_C0SC=0;
			duty_cycle=0;
			TPM1_C0V=(duty_cycle*peri)/100;
			cont_temp=0;
			PIT_TCTRL1=0; 
			PIT_TCTRL0=3; //TEN=1, TIEN=1
			valor_mv=0;
			tv=0;
			velocidad_w=0;
			velocidad_max=0;
			velocidad_l=0;
			mVout=0;
			blue_vol=0;
			
			//NVIC_ICER=(1<<22); //Intr PIT	
		}
		

		
	}
	
}

void FTM0_IRQHandler()
{
	
	if ((TPM0_C0SC & (1<<7))==(1<<7))
	{
		
	}
	if ((TPM0_C2SC & (1<<7))==(1<<7))
		{
			TPM0_C2SC |=(1<<7);
			periodo=TPM0_C2V-tv;
			//frecuencia=1/periodo
			frecuencia=(1000000/periodo)/4; // div 4 por que preescaler=16
			
			if (frecuencia==0)
			{
				velocidad_w=0;
				tv=TPM0_C2V; 
				
				if(compare<velocidad_w)
				{
					if (velocidad_w>velocidad_max)
					{
						velocidad_max=velocidad_w;
						compare=velocidad_w+15;
				
					}
				}
			}
			else if (frecuencia<175)
			{
				velocidad_w=(((frecuencia*1000)/15)*60)/1000;// normalizado a 1000
				tv=TPM0_C2V; 								 // tiempo viejo se guarda en value del timer 
				
				if(compare<frecuencia)
				{
					if (velocidad_w>velocidad_max)
					{
						velocidad_max=velocidad_w;
						compare=frecuencia+5;
				
					}
				}
			}
			//comparar velocidad nueva vs max 
	
			digito=velocidad_w;
			
			//enviar velocidad por el DAC
			mVout=(24*velocidad_max+11651)/10; //conversion de velocidad max a voltje de ref (formula de caracterizacion)
		}
}



void ADC0_IRQHandler() //ADC_ISR tomado de Project_Settings>Startup_Code>kinetis_sysinit.c
{
	//Apaga bandera 
	resultado_canales[ADC_selector++]=ADC0_RA; //Resultado ADC
	if (ADC_selector==2) ADC_selector=0;
	valor_mv=(resultado_canales[0]*3300)/255;
	valor_mv=(valor_mv-0)*(2800-1240)/(3300-0)+1240;
	temperatura=(((resultado_canales[1]*3300)/255)/10);
	corriente=((resultado_canales[2]*3300)/255)/10;

	
}

void UART0_Rx_clean(void)
{
	unsigned char temp;
	UART0_S1|=0x1F;
	do{
		UART0_S1|=0x1F;
	if ((UART0_S1 & (1<<5))!=0) temp=UART0_D;
	}while ((UART0_S1)!=0xC0);
}



void UART0_IRQHandler()
{
unsigned char temp;
	if ((UART0_S1 & (1<<5))==(1<<5))
	{
		temp=UART0_D;
		if ((temp!=13) && (temp>='0') && (temp<='9')) velocidad_l=(velocidad_l*10)+temp-0x30;
		if (temp==13)
		{
		dato_disponible=1;
		NVIC_ICER=(1<<12);
		}
		
	}
	NVIC_ICER=(1<<12);
}

void PORTA_IRQHandler()
{
	if (PORTA_PCR17 & (1<<24))
		{
			PORTA_PCR17=PORTA_PCR17|(1<<24); //Apaga ISF
			// Esto se repite por cada pin que genere interrupciones

			cont++;
			if (cont==3)
			{
				cont=1;
			}
			
			
		}
	if (PORTA_PCR16 & (1<<24))
		{
			PORTA_PCR16=PORTA_PCR16|(1<<24); //Apaga ISF
			// Esto se repite por cada pin que genere interrupciones

			autom++;
		}
		if (autom==3)
			{
				autom=1;
			}
			
}

void PORTD_IRQHandler()
{
	

	NVIC_ICER=(1<<31);
}





int main(void)
{
	// Primer paso, configurar pines
	// Para entradas analogicas (default) no es necesario
	
	clk_init();		//UART clk a 4MHz
	DAC0_init(); // Configure DAC0 PTE30
	SIM_SCGC6|=(1<<27); // Reloj interfaz ADC0
	NVIC_ISER=(1<<15);
	SIM_SCGC6|=(1<<23); //PIT
	PIT_MCR=0;
	PIT_LDVAL0=100000;  //100 ms entre cada channel, clk : 4 MHz
	PIT_TCTRL0=3; //TEN=1, TIEN=1
	
	
	SIM_SCGC5|=(1<<9);    //PORTA
	PORTA_PCR5=(3<<8);    //TPM0_CH2
	SIM_SCGC6|=(1<<24);   //TPM0
	TPM0_SC=(1<<3)+4;   //CMOD=1 (4 mHz), preescaler 16
	TPM0_C2SC=(2<<2)+(1<<6);     //input capture, falling edge, CHIE=1
	
	
	SIM_SCGC5|=(1<<9); // Reloj de interfaz GPIOA
	PORTA_PCR17|=(1<<8)+(11<<16); //B: PTA15 sea GPIO, Intr rising edge (subida) hab intr. 
	// despues de un reset los pines de GPIO son entrada.
	PORTA_PCR16|=(1<<8)+(11<<16);
	
	//init Port D
	SIM_SCGC5|=(1<<12);
	PORTD_PCR4=(1<<8)+(9<<16);
	
	//PIT
	
	SIM_SCGC5|=(1<<9);    //PORTA
	PORTA_PCR12=(3<<8); //TPM1_C0
	SIM_SCGC6|=(1<<25); //TPM1
	
	TPM1_SC=15; //clk 4 MHz, preescaler=128
	//TPM1_MOD=31250;
	peri=(6250/1000);	

	SIM_SCGC4|=(1<<10); 			//UART0
	PORTA_PCR1=(2<<8); 				//UART0
	PORTA_PCR2=(2<<8); 				//UART0
	UART0_BDL=26; 					//baud_rate (9600)=4MHz/(16*9600)
	UART0_C2=12+(1<<6)+(1<<7); 			//TEN=REN=1. Hab Intr Rx
	UART0_Rx_clean();
	NVIC_ISER=(1<<12);
	
	NVIC_ISER=(1<<31);
	NVIC_ISER=(1<<30);
	NVIC_ISER=(1<<22); //Intr PIT
	NVIC_ISER=(1<<12);
	NVIC_IPR4=(1<<14);
	
	while(1)
	{
		if (cont==1 && autom==1)
		{		
				//aux=0;
				if (aux==0)
				{
					valor_mv=0;
					tv=0;
					velocidad_w=0;
					velocidad_max=0;
					velocidad_l=0;
					mVout=0;
					i=0;
					DAC0_DAT0L = i & 0xff; 	//write low byte
					DAC0_DAT0H = (i >> 8);	// write high byte
					//dato_disponible=1;
					aux=4;
				}

				if (dato_disponible==1)
				{
					//aux++;
					NVIC_ISER=(1<<17);    // Intr NVIC TPM0
					blue_vol=(24*velocidad_l+11651)/10; //conversion de velocidad max a voltje de ref (formula de caracterizacion)
					i=(blue_vol*4095/3300);
					DAC0_DAT0L = i & 0xff; 	//write low byte
					DAC0_DAT0H = (i >> 8);	// write high byte
					velocidad_l=0;
					dato_disponible=0;	
				}
				NVIC_ISER=(1<<12);
				
			
		}
		else if (cont==2)
		{
			if (back==0)
			{
				aux=0;
				NVIC_ISER=(1<<22); //Intr PIT
				NVIC_ISER=(1<<17);    // Intr NVIC TPM0
				if (valor_mv==1240) valor_mv=0;
				i=(valor_mv*4095/3300);//ul =unsigend long le idnica que esa variable debe ser una variable de 32bits sin unidad de medida 
				DAC0_DAT0L = i & 0xff; 	//write low byte
				DAC0_DAT0H = (i >> 8);	// write high byte
			}
			else
			{
				i=0;//ul =unsigend long le idnica que esa variable debe ser una variable de 32bits sin unidad de medida 
				DAC0_DAT0L = i & 0xff; 	//write low byte
				DAC0_DAT0H = (i >> 8);	// write high byte
			}
		}
		else if (autom==2)
		{
			if (aux==4)
			{
				valor_mv=0;
				tv=0;
				frecuencia=0;
				compare=0;
				velocidad_w=0;
				velocidad_max=0;
				velocidad_l=0;
				mVout=0;
				periodo=0;
				frecuencia=0;
				i=0;
				DAC0_DAT0L = i & 0xff; 	//write low byte
				DAC0_DAT0H = (i >> 8);	// write high byte
				//dato_disponible=1;
				aux=5;
			}
			aux=0;
			NVIC_ISER=(1<<17);    // Intr NVIC TPM0
			i=(mVout*4095/3300);//ul =unsigend long le idnica que esa variable debe ser una variable de 32bits sin unidad de medida 
			DAC0_DAT0L = i & 0xff; 	//write low byte
			DAC0_DAT0H = (i >> 8);	// write high byte
		}
	}
	return 0;
}
