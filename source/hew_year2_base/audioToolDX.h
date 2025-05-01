// =======================================================
// audioToolDX.h
// 
// DirectX���̃I�[�f�B�I�v���C���[
// 
// ��ҁF鰕��r�i�K�C�@�}���`�����j�@2024/08/06
// =======================================================
#ifndef _AUDIO_TOOL_DX_H
#define _AUDIO_TOOL_DX_H

#include "audioTool.h"
#include "resourceTool.h"
#include <xaudio2.h>

namespace MG {
	// =======================================================
	// DirectX���̃I�[�f�B�I�v���C���[
	// =======================================================
	class AudioPlayerDX : public AudioPlayer {
	protected:
		IXAudio2SourceVoice* sourceVoice;
	public:
		AudioPlayerDX(IXAudio2SourceVoice* sourceVoice, Audio* audio, bool loop = false);
		~AudioPlayerDX();
		void Start();
		void Stop();
		void SetLoop(bool loop);
		void SetVolume(float volume);
		void SetSpeed(float speed);
		bool IsFinished() const;
	};


	// =======================================================
	// DirectX���̃I�[�f�B�I�c�[��
	// =======================================================
	class AudioToolDX : public AudioTool {
	private:
		IXAudio2* m_Xaudio;
		IXAudio2MasteringVoice* m_MasteringVoice;
	public:
		AudioToolDX();
		~AudioToolDX();
		IXAudio2* GetXAudio();
		AudioPlayer* CreateAudioPlayer(Audio* audio, bool loop = false);
	};

} // namespace MG

#endif 