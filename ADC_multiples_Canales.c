#include "derivative.h" /* include peripheral declarations */


unsigned char secuencia_canales[]={(1<<6)+14,(1<<6)+12,};
unsigned char resultado_canales[2];
unsigned char ADC_selector=0;
unsigned char temperatura_antes;
unsigned char temperatura;
unsigned short corriente; 
//unsigned char extra;


void ADC0_IRQHandler()
{
	resultado_canales[ADC_selector++]=ADC0_RA; //Resultado ADC
	if (ADC_selector==2) ADC_selector=0;
	temperatura_antes=resultado_canales[0];
	temperatura=(((temperatura_antes*3300)/255))/10;
	//corriente=resultado_canales[1];
	corriente=((resultado_canales[1]*3300)/255)/10;
	//extra=((resultado_canales[2]*3300)/255)/10;
}

void PIT_IRQHandler()
{
	if (PIT_TFLG0==1)
	{
	PIT_TFLG0=1; //Apaga bandera
	ADC0_SC1A=secuencia_canales[ADC_selector];
	}
	
	if (PIT_TFLG1==1)
	{
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
	clk_init();
	//Para pines de ADC el valor de PCR es default
	SIM_SCGC6|=(1<<27); //ADC0
	
	SIM_SCGC6|=(1<<23); //PIT
	PIT_MCR=0;
	PIT_LDVAL0=100000;  //100 ms entre cada channel, clk : 4 MHz
	PIT_TCTRL0=3; //TEN=1, TIEN=1
	NVIC_ISER=(1<<22); //Intr PIT
	NVIC_ISER=(1<<15); //Hab intr NVIC ADC
	
	while (1);
	return 0;
}

