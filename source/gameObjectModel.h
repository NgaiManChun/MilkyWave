// =======================================================
// gameObjectModel.h
// 
// 3Dモデルゲームオブジェクト
// 
// 作者：魏文俊（ガイ　マンチュン）　2024/11/18
// =======================================================
#ifndef _GAME_OBJECT_MODEL_H
#define _GAME_OBJECT_MODEL_H

#include "gameObject.h"
#include "progress.h"
namespace MG {

	class GameObjectModel : public GameObject {
	public:
		Model* model;
		Animation* animation;
		Progress t;

		GameObjectModel(
			Model* model,
			Animation* animation = nullptr,
			Progress t = Progress{ 1000.0f, true },
			F3 size = { 1.0f, 1.0f, 1.0f }, F3 position = {},
			Quaternion rotate = Quaternion{ 0.0f, 0.0f, 0.0f, 1.0f },
			F4 color = F4{ 1.0f, 1.0f, 1.0f, 1.0f }
		);
		void Update() override;
		void Draw() override;
	};

} // namespace MG

#endif
