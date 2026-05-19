#include <stm32f10x.h>
#include <stdio.h>

//declearing some variables
int c=0,desiredSpeed,a,flag=0;
char str[20];

//defining the usart print string function
void print(char *string){
	while(*string){
		while(!(USART1->SR & USART_SR_TXE));
		USART1->DR = *(string++);
	}
}

//usart interrupt function
void USART1_IRQHandler(void){
	USART1->SR &=! (USART_SR_RXNE);   //disable usart interrupt flag
	int i;
	i = (USART1->DR) - 48;     //turning asci codes into integer numbers
	if(i>=0 && i<10){          //user writes the refrence speed
		a = 10*a + i;
		sprintf(str,"%d",i);
		print(str);
	}
	if(i == -35){             //this will set the refrence speed
		sprintf(str,"\n\rDone.");
		print(str);
		desiredSpeed = a;
		a = 0;
		flag = 1;
	}
}

//using external interrupt as counter
void EXTI4_IRQHandler(void){
	EXTI->PR &=! (EXTI_PR_PR4);
	c++;
}

//counts the number of pulses every second as motor's speed
void TIM1_UP_IRQHandler(void){
	TIM1->SR &=! (TIM_SR_UIF);
	int i,j;
	i=c;
	c=0;
	sprintf(str,"\n\rCurrent Speed = %d",i);
	print(str);

	//this is the pwm signal duty cycle regulation part in regard to the error signal which happens every second
	if(i>desiredSpeed){
		j=i-desiredSpeed;
		if(j>5) TIM3->CCR2 +=5;
		else if(j>4) TIM3->CCR2 +=4;
		else if(j>3) TIM3->CCR2 +=3;
		else if(j>2) TIM3->CCR2 +=2;
		else if(j>1) TIM3->CCR2 +=1;
	}
	if((TIM3->CCR2) > 499) TIM3->CCR2 = 499;
	if(i<desiredSpeed){
		j=desiredSpeed-i;
		if(j>5) TIM3->CCR2 -=50;
		else if(j>4) TIM3->CCR2 -=4;
		else if(j>3) TIM3->CCR2 -=3;
		else if(j>2) TIM3->CCR2 -=2;
		else if(j>1) TIM3->CCR2 -=1;
	}
	if(i==desiredSpeed) TIM3->CCR2 =0;
	if((TIM3->CCR2) < 0) TIM3->CCR2 = 0;
}

	
int main(){
	//performing gpio ports and clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	GPIOA->CRL = 0;
	GPIOA->CRL |= GPIO_CRL_CNF1_0 | GPIO_CRL_CNF0_0;
	GPIOA->CRL |= GPIO_CRL_MODE2_0 | GPIO_CRL_MODE3_0;
	GPIOA->ODR &=! GPIO_ODR_ODR2 | GPIO_ODR_ODR3;
	
	//enabling external interrupt to read motor speed
	GPIOA->CRL |= GPIO_CRL_CNF4_0;
	EXTI->IMR |= EXTI_IMR_MR4;
	EXTI->RTSR |= EXTI_RTSR_TR4;
	NVIC_EnableIRQ(EXTI4_IRQn);
	NVIC_SetPriority(EXTI4_IRQn,2);
	
	//enabling usart to get refrence speed from user
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	GPIOA->CRH = 0;
	GPIOA->CRH |= GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9_0 | GPIO_CRH_CNF10_0;
	USART1->BRR = 40000/384;
	USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
	USART1->CR1 |= USART_CR1_RXNEIE;
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_SetPriority(USART1_IRQn,3);
	
	sprintf(str,"\n\rDesired Speed = ");
	print(str);
	while(!flag);
	
	//enables timer1 to get motor speed and generate pwm signal each second
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	TIM1->PSC = 3999;
	TIM1->ARR = 999;
	TIM1->CR1 |= TIM_CR1_ARPE;
	TIM1->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM1_UP_IRQn);
	NVIC_SetPriority(TIM1_UP_IRQn,1);
	TIM1->CR1 |= TIM_CR1_CEN; 
	
	//generates the pwm signal using timer3
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	GPIOA->CRL |= GPIO_CRL_CNF7_1 | GPIO_CRL_MODE7_0;
	TIM3->PSC = 3999;
	TIM3->ARR = 499;
	TIM3->CR1|= TIM_CR1_ARPE;
	TIM3->CCR2 = 0;
	TIM3->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;
	TIM3->CCER |= TIM_CCER_CC2E | TIM_CCER_CC2P;
	TIM3->CCMR1 |= TIM_CCMR1_OC2PE;
	TIM3->CR1 |= TIM_CR1_CEN;	
	
	while(1){
		//sets the direction of motor's movement
		if((GPIOA->IDR & GPIO_IDR_IDR0) == 0){
			GPIOA->ODR &=! GPIO_ODR_ODR3;
			GPIOA->ODR |= GPIO_ODR_ODR2;
		}
		
		if((GPIOA->IDR & GPIO_IDR_IDR1) == 0){
			GPIOA->ODR &=! GPIO_ODR_ODR2;
			GPIOA->ODR |= GPIO_ODR_ODR3;
		}
	}
}
