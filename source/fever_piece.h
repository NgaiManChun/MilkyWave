#ifndef _FEVER_PIECE_H
#define _FEVER_PIECE_H

#include "obstacle.h"
#include "gameObject.h"
#include "collision.h"
#include "progress.h"
using namespace MG;
using namespace MG::Collision;

class FeverPiece : public Obstacle {
public:
	FeverPiece(
		Model* model,
		ARRANGEMENT* collisionArrangment,
		Animation* destoryAnimation,
		Audio* destoryAudio,
		const float destoryDuration = 500.0f,
		const float restitution = 0.65f,
		const F3& position = {},
		const F3& size = { 1.0f, 1.0f, 1.0f },
		const Quaternion& rotate = { 0.0f, 0.0f, 0.0f, 1.0f },
		const std::string& scope = RESOURCE_SCOPE_GOBAL
	);
	void Update() override;
	void Draw() override;
	void OnCollision(GameObject* gameobject, const std::list<COLLISION_PAIR>& pairs) override;
};

#endif
