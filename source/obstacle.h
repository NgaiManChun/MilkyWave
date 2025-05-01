#ifndef _OBSTACLE_H
#define _OBSTACLE_H

#include "gameObject.h"
#include "gameObjectAudio.h"
#include "collision.h"
#include "progress.h"
using namespace MG;
using namespace MG::Collision;

class Obstacle : public GameObject {
protected:
	Model* model = nullptr;
	Animation* destoryAnimation = nullptr;
	Audio* destoryAudio = nullptr;
	GameObjectAudio* destoryAudioObj = nullptr;
	bool destory = false;
	Progress destoryProgress;
	float restitution = 0.65f; // îΩìÆåWêî
public:
	F3 velocity = {};
	Obstacle(
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
	~Obstacle();
	void Update() override;
	void Draw() override;
	void OnCollision(GameObject* gameobject, const std::list<COLLISION_PAIR>& pairs) override;
	bool GetDestory() const;

};
#endif