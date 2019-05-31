#include <main.hpp>

static std::array<char, 8> buffer = { 'B', 'a', 'a', 'a', 'a', 'a', '\r', '\n' };
constexpr uint32_t baudrate = 115200;

using namespace Periph;
using namespace EmbeddedResources;
using namespace std;

static USART_1 usart = USART_1(OutPin(*GPIOA, 9, OutPin::AFopendrain, OutPin::MHz50), InPin(*GPIOA, 10, InPin::floating), baudrate, buffer);
static Led led1(OutPin(*GPIOA, 3, OutPin::pushpull, OutPin::MHz10));
static Led led2(OutPin(*GPIOA, 0, OutPin::pushpull, OutPin::MHz10));
static Button bigButton(InPin(*GPIOA, 2, InPin::pulldown), Button::NO);
static Button plusButton(InPin(*GPIOB, 15, InPin::pulldown), Button::NO);
static Button minusButton(InPin(*GPIOB, 12, InPin::pulldown), Button::NO);

static GameEngine ge = GameEngine();

static MusicPlayer mp = MusicPlayer();
static array<pair<uint8_t*, uint32_t>, 6> tracks =  {{ 
	{ (uint8_t*)Resources_imperial_march_bin.data(), (uint32_t)Resources_imperial_march_bin.size() },
	{ (uint8_t*)Resources_main_theme_bin.data(), (uint32_t)Resources_main_theme_bin.size() },
	{ (uint8_t*)Resources_boulevard_of_broken_dreams_bin.data(), (uint32_t)Resources_boulevard_of_broken_dreams_bin.size() },
	{ (uint8_t*)Resources_gravity_falls_soundtrack_bin.data(), (uint32_t)Resources_gravity_falls_soundtrack_bin.size() },
	{ (uint8_t*)Resources_super_mario_bin.data(), (uint32_t)Resources_super_mario_bin.size() },
	{ (uint8_t*)Resources_tetris_bin.data(), (uint32_t)Resources_tetris_bin.size() }
}};

static TimerHandle_t secondsTimerHandle = NULL;
static TaskHandle_t xMusicHandle = NULL;

void MCO_out() {
	OutPin(*GPIOA, 8, OutPin::AFopendrain, OutPin::MHz50);
	RCC->CFGR |= RCC_CFGR_MCO_PLLCLK_DIV2;  // select MSO source clock PLL/2
}

int main() {
	RCC_Init();
#ifdef DEBUG
	MCO_out();
#endif // DEBUG
	logic();
}

void logic()
{
	led1.SetHigh();
	usart.Send();
	xTaskCreate(vTaskLed, "LED", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
//	xTaskCreate(vTaskStateMachine, "FSM", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(vTaskPlayerSetup, "Player", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	vTaskStartScheduler();
	
	while(1) {
	
	};
}

void vTaskPlayerSetup(void *parameter)
{
	while (1)
	{
		if (plusButton.PressedDebounced())
		{
			GameEngine::AddPlayer();
		}
		if (minusButton.PressedDebounced())
		{
			GameEngine::RemovePlayer();
		}
		
#ifdef DEBUG
		for (auto i = 0; i < GameEngine::maxPlayers; i++)
			buffer[i] = GameEngine::playerScore[i];
		usart.Send();
#endif // DEBUG
		
		
		if (GameEngine::activePlayers > 1 && bigButton.PressedDebounced())
			break;
		
		vTaskDelay(5);
	}
	xTaskCreate(vTaskTimerSetup, "TaskTimerSetup", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	vTaskDelete(NULL);
}

void vTaskTimerSetup(void *parameter) {
	while (1)
	{
		if (plusButton.PressedDebounced())
		{
			GameEngine::IncrementTurnTime();
		}
		else if (minusButton.PressedDebounced())
		{
			GameEngine::DecrementTurnTime();
		}
		else vTaskDelay(10);
		
		if (bigButton.PressedDebounced()) {
			break;
		}
		vTaskDelay(10);
		
#ifdef DEBUG
		auto [minutes, seconds] = GameEngine::GetTimerValue();
		buffer[0] = minutes;
		buffer[1] = seconds;
		usart.Send();
#endif // DEBUG
		
	}

	secondsTimerHandle = xTimerCreate("SecondsTimer",
		pdMS_TO_TICKS(1000), //counts 1 sec
		pdTRUE, //auto-reload
		NULL, //not assigning ID 
		vTimerCallback // function to call after timer expires
		); 

	xTaskCreate(vTaskConfig, "Config", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	vTaskDelete(NULL);
}

void vTaskConfig(void *parameter) {
	while (1)
	{
		if (plusButton.PressedDebounced())
			GameEngine::countScores = !GameEngine::countScores;
		if (minusButton.PressedDebounced())
		// show round number or change the way it counts
			GameEngine::countScores = !GameEngine::countScores;
		if (bigButton.PressedDebounced()) {
			break;
		}
		vTaskDelay(10);
	}
	xTaskCreate(vTaskTurn, "TaskTurn", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	vTaskDelete(NULL);
}

void vTaskTurn(void *parameter) {
	xTimerReset(secondsTimerHandle, 0);
	
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	
	while (1)
	{
		if (plusButton.PressedDebounced()) {
			xTaskCreate(vTaskTimerSetup, "TaskTimerSetup", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
			break;
		}
		else if (minusButton.PressedDebounced()) {
			xTaskCreate(vTaskConfig, "Config", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
			break;
		}
		else if (bigButton.PressedDebounced()) {
			xTaskCreate(vTaskTurnEnd, "TaskTurnEnd", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
			break;
		} 
		
		else if(GameEngine::timerValue == 0 && xMusicHandle == NULL) 
		{
			xTaskCreate(vTaskOvertime, "vTaskOvertime", configMINIMAL_STACK_SIZE, NULL, 1, &xMusicHandle);
		}
		vTaskDelayUntil(&xLastWakeTime,100);
		led2.Toggle();
		
#ifdef DEBUG
		auto[minutes, seconds] = GameEngine::GetTimerValue();
		buffer[0] = minutes;
		buffer[1] = seconds;
		buffer[2] = GameEngine::currentPlayer;
		buffer[4] = GameEngine::playerScore[0];
		buffer[5] = GameEngine::playerScore[1];
		buffer[6] = GameEngine::playerScore[2];
		usart.Send();
#endif // DEBUG
		
	}
	xTimerStop(secondsTimerHandle, 0);
	GameEngine::ResetTurnTimer();
	if (xMusicHandle) {
		vTaskDelete(xMusicHandle);
		mp.Stop();
		xMusicHandle = NULL;
	}
	vTaskDelete(NULL);
}

void vTaskTurnEnd(void *parameter) {
	if (GameEngine::countScores) {
		int32_t delta = 0;
		while (1) {
			if (bigButton.PressedDebounced()) {
				GameEngine::ChangeScore(delta);
				break;
			}
			else if (plusButton.PressedDebounced())
			{
				delta++;
			}
			else if (minusButton.PressedDebounced())
			{
				delta--;
			}
			vTaskDelay(10);
		}
	}
	GameEngine::NextPlayer();
	xTaskCreate(vTaskTurn, "TaskTurn", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	vTaskDelete(NULL);
}

void vTimerCallback(TimerHandle_t xTimer) {
	if (GameEngine::timerValue > 0)
		GameEngine::timerValue--;
}

void vTaskOvertime(void *parameter) {
	while (1)
	{
		led2.SetHigh();
		mp.Play(tracks[3]);
		led2.SetLow();
		vTaskDelay(1000);
	}
}

/*
void vTaskStateMachine(void *parameter) {
	while (1)
	{
		switch (buttonState)
		{
		case StateIdle:
			{
				if (bigButton.PressedDebounced()) 
					buttonState = StateTurnPressed;
				else if (minusButton.PressedDebounced()) 
				{
					if (plusButton.PressedDebounced())
						buttonState = StatePlusMinusPressed;
					else
						buttonState = StateMinusPressed;
				}
				else if (plusButton.PressedDebounced()) 
					buttonState = StatePlusPressed;
				break;
			}
		default:
			{
				vTaskDelay(100);
				break;
			}
		}
	}
}
*/

void vTaskLed(void *parameter) {
	while (1)
	{
		led1.Toggle();
		vTaskDelay(1000);
	}
}
