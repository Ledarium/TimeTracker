#pragma once

#include <array>
#include <periph.hpp>
#include <task.h>

#define NOTE_C0  16
#define NOTE_CS0 17
#define NOTE_D0  18
#define NOTE_DS0 19
#define NOTE_E0  21
#define NOTE_F0  22
#define NOTE_FS0 23
#define NOTE_G0  24
#define NOTE_GS0 26
#define NOTE_A0  28
#define NOTE_AS0 29
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

const int pitches[] = {
	0x0,
	NOTE_C0,
	NOTE_CS0,
	NOTE_D0,
	NOTE_DS0,
	NOTE_E0,
	NOTE_F0,
	NOTE_FS0,
	NOTE_G0,
	NOTE_GS0,
	NOTE_A0,
	NOTE_AS0,
	NOTE_B0,
	NOTE_C1,
	NOTE_CS1,
	NOTE_D1,
	NOTE_DS1,
	NOTE_E1,
	NOTE_F1,
	NOTE_FS1,
	NOTE_G1,
	NOTE_GS1,
	NOTE_A1,
	NOTE_AS1,
	NOTE_B1,
	NOTE_C2,
	NOTE_CS2,
	NOTE_D2,
	NOTE_DS2,
	NOTE_E2,
	NOTE_F2,
	NOTE_FS2,
	NOTE_G2,
	NOTE_GS2,
	NOTE_A2,
	NOTE_AS2,
	NOTE_B2,
	NOTE_C3,
	NOTE_CS3,
	NOTE_D3,
	NOTE_DS3,
	NOTE_E3,
	NOTE_F3,
	NOTE_FS3,
	NOTE_G3,
	NOTE_GS3,
	NOTE_A3,
	NOTE_AS3,
	NOTE_B3,
	NOTE_C4,
	NOTE_CS4,
	NOTE_D4,
	NOTE_DS4,
	NOTE_E4,
	NOTE_F4,
	NOTE_FS4,
	NOTE_G4,
	NOTE_GS4,
	NOTE_A4,
	NOTE_AS4,
	NOTE_B4,
	NOTE_C5,
	NOTE_CS5,
	NOTE_D5,
	NOTE_DS5,
	NOTE_E5,
	NOTE_F5,
	NOTE_FS5,
	NOTE_G5,
	NOTE_GS5,
	NOTE_A5,
	NOTE_AS5,
	NOTE_B5,
	NOTE_C6,
	NOTE_CS6,
	NOTE_D6,
	NOTE_DS6,
	NOTE_E6,
	NOTE_F6,
	NOTE_FS6,
	NOTE_G6,
	NOTE_GS6,
	NOTE_A6,
	NOTE_AS6,
	NOTE_B6,
	NOTE_C7,
	NOTE_CS7,
	NOTE_D7,
	NOTE_DS7,
	NOTE_E7,
	NOTE_F7,
	NOTE_FS7,
	NOTE_G7,
	NOTE_GS7,
	NOTE_A7,
	NOTE_AS7,
	NOTE_B7,
	NOTE_C8,
	NOTE_CS8,
	NOTE_D8,
	NOTE_DS8,
	0xFF
};

class MusicPlayer {
public:
	enum returnCodes {
		OK,
		STOPPED,
		WRONG_HEADER,
		OUT_OF_LOOP,
		WRONG_BYTE
	};
	enum statusCodes {
		NOT_PLAYING,
		PLAYING
	};
	static constexpr uint8_t maxTonegens = 6;
	static constexpr uint8_t maxChannels = 2;
	static inline std::array <Periph::Timer, maxChannels> channels = { 
		Periph::Timer(*TIM2, Periph::OutPin(*GPIOA, 1, Periph::OutPin::AFpushpull, Periph::OutPin::MHz50)),
		Periph::Timer(*TIM3, Periph::OutPin(*GPIOA, 7, Periph::OutPin::AFpushpull, Periph::OutPin::MHz50))
	};
	static inline std::array <uint16_t, maxTonegens> tonegens = { 0, 0, 0, 0, 0, 0 };
	static inline std::array <uint16_t, maxTonegens> channelOut = { 0, 0, 0, 0, 0, 0 };
	
	inline auto Stop() {
		status = MusicPlayer::NOT_PLAYING;
		for (auto &channel : channels)
			channel.PWM_SetFrequency(Periph::Timer::PWM_MAX);
		
		for (auto &tonegen : tonegens) {
			tonegen = 0;
		}
		for (auto &out : channelOut) {
			out = 0;
		}
	}
	
	inline auto Play(const std::pair<uint8_t*, uint32_t> &track) {
		this->Stop();
		auto melody = track.first;
		auto melodySize = track.second;
		this->status = MusicPlayer::PLAYING;
		TickType_t xLastWakeTime;
		xLastWakeTime = xTaskGetTickCount();
		if (melody[0] != 'P' || melody[1] != 't')
		{
			return MusicPlayer::WRONG_HEADER;
		}
		for (int i = melody[2]; i < melodySize; i++) {
			if (status == MusicPlayer::PLAYING)
			{
				//melody[2] stores the length of header
			   /*   If the high-order bit of the byte is 0, it is a command to delay for a while until
			   the next note change.  The other 7 bits and the 8 bits of the following byte are
			   interpreted as a 15-bit big-endian integer that is the number of milliseconds to
			   wait before processing the next command.  For example, 07 D0
			   would cause a delay of 0x07d0 = 2000 decimal millisconds, or 2 seconds.  Any tones
			   that were playing before the delay command will continue to play.*/
			   auto byte = melody[i];
				if ((melody[i] >> 7) == 0) {
					uint32_t delay = (melody[i] << 8) + melody[i + 1];
					vTaskDelayUntil(&xLastWakeTime, delay);
					i++;
				}
				else {
					switch (melody[i] >> 4) {
					case 0xF : 
						[[fallthrough]];
					case 0xE : {
							this->Stop();
							return MusicPlayer::OK;
						}
					case 0x8 : {
							// 8t Stop playing the note on tone generator t
							auto generator = melody[i] & 0x0F;
							tonegens[generator] = 0;
							break;
						}
						/*    9t nn [vv]
						Start playing note nn on tone generator t, replacing any previous note.
						Generators are numbered starting with 0. The note numbers are the MIDI
						numbers for the chromatic scale, with decimal 69 being Middle A (440 Hz).
						If the -v option was given, the third byte specifies the note volume.*/
					case 0x9 : {
							auto generator = melody[i] & 0x0F;
							tonegens[generator] = pitches[melody[i + 1]];
							i++;
							break;
						}
						/* Ct ii  Change tone generator t to play instrument ii from now on. This will only
						be generated if the -i option was given.*/
					case 0xC : {
							//not impemented
							i++;
							break;
						}
					default: {
							this->Stop();
							return MusicPlayer::WRONG_BYTE;
						}
					}
				}
				//placeholder
				for(auto &channel : channelOut)
					channel = 0;
			
				uint8_t activeChannels = 0;
				uint8_t currentChannel = 0;
				for (auto i = 0; i < tonegens.size(); i++) {
					if (tonegens[i] && activeChannels < maxChannels) {
						activeChannels++;
						channelOut[currentChannel++] = tonegens[i];
					}
				}
			
				for (auto i = 0; i < channels.size(); i++) {
					channels[i].PWM_SetFrequency(channelOut[i] ? channelOut[i] : Periph::Timer::PWM_MAX);
				}
			
			}
			else {
				this->Stop();
				return STOPPED;
			}
		}
		return OUT_OF_LOOP;
	}
	
	MusicPlayer() {
		for (auto &channel : channels)
			channel.PWM_Init();
		channels[1].PWM_SetParam(1000, 250);
	}
	
	~MusicPlayer() {
		this->Stop();
	}
protected:
	bool status = MusicPlayer::NOT_PLAYING;
};