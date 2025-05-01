// =======================================================
// gameObjectAudio.cpp
// 
// �I�[�f�B�I�Q�[���I�u�W�F�N�g
// 
// ��ҁF鰕��r�i�K�C�@�}���`�����j�@2024/11/18
// =======================================================
#include "gameObjectAudio.h"
#include "audioTool.h"

namespace MG {

	GameObjectAudio::GameObjectAudio(Audio* audio, bool loop) : GameObject()
	{
		audioPlayer = GetAudioTool()->CreateAudioPlayer(audio, loop);
	}

	void GameObjectAudio::Draw() {}
	void GameObjectAudio::Update() {}

	GameObjectAudio::~GameObjectAudio()
	{
		delete audioPlayer;
		audioPlayer = nullptr;
	}

	void GameObjectAudio::Play()
	{
		audioPlayer->Start();
	}

	void GameObjectAudio::Stop()
	{
		audioPlayer->Stop();
	}

	void GameObjectAudio::SetLoop(bool loop)
	{
		audioPlayer->SetLoop(loop);
	}

	void GameObjectAudio::SetVolume(float volume)
	{
		audioPlayer->SetVolume(volume);
	}

	void GameObjectAudio::SetSpeed(float speed)
	{
		audioPlayer->SetSpeed(speed);
	}

	bool GameObjectAudio::IsFinished() const
	{
		return audioPlayer->IsFinished();
	}

} // namespace MG