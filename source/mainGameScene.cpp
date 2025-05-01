#include "mainGameScene.h"
#include "renderer.h"
#include "config.h"
#include "obstacle.h"
#include "goal.h"
#include "fever_piece.h"
#include "stone.h"
#include "BGMScene.h"
#include "input.h"
#include "formatTime.h"
#include <atomic>
namespace MainGameScene {

	static constexpr const int LAYER_PLAYER = 3;
	static constexpr const int LAYER_GALLERY = 4;
	static constexpr const int LAYER_ITEMS = 5;
	static constexpr const int LAYER_SURFACE = 6;
	static constexpr const int LAYER_UI = LAYER_2D + 1;
	static constexpr const int LAYER_COUNT = LAYER_UI + 1;
	static constexpr const int LAYER_TALK = LAYER_COUNT + 1;
	static constexpr const int LAYER_CURTAIN = LAYER_TALK + 1;
	static constexpr const int LAYER_PAUSE = LAYER_CURTAIN + 1;
	static constexpr const char* INSTANCE_LIST = "asset\\instance.csv";

	static float easeInExpo(float t) {
		return (powf(2.0f, 8.0f * t) - 1.0f) / 255.0f;
	}
	static float easeInCirc(float t) {
		return 1.0f - sqrtf(1.0f - t);
	}

	static const MainGameScene::_CONFIG CONFIG = LoadConfig<MainGameScene::_CONFIG>("asset\\config.csv", [](const D_KVTABLE& table) -> MainGameScene::_CONFIG {
		F3 GOAL_CAMERA_ANGLE = Normalize(F3{
			TABLE_FLOAT_VALUE(table, "GOAL_CAMERA_ANGLE_X", 3.0f), 
			TABLE_FLOAT_VALUE(table, "GOAL_CAMERA_ANGLE_Y", 2.0f), 
			TABLE_FLOAT_VALUE(table, "GOAL_CAMERA_ANGLE_Z", 3.0f) 
			});

		return {
			TABLE_FLOAT_VALUE(table, "START_OFFSET", 16.0f),
			GOAL_CAMERA_ANGLE,
			TABLE_FLOAT_VALUE(table, "GOAL_CAMERA_DISTANCE", 4.0f),
			TABLE_FLOAT_VALUE(table, "GOAL_CAMERA_ROTATE", -180.0f),
			TABLE_FLOAT_VALUE(table, "ACCELEROMETER_MIN", 0.1f),
			TABLE_FLOAT_VALUE(table, "ACCELEROMETER_MAX", 0.9f)
		};
	});

	static const MainGameScene::_ASSET ASSET = LoadConfig<MainGameScene::_ASSET>("asset\\asset_list.csv", [](const D_KVTABLE& table) -> MainGameScene::_ASSET {
		return {
			TABLE_STR_VALUE(table, "CUTIN_FEVER_TEXTURE", "asset\\texture\\cutin_fever.png"),
			TABLE_STR_VALUE(table, "SE_CURSOR_MOVE", "asset\\sound\\cursor_move.wav"),
			TABLE_STR_VALUE(table, "SE_OK", "asset\\sound\\ok.wav"),
			TABLE_STR_VALUE(table, "SE_FEVER", "asset\\sound\\fever.wav"),
			TABLE_STR_VALUE(table, "SE_READY", "asset\\sound\\ready.wav"),
			TABLE_STR_VALUE(table, "SE_GO", "asset\\sound\\go.wav"),
			TABLE_STR_VALUE(table, "SE_CUTIN", "asset\\sound\\cutin.wav"),
			TABLE_STR_VALUE(table, "SE_GOAL", "asset\\sound\\goal.wav"),
			TABLE_STR_VALUE(table, "SE_FALLING", "asset\\sound\\falling.wav")
		};
	});


	// =======================================================
	// シーン登録
	// =======================================================
	static SceneName sceneName = REGISTER_SCENE("main_game", MainGameScene);

	void MainGameScene::Preload()
	{
		Uninit();

		// ステージ設定
		{
			background = LoadModel(GetCommonString("course_background"));

			MGObject courseCollision = LoadMGO(GetCommonString("course_collision").c_str());
			course = AddGameObject(
				Course(
					LoadModel(GetCommonString("course_surface")),
					LoadAnimation(GetCommonString("course_rail")),
					GetArrangementByMGObject(courseCollision)
				), LAYER_SURFACE
			);
			courseCollision.Release();

			gallery = AddGameObject(
				GameObjectModel(LoadModel(GetCommonString("course_gallery")), LoadAnimation(GetCommonString("course_gallery_animation"))),
				LAYER_GALLERY
			);
			gallery->enable = false;

			// オブジェクト配置
			{
				// CSVファイルから設定を読み込む
				D_TABLE table;
				D_KVTABLE keyValuePair;
				ReadCSVFromPath(INSTANCE_LIST, table);

				// TableデータからKey-Value-Pairへ変換
				TableToKeyValuePair("instance_id", table, keyValuePair);

				MGObject mgo = LoadMGO(GetCommonString("course_items").c_str());
				ARRANGEMENT* items = GetArrangementByMGObject(mgo);
				for (int i = 0; i < items->instanceNum; i++) {

					if (keyValuePair.count(items->instances[i].instance)) {
						auto& row = keyValuePair.at(items->instances[i].instance);
						std::string type = row["type"];

						if (type == "stone") {
							MGObject mgi = LoadMGO(row["collision"].c_str());
							AddGameObject(
								Stone(
									LoadModel(row["model"]),
									GetArrangementByMGObject(mgi),
									LoadAnimation(row["destory"]),
									LoadAudio(row["destory_audio"]),
									std::stof(row["destory_duration"]),
									std::stof(row["restitution"]),
									items->instances[i].position, items->instances[i].scale, items->instances[i].rotate, sceneName), LAYER_ITEMS
							);
							mgi.Release();
						}
						else if (type == "fever_piece") {
							MGObject mgi = LoadMGO(row["collision"].c_str());
							AddGameObject(
								FeverPiece(
									LoadModel(row["model"]),
									GetArrangementByMGObject(mgi),
									LoadAnimation(row["destory"]),
									LoadAudio(row["destory_audio"]),
									std::stof(row["destory_duration"]),
									std::stof(row["restitution"]),
									items->instances[i].position, items->instances[i].scale, items->instances[i].rotate, sceneName), LAYER_ITEMS
							);
							mgi.Release();
						}
						else if (type == "obstacle") {
							MGObject mgi = LoadMGO(row["collision"].c_str());
							AddGameObject(
								Obstacle(
									LoadModel(row["model"]),
									GetArrangementByMGObject(mgi),
									LoadAnimation(row["destory"]),
									LoadAudio(row["destory_audio"]),
									std::stof(row["destory_duration"]),
									std::stof(row["restitution"]),
									items->instances[i].position, items->instances[i].scale, items->instances[i].rotate, sceneName), LAYER_ITEMS
							);
							mgi.Release();
						}
						else if (type == "other") {
							AddGameObject(
								GameObjectModel(
									LoadModel(row["model"]),
									nullptr, {},
									items->instances[i].scale,
									items->instances[i].position,
									items->instances[i].rotate
								), LAYER_ITEMS
							);
						}
						else if (type == "goal") {
							AddGameObject(
								Goal(
									items->instances[i].position, items->instances[i].scale, items->instances[i].rotate, sceneName), LAYER_ITEMS
							);
						}
					}
				}
				mgo.Release();
				items = nullptr;

			}
		}

		// プレイヤー
		{
			player = AddGameObject(Player(), LAYER_PLAYER);
			player->SetStop(true);
		}

		F2 screenSize = GetScreenSize();
		F2 screenCenter = GetScreenCenter();

		// UI
		{
			uiTime = AddGameObject(
				GameObjectText(
					L"",
					FONT{
						screenSize.y * 0.1f,
						"HG創英角ﾎﾟｯﾌﾟ体",
						FONT_WEIGHT_BOLD
					},
					TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,
					TEXT_ORIGIN_HORIZONTAL_RIGHT,
					TEXT_ORIGIN_VERTICAL_TOP,
					{ 300.0f, screenSize.y * 0.1f },
					{ screenSize.x * 0.5f - 20.0f, screenSize.y * 0.5f - 20.0f, 0.0f }
				), LAYER_UI
			);

			uiMap = AddGameObject( UIMap(), LAYER_UI);
			uiMap->SetPlayer(player);

			feverGauge = AddGameObject(
				FeverTimeUI(), LAYER_UI
			);
			feverGauge->position = { -800.0f + 20.0f, 400.0f - 60.0f };
			feverGauge->SetPlayer(player);
			
			uiPause = AddGameObject(
				UIPause(sceneName), LAYER_PAUSE
			);
			uiPause->enable = false;

			Texture* cutinTex = LoadTexture(ASSET.CUTIN_FEVER_TEXTURE);
			float aspect = cutinTex->GetWidth() / cutinTex->GetHeight();
			cutinFever = AddGameObject(
				GameObjectQuad(cutinTex, { 600.0f, 600.0f * aspect }),
				LAYER_UI
			);
			cutinFever->position = { screenSize.x * -2.0f };

			hintsLabel = AddGameObject(
				GameObjectText(
					GetHintsLabel(),
					FONT{
						40.0f,
						"HG創英角ﾎﾟｯﾌﾟ体",
						FONT_WEIGHT_BOLD
					},
					TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,
					TEXT_ORIGIN_HORIZONTAL_LEFT,
					TEXT_ORIGIN_VERTICAL_BOTTOM,
					{ 300.0f, 40.0f * 3.0f },
					{ screenSize.x * -0.5f + 20.0f, screenSize.y * -0.5f + 20.0f, 0.0f }
				), LAYER_UI
			);
		}

		// 復帰カーテン
		curtain = AddGameObject(Curtain(), LAYER_CURTAIN);
		curtain->enable = false;
		curtain->Reset();

		// スタートカウント
		startCountLabel = AddGameObject(
			GameObjectText(
				L"3",
				FONT{
					120.0f,
					"HG創英角ﾎﾟｯﾌﾟ体",
					FONT_WEIGHT_BOLD
				},
				TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,
				TEXT_ORIGIN_HORIZONTAL_CENTER,
				TEXT_ORIGIN_VERTICAL_CENTER,
				{ 300.0f, screenSize.y * 0.5f },
				{ 0.0f, 0.0f, 0.0f }
			), LAYER_COUNT
		);

		SECursor = AddGameObject(
			GameObjectAudio(LoadAudio(ASSET.SE_CURSOR_MOVE), false)
		);

		SEOK = AddGameObject(
			GameObjectAudio(LoadAudio(ASSET.SE_OK), false)
		);

		SEFever = AddGameObject(
			GameObjectAudio(LoadAudio(ASSET.SE_FEVER), true)
		);

		SEReady = AddGameObject(
			GameObjectAudio(LoadAudio(ASSET.SE_READY), false)
		);

		SEGo = AddGameObject(
			GameObjectAudio(LoadAudio(ASSET.SE_GO), false)
		);

		SECutin = AddGameObject(
			GameObjectAudio(LoadAudio(ASSET.SE_CUTIN), false)
		);

		SEGoal = AddGameObject(
			GameObjectAudio(LoadAudio(ASSET.SE_GOAL), false)
		);

		SEFalling = AddGameObject(
			GameObjectAudio(LoadAudio(ASSET.SE_FALLING), false)
		);

		// プロローグ
		prologue = AddGameObject(
			Talk(GetCommonString("prologue")), LAYER_TALK
		);
		prologue->enable = false;

		epilogue = AddGameObject(
			Talk(GetCommonString("epilogue")), LAYER_TALK
		);
		prologue->enable = false;

		// ジャイロ
		gyro = GetCommonBool("gyro");

		// フィーバータイムエフェクトポジション・サイズ
		feverEffects = std::vector<FEVER_EFFECT>(2);

		effectRenderTarget = GetRenderer()->CreateRenderTarget();
	}

	// =======================================================
	// 初期化
	// =======================================================
	void MainGameScene::Init()
	{
		Scene::Init();

		startCountLabel->enable = false;
		cutinFever->enable = false;

		cameraOffset = {};
		startProgress = 0.0f;
		goalProgress = 0.0f;
		cutinFade = 0.0f;
		cutinHold = 0.0f;
		time = 0.0f;

		BGMScene::BGMScene* bgmScene = (BGMScene::BGMScene*)LoadScene("bgm");
		bgmScene->SetCurrentScene(this);
		bgmScene->SetPlaylist(this, { GetCommonString("course_bgm")});
		bgmScene->Play(GetCommonString("course_bgm"));

		Course::GRAVITY gravity = course->GetGravity(0.0f);
		Course::SURFACE_ALIGN align = course->GetSurfaceAlign(gravity.position, gravity.rotate);
		startPosition = gravity.position + Rotate({ 0.0f, align.depth, 0.0f }, gravity.rotate);
		player->position = startPosition;
		player->rotate = gravity.rotate;

		currentCamera->SetRotate(Quaternion::AxisXDegree(12.0f));
		updateFunc = [this]() mutable { UpdatePrologue(); };
	}


	// =======================================================
	// 終了処理
	// =======================================================
	void MainGameScene::Uninit()
	{
		Scene::Uninit();
		if (effectRenderTarget) {
			GetRenderer()->ReleaseRenderTarget(effectRenderTarget);
			effectRenderTarget = nullptr;
		}
	}


	// =======================================================
	// 更新
	// =======================================================
	void MainGameScene::Update()
	{
		if (!HasProcon() && !HasDualSense() && gyro) {
			gyro = false;
			SetCommonBool("gyro", gyro);
		}
		if (updateFunc) {
			updateFunc();
		}
		
	}

	void MainGameScene::UpdatePrologue()
	{
		for (auto& gameObject : gameObjects) {
			if (gameObject->layer == LAYER_PLAYER || gameObject == prologue) {
				gameObject->enable = true;
			}
			else {
				gameObject->enable = false;
			}
		}

		player->SetStop(true);
		player->SetFeverAmount(1.0f);
		player->StartFever();

		// カメラ
		currentCamera->SetRotate(Quaternion::AxisYRadian(PI * GetDeltaTime() * 0.0001) * currentCamera->GetRotate());
		currentCamera->SetPosition(player->position - currentCamera->GetFront() * 2.0f);

		if (IsInputTrigger(INPUT_OK)) {
			prologue->NextLine();
		}

		Scene::Update();

		if (IsInputTrigger(INPUT_START)) {
			prologue->Skip();
		}
		if (prologue->IsEnd()) {
			Progress startCount{ 3000.0f, false };
			startCount = 1.0f;
			player->SetFeverAmount(0.0f);
			SEReady->Play();
			updateFunc = [this, t = startCount]() mutable { UpdateStartCount(t); };
		}
		
	}


	void MainGameScene::UpdateEpilogue()
	{
		epilogue->enable = true;

		if (IsInputTrigger(INPUT_OK)) {
			epilogue->NextLine();
		}

		currentCamera->SetRotate(Quaternion::AxisYDegree(CONFIG.GOAL_CAMERA_ROTATE * GetDeltaTime() * 0.001f) * currentCamera->GetRotate());
		currentCamera->SetPosition(player->position + F3{ 0.0f, 1.0f, 0.0f } - currentCamera->GetFront() * CONFIG.GOAL_CAMERA_DISTANCE);

		Scene::Update();

		if (IsInputTrigger(INPUT_START)) {
			epilogue->Skip();
		}
		if (!InTransition() && epilogue->IsEnd()) {
			SetCommonFloat("result_time", time);
			SceneTransit("result", "star");
		}
		
	}

	void MainGameScene::UpdateStartCount(Progress& t)
	{
		static std::set<unsigned int> layers = {
				LAYER_PLAYER,
				LAYER_ITEMS,
				LAYER_SURFACE,
				LAYER_UI,
				LAYER_COUNT
		};
		for (auto& gameObject : gameObjects) {
			if (layers.count(gameObject->layer)) {
				gameObject->enable = true;
			}
			else {
				gameObject->enable = false;
			}
		}

		startCountLabel->enable = true;
		player->SetStop(true);
		player->position = startPosition + Rotate({ 0.0f, 0.0f, -CONFIG.START_OFFSET * t }, player->rotate);

		float sec = t.GetValue() / 1000.0f;
		int countNumber = ceil(sec);
		float s = sec - floorf(sec);
		if (countNumber == 3) {
			currentCamera->SetPosition(startPosition + Rotate({ -1.0f, -1.0f, -CONFIG.START_OFFSET * 0.8f }, player->rotate));
			currentCamera->SetFront(player->position - currentCamera->GetPosition());
		}
		else if (countNumber == 2) {
			currentCamera->SetPosition(startPosition + Rotate({ 1.0f, 1.0f, -CONFIG.START_OFFSET * 0.5f }, player->rotate));
			currentCamera->SetFront(player->position - currentCamera->GetPosition());
		}
		else if (countNumber == 1) {
			currentCamera->SetPosition(startPosition + Rotate({ 0.0f, -2.0f, -CONFIG.START_OFFSET * 0.2f }, player->rotate));
			currentCamera->SetFront(player->position - currentCamera->GetPosition());
		}
		startCountLabel->SetValue(std::to_wstring(countNumber));
		startCountLabel->color.w = s;
		startCountLabel->size = F3{ 300.0f, GetScreenSize().y * 0.8f } * (1.0f - s);

		if (!InTransition()) {
			t.IncreaseValue(-GetDeltaTime());
		}

		if (t == 0.0f) {
			SEGo->Play();
			player->position = startPosition;
			player->velocity = { 0.0f, 0.0f, 9.0f };
			player->SetStop(false);
			startCountLabel->SetValue(L"START");
			updateFunc = [this]() -> void { UpdateStart(); };
		}
		Scene::Update();
	}

	void MainGameScene::UpdateStart()
	{
		startCountLabel->color.w = 1.0f - startProgress;
		startCountLabel->size = F3{ 300.0f, GetScreenSize().y * 0.8f } * startProgress;
		startProgress.IncreaseValue(GetDeltaTime());

		UpdatePlay();

		if (startProgress == 1.0f) {
			startCountLabel->enable = false;
			updateFunc = [this]() -> void { UpdatePlay(); };
		}
		
	}

	void MainGameScene::UpdatePlay()
	{
		if (gyro) {
			float acceX = GetInputAnalogValue(ANALOG_STATE_ACCE_X);
			bool sign = acceX < 0.0f;
			acceX = abs(acceX);
			acceX = min(CONFIG.ACCELEROMETER_MAX, max(CONFIG.ACCELEROMETER_MIN, acceX));
			acceX = (acceX - CONFIG.ACCELEROMETER_MIN) / CONFIG.ACCELEROMETER_MAX;
			if (sign) acceX = -acceX;
			player->SetPan(acceX);
		}
		else {
			if (IsInputDown(INPUT_LEFT)) {
				analogX -= analogXAcce;
				if (analogX < -1.0f)analogX = -1.0f;
			}
			else if (IsInputDown(INPUT_RIGHT)) {
				analogX += analogXAcce;
				if (analogX > 1.0f)analogX = 1.0f;
			}
			else {
				bool sign = analogX < 0.0f;
				analogX = abs(analogX) - analogXAcce;
				if (analogX < 0.0f) analogX = 0.0f;
				if (sign) {
					analogX = -analogX;
				}
			}
			float analogLeftX = GetInputAnalogValue(ANALOG_STATE_LEFT_X);
			if (abs(analogLeftX) > abs(analogX)) {
				player->SetPan(analogLeftX);
			}
			else {
				player->SetPan(analogX);
			}
			
		}

		// スクリーンのサイズを取得
		F2 screenSize = GetScreenSize();

		if (player->IsFever()) {
			SEFever->SetSpeed(player->IsFever() * 1.0f);
		}
		else {
			SEFever->Stop();
		}

		// フィーバータイム
		if (IsInputTrigger(INPUT_OK)) {
			if (!player->IsFever()) {
				player->StartFever();
				if (player->GetFeverAmount() > 0.0f) {
					SECutin->Play();
					SEFever->Play();
					SEFever->SetSpeed(player->IsFever() * 1.0f);
					cutinFade = 0.0f;
					cutinHold = 0.0f;
					cutin = true;
				}
				
			}
		}

		// カットイン
		if (cutin) {
			cutinFever->enable = true;
			if (cutinFade == 1.0f) {
				if (cutinHold == 1.0f) {
					cutin = false;
				}
				cutinHold.IncreaseValue(GetDeltaTime());
			}
			cutinFade.IncreaseValue(GetDeltaTime());
		}
		else {
			cutinFade.IncreaseValue(-GetDeltaTime());
		}
		cutinFever->position = F3{ (screenSize.x * -0.5f + cutinFever->size.x * 0.5f) * (2.0f - cutinFade), screenSize.y * -0.5f + cutinFever->size.y * 0.5f };


		// コースアウト処理
		if (!player->InCourse()) {
			if (curtain->GetTime() == 0.0f) {
				SEFalling->Play();
			}
			curtain->enable = true;
		}
		if (curtain->enable) {
			if (curtain->GetTime() == 1.0f) {
				curtain->SetReverse(true);

				// プレイヤー転移
				player->ResetPosition();
			}
			if (curtain->GetReverse() && curtain->GetTime() == 0.0f) {
				curtain->enable = false;
				curtain->Reset();
			}
		}
		hintsLabel->SetValue(GetHintsLabel());

		// 時間更新
		time += GetDeltaTime() * 0.001f;
		uiTime->SetValue(StringToWString(FormatTime(time)));

		// カメラ更新
		cameraOffset = cameraOffset * 0.8f + player->velocity * 0.02f * 0.8f;
		currentCamera->SetPosition(player->position + Rotate(F3{ 0.0f, 0.7f, -0.7f } - cameraOffset, player->rotate));
		Course::GRAVITY gravity = course->GetGravity(currentCamera->GetPosition());
		currentCamera->SetFront(
			player->position + Rotate({ 0.0f, 0.2f, 1.0f }, player->rotate) - currentCamera->GetPosition()
			, Lerp(currentCamera->GetUpper(), gravity.upper, GetDeltaTime() * 0.001f * 6.0f)
		);

		Scene::Update();

		for (auto itr = gameObjects.begin(); itr != gameObjects.end(); itr++) {
			GameObject* gameObject = *itr;
			if (!gameObject->enable) continue;
			if (GetLayerType(gameObject->layer) == LAYER_TYPE_2D) continue;
			player->Collision(gameObject); // onCollisionトリガー
		}

		if (player->InGoal()) {
			SEGoal->Play();
			startCountLabel->SetValue(L"CLEAR!");
			startCountLabel->enable = true;
			startCountLabel->size = { 600.0f, 300.0f };
			updateFunc = [this, p0 = currentCamera->GetPosition(), t = Progress{ 1000.0f, false }]() mutable { UpdateGoal(p0, t); };
		}

		float fever = player->IsFever();
		if (fever) {

			float scale = easeInCirc(fever);
			currentCamera->SetAngle(60.0f + 20.0f * scale);

			feverEffects[0] = {
				F3 {
					((float)rand() / RAND_MAX) * 5.0f * ((rand() % 2) ? 1.0f : -1.0f) * scale,
					((float)rand() / RAND_MAX) * 5.0f * ((rand() % 2) ? 1.0f : -1.0f) * scale,
					0.0f
				},
				F3{ screenSize.x, screenSize.y, 0.0f } * (1.0f + 0.1f * scale),
				F4{ 1.0f, 1.0f, 1.0f, 0.2f * scale }
			};

			feverEffects[1] = {
				F3 {
					((float)rand() / RAND_MAX) * 5.0f * ((rand() % 2) ? 1.0f : -1.0f) * scale,
					((float)rand() / RAND_MAX) * 5.0f * ((rand() % 2) ? 1.0f : -1.0f) * scale,
					0.0f
				},
				F3{ screenSize.x, screenSize.y, 0.0f } *(1.0f + 0.06f * scale),
				F4{ 1.0f, 1.0f, 1.0f, 0.2f * scale }
			};
		}
		else {
			currentCamera->SetAngle(60.0f);
		}

		#if _DEBUG
		if (IsInputTrigger(INPUT_SELECT)) {
			Course::GRAVITY gravity = course->GetGravity(0.9f);
			player->position = gravity.position;
			player->rotate = gravity.rotate;
		}
		#endif
		
		if (IsInputTrigger(INPUT_START)) {
			SEOK->Play();
			updateFunc = [this]() -> void { UpdatePause(); };
		}
	}

	void MainGameScene::UpdateGoal(F3& p0, Progress &t)
	{
		static Progress holdT{ 5000.0f, false };
		
		static std::set<unsigned int> layers = {
				LAYER_PLAYER,
				LAYER_GALLERY,
				LAYER_TALK,
				LAYER_COUNT
		};

		if (t == 1.0f) {
			updateFunc = [this]() -> void { UpdateEpilogue(); };
		}

		if (!SEFever->IsFinished()) {
			SEFever->SetVolume(1.0f - t);
		}

		// 減速
		player->velocity *= 1.0f - 2.0f * GetDeltaTime() * 0.001f;
		if (DistanceSquare(player->velocity, {}) < 0.1f) {
			// プレイヤーが止まってから

			for (auto& gameObject : gameObjects) {
				if (!layers.count(gameObject->layer)) {
					gameObject->color.w = 1.0f - t;
				}
			}

			player->velocity = {};
			gallery->enable = true;
			gallery->position = player->position;
			gallery->rotate = player->rotate;
			gallery->color.w = t;
			startCountLabel->color.w = t;
			startCountLabel->position = { 0.0f, 200.0f * t };

			// カメラ更新
			F3 p1 = player->position + Rotate(CONFIG.GOAL_CAMERA_ANGLE * CONFIG.GOAL_CAMERA_DISTANCE, player->rotate);
			currentCamera->SetPosition(Lerp(p0, p1, t));
			currentCamera->SetFront(player->position + F3{ 0.0f, 1.0f, 0.0f } - currentCamera->GetPosition());

			t.IncreaseValue(GetDeltaTime());
		}
		
		Scene::Update();
		
	}

	void MainGameScene::UpdatePause()
	{
		hintsLabel->enable = false;
		uiPause->enable = true;
		uiPause->Update();
		if (!InTransition()) {
			if (IsInputTrigger(INPUT_UP)) {
				SECursor->Play();
				uiPause->Up();
			}
			else if (IsInputTrigger(INPUT_DOWN)) {
				SECursor->Play();
				uiPause->Down();
			}
			bool resume = false;
			if (IsInputTrigger(INPUT_OK)) {
				SEOK->Play();
				UIPause::PAUSE_OPTION selected = uiPause->GetOption();
				if (selected == UIPause::PAUSE_OPTION::PAUSE_OPTION_RESUME) {
					resume = true;
				}
				else if (selected == UIPause::PAUSE_OPTION::PAUSE_OPTION_STAGE) {
					SceneTransit("stage_select", "star");
				}
				else if (selected == UIPause::PAUSE_OPTION::PAUSE_OPTION_TITLE) {
					SceneTransit("title", "star");
				}
				else if (selected == UIPause::PAUSE_OPTION::PAUSE_OPTION_GYRO) {
					gyro = !gyro;
					SetCommonBool("gyro", gyro);
				}
			}
			else if (IsInputTrigger(INPUT_START) || IsInputTrigger(INPUT_CANCEL)) {
				SEOK->Play();
				resume = true;
			}
			if (resume) {
				hintsLabel->enable = true;
				uiPause->enable = false;
				updateFunc = [this]() -> void { UpdateStart(); };
			}
		}
	}

	std::wstring MainGameScene::GetHintsLabel()
	{
		std::wstring hintsStr;
		if (gyro) {
			hintsStr = L"ジャイロで";
		}
		else {
			hintsStr = GetInputLabel(L"{AnalogLeft}");
		}
		hintsStr += L"横移動\n";
		hintsStr += GetInputLabel(L"{OK}") + L"フィーバーゲージを使う\n";
		hintsStr += GetInputLabel(L"{Start}") + L"一時停止";
		return hintsStr;
	}

	// =======================================================
	// 描画
	// =======================================================
	void MainGameScene::Draw()
	{
		Renderer* renderer = GetRenderer();

		// スクリーンの中心座標を取得
		F2 screenCenter = GetScreenCenter();

		// スクリーンのサイズを取得
		F2 screenSize = GetScreenSize();

		float fever = player->IsFever();
		if (fever) {
			// サブ描画ターゲット設定
			renderer->SetRenderTarget(effectRenderTarget);
			renderer->ClearRenderTargetView(effectRenderTarget);
			renderer->SetViewport(screenCenter.x, screenCenter.y, screenSize.x, screenSize.y);
		}

		// 背景
		renderer->ApplyCamera(currentCamera);
		renderer->SetDepthState(DEPTH_STATE_NO_WRITE_DRAW_ALL);
		DrawModel(background, currentCamera->GetPosition());
		renderer->SetDepthState(DEPTH_STATE_ENABLE);

		//Scene::Draw();
		LAYER_TYPE currentLayerType = LAYER_TYPE_NONE;
		for (auto& gameObject : gameObjects) {
			if (!gameObject->enable || gameObject->layer >= LAYER_2D) continue;

			LAYER_TYPE type = GetLayerType(gameObject->layer);
			if (currentLayerType != type) {
				currentLayerType = type;
				if (type == LAYER_TYPE_2D) {
					renderer->ApplyCamera(currentCamera2D);
				}
				else {
					renderer->ApplyCamera(currentCamera);
				}
			}
			gameObject->Draw();
		}

		if (fever) {
			// メイン描画ターゲットに切り替える
			renderer->SetRenderTarget(renderTarget);
			if (renderTarget) {
				renderer->SetViewport(
					renderTarget->texture->GetWidth() * 0.5f,
					renderTarget->texture->GetHeight() * 0.5f,
					renderTarget->texture->GetWidth(),
					renderTarget->texture->GetHeight()
				);
			}
			else {
				renderer->SetViewport(
					GetScreenWidth() * 0.5f,
					GetScreenHeight() * 0.5f,
					GetScreenWidth(),
					GetScreenHeight()
				);
			}

			renderer->ApplyCamera(currentCamera2D);
			DrawQuad(
				effectRenderTarget->texture,
				{ 0.0f, 0.0f, 0.0f },
				{ screenSize.x, screenSize.y, 0.0f },
				{ 0.0f, 0.0f, 0.0f },
				{ 1.0f, 1.0f, 1.0f, 1.0f }
			);

			//renderer->SetBlendState(BLEND_STATE_ADD);
			for (int i = 0; i < feverEffects.size(); i++) {
				DrawQuad(
					effectRenderTarget->texture,
					feverEffects[i].position,
					feverEffects[i].size,
					Quaternion::Identity(),
					feverEffects[i].color
				);
			}
			//renderer->SetBlendState(BLEND_STATE_ALPHA);
		}

		renderer->ApplyCamera(currentCamera2D);
		for (auto& gameObject : gameObjects) {
			if (gameObject->enable && gameObject->layer >= LAYER_2D) {
				gameObject->Draw();
			}
		}
		
	}

	// =======================================================
	// レイヤータイプの定義
	// =======================================================
	/*LAYER_TYPE MainGameScene::GetLayerType(int layer)
	{
		return Scene::GetLayerType(layer);
	}*/

	void MainGameScene::SortGameObjects()
	{
		gameObjects.sort([this](GameObject* a, GameObject* b) -> bool {
			if (a->layer == b->layer) {
				return DistanceSquare(currentCamera->position, a->position) > DistanceSquare(currentCamera->position, b->position);
			}
			return a->layer < b->layer;
		});
	}
}
