#include "gameObjectModel.h"

namespace MG {

	GameObjectModel::GameObjectModel(Model* model, Animation* animation, Progress t, F3 size, F3 position, Quaternion rotate, F4 color)
		: model(model), animation(animation), t(t), GameObject(size, position, rotate, color)
	{
	}

	void GameObjectModel::Update()
	{
		t.IncreaseValue(GetDeltaTime());
	}

	void GameObjectModel::Draw()
	{
		if (animation) {
			DrawModel(model, { { animation, animation->rawAnimation->frames * t } }, position, size, rotate, color);
		}
		else {
			DrawModel(model, position, size, rotate, color);
		}
		
	}

} // namespace MG