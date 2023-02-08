#include "audio.h"
#include <cstring>

Hyper_BandCHIP::Audio::Audio() : attack_time(0.005), decay_time(0.005), sustain_level(1.0), release_time(0.200), current_state_time(0.0), start_level(0.0), current_level(0.0), frequency(220.0), current_period(0.0), egs_accumulator(0.0), EGS(EnvelopeGeneratorState::Off), processing(true)
{
	egs_tp = high_resolution_clock::now();
	SDL_AudioSpec desired;
	SDL_zero(desired);
	// desired.freq = 48000;
	desired.freq = 192000;
	desired.channels = 2;
	// desired.samples = 512;
	desired.samples = 4096;
	desired.format = AUDIO_S32;
	/*
	desired.callback = &AudioHandler;
	desired.userdata = this;
	*/
	AudioDevice = SDL_OpenAudioDevice(nullptr, 0, &desired, &audio_spec, 0);
	SDL_PauseAudioDevice(AudioDevice, 0);
	ProcessingThread = std::thread(Audio::AudioProcessor, this);
}

Hyper_BandCHIP::Audio::~Audio()
{
	processing = false;
	ProcessingThread.join();
	SDL_PauseAudioDevice(AudioDevice, 1);
	SDL_CloseAudioDevice(AudioDevice);
}

void Hyper_BandCHIP::Audio::SetEnvelopeGeneratorState(EnvelopeGeneratorState NewState)
{
	EGS = NewState;
	current_state_time = 0.0;
	start_level = current_level;
}

Hyper_BandCHIP::EnvelopeGeneratorState Hyper_BandCHIP::Audio::GetEnvelopeGeneratorState() const
{
	return EGS;
}

void Hyper_BandCHIP::Audio::AudioProcessor(Audio* audio)
{
	std::chrono::high_resolution_clock::time_point audio_tp = high_resolution_clock::now();
	double audio_accumulator = 0.0;
	while (audio->processing)
	{
		AudioFrame current_frame;
		for (size_t i = 0; i < current_frame.size(); )
		{
			bool idle = true;
			std::chrono::high_resolution_clock::time_point current_tp = high_resolution_clock::now();
			std::chrono::duration<double> delta_time = current_tp - audio_tp;
			if (delta_time.count() > 0.25)
			{
				delta_time = std::chrono::duration<double>(0.25);
			}
			audio_accumulator += delta_time.count();
			audio_tp = current_tp;
			while (audio_accumulator >= 1.0 / static_cast<double>(audio->audio_spec.freq) && i < current_frame.size())
			{
				if (idle)
				{
					idle = false;
				}
				audio_accumulator -= 1.0 / static_cast<double>(audio->audio_spec.freq);
				double value = 0.0;
				if (audio->current_period < 0.5 / audio->frequency)
				{
					value = 1.0;
				}
				else
				{
					value = -1.0;
				}
				value *= (0.3 * audio->current_level);
				int final_value = (value >= 0.0) ? static_cast<int>(static_cast<double>(INT32_MAX) * value) : static_cast<int>(-(static_cast<double>(INT32_MIN)) * value);
				current_frame[i] = final_value;
				current_frame[i + 1] = final_value;
				audio->current_period += 1.0 / static_cast<double>(audio->audio_spec.freq);
				if (audio->current_period >= 1.0 / audio->frequency)
				{
					audio->current_period -= 1.0 / audio->frequency;
				}
				switch (audio->EGS)
				{
				case EnvelopeGeneratorState::Off:
				{
					if (audio->current_level != 0.0)
					{
						audio->current_level = 0.0;
					}
					break;
				}
				case EnvelopeGeneratorState::Attack:
				{
					bool auto_change = false;
					if (audio->attack_time > 0.0)
					{
						audio->current_state_time += 1.0 / static_cast<double>(audio->audio_spec.freq);
						audio->current_level = audio->start_level + ((1.0 - audio->start_level) * (audio->current_state_time / audio->attack_time));
					}
					else
					{
						audio->current_level = 1.0;
						auto_change = true;
					}
					if (audio->current_state_time >= audio->attack_time)
					{
						audio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Decay);
					}
					if (!auto_change)
					{
						break;
					}
				}
				case EnvelopeGeneratorState::Decay:
				{
					if (audio->decay_time > 0.0)
					{
						audio->current_state_time += 1.0 / static_cast<double>(audio->audio_spec.freq);
						audio->current_level = audio->start_level - ((audio->start_level - audio->sustain_level) * (audio->current_state_time / audio->decay_time));
					}
					else
					{
						audio->current_level = audio->sustain_level;
					}
					if (audio->current_state_time >= audio->decay_time)
					{
						audio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Sustain);
					}
					break;
				}
				case EnvelopeGeneratorState::Sustain:
				{
					if (audio->current_level != audio->sustain_level)
					{
						audio->current_level = audio->sustain_level;
					}
					break;
				}
				case EnvelopeGeneratorState::Release:
				{
					if (audio->release_time > 0.0)
					{
						audio->current_state_time += 1.0 / static_cast<double>(audio->audio_spec.freq);
						audio->current_level = audio->start_level - (audio->start_level * (audio->current_state_time / audio->release_time));
					}
					else
					{
						audio->current_level = 0.0;
					}
					if (audio->current_state_time >= audio->release_time)
					{
						audio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Off);
					}
					break;
				}
				}
				i += 2;
			}
			if (idle)
			{
				SDL_Delay(1);
			}
		}
		constexpr size_t buffer_max = current_frame.size() * sizeof(int) * 2; // Allowing up to two frames in the queue should be more than sufficient.
		while (SDL_GetQueuedAudioSize(audio->AudioDevice) >= buffer_max) // Necessary for handling playback devices that likely have higher latency (which leads to audio that eventually becomes delayed and possibly running out of memory if not handled).
		{
			SDL_Delay(10);
			audio_tp = high_resolution_clock::now(); // Audio Time Point must be reset after the delay in order to give the playback device more time to process queued audio.  Otherwise, it'll keep building up each frame.  Does not impair audio quality.
		}
		SDL_QueueAudio(audio->AudioDevice, current_frame.data(), current_frame.size() * sizeof(int));
	}
}

/*
void Hyper_BandCHIP::Audio::AudioHandler(void *userdata, Uint8 *stream, int len)
{
	high_resolution_clock::time_point start_tp = high_resolution_clock::now();
	Audio *CurrentAudio = static_cast<Audio *>(userdata);
	struct AudioChannelData
	{
		int left;
		int right;
	};
	for (int i = 0; i < len / sizeof(AudioChannelData); ++i)
	{
		AudioChannelData CurrentAudioData;
		double value = 0.0;
		if (CurrentAudio->current_period < 0.5 / CurrentAudio->frequency)
		{
			value = 1.0;
		}
		else
		{
			value = -1.0;
		}
		value *= (0.3 * CurrentAudio->current_level);
		if (value >= 0.0)
		{
			int i_value = static_cast<int>(2147483647.0 * value);
			CurrentAudioData = { i_value, i_value };
		}
		else
		{
			int i_value = static_cast<int>(2147483648.0 * value);
			CurrentAudioData = { i_value, i_value };
		}
		memcpy(&stream[i * sizeof(AudioChannelData)], &CurrentAudioData, sizeof(AudioChannelData));
		CurrentAudio->current_period += 1.0 / static_cast<double>(CurrentAudio->audio_spec.freq);
		if (CurrentAudio->current_period >= 1.0 / CurrentAudio->frequency)
		{
			CurrentAudio->current_period -= 1.0 / CurrentAudio->frequency;
		}
		switch (CurrentAudio->EGS)
		{
			case EnvelopeGeneratorState::Off:
			{
				if (CurrentAudio->current_level != 0.0)
				{
					CurrentAudio->current_level = 0.0;
				}
				break;
			}
			case EnvelopeGeneratorState::Attack:
			{
				bool auto_change = false;
				if (CurrentAudio->attack_time > 0.0)
				{
					CurrentAudio->current_state_time += 1.0 / static_cast<double>(CurrentAudio->audio_spec.freq);
					CurrentAudio->current_level = CurrentAudio->start_level + ((1.0 - CurrentAudio->start_level) * (CurrentAudio->current_state_time / CurrentAudio->attack_time));
				}
				else
				{
					CurrentAudio->current_level = 1.0;
					auto_change = true;
				}
				if (CurrentAudio->current_state_time >= CurrentAudio->attack_time)
				{
					CurrentAudio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Decay);
				}
				if (!auto_change)
				{
					break;
				}
			}
			case EnvelopeGeneratorState::Decay:
			{
				if (CurrentAudio->decay_time > 0.0)
				{
					CurrentAudio->current_state_time += 1.0 / static_cast<double>(CurrentAudio->audio_spec.freq);
					CurrentAudio->current_level = CurrentAudio->start_level - ((CurrentAudio->start_level - CurrentAudio->sustain_level) * (CurrentAudio->current_state_time / CurrentAudio->decay_time));
				}
				else
				{
					CurrentAudio->current_level = CurrentAudio->sustain_level;
				}
				if (CurrentAudio->current_state_time >= CurrentAudio->decay_time)
				{
					CurrentAudio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Sustain);
				}
				break;
			}
			case EnvelopeGeneratorState::Sustain:
			{
				if (CurrentAudio->current_level != CurrentAudio->sustain_level)
				{
					CurrentAudio->current_level = CurrentAudio->sustain_level;
				}
				break;
			}
			case EnvelopeGeneratorState::Release:
			{
				if (CurrentAudio->release_time > 0.0)
				{
					CurrentAudio->current_state_time += 1.0 / static_cast<double>(CurrentAudio->audio_spec.freq);
					CurrentAudio->current_level = CurrentAudio->start_level - (CurrentAudio->start_level * (CurrentAudio->current_state_time / CurrentAudio->release_time));
				}
				else
				{
					CurrentAudio->current_level = 0.0;
				}
				if (CurrentAudio->current_state_time >= CurrentAudio->release_time)
				{
					CurrentAudio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Off);
				}
				break;
			}
		}
	}
}
*/