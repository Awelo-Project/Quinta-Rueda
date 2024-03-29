#include "derivative.h" /* include peripheral declarations */
unsigned short tv=0; //tiempo viejo
unsigned long frecuencia;
unsigned short velocidad_w;
unsigned short velocidad_max=0;
unsigned short mVout;
unsigned long DAC_data,i;

void DAC0_init(void) {
	SIM_SCGC6 |= (1<<31);   		// clock to DAC module SIM_SCG6=SIMSCG6|(1<<31)
	DAC0_C0 = (1<<7)+ (1<<5);     	// enable DAC and use software trigger
}	

void FTM0_IRQHandler()
{
	unsigned short periodo;
	if ((TPM0_C0SC & (1<<7))==(1<<7))
	{
	}
	if ((TPM0_C2SC & (1<<7))==(1<<7))
	{
		TPM0_C2SC |=(1<<7);
		periodo=TPM0_C2V-tv;
		//frecuencia=1/periodo
		frecuencia=(1000000/periodo)/4;
		velocidad_w=(((frecuencia*1000)/15)*60)/1000;// normalizado a 1000
		tv=TPM0_C2V; 								 // tiempo viejo se guarda en value del timer 
		/*
		//comparar velocidad nueva vs max 
		if (velocidad_w>velocidad_max)
		{
			velocidad_max=velocidad_w;
		}
		
		//enviar velocidad por el DAC
		mVout=velocidad_max*2359+1165100; //conversion de velocidad max a voltje de ref (formula de caracterizacion)
		i=(mVout*4095UL/3300);//ul =unsigend long le idnica que esa variable debe ser una variable de 32bits sin unidad de medida 
		DAC0_DAT0L = i & 0xff; 	//write low byte
		DAC0_DAT0H = (i >> 8);	// write high byte*/
	}
}
void clk_init (void)

{
// FIRC = 4 MHz. BusClk = 4 MHz
// UART0: FIRC. UART1: BusClk. UART2: BusClk. TPM: FIRC. IIC: BusClk. PIT:
MCG_C1|=(1<<6) + (1<<1); //MCGOUTCLK : IRCLK. CG: Clock gate, MCGIRCLK enable pag 116
MCG_C2|=1; //Mux IRCLK : FIRC (4 MHz) pag 116
MCG_SC=0; //Preescaler FIRC 1:1 pag 116
SIM_CLKDIV1=0; //OUTDIV4=OUTDIV1= 1:1 pag 116. Busclk 4 MHz
SIM_SOPT2|=15<<24; //Seleccion MCGIRCLK tanto para UART0 como paraTPM
}



int main(void)
{
    clk_init();
    DAC0_init(); // Configure DAC0 PTE30
    SIM_SCGC5|=(1<<9);    //PORTA
    PORTA_PCR5=(3<<8);    //TPM0_CH2
    SIM_SCGC6|=(1<<24);   //TPM0
    TPM0_SC=(1<<3)+4;   //CMOD=1 (4 mHz), preescaler 16
    TPM0_C2SC=(2<<2)+(1<<6);     //input capture, falling edge, CHIE=1
    NVIC_ISER=(1<<17);    // Intr NVIC TPM0
    
    while (1);
return 0;
}
