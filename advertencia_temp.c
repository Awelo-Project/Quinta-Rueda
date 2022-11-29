/*
 * main implementation: use this 'C' sample to create your own application
 *
 */





#include "derivative.h" /* include peripheral declarations */

unsigned char temp=65;
unsigned char cont_temp=0;
unsigned char duty_cycle;
unsigned char peri;

void tempWaring(temperatura)
{	
	if (temperatura>=60) // verifica que la temperatura
	{	
		//inicia conteo
		PIT_TCTRL1=3; //TEN=1, TIEN=1
		NVIC_ISER=(1<<22); //Intr PIT
	}	
	else
	{
		//DAC0_C0 |= (1<<7);    	// Disable DAC 
	}
}

void PIT_IRQHandler()
{

	if (PIT_TFLG1==1) //significa que paso 1 seg
		{
		PIT_TFLG1=1; //Apaga bandera
			
			if (cont_temp%2==0)//
			{
				//GPIOA_PDOR=(1<<12); //prende el buzzer. 
				
				duty_cycle=75;
				TPM1_C0V=(duty_cycle*peri)/100;
			}
			if (cont_temp%2!=0)
			{
				//GPIOA_PDOR=0; //apaga el buzzer.

				duty_cycle=0;
				TPM1_C0V=(duty_cycle*peri)/100;
			}
			if (cont_temp>=60)
			{
				
				//GPIOA_PDOR=0; //apaga el buzzer.
				TPM1_C0SC=0;
				cont_temp=0;
				//DAC0_C0 |= (0<<7);    	// Disable DAC 
				NVIC_ICER=(1<<22); //Intr PIT	
			}
			cont_temp++;
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
	SIM_SOPT2|=15<<24; //Seleccion MCGIRCLK tanto para UART0 como para TPM
}


int main(void)
{
	//PIT
	SIM_SCGC6|=(1<<23); 
	PIT_MCR=0;
	PIT_LDVAL1=5000000;  //1s por interrupcion, clk : 4 MHz
	
	
	
	//GPIO BUZZER
	//SIM_SCGC5|=(1<<9); 
	//PORTA_PCR12|=(1<<8); //PTA12 sea GPIO
	//GPIOA_PDDR|=(1<<12);
	
	// TPM init
	clk_init();
	SIM_SCGC5|=(1<<13); //PORTE
	PORTE_PCR20=(3<<8); //TPM1_C0
	SIM_SCGC6|=(1<<25); //TPM1
	
	TPM1_SC=15; //clk 4 MHz, preescaler=128
	//TPM1_MOD=31250;
	peri=(6250/1000);
	TPM1_MOD=peri;
	TPM1_C0SC=(9<<2); //PWM edge aligned, set on match
	//duty_cycle=75;
	duty_cycle=75;
	TPM1_C0V=(duty_cycle*peri)/100;
	



	while (1)
	{
		tempWaring(temp);
	}
	return 0;
}

