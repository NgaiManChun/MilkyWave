#ifndef _PREVIEW_SCENE_H
#define _PREVIEW_SCENE_H

#include "scene.h"
#include "course.h"
#include "progress.h"
#include <atomic>
using namespace MG;

namespace PreviewScene {
	class PreviewScene : public Scene {
	private:
		Animation* cameraRoll;
		Model* background;
		Course* course;
		Texture* whiteTexture;
		Progress t{ 60000.0f, true };
		std::atomic<bool> locked = false;
		std::atomic<bool> busy = false;
	public:
		void Init() override;
		void Uninit() override;
		void Update() override;
		void Draw() override;
		//LAYER_TYPE GetLayerType(int layer) override;
		void SortGameObjects() override;

		void LoadCourse(const std::string& background, const std::string& rail, 
			const std::string& collision,
			const std::string& surface, const std::string& items, const std::string& camera);
		void Lock();
		void Unlock();
		bool IsLocked() const;
		bool IsBusy() const;
	};
}

#endif