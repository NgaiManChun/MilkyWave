#ifndef _GOAL_H
#define _GOAL_H

#include "gameObject.h"
#include "collision.h"
#include "progress.h"
#include <list>
using namespace MG;
using namespace MG::Collision;

class Goal : public GameObject {
private:
	Model* model = nullptr;
	bool inGoal = false;
	Progress uvOffsetT = { 1000.0f, true };
	Progress goalT = { 1000.0f, false };
public:
	Goal(
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
