// =======================================================
// audioTool.h
// 
// ��ҁF鰕��r�i�K�C�@�}���`�����j�@2024/08/06
// =======================================================
#ifndef _AUDIO_TOOL_H
#define _AUDIO_TOOL_H

#include "MGCommon.h"

namespace MG {
	// =======================================================
	// �I�[�f�B�I�c�[���A���ۃN���X
	// =======================================================
	class AudioTool {
	protected:
	public:
		virtual AudioPlayer* CreateAudioPlayer(Audio* audio, bool loop = false) = 0;
	};


	// =======================================================
	// �I�[�f�B�I�v���C���[�A���ۃN���X
	// =======================================================
	class AudioPlayer {
	protected:
		Audio* audio;
		bool loop;
	public:
		AudioPlayer(Audio* audio, bool loop = false);
		virtual ~AudioPlayer();
		void SetAudio(Audio* audio);
		virtual void SetLoop(bool loop);
		virtual std::string GetType();
		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual void SetVolume(float volume) = 0;
		virtual void SetSpeed(float speed) = 0;
		virtual bool IsFinished() const = 0;
	};

} // namespace MG

#endif 