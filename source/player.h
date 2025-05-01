#ifndef _PLAYER_H
#define _PLAYER_H

#include "gameObject.h"
#include "gameObjectAudio.h"
#include "collision.h"
#include "progress.h"
#include "course.h"
#include "starEffect.h"
using namespace MG;
using namespace MG::Collision;

class Surface;
class Player : public GameObject {
public:
	struct _CONFIG {
		float PLAYER_MAX_VELOCITY = 9.0f;
		float PLAYER_MAX_VELOCITY_FEVER = 12.0f;
		float PLAYER_ACCELERATION = 3.0f;
		float PLAYER_ACCELERATION_FEVER = 10.0f;
		float PLAYER_FALLING_ACCELERATION = 6.0f;
		float DECELERATION_PERCENT = 0.1f;
		float DECELERATION_CONSTANT = 0.01f;
		float POSITION_RESET_UP_OFFSET = 3.0f;
		float HORIZONTAL_ACCELERATION = 1.0f;
		float SPLASH_MIN_VELOCITY = 1.0f;
		int FEVER_AMOUNT_MAX = 5;
		float FEVER_TIME = 10000.0f;
		float SURFACE_THICKNESS = 4.0f;
	};

	struct _ASSET {
		std::string BOAT_MODEL;
		std::string BOAT_COLLISION;
		std::string TURBO_MODEL;
		std::string TURBO_IGNITION;
		std::string TURBO_ROTATE;
		std::string BOAT_LEFT_ANIMATION;
		std::string BOAT_RIGHT_ANIMATION;
		std::string SE_SWASH;
		std::string SE_DAMAGE;
	};

	struct SPLASH {
		StarEffect* object;
		F3 offset;
	};

	struct FEVER_STOCK {
		unsigned long timestamp;
		F2 p0;
		F2 effectControls[3];
		float amount;
		Progress hold{ 500.0f, false };
	};
private:
	Course* course;
	Model* boat;
	Model* turbo;
	std::vector<SPLASH> splashEffects;
	std::list<FEVER_STOCK> feverStocks;
	Course::GRAVITY gravity = { {}, Quaternion::Identity(), 0.0f };
	Course::SURFACE_ALIGN surfaceAlign;
	Progress invincible = { 2000.0f, false };
	Progress blinking = { 300.0f, true };
	Progress feverAmount = { 5.0f, false };
	Progress feverTransit = { 500.0f, false };
	Progress turboRotateProgress = { 1000.0f, true };
	Progress inCourse = { 1000.0f, false };
	bool fever = false;
	bool stop = false;
	bool inGoal = false;
	bool resetVelocity = false;
	Animation* turboIgnition;
	Animation* turboRotate;
	Animation* leftAnim;
	Animation* rightAnim;
	float pan = 0.0f;
	GameObjectAudio SESwash;
	GameObjectAudio SEDamage;
	bool swash = false;
public:
	F3 velocity = {};

	Player(const F3& size = { 1.0f, 1.0f, 1.0f });
	~Player();
	void Update() override;
	void Draw() override;
	bool InCourse() const;
	void ResetPosition();
	void OnCollision(GameObject* gameobject, const std::list<COLLISION_PAIR>& pairs);
	float GetFeverAmount() const;
	void SetFeverAmount(float amount);
	float IsFever();
	void SetStop(bool stop);
	float GetGravityT() const;
	bool InGoal() const;
	float GetDepth() const;
	void SetPan(float pan);
	void StartFever();
	const std::list<FEVER_STOCK>& GetFeverStocks() const;
};

#endif
