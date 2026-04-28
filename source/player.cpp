// プレイヤークラス実装

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

// フィーバーゲージへのチャージ速度（毎秒）
static constexpr const float FEVER_STOCK_CHARAGE_SPEED = 3000.0f;

// =======================================================
// 設定値ロード
// =======================================================
static const Player::_CONFIG CONFIG = LoadConfig<Player::_CONFIG>("asset\\config.csv", [](const D_KVTABLE& table) -> Player::_CONFIG {
	return {
		// 通常時最大速度
		TABLE_FLOAT_VALUE(table, "PLAYER_MAX_VELOCITY", 9.0f),
		// フィーバー時最大速度
		TABLE_FLOAT_VALUE(table, "PLAYER_MAX_VELOCITY_FEVER", 12.0f),
		// 通常加速
		TABLE_FLOAT_VALUE(table, "PLAYER_ACCELERATION", 3.0f),
		// フィーバー加速
		TABLE_FLOAT_VALUE(table, "PLAYER_ACCELERATION_FEVER", 10.0f),
		// 落下時加速度
		TABLE_FLOAT_VALUE(table, "PLAYER_FALLING_ACCELERATION", 6.0f),
		// 減速（割合）
		TABLE_FLOAT_VALUE(table, "DECELERATION_PERCENT", 0.1f),
		// 減速（定数）
		TABLE_FLOAT_VALUE(table, "DECELERATION_CONSTANT", 0.01f),
		// リスポーン時の上方向オフセット
		TABLE_FLOAT_VALUE(table, "POSITION_RESET_UP_OFFSET", 3.0f),
		// 横移動加速
		TABLE_FLOAT_VALUE(table, "HORIZONTAL_ACCELERATION", 1.0f),
		// 水しぶき発生最小速度
		TABLE_FLOAT_VALUE(table, "SPLASH_MIN_VELOCITY", 1.0f),
		// フィーバー最大ストック数
		TABLE_INT_VALUE(table, "FEVER_AMOUNT_MAX", 5),
		// フィーバー持続時間
		TABLE_FLOAT_VALUE(table, "FEVER_TIME", 10000.0f),
		// 水面判定の厚み
		TABLE_FLOAT_VALUE(table, "SURFACE_THICKNESS", 4.0f)
	};
	});

// =======================================================
// アセットロード
// =======================================================
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

	// モデル・アニメーションロード
	boat = LoadModel(ASSET.BOAT_MODEL);
	turbo = LoadModel(ASSET.TURBO_MODEL);
	turboIgnition = LoadAnimation(ASSET.TURBO_IGNITION);
	turboRotate = LoadAnimation(ASSET.TURBO_ROTATE);
	leftAnim = LoadAnimation(ASSET.BOAT_LEFT_ANIMATION);
	rightAnim = LoadAnimation(ASSET.BOAT_RIGHT_ANIMATION);

	// 当たり判定ロード
	MGObject mgi = LoadMGO(ASSET.BOAT_COLLISION.c_str());
	ARRANGEMENT* arrangement = GetArrangementByMGObject(mgi);
	AddCollisionUnits(arrangement);

	// 水しぶきエフェクト生成
	splashEffects.reserve(arrangement->instanceNum);
	for (int i = 0; i < arrangement->instanceNum; i++) {
		MODEL_INSTANCE& instance = arrangement->instances[i];
		if (!strcmp(instance.instance, "splash")) {
			// インスタンス名が"splash"ならエフェクトとして登録
			splashEffects.push_back(SPLASH{ new StarEffect(position + instance.position), instance.position });
		}
	}
	splashEffects.shrink_to_fit();
	mgi.Release();

	// フィーバー初期化
	feverAmount = { CONFIG.FEVER_TIME, false };

	inCourse = 1.0f;
}

Player::~Player()
{
	// エフェクト解放（ポインタ管理注意）
	splashEffects.clear();
}

void Player::Update()
{
	float deltaTimeSec = GetDeltaTime() * 0.001f;

	if (!stop) {

		// ===== フィーバーゲージ処理 =====

		// 使い切ったストック削除
		feverStocks.remove_if([](Player::FEVER_STOCK& feverStock) {
			return (feverStock.hold == 1.0f && feverStock.amount <= 0.0f);
			});

		// ストックを徐々にゲージへ加算
		for (Player::FEVER_STOCK& feverStock : feverStocks) {
			if (feverStock.hold == 1.0f) {
				float charage = min(FEVER_STOCK_CHARAGE_SPEED * deltaTimeSec, feverStock.amount);
				feverStock.amount -= charage;
				feverAmount.IncreaseValue(charage);
			}
			// UI到達までの時間進行
			feverStock.hold.IncreaseValue(GetDeltaTime());
		}

		// 速度リセット
		if (resetVelocity) {
			velocity = {};
		}
		resetVelocity = false;

		F3 acceleration = {};

		// ===== 水面判定 =====
		if (surfaceAlign.depth > 0.0f && surfaceAlign.depth < CONFIG.SURFACE_THICKNESS * ((fever) ? 3.0f : 1.0f)) {

			// 水中処理

			// 重力方向取得
			F3 gravityDirect = Rotate({ 0.0f, -1.0f, 0.0f }, gravity.rotate);

			// 浮力的な処理（深さ依存）
			acceleration -= gravityDirect * surfaceAlign.depth * 10.0f;

			// 落下中なら抵抗追加
			if (Dot(Normalize(velocity), gravityDirect) > 0.0f) {
				acceleration += gravityDirect * velocity * 10.0f;
			}

			// 前進加速
			acceleration.z = fever ? CONFIG.PLAYER_ACCELERATION_FEVER : CONFIG.PLAYER_ACCELERATION;

			// 横移動
			acceleration.x = pan * CONFIG.HORIZONTAL_ACCELERATION;

			// 姿勢補正（水面に沿う）
			F3 forward = Rotate({ 0.0f, 0.0f, 1.0f }, rotate);
			F3 upper = Rotate({ 0.0f, 1.0f, 0.0f }, rotate);

			forward = Lerp(forward, Normalize(gravity.forward + Rotate({ 0.0f, 0.0f, 3.0f }, surfaceAlign.rotate)), deltaTimeSec * 6.0f);
			upper = Lerp(upper, Normalize(gravity.upper + Rotate({ 0.0f, 3.0f, 0.0f }, surfaceAlign.rotate)), deltaTimeSec * 6.0f);

			rotate = Quaternion(forward, upper);
		}
		else {
			// 空中処理

			F3 gravityDirect = Rotate({ 0.0f, -1.0f, 0.0f }, gravity.rotate);

			// 上向き速度なら減衰
			if (Dot(Normalize(velocity), gravityDirect) < 0.0f) {
				acceleration += gravityDirect * velocity * 10.0f;
			}

			// 落下加速
			acceleration += gravityDirect * CONFIG.PLAYER_FALLING_ACCELERATION;
		}

		// ===== 物理更新 =====

		velocity += acceleration * deltaTimeSec;
		position += Rotate(velocity, rotate) * deltaTimeSec;

		// エフェクト追従
		for (auto& splashEffect : splashEffects) {
			if (!splashEffect.object->scene && scene) {
				scene->AddGameObjectPtr(splashEffect.object, layer);
			}
			splashEffect.object->position = position + Rotate(splashEffect.offset, rotate);
		}

		// ===== 減速処理 =====

		float maxVelocity = fever ? CONFIG.PLAYER_MAX_VELOCITY_FEVER : CONFIG.PLAYER_MAX_VELOCITY;

		// 速度制限
		if (DistanceSquare(velocity, {}) > maxVelocity * maxVelocity) {
			velocity = Normalize(velocity) * maxVelocity;
		}

		// 減衰
		velocity *= 1.0f - CONFIG.DECELERATION_PERCENT * deltaTimeSec;

		float decelerationConstant = CONFIG.DECELERATION_CONSTANT * deltaTimeSec;

		// 各軸ごとの微減速
		velocity.x *= 1.0f - min(decelerationConstant / fabsf(velocity.x), 1.0f);
		velocity.y *= 1.0f - min(decelerationConstant / fabsf(velocity.y), 1.0f);
		velocity.z *= 1.0f - min(decelerationConstant / fabsf(velocity.z), 1.0f);

		// 着水音リセット
		if (SESwash.IsFinished()) {
			swash = false;
		}
	}

	// フィーバー終了判定
	if (feverAmount == 0.0f) {
		fever = false;
	}

	// ゴール後処理
	if (inGoal) {
		fever = false;
		position += Rotate(velocity, rotate) * deltaTimeSec;
	}

	// フィーバー中の処理
	if (fever) {
		feverTransit.IncreaseValue(GetDeltaTime());
		turboRotateProgress.IncreaseValue(GetDeltaTime());
		feverAmount.IncreaseValue(-GetDeltaTime());
	}
	else {
		feverTransit.IncreaseValue(-GetDeltaTime());
	}

	// 無敵時点滅
	if (invincible != 0.0f) {
		color.w = fabsf(sinf(blinking * 2 * PI));
	}
	else {
		color.w = 1.0f;
	}

	// 状態更新
	surfaceAlign = {};
	blinking.IncreaseValue(GetDeltaTime());
	invincible.IncreaseValue(-GetDeltaTime());

	if (!stop) {
		inCourse.IncreaseValue(-GetDeltaTime());
	}

	UpdateWorldCollisionUnits();
}

void Player::Draw() 
{
	
	// 左右に傾けるアニメーション
	if (pan < 0.0f) {
		DrawModel(boat, { {leftAnim, leftAnim->rawAnimation->frames * abs(pan) }}, position, size, rotate, color);
	}
	else if (pan > 0.0f) {
		DrawModel(boat, { {rightAnim, rightAnim->rawAnimation->frames * pan } }, position, size, rotate, color);
	}
	else {
		DrawModel(boat, position, size, rotate, color);
	}
	
	// ブースター点火アニメーション
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
	// コースと衝突している場合
	if (IS_TYPE(*gameObject, Course)) {
		Course* course = (Course*)gameObject;
		this->course = course;

		// 現在位置における重力方向と水面情報を取得
		gravity = course->GetGravity(position);
		surfaceAlign = course->GetSurfaceAlign(position, gravity.rotate);

		// 水面から深すぎない範囲なら、コース内にいる扱い
		// フィーバー中は許容範囲を広げる
		if (surfaceAlign.depth < CONFIG.SURFACE_THICKNESS* ((fever) ? 3.0f : 1.0f)) {

			// 水面に入った瞬間、一定以上の落下速度があれば着水SEを再生
			if (surfaceAlign.depth > 0.0f && velocity.y < -CONFIG.SPLASH_MIN_VELOCITY * 2.0f && !swash) {
				SESwash.Play();
				swash = true;
			}

			// コース内判定を維持
			inCourse = 1.0f;
		}

		// ゴール前かつ一定以上の速度がある場合、水しぶきを出す
		bool enableSplash =
			!inGoal &&
			DistanceSquare({}, velocity) > CONFIG.SPLASH_MIN_VELOCITY * CONFIG.SPLASH_MIN_VELOCITY;

		// 各水しぶきエフェクトが水面付近にあるか確認して再生・停止を切り替える
		for (auto& splashEffect : splashEffects) {
			auto align = course->GetSurfaceAlign(splashEffect.object->position, gravity.rotate);

			splashEffect.object->SetStarted(
				enableSplash &&
				align.depth > 0.0f &&
				align.depth < CONFIG.SURFACE_THICKNESS * ((fever) ? 3.0f : 1.0f)
			);
		}
	}

	// 障害物または石と衝突した場合
	else if (IS_TYPE(*gameObject, Obstacle) || IS_TYPE(*gameObject, Stone)) {

		// 破壊済みでなければダメージ判定
		if (!((Obstacle*)gameObject)->GetDestory()) {

			// 無敵中ではなく、フィーバー中でもなければダメージを受ける
			if (invincible == 0.0f && !fever) {
				SEDamage.Play();

				// 速度をリセットし、一定時間無敵にする
				resetVelocity = true;
				invincible = 1.0f;

				// 点滅演出を最初から開始
				blinking = 0.0f;
			}
		}
	}

	// フィーバーアイテムと衝突した場合
	else if (IS_TYPE(*gameObject, FeverPiece)) {

		// 破壊済みでなければフィーバーゲージをチャージ
		if (!((FeverPiece*)gameObject)->GetDestory()) {
			float h = GetScreenHeight() * 0.5f;

			// チャージ情報を一時保存する
			// ここでは即座にゲージを増やさず、UI演出後に少しずつ加算する
			feverStocks.push_back({
				timeGetTime(),

				// アイテム取得時の画面上の2D座標
				scene->GetCurrentCamera()->GetScreenPosition(position),

				// ベジェ曲線用のランダムな制御点
				// アイテム破片がゲージUIへ飛んでいく演出に使う
				{
					F2{ ((float)rand() / RAND_MAX) * h * ((rand() % 2) ? 1.0f : -1.0f),
						((float)rand() / RAND_MAX) * h * ((rand() % 2) ? 1.0f : -1.0f) },

					F2{ ((float)rand() / RAND_MAX) * h * ((rand() % 2) ? 1.0f : -1.0f),
						((float)rand() / RAND_MAX) * h * ((rand() % 2) ? 1.0f : -1.0f) },

					F2{ ((float)rand() / RAND_MAX) * h * ((rand() % 2) ? 1.0f : -1.0f),
						((float)rand() / RAND_MAX) * h * ((rand() % 2) ? 1.0f : -1.0f) }
				},

				// 1個分のフィーバーゲージ増加量
				1.0f / CONFIG.FEVER_AMOUNT_MAX * CONFIG.FEVER_TIME
				});
		}
	}

	// ゴールと衝突した場合
	else if (IS_TYPE(*gameObject, Goal)) {

		// プレイヤー操作を停止し、ゴール状態へ移行
		stop = true;
		inGoal = true;

		// ゴール後は水しぶきエフェクトを止める
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
