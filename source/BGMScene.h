#ifndef _BGM_SCENE_H
#define _BGM_SCENE_H

#include "scene.h"
#include "progress.h"
#include "gameObjectAudio.h"
#include <functional>
#include <set>
using namespace MG;

namespace BGMScene {
	class BGMScene : public Scene {
	private:
		Scene* currentScene = nullptr;
		GameObjectAudio* nowPlaying = nullptr;
		std::map<Scene*, std::set<std::string>> scenePlaylist;
		std::unordered_map<std::string, GameObjectAudio*> audioObjs;
		Progress transitionT{ 2000.0f, false };
	public:
		void Init() override;
		void Uninit() override;
		void Update() override;
		void Draw() override;
		void SetPlaylist(Scene* scene, const std::set<std::string>& playlist);
		void SetCurrentScene(Scene* scene);
		void Play(const std::string& path);
		void Stop();
		void SetTransitDuration(float minSec);
	};
} // namespace BGMScene

#endif