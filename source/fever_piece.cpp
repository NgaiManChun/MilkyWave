#include "fever_piece.h"
#include "player.h"
#include "scene.h"
#include "renderer.h"

constexpr const char* FEVER_PIECE_MODEL = "asset\\model\\fever_piece.mgm";
constexpr const char* FEVER_PIECE_COLLISION = "asset\\model\\fever_piece_collision.mgi";
constexpr const char* FEVER_PIECE_DESTORY = "asset\\model\\fever_piece_destory.mga";

FeverPiece::FeverPiece(Model* model, ARRANGEMENT* collisionArrangment, Animation* destoryAnimation, Audio* destoryAudio,
	const float destoryDuration, const float restitution, const F3& position, const F3& size,
	const Quaternion& rotate, const std::string& scope) :
	Obstacle(model, collisionArrangment, destoryAnimation, destoryAudio, destoryDuration, restitution, position, size, rotate, scope)
{
}

void FeverPiece::Update()
{
	if (!destory) {
		rotate = Quaternion::AxisRadian(Rotate({ 0.0f, 1.0f, 0.0f }, rotate), 0.1f) * rotate;
	}
	Obstacle::Update();
}

void FeverPiece::Draw()
{
	if (destory) {
		GetRenderer()->SetDepthState(DEPTH_STATE_NO_WRITE);
		Obstacle::Draw();
		GetRenderer()->SetDepthState(DEPTH_STATE_ENABLE);
	}
	else {
		Obstacle::Draw();
	}
	
}

void FeverPiece::OnCollision(GameObject* gameobject, const std::list<COLLISION_PAIR>& pairs)
{
	if (IS_TYPE(*gameobject, Player)) {
		destory = true;
	}
}
