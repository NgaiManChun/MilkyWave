#include "BGMScene.h"
#include "scene.h"
#include "CSVResource.h"
#include "config.h"
#include "gameObjectText.h"
#include "progress.h"
#include "input.h"
#include "mainGameScene.h"
#include "previewScene.h"
#include <thread>
#include <atomic>
using namespace MG;

namespace StageSelectScene {

	static constexpr const char* STATE_LIST = "asset\\stage.csv";
	static constexpr const char* TEXTURE_BAR = "asset\\texture\\bar2.png";
	static constexpr const char* TEXTURE_WHITE = "asset\\texture\\white.png";
	static constexpr const char* TEXTURE_SELECT_BG = "asset\\texture\\stage_select_BG.png";

	static constexpr const int LAYER_BG = LAYER_2D + 1;
	static constexpr const int LAYER_DEV = LAYER_BG + 1;
	static constexpr const int LAYER_PREVIEW = LAYER_DEV + 1;
	static constexpr const int LAYER_TITLE = LAYER_PREVIEW + 1;
	static constexpr const int LAYER_LOADING = LAYER_TITLE + 1;

	// =======================================================
	// クラス定義
	// =======================================================
	class StageSelectScene : public Scene {
	public:
		struct STAGE_DATA {
			std::string stage;
			std::string name;
			std::string background;
			std::string bgm;
			std::string rail;
			std::string collision;
			std::string surface;
			std::string items;
			std::string camera;
			std::string gallery;
			std::string gallery_animation;
			float rank_s_time;
			float rank_a_time;
			float rank_b_time;
			std::string prologue;
			std::string epilogue;
		};
		
		static struct _CONFIG {
			float PLAYER_MAX_VELOCITY;
			float PLAYER_MAX_VELOCITY_FEVER;
		};
		static struct _ASSET {
			std::string BGM_TITLE;
			std::string SE_CURSOR_MOVE;
			std::string SE_OK;
		};
	private:
		BGMScene::BGMScene* bgmScene;
		std::vector<STAGE_DATA> stageData;
		std::atomic<bool> stageLoaded = false;
		bool stageLoading = false;

		GameObjectText* labelSelect;
		GameObjectText* labelHints;
		std::vector<GameObjectText*> textObjs;
		int currentIndex;

		GameObjectQuad* background;
		GameObjectQuad* dev;
		GameObjectQuad* preview;
		GameObjectText* txtLoading;
		GameObjectAudio* SECursor;
		GameObjectAudio* SEOK;
		Progress flip{ 1000.0f, false };
		Progress roll{ 300.0f, false };
		Progress previewFade{ 500.0f, false };
		int rollDirect = 1;
		float titleWidth = 600.0f;
		float titlePadding = 20.0f;
		F3 currentPosition;
		F3 currentSize;
		RenderTarget* previewRenderTarget;
		PreviewScene::PreviewScene* previewScene;
		std::thread* loadThread;
		std::thread* previewThread;
		bool previewChanged = false;
		int previewIndex = 0;
		std::atomic<bool> previewReady = false;
		bool gyro = true;
	public:
		void Init() override;
		void Uninit() override;
		void Update() override;
		void Draw() override;
		//LAYER_TYPE GetLayerType(int layer) override;
	};

	static const StageSelectScene::_CONFIG CONFIG = LoadConfig<StageSelectScene::_CONFIG>("asset\\config.csv", [](const D_KVTABLE& table) -> StageSelectScene::_CONFIG {
		return {
			TABLE_FLOAT_VALUE(table, "PLAYER_MAX_VELOCITY", 9.0f),
			TABLE_FLOAT_VALUE(table, "PLAYER_MAX_VELOCITY_FEVER", 12.0f)
		};
	});

	static const StageSelectScene::_ASSET ASSET = LoadConfig<StageSelectScene::_ASSET>("asset\\asset_list.csv", [](const D_KVTABLE& table) -> StageSelectScene::_ASSET {
		return {
			TABLE_STR_VALUE(table, "BGM_TITLE", "asset\\sound\\title.wav"),
			TABLE_STR_VALUE(table, "SE_CURSOR_MOVE", "asset\\sound\\cursor_move.wav"),
			TABLE_STR_VALUE(table, "SE_OK", "asset\\sound\\ok.wav")
		};
	});


	// =======================================================
	// シーン登録
	// =======================================================
	static SceneName sceneName = REGISTER_SCENE("stage_select", StageSelectScene);

	// =======================================================
	// 初期化
	// =======================================================
	void StageSelectScene::Init()
	{
		Scene::Init();

		F2 screenSize = GetScreenSize();

		currentPosition = { screenSize.x * -0.5f + titleWidth * 0.5f, 0.0f };
		currentSize = { titleWidth - titlePadding - titlePadding, 70.0f };

		background = AddGameObject(
			GameObjectQuad(
				LoadTexture(TEXTURE_SELECT_BG),
				{ screenSize.x, screenSize.y }
			),
			LAYER_BG
		);

		dev = AddGameObject(
			GameObjectQuad(
				LoadTexture(TEXTURE_WHITE),
				{ titleWidth * 2.0f, screenSize.y },
				{ screenSize.x * -0.5f - titleWidth , 0.0f }
			),
			LAYER_DEV
		);
		dev->color = { 1.0f, 0.7843f, 0.0431f, 1.0f };

		labelSelect = AddGameObject(
			GameObjectText(
				L"ステージ選択",
				FONT{
					70.0f,
					"HG創英角ﾎﾟｯﾌﾟ体",
					FONT_WEIGHT_BOLD
				},
				TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,
				TEXT_ORIGIN_HORIZONTAL_CENTER,
				TEXT_ORIGIN_VERTICAL_CENTER,
				currentSize, { screenSize.x * -0.5f + titleWidth * 0.5f, screenSize.y * 0.5f - 100.0f }
			), LAYER_TITLE
		);
		labelSelect->color = { 0.0f, 0.0f, 0.0f, 0.0f };

		labelHints = AddGameObject(
			GameObjectText(
				L"↑↓選択 🅰決定 🅱戻る\nジャイロ：←オン→",
				FONT{
					70.0f,
					"HG創英角ﾎﾟｯﾌﾟ体",
					FONT_WEIGHT_BOLD
				},
				TEXT_RESIZE_KEEP_RATIO_BY_WIDTH,
				TEXT_ORIGIN_HORIZONTAL_CENTER,
				TEXT_ORIGIN_VERTICAL_BOTTOM,
				currentSize, { screenSize.x * -0.5f + titleWidth * 0.5f, screenSize.y * -0.5f + 20.0f }
			), LAYER_TITLE
		);
		labelHints->color = { 0.0f, 0.0f, 0.0f, 0.0f };


		// ステージリスト読み込む
		{
			D_TABLE table;
			D_KVTABLE keyValuePair;
			ReadCSVFromPath(STATE_LIST, table);
			TableToKeyValuePair("key", table, keyValuePair);

			stageData.reserve(table.size());
			textObjs.reserve(table.size());

			int line = 0;
			for (auto& row : keyValuePair) {

				stageData.push_back(
					{
						row.second["stage"],
						row.second["name"],
						row.second["background"],
						row.second["bgm"],
						row.second["rail"],
						row.second["collision"],
						row.second["surface"],
						row.second["items"],
						row.second["camera"],
						row.second["goal_gallery"],
						row.second["goal_gallery_animation"],
						std::stof(row.second["rank_s_time"]),
						std::stof(row.second["rank_a_time"]),
						std::stof(row.second["rank_b_time"]),
						row.second["prologue"],
						row.second["epilogue"]
					}
				);
				GameObjectText* textObj = AddGameObject(
					GameObjectText(
						StringToWString(row.second["name"]),
						FONT{
							70.0f,
							"HG創英角ﾎﾟｯﾌﾟ体",
							FONT_WEIGHT_BOLD
						},
						TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,
						TEXT_ORIGIN_HORIZONTAL_CENTER,
						TEXT_ORIGIN_VERTICAL_CENTER,
						currentSize
					), LAYER_TITLE
				);
				textObj->color = { 0.0f, 0.0f, 0.0f, 0.0f };
				textObjs.push_back(textObj);
				textObj->Update();
				if (textObj->size.x > currentSize.x) {
					textObj->resize = TEXT_RESIZE_USE_SCALE;
					textObj->size = currentSize;
				}
			}
		}

		txtLoading = AddGameObject(
			GameObjectText(
				L"NOW LOADING...",
				FONT{
					50.0f,
					"HG創英角ﾎﾟｯﾌﾟ体",
					FONT_WEIGHT_BOLD
				},
				TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,
				TEXT_ORIGIN_HORIZONTAL_RIGHT,
				TEXT_ORIGIN_VERTICAL_BOTTOM,
				{ 300.0f, 50.0f }, { screenSize.x * 0.5f - titlePadding, screenSize.y * -0.5f + titlePadding }
			), LAYER_LOADING
		);
		txtLoading->enable = false;
		
		bgmScene = (BGMScene::BGMScene*)LoadScene("bgm");
		bgmScene->SetCurrentScene(this);
		bgmScene->SetPlaylist(this, { ASSET.BGM_TITLE });
		bgmScene->Play(ASSET.BGM_TITLE);

		SECursor = AddGameObject(
			GameObjectAudio(LoadAudio(ASSET.SE_CURSOR_MOVE), false)
		);

		SEOK = AddGameObject(
			GameObjectAudio(LoadAudio(ASSET.SE_OK), false)
		);
		
		// currentIndex = 0;
		roll = 1.0f;
		flip = 0.0f;
		previewFade = 0.0f;
		if (IsContainBool("gyro")) {
			gyro = GetCommonBool("gyro");
		}
		else {
			gyro = true;
			SetCommonBool("gyro", gyro);
		}
		stageLoaded.store(false, std::memory_order_relaxed);;
		loadThread = nullptr;

		previewChanged = true;
		previewReady.store(false, std::memory_order_relaxed);

		previewScene = (PreviewScene::PreviewScene*)LoadScene("preview");
		previewRenderTarget = GetRenderer()->CreateRenderTarget(screenSize.x - titleWidth, screenSize.y);
		previewScene->SetRenderTarget(previewRenderTarget);

		preview = AddGameObject(
			GameObjectQuad(
				previewScene->GetRenderTarget()->texture, 
				//LoadTexture(TEXTURE_WHITE),
				{ screenSize.x - titleWidth, screenSize.y },
				{ titleWidth * 0.5f, 0.0f }
			), LAYER_PREVIEW
		);
		preview->color.w = 0.0f;
		StartScene("preview");
	}


	// =======================================================
	// 終了処理
	// =======================================================
	void StageSelectScene::Uninit()
	{
		if (previewRenderTarget) {
			GetRenderer()->ReleaseRenderTarget(previewRenderTarget);
			previewRenderTarget = nullptr;
		}
		EndScene("preview");
		previewScene = nullptr;

		Scene::Uninit();
		textObjs.clear();
		stageData.clear();
		if (loadThread) {
			if (loadThread->joinable()) {
				loadThread->join();
			}
			else {
				while (stageLoading && !stageLoaded.load(std::memory_order_relaxed)) {
					Sleep(50);
				}
			}
			delete loadThread;
		}
		loadThread = nullptr;
		if (previewThread) {
			if (previewThread->joinable()) {
				previewThread->join();
			}
			else {
				while (!previewReady.load(std::memory_order_relaxed)) {
					Sleep(50);
				}
			}
			delete previewThread;
		}
		previewThread = nullptr;
	}

	float easeOutElastic(float t) {
		float t2 = (t - 1) * (t - 1);
		return 1 - t2 * t2 * cos(t * PI * 4.5f);
	}

	// =======================================================
	// 更新
	// =======================================================
	void StageSelectScene::Update()
	{
		if (!InTransition() && stageLoaded.load(std::memory_order_relaxed)) {
			stageLoading = false;
			txtLoading->enable = false;
			SceneTransit("main_game", "star");
			return;
		}
		if (!stageLoading && !stageLoaded.load(std::memory_order_relaxed)) {
			if (roll == 1.0f) {
				if (IsInputDown(INPUT_DOWN) && currentIndex < stageData.size() - 1) {
					SECursor->Play();
					currentIndex += 1;
					roll = 0.0f;
					rollDirect = 1;
					previewChanged = true;
					previewReady.store(false, std::memory_order_relaxed);
				}
				else if (IsInputDown(INPUT_UP) && currentIndex > 0) {
					SECursor->Play();
					currentIndex -= 1;
					roll = 0.0f;
					rollDirect = -1;
					previewChanged = true;
					previewReady.store(false, std::memory_order_relaxed);
				}
			}
		}
		
		if (!stageLoading && !stageLoaded.load(std::memory_order_relaxed) && !InTransition() && (IsInputTrigger(INPUT_START) || IsInputTrigger(INPUT_OK))) {
			SEOK->Play();
			flip = 1.0f;
			roll = 1.0f;
			stageLoading = true;

			// 背景
			SetCommonString("stage_key", stageData[currentIndex].stage);

			// 背景
			SetCommonString("course_background", stageData[currentIndex].background);

			// BGM
			SetCommonString("course_bgm", stageData[currentIndex].bgm);

			// レール
			SetCommonString("course_rail", stageData[currentIndex].rail);

			// コース範囲判定
			SetCommonString("course_collision", stageData[currentIndex].collision);

			// 波
			SetCommonString("course_surface", stageData[currentIndex].surface);

			// オブジェクト配置
			SetCommonString("course_items", stageData[currentIndex].items);

			// ギャラリー
			SetCommonString("course_gallery", stageData[currentIndex].gallery);

			// ギャラリーアニメーション
			SetCommonString("course_gallery_animation", stageData[currentIndex].gallery_animation);

			// 各ランク最低時間
			SetCommonFloat("course_rank_s_time", stageData[currentIndex].rank_s_time);
			SetCommonFloat("course_rank_a_time", stageData[currentIndex].rank_a_time);
			SetCommonFloat("course_rank_b_time", stageData[currentIndex].rank_b_time);

			// ジャイロ設定
			SetCommonBool("gyro", gyro);

			// プロローグ
			SetCommonString("prologue", stageData[currentIndex].prologue);

			// エピローグ
			SetCommonString("epilogue", stageData[currentIndex].epilogue);

			Scene* scene = LoadScene("main_game");
			MainGameScene::MainGameScene* mainGameScene = (MainGameScene::MainGameScene*)scene;
			if (mainGameScene) {
				txtLoading->enable = true;
				loadThread = new std::thread([this, mainGameScene]() {
					mainGameScene->Preload();
					stageLoaded.store(true, std::memory_order_release);
				});
				loadThread->detach();
			}
		}
		else if (!stageLoading && !InTransition() && IsInputTrigger(INPUT_CANCEL)) {
			SceneTransit("title", "star");
		}
		
		F2 screenCenter = GetScreenCenter();
		F2 screenSize = GetScreenSize();

		labelSelect->color.w = flip;
		labelHints->color.w = flip;

		if (IsInputTrigger(INPUT_LEFT) || IsInputTrigger(INPUT_RIGHT)) {
			gyro = !gyro;
			SetCommonBool("gyro", gyro);
		}
		std::wstring label = GetInputLabel(L"↑") + GetInputLabel(L"↓") + L"選択 ";
		label += GetInputLabel(L"{OK}") + L"決定 ";
		label += GetInputLabel(L"{Cancel}") + L"戻る ";

		if (HasProcon() || HasDualSense()) {
			label += L"\nジャイロ：";
			if (gyro) {
				label += GetInputLabel(L"←") + L" On  " + GetInputLabel(L"→");
			}
			else {
				label += GetInputLabel(L"←") + L" Off  " + GetInputLabel(L"→");
			}
		}
		else if(gyro){
			gyro = false;
			SetCommonBool("gyro", gyro);
		}

		
		labelHints->SetValue(label);

		for (int i = 0; i < textObjs.size(); i++) {
			
			float sub = (float)currentIndex - (1.0f - roll) * rollDirect - (float)i;
			float absSub = abs(sub);
			textObjs[i]->position = currentPosition + F3{0.0f, 300.0f * 0.3f * sub};
			textObjs[i]->size = currentSize * (1.0f - 0.3f * absSub);
			//textObjs[i]->color.w = (1.0f - 0.3f * abs(sub)) * flip;
			textObjs[i]->color = { 0.3f - absSub, 0.6f - absSub, 1.0f - absSub, (1.0f - 0.3f * absSub) * flip };
		}
		
		static Progress bgT{ 15000.0f, true };
		background->uvOffset = background->uvRange * -bgT;

		Scene::Update();

		bgT.IncreaseValue(GetDeltaTime());
		

		if (!InTransition()) {
			dev->position.x = screenSize.x * -0.5f - dev->size.x * 0.5f + easeOutElastic(flip) * dev->size.x * 0.5f;
			flip.IncreaseValue(GetDeltaTime());
		}

		roll.IncreaseValue(GetDeltaTime());
		
		preview->color.w = previewFade;

		if (previewThread && previewReady.load(std::memory_order_relaxed)) {
			delete previewThread;
			previewThread = nullptr;
			if (previewIndex == currentIndex) {
				previewChanged = false;
			}
			else {
				previewReady.store(false, std::memory_order_release);
			}
			
		}
		if (flip == 1.0f && previewChanged && !previewReady && !previewThread) {
			previewIndex = currentIndex;
			previewThread = new std::thread([this](
				const std::string background, 
				const std::string rail, 
				const std::string collision,
				const std::string surface, 
				const std::string items,
				const std::string camera
			) {
				previewScene->Lock();
				while (previewScene->IsBusy()) {
					Sleep(10);
				}
				previewScene->Uninit();
				previewScene->LoadCourse(background, rail, collision, surface, items, camera);
				previewScene->Unlock();
				previewReady.store(true, std::memory_order_release);
			}, 
				stageData[currentIndex].background, 
				stageData[currentIndex].rail, 
				stageData[currentIndex].collision,
				stageData[currentIndex].surface, 
				stageData[currentIndex].items,
				stageData[currentIndex].camera
			);
			previewThread->detach();
		}

		if (previewReady.load(std::memory_order_relaxed)) {
			previewFade.IncreaseValue(GetDeltaTime());
		}
		else {
			previewFade.IncreaseValue(-GetDeltaTime());
		}
	}

	// =======================================================
	// 描画
	// =======================================================
	void StageSelectScene::Draw()
	{
		Scene::Draw();
	}

	// =======================================================
	// レイヤータイプの定義
	// =======================================================
	/*LAYER_TYPE StageSelectScene::GetLayerType(int layer)
	{
		return Scene::GetLayerType(layer);
	}*/
}
