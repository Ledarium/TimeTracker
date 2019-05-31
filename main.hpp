#ifndef MAIN_H
#define MAIN_H

#include "stm32f1xx.h"

constexpr uint32_t SYSCLK = CLOCK;
constexpr uint32_t AHBCLK = SYSCLK;
constexpr uint32_t APB1CLK = AHBCLK / 2;
constexpr uint32_t APB2CLK = AHBCLK;

#include "FreeRTOS.h"
#include "task.h" 
#include "queue.h"
#include "timers.h"
#include <EmbeddedResources.h>
#include <utils.hpp>
#include <periph.hpp>
#include <Music.hpp>
#include <GameEngine.hpp>
#include <random>

void MCO_out();

void logic();

void vTaskLed(void *parameter);
void vTaskButton(void *parameter);
void vTaskDisplay(void *parameter);

void vTaskBeep(void *parameter);
void vTaskOvertime(void *parameter);

void vTaskStateMachine(void *parameter);

void vTaskPlayerSetup(void *parameter);
void vTaskConfig(void *parameter);
void vTaskTimerSetup(void *parameter);
void vTaskTurn(void *parameter);
void vTaskTurnEnd(void *parameter);

void vTimerCallback(TimerHandle_t xTimer);

#endif // !MAIN_H