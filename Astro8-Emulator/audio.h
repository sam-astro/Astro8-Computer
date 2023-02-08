#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <chrono>
#include <array>
#include <cstring>
#include <cmath>
#include <chrono>
#include <thread>
#include <SDL.h>

using namespace std::chrono;

namespace Hyper_BandCHIP
{
	enum class EnvelopeGeneratorState { Off, Attack, Decay, Sustain, Release };

	class Audio
	{
		using AudioFrame = std::array<int, 8192>;

	public:
		Audio();
		~Audio();
		void SetEnvelopeGeneratorState(EnvelopeGeneratorState NewState);
		EnvelopeGeneratorState GetEnvelopeGeneratorState() const;
		static void AudioProcessor(Audio* audio);
		// static void AudioHandler(void *userdata, Uint8 *stream, int len);  Deprecated, replaced by a better audio processor that handles audio in real-time on a completely separate thread.
	private:
		double attack_time;
		double decay_time;
		double sustain_level;
		double release_time;
		double current_state_time;
		double start_level;
		double current_level;
		double frequency;
		double current_period;
		high_resolution_clock::time_point egs_tp;
		double egs_accumulator;
		EnvelopeGeneratorState EGS;
		bool processing;
		SDL_AudioSpec audio_spec;
		SDL_AudioDeviceID AudioDevice;
		std::thread ProcessingThread;
	};

	template <unsigned char channels, unsigned char voices>
	class SampleAudio
	{
		using AudioFrame = std::array<int, 4096 * channels>;

	public:
		SampleAudio() : processing(true)
		{
			for (unsigned char i = 0; i < voices; ++i)
			{
				memset(voice_list[i].audio_buffer.data(), 0x00, 16);
				for (unsigned char c = 0; c < channels; ++c)
				{
					voice_list[i].channel_output[c] = true;
				}
				voice_list[i].playback_rate = 4000.0;
				voice_list[i].volume = 1.0;
				voice_list[i].pos = 0;
				voice_list[i].b_offset = 0;
				voice_list[i].time = 0.0;
				voice_list[i].pause = true;
			}
			SDL_AudioSpec desired;
			SDL_zero(desired);
			desired.freq = 192000;
			desired.channels = channels;
			// desired.samples = 1600;
			desired.samples = 4096;
			desired.format = AUDIO_S32;
			/*
			desired.callback = &AudioHandler;
			desired.userdata = this;
			*/
			AudioDevice = SDL_OpenAudioDevice(nullptr, 0, &desired, &audio_spec, 0);
			SDL_PauseAudioDevice(AudioDevice, 0);
			ProcessingThread = std::thread(SampleAudio<channels, voices>::AudioProcessor, this);
		}
		~SampleAudio()
		{
			processing = false;
			ProcessingThread.join();
			SDL_PauseAudioDevice(AudioDevice, 1);
			SDL_CloseAudioDevice(AudioDevice);
		}
		void InitializeAudioBuffer(unsigned char voice = 0)
		{
			if (voice < voices)
			{
				memset(voice_list[voice].audio_buffer.data(), 0x00, 16);
			}
		}
		void Reset(unsigned char voice = 0)
		{
			if (voice < voices)
			{
				voice_list[voice].pos = 0;
				voice_list[voice].b_offset = 0;
				voice_list[voice].time = 0.0;
			}
		}
		void SetChannelOutput(unsigned char channel_mask, unsigned char voice)
		{
			if (voice < voices)
			{
				for (unsigned char c = 0; c < channels; ++c)
				{
					voice_list[voice].channel_output[c] = (channel_mask & (0x01 << c));
				}
			}
		}
		void SetPlaybackRate(unsigned char rate, unsigned char voice = 0)
		{
			if (voice < voices)
			{
				voice_list[voice].playback_rate = 4000.0 * std::pow(2.0, ((static_cast<double>(rate) - 64.0) / 48.0));
			}
		}
		void SetVolume(unsigned char volume, unsigned char voice)
		{
			if (voice < voices)
			{
				voice_list[voice].volume = static_cast<double>(volume) / 255.0;
			}
		}
		void CopyToAudioBuffer(const unsigned char* memory, unsigned short start_address, unsigned char voice = 0)
		{
			if (voice < voices)
			{
				for (unsigned char i = 0; i < 16; ++i)
				{
					voice_list[voice].audio_buffer[i] = memory[static_cast<unsigned short>(start_address + i)];
				}
			}
		}
		void PauseAudio(bool pause, unsigned char voice = 0)
		{
			if (voice < voices)
			{
				if (voice_list[voice].pause != pause)
				{
					voice_list[voice].pause = pause;
				}
			}
		}
		bool IsPaused(unsigned char voice = 0) const
		{
			return voice_list[voice].pause;
		}
		static void AudioProcessor(SampleAudio<channels, voices>* Audio)
		{
			std::chrono::high_resolution_clock::time_point audio_tp = high_resolution_clock::now();
			double audio_accumulator = 0.0;
			while (Audio->processing)
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
					while (audio_accumulator >= 1.0 / static_cast<double>(Audio->audio_spec.freq) && i < current_frame.size())
					{
						if (idle)
						{
							idle = false;
						}
						audio_accumulator -= 1.0 / static_cast<double>(Audio->audio_spec.freq);
						for (unsigned char current_channel = 0; current_channel < channels; ++current_channel)
						{
							int value = 0;
							for (unsigned char v = 0; v < voices; ++v)
							{
								if (!Audio->voice_list[v].pause)
								{
									if (Audio->voice_list[v].channel_output[current_channel])
									{
										if (Audio->voice_list[v].audio_buffer[Audio->voice_list[v].pos] & (0x80 >> (Audio->voice_list[v].b_offset)))
										{
											int tmp = value + static_cast<int>(static_cast<double>(INT32_MAX) * Audio->voice_list[v].volume);
											if (tmp < value)
											{
												tmp = INT32_MAX;
											}
											value = tmp;
										}
									}
									if (current_channel == channels - 1)
									{
										Audio->voice_list[v].time += 1.0 / static_cast<double>(Audio->audio_spec.freq);
										double current_playback_rate = 1.0 / Audio->voice_list[v].playback_rate;
										while (Audio->voice_list[v].time >= current_playback_rate)
										{
											Audio->voice_list[v].time -= current_playback_rate;
											++Audio->voice_list[v].b_offset;
											if (Audio->voice_list[v].b_offset > 7)
											{
												Audio->voice_list[v].b_offset = 0;
												if (Audio->voice_list[v].pos < 15)
												{
													++Audio->voice_list[v].pos;
												}
												else
												{
													Audio->voice_list[v].pos = 0;
												}
											}
										}
									}
								}
							}
							current_frame[i] = static_cast<int>(static_cast<double>(value) * 0.20);
							++i;
						}
					}
					if (idle)
					{
						SDL_Delay(1);
					}
				}
				constexpr size_t buffer_max = current_frame.size() * sizeof(int) * 2; // Allowing up to two frames in the queue should be more than sufficient.
				while (SDL_GetQueuedAudioSize(Audio->AudioDevice) >= buffer_max) // Necessary for handling playback devices that likely have higher latency (which leads to audio that eventually becomes delayed and possibly running out of memory if not handled).
				{
					SDL_Delay(10);
					audio_tp = high_resolution_clock::now(); // Audio Time Point must be reset after the delay in order to give the playback device more time to process queued audio.  Otherwise, it'll keep building up each frame.  Does not impair audio quality.
				}
				SDL_QueueAudio(Audio->AudioDevice, current_frame.data(), current_frame.size() * sizeof(int));
			}
		}
		/*
		static void AudioHandler(void *userdata, Uint8 *stream, int len)  Deprecated, replaced by a better audio processor that handles audio in real-time on a completely separate thread.
		{
			SampleAudio<channels, voices> *Audio = static_cast<SampleAudio<channels, voices> *>(userdata);
			for (int i = 0; i < len / sizeof(short); ++i)
			{
				int value = 0;
				unsigned char current_channel = static_cast<unsigned char>(i % channels);
				for (unsigned char v = 0; v < voices; ++v)
				{
					if (!Audio->voice_list[v].pause)
					{
						if (Audio->voice_list[v].channel_output[current_channel])
						{
							if (Audio->voice_list[v].audio_buffer[Audio->voice_list[v].pos] & (0x80 >> (Audio->voice_list[v].b_offset)))
							{
								int tmp = value + static_cast<short>(static_cast<double>(INT16_MAX) * Audio->voice_list[v].volume);
								if (tmp < value)
								{
									tmp = INT16_MAX;
								}
								value = tmp;
							}
						}
						if (current_channel == channels - 1)
						{
							Audio->voice_list[v].time += 1.0 / static_cast<double>(Audio->audio_spec.freq);
							double current_playback_rate = 1.0 / Audio->voice_list[v].playback_rate;
							while (Audio->voice_list[v].time >= current_playback_rate)
							{
								Audio->voice_list[v].time -= current_playback_rate;
								++Audio->voice_list[v].b_offset;
								if (Audio->voice_list[v].b_offset > 7)
								{
									Audio->voice_list[v].b_offset = 0;
									if (Audio->voice_list[v].pos < 15)
									{
										++Audio->voice_list[v].pos;
									}
									else
									{
										Audio->voice_list[v].pos = 0;
									}
								}
							}
						}
					}
				}
				value = static_cast<short>(static_cast<double>(value) * 0.20);
				memcpy(&stream[i * sizeof(short)], &value, sizeof(short));
			}
		}
		*/
	private:
		struct
		{
			std::array<unsigned char, 16> audio_buffer;
			std::array<bool, channels> channel_output;
			double playback_rate;
			double volume;
			int pos;
			unsigned char b_offset;
			double time;
			bool pause;
		} voice_list[voices];
		bool processing;
		SDL_AudioSpec audio_spec;
		SDL_AudioDeviceID AudioDevice;
		std::thread ProcessingThread;
	};
}

#endif