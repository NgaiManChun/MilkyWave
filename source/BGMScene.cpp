#include "BGMScene.h"
namespace BGMScene {

	// =======================================================
	// シーン登録
	// =======================================================
	static SceneName sceneName = REGISTER_SCENE("bgm", BGMScene);

	void BGMScene::SetTransitDuration(float minSec)
	{
		float t = transitionT;
		transitionT = { minSec, false };
		transitionT = t;
	}

	// =======================================================
	// 初期化
	// =======================================================
	void BGMScene::Init()
	{
		Scene::Init();
	}


	// =======================================================
	// 終了処理
	// =======================================================
	void BGMScene::Uninit()
	{
		for (auto& pair : scenePlaylist) {
			pair.second.clear();
		}
		scenePlaylist.clear();
		audioObjs.clear();
		Scene::Uninit();
	}


	// =======================================================
	// 更新
	// =======================================================
	void BGMScene::Update()
	{
		std::list<std::string> removeList;
		for (auto& audioPair : audioObjs) {
			GameObjectAudio* audio = audioPair.second;
			if (audio == nowPlaying) {
				audio->SetVolume(transitionT);
			}
			else {
				if (transitionT == 1.0f) {
					if (scenePlaylist[currentScene].count(audioPair.first) == 0) {
						removeList.push_back(audioPair.first);
					}
					audio->Stop();
				}
				else {
					audio->SetVolume(1.0f - transitionT);
				}
			}
		}
		for (std::string& path : removeList) {
			DeleteGameObject(audioObjs[path]);
			audioObjs.erase(path);
		}

		transitionT.IncreaseValue(GetDeltaTime());
	}


	// =======================================================
	// 描画
	// =======================================================
	void BGMScene::Draw()
	{

	}

	void BGMScene::SetPlaylist(Scene* scene, const std::set<std::string>& playlist)
	{
		scenePlaylist[scene] = playlist;
	}

	void BGMScene::SetCurrentScene(Scene* scene)
	{
		currentScene = scene;
	}

	void BGMScene::Play(const std::string& path)
	{
		if (scenePlaylist[currentScene].count(path)) {
			if (audioObjs.count(path) == 0) {
				audioObjs[path] = AddGameObject(
					GameObjectAudio(LoadAudio(path), true)
				);
			}
			if (nowPlaying != audioObjs[path]) {
				transitionT = 0.0f;
				audioObjs[path]->SetVolume(transitionT);
				audioObjs[path]->Play();
				nowPlaying = audioObjs[path];
			}
		}
	}

	void BGMScene::Stop()
	{
		nowPlaying = nullptr;
	}

	// =======================================================
	// レイヤータイプの定義
	// =======================================================
	/*LAYER_TYPE BGMScene::GetLayerType(int layer)
	{
		return Scene::GetLayerType(layer);
	}*/
}
