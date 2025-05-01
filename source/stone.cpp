#include "stone.h"
#include "player.h"
#include "scene.h"

Stone::Stone(Model* model, ARRANGEMENT* collisionArrangment, Animation* destoryAnimation, Audio* destoryAudio,
	const float destoryDuration, const float restitution, const F3& position, const F3& size,
	const Quaternion& rotate, const std::string& scope) : 
	Obstacle(model, collisionArrangment, destoryAnimation, destoryAudio, destoryDuration, restitution, position, size, rotate, scope)
{
	selfRotate = Quaternion::AxisRadian(Normalize(F3{ (float)rand(), (float)rand(), (float)rand() }), (float)rand() / RAND_MAX * 2.0f * PI);
	if (rand() % 2 == 0) {
		selfRotate = selfRotate.Inverse();
	}
}

void Stone::Update()
{
	if (!destory) {
		rotate = Lerp(rotate, rotate * selfRotate, GetDeltaTime() * 0.001f);
	}
	Obstacle::Update();
}
