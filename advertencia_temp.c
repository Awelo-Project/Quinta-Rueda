/*
 * main implementation: use this 'C' sample to create your own application
 *
 */





#include "derivative.h" /* include peripheral declarations */

unsigned char temp=60;
unsigned char cont_temp=0;

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
				GPIOA_PDOR=(1<<12); //prende el buzzer. 
			}
			if (cont_temp%2!=0)
			{
				GPIOA_PDOR=0; //apaga el buzzer.
			}
			if (cont_temp>=5)
			{
				NVIC_ICER=(1<<22); //Intr PIT	
				GPIOA_PDOR=0; //apaga el buzzer.
				cont_temp=0;
				//DAC0_C0 |= (0<<7);    	// Disable DAC 
			}
			cont_temp++;
		}	
	
}
	


int main(void)
{
	//PIT
	SIM_SCGC6|=(1<<23); 
	PIT_MCR=0;
	PIT_LDVAL1=10000000;  //1s por interrupcion, clk : 4 MHz
	
	
	
	//GPIO BUZZER
		SIM_SCGC5|=(1<<9); 
		PORTA_PCR12|=(1<<8); //PTA12 sea GPIO
		GPIOA_PDDR|=(1<<12);
		tempWaring(temp);



	while (1);
	return 0;
}
