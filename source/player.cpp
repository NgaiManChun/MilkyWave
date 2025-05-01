#include "player.h"
#include "course.h"
#include "keyboard.h"
#include "resourceTool.h"
#include "stone.h"
#include "fever_piece.h"
#include "goal.h"
#include "drawTool.h"
#include "renderer.h"
#include "config.h"
#include "obstacle.h"
#include "scene.h"

static constexpr const float FEVER_STOCK_CHARAGE_SPEED = 3000.0f;

static const Player::_CONFIG CONFIG = LoadConfig<Player::_CONFIG>("asset\\config.csv", [](const D_KVTABLE& table) -> Player::_CONFIG {
	return {
		TABLE_FLOAT_VALUE(table, "PLAYER_MAX_VELOCITY", 9.0f),
		TABLE_FLOAT_VALUE(table, "PLAYER_MAX_VELOCITY_FEVER", 12.0f),
		TABLE_FLOAT_VALUE(table, "PLAYER_ACCELERATION", 3.0f),
		TABLE_FLOAT_VALUE(table, "PLAYER_ACCELERATION_FEVER", 10.0f),
		TABLE_FLOAT_VALUE(table, "PLAYER_FALLING_ACCELERATION", 6.0f),
		TABLE_FLOAT_VALUE(table, "DECELERATION_PERCENT", 0.1f),
		TABLE_FLOAT_VALUE(table, "DECELERATION_CONSTANT", 0.01f),
		TABLE_FLOAT_VALUE(table, "POSITION_RESET_UP_OFFSET", 3.0f),
		TABLE_FLOAT_VALUE(table, "HORIZONTAL_ACCELERATION", 1.0f),
		TABLE_FLOAT_VALUE(table, "SPLASH_MIN_VELOCITY", 1.0f),
		TABLE_INT_VALUE(table, "FEVER_AMOUNT_MAX", 5),
		TABLE_FLOAT_VALUE(table, "FEVER_TIME", 10000.0f),
		TABLE_FLOAT_VALUE(table, "SURFACE_THICKNESS", 4.0f)
	};
});

static const Player::_ASSET ASSET = LoadConfig<Player::_ASSET>("asset\\asset_list.csv", [](const D_KVTABLE& table) -> Player::_ASSET {
	return {
		TABLE_STR_VALUE(table, "BOAT_MODEL", "asset\\model\\boat.mgm"),
		TABLE_STR_VALUE(table, "BOAT_COLLISION", "asset\\model\\boat_collision.mgi"),
		TABLE_STR_VALUE(table, "TURBO_MODEL", "asset\\model\\boat_turbo.mgm"),
		TABLE_STR_VALUE(table, "TURBO_IGNITION", "asset\\model\\boat_turbo_ignition.mga"),
		TABLE_STR_VALUE(table, "TURBO_ROTATE", "asset\\model\\boat_turbo_rotate.mga"),
		TABLE_STR_VALUE(table, "BOAT_LEFT_ANIMATION", "asset\\model\\boat_left_anim.mga"),
		TABLE_STR_VALUE(table, "BOAT_RIGHT_ANIMATION", "asset\\model\\boat_right_anim.mga"),
		TABLE_STR_VALUE(table, "SE_SWASH", "asset\\sound\\Landing_SE.wav"),
		TABLE_STR_VALUE(table, "SE_DAMAGE", "asset\\sound\\damage.wav"),
	};
});

Player::Player(const F3& size) : 
	GameObject(size), 
	SESwash(GameObjectAudio(LoadAudio(ASSET.SE_SWASH), false)),
	SEDamage(GameObjectAudio(LoadAudio(ASSET.SE_DAMAGE), false)) {

	boat = LoadModel(ASSET.BOAT_MODEL);
	turbo = LoadModel(ASSET.TURBO_MODEL);
	turboIgnition = LoadAnimation(ASSET.TURBO_IGNITION);
	turboRotate = LoadAnimation(ASSET.TURBO_ROTATE);
	leftAnim = LoadAnimation(ASSET.BOAT_LEFT_ANIMATION);
	rightAnim = LoadAnimation(ASSET.BOAT_RIGHT_ANIMATION);

	MGObject mgi = LoadMGO(ASSET.BOAT_COLLISION.c_str());
	ARRANGEMENT* arrangement = GetArrangementByMGObject(mgi);
	AddCollisionUnits(arrangement);

	splashEffects.reserve(arrangement->instanceNum);
	for (int i = 0; i < arrangement->instanceNum; i++) {
		MODEL_INSTANCE& instance = arrangement->instances[i];
		if (!strcmp(instance.instance, "splash")) {
			splashEffects.push_back(SPLASH{ new StarEffect(position + instance.position), instance.position });
		}
	}
	splashEffects.shrink_to_fit();
	mgi.Release();

	feverAmount = { CONFIG.FEVER_TIME, false };

	inCourse = 1.0f;

}

Player::~Player()
{
	splashEffects.clear();
	
}

void Player::Update() {

	float deltaTimeSec = GetDeltaTime() * 0.001f;
	
	if (!stop) {

		feverStocks.remove_if([](Player::FEVER_STOCK& feverStock) {
			return (feverStock.hold == 1.0f && feverStock.amount <= 0.0f);
		});
		for (Player::FEVER_STOCK& feverStock : feverStocks) {
			if (feverStock.hold == 1.0f) {
				float charage = min(FEVER_STOCK_CHARAGE_SPEED * deltaTimeSec, feverStock.amount);
				feverStock.amount -= charage;
				feverAmount.IncreaseValue(charage);
			}
			feverStock.hold.IncreaseValue(GetDeltaTime());
		}


		if (resetVelocity) {
			velocity = {};
		}

		resetVelocity = false;
		F3 acceleration = {};
		float waterDepth = 0.0f;

		// 水面チェック
		if (surfaceAlign.depth > 0.0f && surfaceAlign.depth < CONFIG.SURFACE_THICKNESS * ((fever)?3.0f:1.0f)) {
			// 水面下

			// 引力
			F3 gravityDirect = Rotate({ 0.0f, -1.0f, 0.0f }, gravity.rotate);
			acceleration -= gravityDirect * surfaceAlign.depth * 10.0f;
			if (Dot(Normalize(velocity), gravityDirect) > 0.0f) {
				acceleration += gravityDirect * velocity * 10.0f;
			}

			if (fever) {
				acceleration.z += CONFIG.PLAYER_ACCELERATION_FEVER;
			}
			else {
				acceleration.z = CONFIG.PLAYER_ACCELERATION;
			}

			acceleration.x = pan * CONFIG.HORIZONTAL_ACCELERATION;

			F3 forward = Rotate({ 0.0f, 0.0f, 1.0f }, rotate);
			F3 upper = Rotate({ 0.0f, 1.0f, 0.0f }, rotate);


			forward = Lerp(forward, Normalize(gravity.forward + Rotate({ 0.0f, 0.0f, 3.0f }, surfaceAlign.rotate)), deltaTimeSec * 6.0f);
			upper = Lerp(upper, Normalize(gravity.upper + Rotate({ 0.0f, 3.0f, 0.0f }, surfaceAlign.rotate)), deltaTimeSec * 6.0f);
			
			rotate = Quaternion(forward, upper);

			
		}
		else {
			// 水面以上

			// 引力
			F3 gravityDirect = Rotate({ 0.0f, -1.0f, 0.0f }, gravity.rotate);
			if (Dot(Normalize(velocity), gravityDirect) < 0.0f) {
				acceleration += gravityDirect * velocity * 10.0f;
			}
			acceleration += gravityDirect * CONFIG.PLAYER_FALLING_ACCELERATION;

		}

		// 速度更新
		velocity += acceleration * deltaTimeSec;

		// ポジション更新
		position += Rotate(velocity, rotate) * deltaTimeSec;
		for (auto& splashEffect : splashEffects) {
			if (!splashEffect.object->scene && scene) {
				scene->AddGameObjectPtr(splashEffect.object, layer);
			}
			splashEffect.object->position = position + Rotate(splashEffect.offset, rotate);
		}
		
		
		// 減速
		float maxVelocity = CONFIG.PLAYER_MAX_VELOCITY;
		if (fever) {
			maxVelocity = CONFIG.PLAYER_MAX_VELOCITY_FEVER;
		}
		if (DistanceSquare(velocity, {}) > maxVelocity * maxVelocity) {
			velocity = Normalize(velocity) * maxVelocity;
		}
		velocity *= 1.0f - CONFIG.DECELERATION_PERCENT * deltaTimeSec;
		float decelerationConstant = CONFIG.DECELERATION_CONSTANT * deltaTimeSec;
		velocity.x *= 1.0f - min(decelerationConstant / fabsf(velocity.x), 1.0f);
		velocity.y *= 1.0f - min(decelerationConstant / fabsf(velocity.y), 1.0f);
		velocity.z *= 1.0f - min(decelerationConstant / fabsf(velocity.z), 1.0f);

		if (SESwash.IsFinished()) {
			swash = false;
		}
	}
	
	#ifdef _DEBUG
	if (Keyboard_IsKeyDownTrigger(KK_F)) {
		feverAmount.IncreaseValue(1.0f / CONFIG.FEVER_AMOUNT_MAX * CONFIG.FEVER_TIME);
	}
	#endif

	if (feverAmount == 0.0f) {
		fever = false;
	}

	if (inGoal) {
		fever = false;
		position += Rotate(velocity, rotate) * deltaTimeSec;
	}

	if (fever) {
		feverTransit.IncreaseValue(GetDeltaTime());
		turboRotateProgress.IncreaseValue(GetDeltaTime());
		feverAmount.IncreaseValue(-GetDeltaTime());
	}
	else {
		feverTransit.IncreaseValue(-GetDeltaTime());
	}

	if (invincible != 0.0f) {
		color.w = fabsf(sinf(blinking * 2 * PI));
	}
	else {
		color.w = 1.0f;
	}

	surfaceAlign = {};

	blinking.IncreaseValue(GetDeltaTime());
	invincible.IncreaseValue(-GetDeltaTime());
	if (!stop) {
		inCourse.IncreaseValue(-GetDeltaTime());
	}
	
	UpdateWorldCollisionUnits();
}

void Player::Draw() {
	
	if (pan < 0.0f) {
		DrawModel(boat, { {leftAnim, leftAnim->rawAnimation->frames * abs(pan) }}, position, size, rotate, color);
	}
	else if (pan > 0.0f) {
		DrawModel(boat, { {rightAnim, rightAnim->rawAnimation->frames * pan } }, position, size, rotate, color);
	}
	else {
		DrawModel(boat, position, size, rotate, color);
	}
	

	if (feverTransit > 0.0f) {
		GetRenderer()->SetDepthState(DEPTH_STATE_NO_WRITE);
		GetRenderer()->SetBlendState(BLEND_STATE_ADD);
		DrawModel(turbo,
			{
				{ turboIgnition, feverTransit * turboIgnition->rawAnimation->frames },
				{ turboRotate, turboRotateProgress * turboRotate->rawAnimation->frames },
			},
			position, size, rotate, { 1.0f, 1.0f, 1.0f, 0.5f });
		GetRenderer()->SetBlendState(BLEND_STATE_ALPHA);
		GetRenderer()->SetDepthState(DEPTH_STATE_ENABLE);
	}
}

bool Player::InCourse() const
{
	return inCourse || inGoal;
}

void Player::ResetPosition()
{
	
	rotate = gravity.rotate;
	position = gravity.position;

	if (course) {
		Course::SURFACE_ALIGN surfaceAlign = course->GetSurfaceAlign(position, gravity.rotate);
		position += Rotate({ 0.0f, CONFIG.POSITION_RESET_UP_OFFSET + surfaceAlign.depth, 0.0f }, rotate);
	}
	else {
		position += Rotate({ 0.0f, CONFIG.POSITION_RESET_UP_OFFSET, 0.0f }, rotate);
	}

	resetVelocity = true;
	fever = false;
}

void Player::OnCollision(GameObject* gameObject, const std::list<COLLISION_PAIR>& pairs)
{
	if (IS_TYPE(*gameObject, Course)) {
		Course* course = (Course*)gameObject;
		this->course = course;
		gravity = course->GetGravity(position);
		surfaceAlign = course->GetSurfaceAlign(position, gravity.rotate);
		
		// 深すぎるとアウト扱い
		if (surfaceAlign.depth < CONFIG.SURFACE_THICKNESS * ((fever) ? 3.0f : 1.0f)) {

			if (surfaceAlign.depth > 0.0f && velocity.y < -CONFIG.SPLASH_MIN_VELOCITY * 2.0f && !swash) {
				SESwash.Play();
				swash = true;
			}

			inCourse = 1.0f;
		}

		bool enableSplash = !inGoal && DistanceSquare({}, velocity) > CONFIG.SPLASH_MIN_VELOCITY * CONFIG.SPLASH_MIN_VELOCITY;
		for (auto& splashEffect : splashEffects) {
			auto align = course->GetSurfaceAlign(splashEffect.object->position, gravity.rotate);
			splashEffect.object->SetStarted(enableSplash && align.depth > 0.0f && align.depth < CONFIG.SURFACE_THICKNESS * ((fever) ? 3.0f : 1.0f));
		}
		
	}
	else if (IS_TYPE(*gameObject, Obstacle) || IS_TYPE(*gameObject, Stone)) {
		if (!((Obstacle*)gameObject)->GetDestory()) {
			if (invincible == 0.0f && !fever) {
				SEDamage.Play();
				resetVelocity = true;
				invincible = 1.0f;
				blinking = 0.0f;
			}
		}
	}
	else if (IS_TYPE(*gameObject, FeverPiece)) {
		if (!((FeverPiece*)gameObject)->GetDestory()) {
			float h = GetScreenHeight() * 0.5f;
			feverStocks.push_back({
				timeGetTime(),
				scene->GetCurrentCamera()->GetScreenPosition(position),
				{
					F2{ ((float)rand() / RAND_MAX) * h * ((rand() % 2) ? 1.0f : -1.0f), ((float)rand() / RAND_MAX) * h * ((rand() % 2) ? 1.0f : -1.0f) },
					F2{ ((float)rand() / RAND_MAX) * h * ((rand() % 2) ? 1.0f : -1.0f), ((float)rand() / RAND_MAX) * h * ((rand() % 2) ? 1.0f : -1.0f) },
					F2{ ((float)rand() / RAND_MAX) * h * ((rand() % 2) ? 1.0f : -1.0f), ((float)rand() / RAND_MAX) * h * ((rand() % 2) ? 1.0f : -1.0f) }
				},
				1.0f / CONFIG.FEVER_AMOUNT_MAX * CONFIG.FEVER_TIME
			});
		}
	}
	else if (IS_TYPE(*gameObject, Goal)) {
		stop = true;
		inGoal = true;
		for (auto& splashEffect : splashEffects) {
			splashEffect.object->SetStarted(false);
		}
	}
	
}

float Player::GetFeverAmount() const
{
	return feverAmount;
}

void Player::SetFeverAmount(float amount)
{
	feverAmount = amount;
}

float Player::IsFever()
{
	return feverTransit;
}

void Player::SetStop(bool stop)
{
	this->stop = stop;
}

float Player::GetGravityT() const
{
	return gravity.t;
}

bool Player::InGoal() const
{
	return inGoal;
}

float Player::GetDepth() const
{
	return surfaceAlign.depth;
}

void Player::SetPan(float pan)
{
	this->pan = pan;
}

void Player::StartFever()
{
	if (feverAmount > 0.0f) {
		fever = true;
	}
}

const std::list<Player::FEVER_STOCK>& Player::GetFeverStocks() const
{
	return feverStocks;
}
