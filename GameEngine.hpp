#pragma once
#include <array>

//c++17
class GameEngine {
public:
	static constexpr uint8_t maxPlayers = 6;
	inline static uint8_t activePlayers;
	inline static std::array<int32_t, maxPlayers> playerScore;
	//static std::array<int32_t, maxPlayers> playerPort;
	inline static bool countScores;
	inline static uint8_t currentPlayer;
	
	static inline uint8_t timerValue = 0;
	
	GameEngine() {
		activePlayers = 0;
		playerScore = { -1, -1, -1, -1, -1, -1 };
		//playerPort = { -1, -1, -1, -1, -1, -1 };
		countScores = false;
		currentPlayer = 0;
		timerValue = turnTimeSeconds;
	}
	
	static inline void AddPlayer() {
		if (activePlayers < maxPlayers) {
			playerScore[activePlayers++] = 0;
		}
	}	
	
	static inline void RemovePlayer() {
		if (activePlayers > 0) {
			playerScore[activePlayers--] = -1;
		}
	}
	
	static inline auto GetCurrentPlayer() {
		return currentPlayer;
	}
	
	static inline auto GetTimerValue() {
		return std::pair{ timerValue / 60, timerValue % 60 };
	}
	
	static inline void IncrementTurnTime() {
		if (turnTimeSeconds + timerStep <= timerMax)
			turnTimeSeconds += timerStep;
		else
			turnTimeSeconds = timerStep;
		timerValue = turnTimeSeconds;
	}
	
	static inline void DecrementTurnTime() {
		if (turnTimeSeconds > timerStep)
			turnTimeSeconds -= timerStep;
		else 
			turnTimeSeconds = timerMax;
		timerValue = turnTimeSeconds;
	}
	
	static inline void ChangeScore(int8_t delta) {
		playerScore[currentPlayer] += delta;
	}
	
	static inline void ResetTurnTimer() {
		timerValue = turnTimeSeconds;
	}
	
	static inline void NextPlayer() {
		currentPlayer = (currentPlayer + 1) % activePlayers;
		ResetTurnTimer();
	}
protected:
	static inline uint8_t turnTimeSeconds = 5;
	static constexpr uint8_t timerMax = 180;
	static constexpr uint8_t timerStep = 5;
};