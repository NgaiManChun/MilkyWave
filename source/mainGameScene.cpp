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

// =======================================================
// メインゲームシーン
// ステージ生成、UI生成、ゲーム進行、ポーズ、描画を管理する
// =======================================================
namespace MainGameScene {

	// 描画・更新順を制御するレイヤー定義
	static constexpr const int LAYER_PLAYER = 3;
	static constexpr const int LAYER_GALLERY = 4;
	static constexpr const int LAYER_ITEMS = 5;
	static constexpr const int LAYER_SURFACE = 6;
	static constexpr const int LAYER_UI = LAYER_2D + 1;
	static constexpr const int LAYER_COUNT = LAYER_UI + 1;
	static constexpr const int LAYER_TALK = LAYER_COUNT + 1;
	static constexpr const int LAYER_CURTAIN = LAYER_TALK + 1;
	static constexpr const int LAYER_PAUSE = LAYER_CURTAIN + 1;

	// 配置オブジェクトの設定CSV
	static constexpr const char* INSTANCE_LIST = "asset\\instance.csv";

	// フィーバー演出用イージング
	static float easeInExpo(float t) {
		return (powf(2.0f, 8.0f * t) - 1.0f) / 255.0f;
	}

	// フィーバー演出用イージング
	static float easeInCirc(float t) {
		return 1.0f - sqrtf(1.0f - t);
	}

	// =======================================================
	// 設定値ロード
	// =======================================================
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

	// =======================================================
	// アセットロード
	// =======================================================
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

		Uninit();

		// =======================================================
		// ステージ関連のロード
		// 背景、コース、観客、配置オブジェクトを生成する
		// =======================================================
		{
			background = LoadModel(GetCommonString("course_background"));

			// コースの見た目・当たり判定・レールアニメーションを生成
			MGObject courseCollision = LoadMGO(GetCommonString("course_collision").c_str());
			course = AddGameObject(
				Course(
					LoadModel(GetCommonString("course_surface")),
					LoadAnimation(GetCommonString("course_rail")),
					GetArrangementByMGObject(courseCollision)
				), LAYER_SURFACE
			);
			courseCollision.Release();

			// ゴール後に表示する観客モデル
			gallery = AddGameObject(
				GameObjectModel(
					LoadModel(GetCommonString("course_gallery")),
					LoadAnimation(GetCommonString("course_gallery_animation"))
				),
				LAYER_GALLERY
			);
			gallery->enable = false;

			// =======================================================
			// コース上の配置オブジェクト生成
			// Maya等から出力した配置情報とCSV設定を紐づけて生成する
			// =======================================================
			{
				D_TABLE table;
				D_KVTABLE keyValuePair;

				// instance.csvを読み込み
				ReadCSVFromPath(INSTANCE_LIST, table);

				// instance_idをキーにして検索しやすい形へ変換
				TableToKeyValuePair("instance_id", table, keyValuePair);

				// 配置データを読み込み
				MGObject mgo = LoadMGO(GetCommonString("course_items").c_str());
				ARRANGEMENT* items = GetArrangementByMGObject(mgo);

				for (int i = 0; i < items->instanceNum; i++) {

					// CSVに設定があるインスタンスだけ生成
					if (keyValuePair.count(items->instances[i].instance)) {
						auto& row = keyValuePair.at(items->instances[i].instance);
						std::string type = row["type"];

						// 石オブジェクト
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
									items->instances[i].position,
									items->instances[i].scale,
									items->instances[i].rotate,
									sceneName
								),
								LAYER_ITEMS
							);
							mgi.Release();
						}

						// フィーバーゲージ回収アイテム
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
									items->instances[i].position,
									items->instances[i].scale,
									items->instances[i].rotate,
									sceneName
								),
								LAYER_ITEMS
							);
							mgi.Release();
						}

						// 障害物
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
									items->instances[i].position,
									items->instances[i].scale,
									items->instances[i].rotate,
									sceneName
								),
								LAYER_ITEMS
							);
							mgi.Release();
						}

						// 通常の背景・装飾モデル
						else if (type == "other") {
							AddGameObject(
								GameObjectModel(
									LoadModel(row["model"]),
									nullptr,
									{},
									items->instances[i].scale,
									items->instances[i].position,
									items->instances[i].rotate
								),
								LAYER_ITEMS
							);
						}

						// ゴール判定
						else if (type == "goal") {
							AddGameObject(
								Goal(
									items->instances[i].position,
									items->instances[i].scale,
									items->instances[i].rotate,
									sceneName
								),
								LAYER_ITEMS
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

	// =======================================================
	// プロローグステート更新
	// =======================================================
	void MainGameScene::UpdatePrologue()
	{
		// プロローグ中はプレイヤーと会話UIだけ表示する
		for (auto& gameObject : gameObjects) {
			if (gameObject->layer == LAYER_PLAYER || gameObject == prologue) {
				gameObject->enable = true;
			}
			else {
				gameObject->enable = false;
			}
		}

		// プロローグ中はプレイヤーを停止させる
		player->SetStop(true);

		// 演出用にフィーバー状態にする
		player->SetFeverAmount(1.0f);
		player->StartFever();

		// カメラをプレイヤーの周囲でゆっくり回転させる
		currentCamera->SetRotate(
			Quaternion::AxisYRadian(PI * GetDeltaTime() * 0.0001) * currentCamera->GetRotate()
		);
		currentCamera->SetPosition(player->position - currentCamera->GetFront() * 2.0f);

		// 決定入力で次のセリフへ進める
		if (IsInputTrigger(INPUT_OK)) {
			prologue->NextLine();
		}

		Scene::Update();

		// START入力でプロローグをスキップ
		if (IsInputTrigger(INPUT_START)) {
			prologue->Skip();
		}

		// プロローグ終了後、スタートカウントへ移行
		if (prologue->IsEnd()) {
			Progress startCount{ 3000.0f, false };
			startCount = 1.0f;

			// 本編開始前にフィーバーゲージをリセット
			player->SetFeverAmount(0.0f);

			SEReady->Play();

			// カウントダウン更新処理へ切り替え
			updateFunc = [this, t = startCount]() mutable { UpdateStartCount(t); };
		}
		
	}

	// =======================================================
	// エピローグステート更新
	// =======================================================
	void MainGameScene::UpdateEpilogue()
	{
		// エピローグ会話UIを表示
		epilogue->enable = true;

		// 決定入力で次のセリフへ進める
		if (IsInputTrigger(INPUT_OK)) {
			epilogue->NextLine();
		}

		// ゴール後カメラをプレイヤー中心に回転させる
		currentCamera->SetRotate(
			Quaternion::AxisYDegree(CONFIG.GOAL_CAMERA_ROTATE * GetDeltaTime() * 0.001f) * currentCamera->GetRotate()
		);
		currentCamera->SetPosition(
			player->position + F3{ 0.0f, 1.0f, 0.0f } - currentCamera->GetFront() * CONFIG.GOAL_CAMERA_DISTANCE
		);

		Scene::Update();

		// START入力でエピローグをスキップ
		if (IsInputTrigger(INPUT_START)) {
			epilogue->Skip();
		}

		// エピローグ終了後、リザルトへ遷移
		if (!InTransition() && epilogue->IsEnd()) {
			// リザルトシーンで使うクリアタイムを保存
			SetCommonFloat("result_time", time);

			SceneTransit("result", "star");
		}
		
	}

	// =======================================================
	// カウントダウンステート更新
	// =======================================================
	void MainGameScene::UpdateStartCount(Progress& t)
	{
		// カウントダウン中に表示するレイヤー
		static std::set<unsigned int> layers = {
			LAYER_PLAYER,
			LAYER_ITEMS,
			LAYER_SURFACE,
			LAYER_UI,
			LAYER_COUNT
		};

		// 必要なレイヤーだけ表示する
		for (auto& gameObject : gameObjects) {
			if (layers.count(gameObject->layer)) {
				gameObject->enable = true;
			}
			else {
				gameObject->enable = false;
			}
		}

		// カウント表示を有効化
		startCountLabel->enable = true;

		// カウント中はプレイヤー操作を停止
		player->SetStop(true);

		// カウント演出として、プレイヤーをスタート位置の少し後ろから前へ移動させる
		player->position = startPosition + Rotate(
			{ 0.0f, 0.0f, -CONFIG.START_OFFSET * t },
			player->rotate
		);

		// 残り秒数から表示するカウント番号を計算
		float sec = t.GetValue() / 1000.0f;
		int countNumber = ceil(sec);

		// 秒の小数部分。文字のフェード・拡大縮小に使用
		float s = sec - floorf(sec);

		// カウントごとにカメラ位置を切り替える
		if (countNumber == 3) {
			currentCamera->SetPosition(
				startPosition + Rotate({ -1.0f, -1.0f, -CONFIG.START_OFFSET * 0.8f }, player->rotate)
			);
			currentCamera->SetFront(player->position - currentCamera->GetPosition());
		}
		else if (countNumber == 2) {
			currentCamera->SetPosition(
				startPosition + Rotate({ 1.0f, 1.0f, -CONFIG.START_OFFSET * 0.5f }, player->rotate)
			);
			currentCamera->SetFront(player->position - currentCamera->GetPosition());
		}
		else if (countNumber == 1) {
			currentCamera->SetPosition(
				startPosition + Rotate({ 0.0f, -2.0f, -CONFIG.START_OFFSET * 0.2f }, player->rotate)
			);
			currentCamera->SetFront(player->position - currentCamera->GetPosition());
		}

		// カウント数字を表示
		startCountLabel->SetValue(std::to_wstring(countNumber));

		// カウント文字を徐々に透明にする
		startCountLabel->color.w = s;

		// カウント文字のサイズ演出
		startCountLabel->size = F3{ 300.0f, GetScreenSize().y * 0.8f } *(1.0f - s);

		// シーン遷移中でなければカウントを進める
		if (!InTransition()) {
			t.IncreaseValue(-GetDeltaTime());
		}

		// カウント終了後、ゲーム開始
		if (t == 0.0f) {
			SEGo->Play();

			// プレイヤーを正式なスタート位置へ戻す
			player->position = startPosition;

			// 初速を与えてスタート感を出す
			player->velocity = { 0.0f, 0.0f, 9.0f };

			// プレイヤー操作開始
			player->SetStop(false);

			// START表示に切り替える
			startCountLabel->SetValue(L"START");

			// スタート演出更新へ切り替え
			updateFunc = [this]() -> void { UpdateStart(); };
		}

		Scene::Update();
	}

	// =======================================================
	// ゲームスタートステート更新
	// =======================================================
	void MainGameScene::UpdateStart()
	{
		// START表示を徐々に消す
		startCountLabel->color.w = 1.0f - startProgress;

		// START表示を拡大する
		startCountLabel->size = F3{ 300.0f, GetScreenSize().y * 0.8f } *startProgress;

		// スタート演出の進行度を進める
		startProgress.IncreaseValue(GetDeltaTime());

		// START演出中もゲーム本編は進める
		UpdatePlay();

		// START演出が終わったら表示を消し、通常プレイ更新へ完全移行
		if (startProgress == 1.0f) {
			startCountLabel->enable = false;
			updateFunc = [this]() -> void { UpdatePlay(); };
		}
		
	}

	// =======================================================
	// ゲーム本編ステート更新
	// =======================================================
	void MainGameScene::UpdatePlay()
	{
		// =======================================================
		// プレイヤー入力処理
		// ジャイロが有効なら傾き入力、無効ならキー・スティック入力を使う
		// =======================================================
		if (gyro) {
			float acceX = GetInputAnalogValue(ANALOG_STATE_ACCE_X);

			// 符号を一度保存して、絶対値で入力範囲を補正
			bool sign = acceX < 0.0f;
			acceX = abs(acceX);

			// 小さすぎる入力を無視し、大きすぎる入力を制限
			acceX = min(CONFIG.ACCELEROMETER_MAX, max(CONFIG.ACCELEROMETER_MIN, acceX));

			// 0.0～1.0の範囲へ正規化
			acceX = (acceX - CONFIG.ACCELEROMETER_MIN) / CONFIG.ACCELEROMETER_MAX;

			if (sign) acceX = -acceX;

			player->SetPan(acceX);
		}
		else {
			// キーボード・デジタル入力用の疑似アナログ値
			if (IsInputDown(INPUT_LEFT)) {
				analogX -= analogXAcce;
				if (analogX < -1.0f) analogX = -1.0f;
			}
			else if (IsInputDown(INPUT_RIGHT)) {
				analogX += analogXAcce;
				if (analogX > 1.0f) analogX = 1.0f;
			}
			else {
				// 入力がない場合は中央に戻す
				bool sign = analogX < 0.0f;
				analogX = abs(analogX) - analogXAcce;
				if (analogX < 0.0f) analogX = 0.0f;
				if (sign) analogX = -analogX;
			}

			// スティック入力があれば、より大きい入力を採用
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


		// =======================================================
		// フィーバー開始処理
		// OK入力でフィーバーゲージを使用する
		// =======================================================
		if (IsInputTrigger(INPUT_OK)) {
			if (!player->IsFever()) {
				player->StartFever();

				// ゲージが残っていれば演出・SE開始
				if (player->GetFeverAmount() > 0.0f) {
					SECutin->Play();
					SEFever->Play();
					SEFever->SetSpeed(player->IsFever());
					cutinFade = 0.0f;
					cutinHold = 0.0f;
					cutin = true;
				}
			}
		}

		// =======================================================
		// フィーバー発動カットイン処理
		// =======================================================
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


		// =======================================================
		// コースアウト処理
		// カーテン演出後、プレイヤーを安全位置へ戻す
		// =======================================================
		if (!player->InCourse()) {
			if (curtain->GetTime() == 0.0f) {
				SEFalling->Play();
			}
			curtain->enable = true;
		}

		if (curtain->enable) {
			if (curtain->GetTime() == 1.0f) {
				curtain->SetReverse(true);
				player->ResetPosition();
			}

			if (curtain->GetReverse() && curtain->GetTime() == 0.0f) {
				curtain->enable = false;
				curtain->Reset();
			}
		}

		// 操作ヒント（入力デバイスに随時対応）
		hintsLabel->SetValue(GetHintsLabel());

		// =======================================================
		// タイム更新
		// =======================================================
		time += GetDeltaTime() * 0.001f;
		uiTime->SetValue(StringToWString(FormatTime(time)));


		// =======================================================
		// カメラ更新
		// =======================================================
		cameraOffset = cameraOffset * 0.8f + player->velocity * 0.02f * 0.8f;
		currentCamera->SetPosition(player->position + Rotate(F3{ 0.0f, 0.7f, -0.7f } - cameraOffset, player->rotate));
		Course::GRAVITY gravity = course->GetGravity(currentCamera->GetPosition());
		currentCamera->SetFront(
			player->position + Rotate({ 0.0f, 0.2f, 1.0f }, player->rotate) - currentCamera->GetPosition()
			, Lerp(currentCamera->GetUpper(), gravity.upper, GetDeltaTime() * 0.001f * 6.0f)
		);

		Scene::Update();


		// =======================================================
		// プレイヤーとの当たり判定
		// 2Dレイヤー以外の有効オブジェクトを対象にする
		// =======================================================
		for (auto itr = gameObjects.begin(); itr != gameObjects.end(); itr++) {
			GameObject* gameObject = *itr;
			if (!gameObject->enable) continue;
			if (GetLayerType(gameObject->layer) == LAYER_TYPE_2D) continue;
			player->Collision(gameObject); // onCollisionトリガー
		}

		// =======================================================
		// ゴール判定
		// =======================================================
		if (player->InGoal()) {
			SEGoal->Play();
			startCountLabel->SetValue(L"CLEAR!");
			startCountLabel->enable = true;
			startCountLabel->size = { 600.0f, 300.0f };
			updateFunc = [this, p0 = currentCamera->GetPosition(), t = Progress{ 1000.0f, false }]() mutable { UpdateGoal(p0, t); };
		}

		// =======================================================
		// フィーバー時エフェクト処理
		// =======================================================
		float fever = player->IsFever();
		if (fever) {

			// カメラ引き
			float scale = easeInCirc(fever);
			currentCamera->SetAngle(60.0f + 20.0f * scale);

			// ブラー効果レンダーターゲット設定
			feverEffects[0] = {
				F3 {
					((float)rand() / RAND_MAX) * 5.0f * ((rand() % 2) ? 1.0f : -1.0f) * scale,
					((float)rand() / RAND_MAX) * 5.0f * ((rand() % 2) ? 1.0f : -1.0f) * scale,
					0.0f
				},																// ポジション
				F3{ screenSize.x, screenSize.y, 0.0f } * (1.0f + 0.1f * scale),	// サイズ
				F4{ 1.0f, 1.0f, 1.0f, 0.2f * scale }							// カラー
			};

			feverEffects[1] = {
				F3 {
					((float)rand() / RAND_MAX) * 5.0f * ((rand() % 2) ? 1.0f : -1.0f) * scale,
					((float)rand() / RAND_MAX) * 5.0f * ((rand() % 2) ? 1.0f : -1.0f) * scale,
					0.0f
				},																 // ポジション
				F3{ screenSize.x, screenSize.y, 0.0f } *(1.0f + 0.06f * scale),	 // サイズ
				F4{ 1.0f, 1.0f, 1.0f, 0.2f * scale }							 // カラー
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
		
		// =======================================================
		// 一時停止処理
		// =======================================================
		if (IsInputTrigger(INPUT_START)) {
			SEOK->Play();
			updateFunc = [this]() -> void { UpdatePause(); };
		}
	}

	// =======================================================
	// ゴールステート更新
	// =======================================================
	void MainGameScene::UpdateGoal(F3& p0, Progress &t)
	{
		// ゴール演出中に表示・更新対象として残すレイヤー
		static std::set<unsigned int> layers = {
				LAYER_PLAYER,
				LAYER_GALLERY,
				LAYER_TALK,
				LAYER_COUNT
		};

		// ゴール演出が完了したらエピローグへ移行
		if (t == 1.0f) {
			updateFunc = [this]() -> void { UpdateEpilogue(); };
		}

		// フィーバーSEが再生中なら、ゴール演出の進行に合わせて音量を下げる
		if (!SEFever->IsFinished()) {
			SEFever->SetVolume(1.0f - t);
		}

		// プレイヤーを徐々に減速させる
		player->velocity *= 1.0f - 2.0f * GetDeltaTime() * 0.001f;

		// プレイヤーが十分遅くなったら、ゴール演出本体を開始
		if (DistanceSquare(player->velocity, {}) < 0.1f) {

			// 指定レイヤー以外のオブジェクトを徐々にフェードアウト
			for (auto& gameObject : gameObjects) {
				if (!layers.count(gameObject->layer)) {
					gameObject->color.w = 1.0f - t;
				}
			}

			// プレイヤーを完全停止
			player->velocity = {};

			// 観客モデルをプレイヤー位置に表示
			gallery->enable = true;
			gallery->position = player->position;
			gallery->rotate = player->rotate;
			gallery->color.w = t;

			// CLEAR表示をフェードインしつつ上へ移動
			startCountLabel->color.w = t;
			startCountLabel->position = { 0.0f, 200.0f * t };

			// ゴール演出用のカメラ位置を計算
			F3 p1 = player->position + Rotate(CONFIG.GOAL_CAMERA_ANGLE * CONFIG.GOAL_CAMERA_DISTANCE, player->rotate);

			// 現在位置p0からゴール演出用位置p1へカメラを補間
			currentCamera->SetPosition(Lerp(p0, p1, t));

			// カメラをプレイヤー少し上方向へ向ける
			currentCamera->SetFront(player->position + F3{ 0.0f, 1.0f, 0.0f } - currentCamera->GetPosition());

			// ゴール演出の進行度を進める
			t.IncreaseValue(GetDeltaTime());
		}
		
		// シーン全体を更新
		Scene::Update();
		
	}

	// =======================================================
	// 一時停止ステート更新
	// =======================================================
	void MainGameScene::UpdatePause()
	{
		// プレイ中ヒントは非表示、ポーズUIを表示
		hintsLabel->enable = false;
		uiPause->enable = true;

		// ポーズUI自体の更新（選択状態など）
		uiPause->Update();

		// シーン遷移中でなければ入力受付
		if (!InTransition()) {

			// 上入力：カーソルを上へ移動
			if (IsInputTrigger(INPUT_UP)) {
				SECursor->Play();
				uiPause->Up();
			}
			// 下入力：カーソルを下へ移動
			else if (IsInputTrigger(INPUT_DOWN)) {
				SECursor->Play();
				uiPause->Down();
			}

			bool resume = false; // 一時停止解除フラグ

			// 決定入力
			if (IsInputTrigger(INPUT_OK)) {
				SEOK->Play();

				// 現在選択されているメニューを取得
				UIPause::PAUSE_OPTION selected = uiPause->GetOption();

				if (selected == UIPause::PAUSE_OPTION::PAUSE_OPTION_RESUME) {
					// ゲームに戻る
					resume = true;
				}
				else if (selected == UIPause::PAUSE_OPTION::PAUSE_OPTION_STAGE) {
					// ステージ選択へ遷移
					SceneTransit("stage_select", "star");
				}
				else if (selected == UIPause::PAUSE_OPTION::PAUSE_OPTION_TITLE) {
					// タイトルへ戻る
					SceneTransit("title", "star");
				}
				else if (selected == UIPause::PAUSE_OPTION::PAUSE_OPTION_GYRO) {
					// ジャイロ操作ON/OFF切り替え
					gyro = !gyro;
					SetCommonBool("gyro", gyro);
				}
			}
			// スタート or キャンセル入力でもポーズ解除
			else if (IsInputTrigger(INPUT_START) || IsInputTrigger(INPUT_CANCEL)) {
				SEOK->Play();
				resume = true;
			}

			// ポーズ解除処理
			if (resume) {
				hintsLabel->enable = true;   // ヒント再表示
				uiPause->enable = false;     // ポーズUI非表示

				// ゲーム本編の更新に戻す（カウント後の状態へ）
				updateFunc = [this]() -> void { UpdateStart(); };
			}
		}
	}

	// =======================================================
	// 操作ヒント文字列を取得
	// =======================================================
	std::wstring MainGameScene::GetHintsLabel()
	{
		std::wstring hintsStr;

		// 横移動の説明
		// ジャイロONなら「ジャイロで」、OFFならスティック表記を表示
		if (gyro) {
			hintsStr = L"ジャイロで";
		}
		else {
			// 入力デバイスに応じたラベル（例：Lスティック）を取得
			hintsStr = GetInputLabel(L"{AnalogLeft}");
		}

		// 横移動説明を追加
		hintsStr += L"横移動\n";

		// フィーバー使用方法（OKボタン）
		hintsStr += GetInputLabel(L"{OK}") + L"フィーバーゲージを使う\n";

		// ポーズ操作（STARTボタン）
		hintsStr += GetInputLabel(L"{Start}") + L"一時停止";

		// 完成したヒント文字列を返す
		return hintsStr;
	}

	// =======================================================
	// 描画
	// =======================================================
	void MainGameScene::Draw()
	{
		Renderer* renderer = GetRenderer();

		// スクリーン中心座標を取得
		F2 screenCenter = GetScreenCenter();

		// スクリーンサイズを取得
		F2 screenSize = GetScreenSize();

		// フィーバー状態の進行度を取得
		float fever = player->IsFever();

		if (fever) {
			// フィーバー中は一度サブ描画ターゲットへ描画する
			// 後でこの結果を揺らし・拡大などのエフェクトとして重ねる
			renderer->SetRenderTarget(effectRenderTarget);
			renderer->ClearRenderTargetView(effectRenderTarget);
			renderer->SetViewport(screenCenter.x, screenCenter.y, screenSize.x, screenSize.y);
		}

		// =======================================================
		// 背景描画
		// =======================================================

		// 3Dカメラを適用
		renderer->ApplyCamera(currentCamera);

		// 背景は深度を書き込まず、常に描画する
		renderer->SetDepthState(DEPTH_STATE_NO_WRITE_DRAW_ALL);
		DrawModel(background, currentCamera->GetPosition());

		// 通常の深度設定に戻す
		renderer->SetDepthState(DEPTH_STATE_ENABLE);

		// =======================================================
		// 3Dオブジェクト描画
		// =======================================================

		LAYER_TYPE currentLayerType = LAYER_TYPE_NONE;

		for (auto& gameObject : gameObjects) {
			// 無効なオブジェクト、または2Dレイヤーはここでは描画しない
			if (!gameObject->enable || gameObject->layer >= LAYER_2D) continue;

			// レイヤー種類が変わったら対応するカメラに切り替える
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

			// オブジェクト描画
			gameObject->Draw();
		}

		if (fever) {
			// =======================================================
			// フィーバー時のポストエフェクト描画
			// =======================================================

			// 描画先をメイン描画ターゲットへ戻す
			renderer->SetRenderTarget(renderTarget);

			// メイン描画ターゲットのサイズに合わせてビューポートを設定
			if (renderTarget) {
				renderer->SetViewport(
					renderTarget->texture->GetWidth() * 0.5f,
					renderTarget->texture->GetHeight() * 0.5f,
					renderTarget->texture->GetWidth(),
					renderTarget->texture->GetHeight()
				);
			}
			else {
				// 通常の画面描画時
				renderer->SetViewport(
					GetScreenWidth() * 0.5f,
					GetScreenHeight() * 0.5f,
					GetScreenWidth(),
					GetScreenHeight()
				);
			}

			// 2Dカメラに切り替えて、サブ描画ターゲットの内容を画面へ描画
			renderer->ApplyCamera(currentCamera2D);

			// サブ描画ターゲットを通常サイズで描画
			DrawQuad(
				effectRenderTarget->texture,
				{ 0.0f, 0.0f, 0.0f },
				{ screenSize.x, screenSize.y, 0.0f },
				{ 0.0f, 0.0f, 0.0f },
				{ 1.0f, 1.0f, 1.0f, 1.0f }
			);

			// サブ描画ターゲットを少しずらして重ね、フィーバー中の残像・揺れ表現を作る
			for (int i = 0; i < feverEffects.size(); i++) {
				DrawQuad(
					effectRenderTarget->texture,
					feverEffects[i].position,
					feverEffects[i].size,
					Quaternion::Identity(),
					feverEffects[i].color
				);
			}
		}

		// =======================================================
		// 2D UI描画
		// =======================================================

		// 2Dカメラに切り替え
		renderer->ApplyCamera(currentCamera2D);

		for (auto& gameObject : gameObjects) {
			// 2Dレイヤーの有効なオブジェクトだけ描画
			if (gameObject->enable && gameObject->layer >= LAYER_2D) {
				gameObject->Draw();
			}
		}
	}

	void MainGameScene::SortGameObjects()
	{
		// レイヤー順に並べ替える
		// 同じレイヤー内では、カメラから遠い順に描画する
		// 半透明オブジェクトを奥から手前へ描くためのソート
		gameObjects.sort([this](GameObject* a, GameObject* b) -> bool {
			if (a->layer == b->layer) {
				return DistanceSquare(currentCamera->position, a->position) > DistanceSquare(currentCamera->position, b->position);
			}

			return a->layer < b->layer;
		});
	}
}
