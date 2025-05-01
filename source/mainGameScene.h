#ifndef _MAIN_GAME_SCENE_H
#define _MAIN_GAME_SCENE_H

#include "scene.h"
#include "gameObjectText.h"
#include "gameObjectModel.h"

#include "course.h"
#include "player.h"

#include "curtain.h"
#include "uiMap.h"
#include "uiPause.h"
#include "feverTime.h"
#include "talk.h"

#include <functional>
#include <thread>
#include <string>
using namespace MG;

namespace MainGameScene {

	class MainGameScene : public Scene {
	public:
		struct _CONFIG {
			float START_OFFSET = 16.0f;
			F3 GOAL_CAMERA_ANGLE = Normalize(F3{ 3.0f, 2.0f, 3.0f });
			float GOAL_CAMERA_DISTANCE = 4.0f;
			float GOAL_CAMERA_ROTATE = -180.0f;
			float ACCELEROMETER_MIN = 0.1f;
			float ACCELEROMETER_MAX = 0.9f;
		};

		struct _ASSET {
			std::string CUTIN_FEVER_TEXTURE;
			std::string SE_CURSOR_MOVE;
			std::string SE_OK;
			std::string SE_FEVER;
			std::string SE_READY;
			std::string SE_GO;
			std::string SE_CUTIN;
			std::string SE_GOAL;
			std::string SE_FALLING;
		};

		struct FEVER_EFFECT {
			F3 position;
			F3 size;
			F4 color;
		};
	private:
		Model* background;
		Player* player;
		Course* course;
		GameObjectModel* gallery;
		GameObjectQuad* cutinFever;

		GameObjectText* uiTime;
		UIMap* uiMap;
		UIPause* uiPause;
		FeverTimeUI* feverGauge;
		Curtain* curtain;
		GameObjectText* startCountLabel;
		GameObjectText* hintsLabel;

		GameObjectAudio* SECursor;
		GameObjectAudio* SEOK;
		GameObjectAudio* SEFever;
		GameObjectAudio* SEReady;
		GameObjectAudio* SEGo;
		GameObjectAudio* SECutin;
		GameObjectAudio* SEGoal;
		GameObjectAudio* SEFalling;

		Talk* prologue;
		Talk* epilogue;

		F3 startPosition = {};
		F3 cameraOffset = {};
		Progress startProgress = { 1000.0f, false };
		Progress goalProgress = { 1000.0f, false };
		Progress cutinFade = { 300.0f, false };
		Progress cutinHold = { 1500.0f, false };
		bool cutin = false;

		bool gyro = false;
		float analogX = 0.0f;
		float analogXAcce = 1.0f / 15.0f;

		std::vector<FEVER_EFFECT> feverEffects;

		float time = 0.0f;

		RenderTarget* effectRenderTarget;

		std::function<void()> updateFunc;
	private:
		void UpdatePrologue();
		void UpdateEpilogue();
		void UpdateStartCount(Progress& t);
		void UpdateStart();
		void UpdatePlay();
		void UpdateGoal(F3& p0, Progress& t);
		void UpdatePause();
		std::wstring GetHintsLabel();
	public:
		void Preload();
		void Init() override;
		void Uninit() override;
		void Update() override;
		void Draw() override;
		void SortGameObjects() override;
	};
}
#endif