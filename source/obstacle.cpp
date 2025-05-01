#include "obstacle.h"
#include "scene.h"
#include "player.h"

Obstacle::Obstacle(Model* model, ARRANGEMENT* collisionArrangment, Animation* destoryAnimation, Audio* destoryAudio,
	const float destoryDuration, const float restitution, const F3& position, const F3& size,
	const Quaternion& rotate, const std::string& scope): GameObject(size, position, rotate), model(model), destoryAnimation(destoryAnimation), destoryAudio(destoryAudio), restitution(restitution)
{
	destoryProgress = Progress{ destoryDuration, false };
	AddCollisionUnits(collisionArrangment);
	if (destoryAudio) {
		destoryAudioObj = new GameObjectAudio(destoryAudio, false);
	}
}

Obstacle::~Obstacle()
{
	if (destoryAudioObj) {
		delete destoryAudioObj;
		destoryAudioObj = nullptr;
	}
}

void Obstacle::Update()
{
	if (destory) {
		color.w = 1.0f - destoryProgress;
		if (destoryProgress == 0.0f && destoryAudioObj) {
			destoryAudioObj->Play();
		}
		destoryProgress.IncreaseValue(GetDeltaTime());
	}
	if (destoryProgress == 1.0f) {
		if (destoryAudioObj) {
			if (destoryAudioObj->IsFinished()) {
				scene->DeleteGameObject(this);
			}
		}
		else {
			scene->DeleteGameObject(this);
		}
	}
	position += velocity * GetDeltaTime() * 0.001f;

	UpdateWorldCollisionUnits();
}

void Obstacle::Draw()
{
	if (destory) {
		DrawModel(model, { { destoryAnimation, destoryAnimation->rawAnimation->frames * destoryProgress } }, position, size, rotate, color);
	}
	else {
		DrawModel(model, position, size, rotate, color);
	}
}

void Obstacle::OnCollision(GameObject* gameobject, const std::list<COLLISION_PAIR>& pairs)
{
	if (IS_TYPE(*gameobject, Player) && !destory) {

		// ”ò‚Î‚³‚ê‰‰o
		F3 vector = Normalize(position - gameobject->position);
		F3 playerWorldVelocity = Rotate(((Player*)gameobject)->velocity, gameobject->rotate);
		float scalar = Dot(vector, playerWorldVelocity);
		if (scalar > 0) {
			velocity = vector * scalar * restitution;
		}

		destory = true;
	}
}

bool Obstacle::GetDestory() const
{
	return destory;
}
