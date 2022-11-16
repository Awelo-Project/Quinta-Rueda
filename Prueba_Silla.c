/*
 * main implementation: use this 'C' sample to create your own application
 *
 */





#include "derivative.h" /* include peripheral declarations */

unsigned char resultado_ADC;
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

void PIT_IRQHandler()
{
	if (PIT_TFLG0==1)
	{
	PIT_TFLG0=1; //Apaga bandera
	ADC0_SC1A=(1<<6)+9; //Canal 9 (PTB1), inicia conversion
	
	}
	if (PIT_TFLG1==1)
	{
		PIT_TFLG1=1; //Apaga bandera
		
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
		if (frecuencia<175)
		{
			velocidad_w=(((frecuencia*1000)/15)*60)/1000;// normalizado a 1000
			tv=TPM0_C2V; 								 // tiempo viejo se guarda en value del timer 
		}
		//comparar velocidad nueva vs max 
		if (velocidad_w>velocidad_max)
		{
			velocidad_max=velocidad_w;
	
		}

		
		//enviar velocidad por el DAC
		mVout=(24*velocidad_max+11651)/10; //conversion de velocidad max a voltje de ref (formula de caracterizacion)
		i=(mVout*4095/3300);//ul =unsigend long le idnica que esa variable debe ser una variable de 32bits sin unidad de medida 
		DAC0_DAT0L = i & 0xff; 	//write low byte
		DAC0_DAT0H = (i >> 8);	// write high byte
	}
}



void ADC0_IRQHandler() //ADC_ISR tomado de Project_Settings>Startup_Code>kinetis_sysinit.c
{
	//Apaga bandera 
	resultado_ADC=ADC0_RA; // Resultado del ADC, Apaga COCO
	valor_mv=(resultado_ADC*3300)/255;
	i=(valor_mv*4095/3300);//ul =unsigend long le idnica que esa variable debe ser una variable de 32bits sin unidad de medida 
	DAC0_DAT0L = i & 0xff; 	//write low byte
	DAC0_DAT0H = (i >> 8);	// write high byte
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
	PIT_LDVAL1=100000;  //100 ms entre cada channel, clk : 4 MHz
	PIT_TCTRL1=3; //TEN=1, TIEN=1
	
	
	
	SIM_SCGC5|=(1<<9);    //PORTA
	PORTA_PCR5=(3<<8);    //TPM0_CH2
	SIM_SCGC6|=(1<<24);   //TPM0
	TPM0_SC=(1<<3)+4;   //CMOD=1 (4 mHz), preescaler 16
	TPM0_C2SC=(2<<2)+(1<<6);     //input capture, falling edge, CHIE=1
	
	
	SIM_SCGC5|=(1<<9); // Reloj de interfaz GPIOA
	PORTA_PCR17|=(1<<8)+(11<<16); //B: PTA15 sea GPIO, Intr rising edge (subida) hab intr. 
	// despues de un reset los pines de GPIO son entrada.
	PORTA_PCR16|=(1<<8)+(11<<16);
	NVIC_ISER=(1<<30);
	
	
	
	while(1)
	{
		if (cont==1 && autom==1)
		{
			NVIC_ICER=(1<<22); //Intr PIT
			NVIC_ICER=(1<<17);    // Intr NVIC TPM0
			valor_mv=0;
			tv=0;
			velocidad_w=0;
			velocidad_max=0;
			mVout=0;
			i=0;
			i=(valor_mv*4095/3300);//ul =unsigend long le idnica que esa variable debe ser una variable de 32bits sin unidad de medida 
			DAC0_DAT0L = i & 0xff; 	//write low byte
			DAC0_DAT0H = (i >> 8);	// write high byte
			
		}
		else if (cont==2)
		{
			NVIC_ISER=(1<<22); //Intr PIT
		}
		else if (autom==2)
		{
			NVIC_ISER=(1<<17);    // Intr NVIC TPM0
		}
	}
	return 0;
}
