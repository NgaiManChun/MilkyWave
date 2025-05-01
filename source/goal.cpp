#include "goal.h"
#include "player.h"
#include "resourceTool.h"
#include "renderer.h"

static constexpr const char* GOAL_MODEL = "asset\\model\\goal.mgm";
static constexpr const char* GOAL_COLLISION = "asset\\model\\goal_collision.mgi";

Goal::Goal(const F3& position, const F3& size, const Quaternion& rotate, const std::string& scope) : GameObject(size, position, rotate)
{
	model = LoadModel(GOAL_MODEL, scope);
	MGObject mgi = LoadMGO(GOAL_COLLISION);
	ARRANGEMENT* arrangement = GetArrangementByMGObject(mgi);
	AddCollisionUnits(arrangement);
	mgi.Release();
}

void Goal::Update()
{
	if (inGoal) {
		color.w = 0.2f * (1.0f - goalT);
		goalT.IncreaseValue(GetDeltaTime());
		if (goalT == 0.0f) {
			enable = false;
		}
	}
	uvOffsetT.IncreaseValue(GetDeltaTime());
	UpdateWorldCollisionUnits();
}

void Goal::Draw()
{
	GetRenderer()->SetDepthState(DEPTH_STATE_NO_WRITE);
	DrawModel(model, position, size, rotate, color, { uvOffsetT, 0.0f });
	GetRenderer()->SetDepthState(DEPTH_STATE_ENABLE);
}

void Goal::OnCollision(GameObject* gameobject, const std::list<COLLISION_PAIR>& pairs)
{
	if (typeid(*gameobject) == typeid(Player)) {
		inGoal = true;
	}
}
