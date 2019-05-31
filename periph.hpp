#pragma once

#include "stm32f1xx.h"
#include "utils.hpp"
#include "task.h"
#include "lcd_hd44780_i2c.h"

constexpr uint32_t portcount = 16;
constexpr uint32_t modulo = portcount / 2;

namespace Periph
{
	class Pin {
	public:
		Pin(GPIO_TypeDef &portName, uint32_t pinNum) 
			: port(portName)
			, pin(pinNum)
			, controlRegister(pinNum < modulo ? portName.CRL : portName.CRH) 
		{
			switch ((uint32_t)&portName)
			{
			case (GPIOA_BASE) : { RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; }
				break;
			case (GPIOB_BASE) : { RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; }
				break;
			case (GPIOC_BASE) : { RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; }
				break;
			case (GPIOD_BASE) : { RCC->APB2ENR |= RCC_APB2ENR_IOPDEN; }
				break;
			}
		}
		;
	protected:
		GPIO_TypeDef &port;
		uint32_t pin;
		volatile uint32_t &controlRegister;
	};
	
	class OutPin : public Pin {
	public:
		enum PinType { pushpull, opendrain, AFpushpull, AFopendrain };
		enum PinSpeed { MHz10 = 0x1U, MHz2 = 0x2U, MHz50 = 0x3U };
		OutPin(GPIO_TypeDef &portName, uint32_t pinNum, PinType pintype, PinSpeed pinspeed)
			: Pin(portName, pinNum)
		{
			if (pintype == AFpushpull || AFopendrain)
			{
				RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
			}
			this->controlRegister &= ~(0xF << 4*(pinNum % (modulo)));
			this->controlRegister |= ((pintype << 2 | pinspeed) << 4*(pinNum % (modulo)));
		}
		;
			
		inline void Toggle() const { utils::toggleBit(this->port.ODR, this->pin); }
		inline void SetHigh() const { utils::setBit(this->port.ODR, this->pin); }
		inline void SetLow() const { utils::clearBit(this->port.ODR, this->pin); }
	};
	
	class InPin : public Pin {
	public:
		enum PinType { analog, floating, pulldown, pullup };
		InPin(GPIO_TypeDef &portName, uint32_t pinNum, PinType pintype)
			: Pin(portName, pinNum)
		{
			if (pintype == pullup) //lifehack
				{
					utils::setBit(this->port.ODR, this->pin);
					pintype = pulldown;
				}
			uint32_t mask = 0xF << 4*(pinNum % (modulo));   // mask to reset bits
			uint32_t expr = (pintype << 2) << 4*(pinNum % (modulo));   // value to write
			this->controlRegister &= ~(mask);
			this->controlRegister |= (expr);
		};
			
		inline bool State() const { return utils::checkBit(this->port.IDR, this->pin); }
	};
	
	class Led {
	public:
		Led(OutPin ledPin)
			: pin(ledPin) {}
		;
			
		inline void Toggle() const { pin.Toggle(); }
		inline void SetHigh() const { pin.SetHigh(); }
		inline void SetLow() const { pin.SetLow(); }
	private:
		OutPin pin;
	};

	class Button {
	public:
		static inline uint32_t debounceTimeout = 5; // c++17
		enum ButtonType { NO, NC };
		Button(InPin buttonPin, ButtonType buttonType)
			: pin(buttonPin)
			, type(buttonType) {}
		;
			
		inline bool Pressed() const { return (pin.State() ^ type); };
		inline bool PressedDebounced() {
			if (this->Pressed()) {
				vTaskDelay(debounceTimeout);
				if (this->Pressed()) {
					return true;
				}
				else return false;
			}
			else return false;
		}
			
		private:
		InPin pin;
		ButtonType type;
	};
	
	class USART_1 {
	public:
		USART_1(const OutPin txPin, const InPin rxPin, uint32_t baudrate, const std::array<char, 8> &buf)
			: tx(txPin)
			, rx(rxPin)
		{
			RCC->APB2ENR |= RCC_APB2ENR_USART1EN;  	//	clocking usart
			//RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; 		//	alternate function clocking
			
			uint32_t speedConst = (APB2CLK + baudrate / 2) / baudrate;
			USART1->BRR = speedConst;
			
			USART1->CR1 |= USART_CR1_TE;  //	transmit enable
			USART1->CR1 |= USART_CR1_RE;  //	recieve --
			USART1->CR1 |= USART_CR1_UE;  //	uart --

			RCC->AHBENR |= RCC_AHBENR_DMA1EN;  //	enable DMA
				
			//send
			DMA1_Channel4->CPAR = (uint32_t)&USART1->DR;
			DMA1_Channel4->CMAR = (uint32_t)buf.begin();
			DMA1_Channel4->CNDTR = 8;
				
			DMA1_Channel4->CCR  &=	~DMA_CCR_CIRC;  								// Disable cycle mode
			DMA1_Channel4->CCR  &=	~DMA_CCR_PINC;  								// Disable increment pointer periphery
				
			DMA1_Channel4->CCR  &=	~DMA_CCR_PSIZE;  								// Size data periphery - 8 bit
			DMA1_Channel4->CCR  &=	~DMA_CCR_MSIZE;   							// Size data memory - 8 bit
				
			DMA1_Channel4->CCR	|=	DMA_CCR_DIR;
			DMA1_Channel4->CCR  |=	DMA_CCR_MINC;  								// Enable increment pointer memory
				
			USART1->CR3 |= USART_CR3_DMAT; 
		}
		;
			
		void Send() {
			DMA1_Channel4->CCR  &= ~DMA_CCR_EN;      
			DMA1_Channel4->CNDTR =  8;      
			DMA1->IFCR          |=  DMA_IFCR_CTCIF4;   							// Status flag end of exchange
			DMA1_Channel4->CCR |= DMA_CCR_EN;
		}
			
		//uint32_t txBufferSize;
			
		private :
		InPin rx;
		OutPin tx;
		
	};
	
	class Timer {
	public:
		static constexpr uint32_t PWM_COUNTS = 1000;
		static constexpr uint32_t PWM_MAX = SYSCLK / PWM_COUNTS;
		Timer(TIM_TypeDef &timerName, OutPin pin) : timer(timerName)
		{
			switch ((uint32_t)&timerName)
			{
			//case (TIM1_BASE) : { RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; }
			//	break;
			case (TIM2_BASE) : { RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; }
				break;
			case (TIM3_BASE) : { RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; }
				break;
			case (TIM4_BASE) : { RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; }
				break;
			}
		}
		
		inline void PWM_Init() {
			PWM_SetFrequency(PWM_MAX);
			PWM_SetParam(1000, 500);
			
			timer.CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;  // enable PWM mode 1 on channel
			timer.CCER |= TIM_CCER_CC2E;  // enable 
			timer.CCER &= ~TIM_CCER_CC2P;  // straigt polarity
			
			timer.CR1 &= ~TIM_CR1_DIR;   // count up
			timer.CR1 |= TIM_CR1_CEN;    // enable timer
		}
		
		inline void PWM_SetParam(uint32_t ARR, uint32_t CCR2) {
			timer.ARR = ARR;    // counts
			timer.CCR2 = CCR2;    // counts till enable (duty cycle)
		}
		
		inline void PWM_SetFrequency(uint32_t freq) {
			assert(freq > 0);
			timer.PSC = (SYSCLK / 1000 / freq) - 1;
		}
		
	protected:
		TIM_TypeDef &timer;
	};
}

void RCC_Init() {
	// Enable 8MHz HSE
	RCC->CR |= ((uint32_t)RCC_CR_HSEON);
	// Ready start HSE
	while(!(RCC->CR & RCC_CR_HSERDY));
	
	// Clock Flash memory
	FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;
	
	constexpr uint32_t AHBdivider = SYSCLK / AHBCLK;
	switch (AHBdivider) {
	case 1U : { RCC->CFGR |= RCC_CFGR_HPRE_DIV1; }
		break;
	case 2U : { RCC->CFGR |= RCC_CFGR_HPRE_DIV2; }
		break;
	case 4U : { RCC->CFGR |= RCC_CFGR_HPRE_DIV4; }
		break;
	}
	
	constexpr uint32_t APB1divider = SYSCLK / APB1CLK;
	switch (APB1divider) {
	case 1U : 
		RCC->CFGR |= RCC_CFGR_PPRE1_DIV1;
		break;
	case 2U : 
		RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
		break;
	case 4U :
		RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;
		break;
	}
	
	constexpr uint32_t APB2divider = SYSCLK / APB2CLK;
	switch (APB2divider) {
	case 1U : 
		RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;
		break;
	case 2U : 
		RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;
		break;
	case 4U :
		RCC->CFGR |= RCC_CFGR_PPRE2_DIV4;
		break;
	}
	
	RCC->CFGR &= ~RCC_CFGR_PLLMULL;
	RCC->CFGR &= ~RCC_CFGR_PLLXTPRE;
	
	// PLL source HSE 
	RCC->CFGR |= RCC_CFGR_PLLSRC;
	
	//RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE_DIV2;
	// PLL x9: clock = 8 MHz * 9 = 72 MHz
	RCC->CFGR |= RCC_CFGR_PLLMULL9;
	
	// enable PLL
	RCC->CR |= RCC_CR_PLLON;
	while ((RCC->CR & RCC_CR_PLLRDY) == 0) {}      							// wait till PLL is ready
	
	// clear SW bits
	RCC->CFGR &= ~RCC_CFGR_SW;
	// select source SYSCLK = PLL
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {} 		// wait till PLL is used
	
}
