#include "scene.h"
#include "progress.h"
#include "gameObjectText.h"
#include "gameObjectAudio.h"
#include "input.h"
#include "config.h"
#include "formatTime.h"
#include "BGMScene.h"
#include <functional>
#include <fstream>
#include <iostream>
using namespace MG;

namespace ResultScene {

	// =======================================================
	// リザルトシーン
	// ・クリアタイム表示
	// ・ベストタイム更新
	// ・ランク判定
	// ・ランク演出、キャラ演出
	// ・ステージ選択へ戻る
	// =======================================================

	static constexpr const char* WHITE_TEXTURE = "asset\\texture\\white.png";
	static constexpr const int LAYER_BACKGROUND = LAYER_2D + 1;
	static constexpr const int LAYER_ILLUSTRATION = LAYER_BACKGROUND + 1;
	static constexpr const int LAYER_LABEL = LAYER_ILLUSTRATION + 1;
	static constexpr const int LAYER_HINTS = LAYER_LABEL + 1;


	enum RANK {
		RANK_S = 0,
		RANK_A,
		RANK_B,
		RANK_C,
		RANK_MAX
	};

	class ResultScene : public Scene {
	public:
		struct _CONFIG {
			float LINE_HEIGHT = 100.0f;
			float LABEL_LEFT = -650.0f;
			float LABEL_TOP = 300.0f;
			float MARGIN_X = 250.0f;
			F4 RANK_S_COLOR;
			F4 RANK_A_COLOR;
			F4 RANK_B_COLOR;
			F4 RANK_C_COLOR;
		};

		struct _ASSET {
			std::string REFLECTION_TEXTURE;
			std::string RESULT_MII_S_TEXTURE;
			std::string RESULT_MII_A_TEXTURE;
			std::string RESULT_MII_B_TEXTURE;
			std::string RESULT_MII_C_TEXTURE;
			std::string RESULT_MII_SHADOW_TEXTURE;
			std::string BGM_RESULT;
			std::string SE_RANK_S;
			std::string SE_RANK_A;
			std::string SE_RANK_B;
			std::string SE_RANK_C;
		};
	private:
		RANK rank;
		F4 rankColor;
		std::vector<GameObjectText*> labelObjs;
		GameObjectText* timeObj;
		GameObjectText* bestTimeObj;
		GameObjectText* newBestObj;
		bool newBest = false;
		GameObjectQuad* rotateBackground;
		GameObjectQuad* mii;
		GameObjectQuad* miiShadow;
		GameObjectQuad* rankQuad;
		GameObjectQuad* rankQuadEffect;
		GameObjectText* nextLabel;
		GameObjectAudio* rankSE;
		RenderTarget* rankMaskRenderTarget;
		RenderTarget* rankRenderTarget;
		RenderTarget* shadowMaskRenderTarget;
		RenderTarget* shadowRenderTarget;
		Texture* whiteTex;
		Texture* reflectionTexture;
		Texture* miiTexture;
		Texture* miiShadowTexture;
		Texture* rankTexture;
		Progress shadowOffset{ 20000.0f, true };
		Progress reflectionOffset{ 6000.0f, true };

		BGMScene::BGMScene* bgmScene;

		std::function<void()> updateFunc;

		void UpdateBackground(Progress& t);
		void UpdateLabels(Progress& t);
		void UpdateTime(Progress& t);
		void UpdateRank(Progress& t, Progress& s);
		void UpdateMii(Progress& t, Progress& s);
		void UpdateStandby(Progress& t);
	public:
		void Preload();
		void Init() override;
		void Uninit() override;
		void Update() override;
		void Draw() override;
	};

	// =======================================================
	// シーン登録
	// =======================================================
	static SceneName sceneName = REGISTER_SCENE("result", ResultScene);


	static const ResultScene::_CONFIG CONFIG = LoadConfig<ResultScene::_CONFIG>("asset\\config.csv", [](const D_KVTABLE& table) -> ResultScene::_CONFIG {
		F4 RANK_S_COLOR = F4{ 0xB1, 0xA5, 0x6D, 0xFF } / 255.0f;
		F4 RANK_A_COLOR = F4{ 0xF0, 0x39, 0x0A, 0xFF } / 255.0f;
		F4 RANK_B_COLOR = F4{ 0x08, 0x07, 0xA1, 0xFF } / 255.0f;
		F4 RANK_C_COLOR = F4{ 0x08, 0x9E, 0x3A, 0xFF } / 255.0f;
		
		if (table.count("RANK_S_COLOR")) {
			unsigned long hex = std::stoul(table.at("RANK_S_COLOR").at("value"), nullptr, 16);
			RANK_S_COLOR = F4{ (float)(hex >> 24 & 0xFF), (float)(hex >> 16 & 0xFF), (float)(hex >> 8 & 0xFF), (float)(hex & 0xFF) } / 255.0f;
		}
		if (table.count("RANK_A_COLOR")) {
			unsigned long hex = std::stoul(table.at("RANK_A_COLOR").at("value"), nullptr, 16);
			RANK_A_COLOR = F4{ (float)(hex >> 24 & 0xFF), (float)(hex >> 16 & 0xFF), (float)(hex >> 8 & 0xFF), (float)(hex & 0xFF) } / 255.0f;
		}
		if (table.count("RANK_B_COLOR")) {
			unsigned long hex = std::stoul(table.at("RANK_B_COLOR").at("value"), nullptr, 16);
			RANK_B_COLOR = F4{ (float)(hex >> 24 & 0xFF), (float)(hex >> 16 & 0xFF), (float)(hex >> 8 & 0xFF), (float)(hex & 0xFF) } / 255.0f;
		}
		if (table.count("RANK_C_COLOR")) {
			unsigned long hex = std::stoul(table.at("RANK_C_COLOR").at("value"), nullptr, 16);
			RANK_C_COLOR = F4{ (float)(hex >> 24 & 0xFF), (float)(hex >> 16 & 0xFF), (float)(hex >> 8 & 0xFF), (float)(hex & 0xFF) } / 255.0f;
		}

		return {
			TABLE_FLOAT_VALUE(table, "RESULT_LINE_HEIGHT", 100.0f),
			TABLE_FLOAT_VALUE(table, "RESULT_LABEL_LEFT", -650.0f),
			TABLE_FLOAT_VALUE(table, "RESULT_LABEL_TOP", 300.0f),
			TABLE_FLOAT_VALUE(table, "MARGIN_X", 200.0f),
			RANK_S_COLOR,
			RANK_A_COLOR,
			RANK_B_COLOR,
			RANK_C_COLOR
		};
	});

	static const ResultScene::_ASSET ASSET = LoadConfig<ResultScene::_ASSET>("asset\\asset_list.csv", [](const D_KVTABLE& table) -> ResultScene::_ASSET {
		return {
			TABLE_STR_VALUE(table, "REFLECTION_TEXTURE", "asset\\texture\\reflection.png"),
			TABLE_STR_VALUE(table, "RESULT_MII_S_TEXTURE", "asset\\texture\\result_mii\\s.png"),
			TABLE_STR_VALUE(table, "RESULT_MII_A_TEXTURE", "asset\\texture\\result_mii\\a.png"),
			TABLE_STR_VALUE(table, "RESULT_MII_B_TEXTURE", "asset\\texture\\result_mii\\b.png"),
			TABLE_STR_VALUE(table, "RESULT_MII_C_TEXTURE", "asset\\texture\\result_mii\\c.png"),
			TABLE_STR_VALUE(table, "RESULT_MII_SHADOW_TEXTURE", "asset\\texture\\UI_background_starnight3.png"),
			TABLE_STR_VALUE(table, "BGM_RESULT", "asset\\sound\\Result_BGM.wav"),
			TABLE_STR_VALUE(table, "SE_RANK_S", "asset\\sound\\rank_s.wav"),
			TABLE_STR_VALUE(table, "SE_RANK_A", "asset\\sound\\rank_a.wav"),
			TABLE_STR_VALUE(table, "SE_RANK_B", "asset\\sound\\rank_b.wav"),
			TABLE_STR_VALUE(table, "SE_RANK_C", "asset\\sound\\rank_c.wav")
		};
	});

	// =======================================================
	// 初期化
	// =======================================================
	void ResultScene::Init()
	{
		Scene::Init();
		F2 screenSize = GetScreenSize();

		// =======================================================
		// 共通テクスチャ読み込み
		// =======================================================
		whiteTex = LoadTexture(WHITE_TEXTURE);
		reflectionTexture = LoadTexture(ASSET.REFLECTION_TEXTURE);
		miiShadowTexture = LoadTexture(ASSET.RESULT_MII_SHADOW_TEXTURE);


		// =======================================================
		// ベストタイム読み込み・更新判定
		// =======================================================

		// メインゲームから渡されたクリアタイムを取得
		float resultTime = GetCommonFloat("result_time");

		std::string stageKey = GetCommonString("stage_key");
		std::string filename = "best_" + stageKey + ".mgd";
		float best = 0.0f;

		// 保存済みベストタイムを読み込む
		MGObject mgo = LoadMGO(filename.c_str());
		if (mgo.size != 0) {
			memcpy(&best, mgo.data, sizeof(best));
		}

		// 初回、または今回のタイムがベストなら更新
		if (resultTime < best || best == 0.0f) {
			best = resultTime;
			newBest = true;
		}
		else {
			newBest = false;
		}
		

		// =======================================================
		// 背景
		// =======================================================
		{
			// ベース背景
			AddGameObject(
				GameObjectQuad(whiteTex, { screenSize.x, screenSize.y }),
				LAYER_BACKGROUND
			)->color = { 1.0f, 0.87f, 0.35f, 1.0f };

			// 斜めフリップ演出
			rotateBackground = AddGameObject(
				GameObjectQuad(whiteTex, { 0.0f, screenSize.y * 2.0f }, { screenSize.x * -0.5f }),
				LAYER_BACKGROUND
			);
			rotateBackground->color = { 1.0f, 0.74f, 0.35f, 1.0f };
		}

		// =======================================================
		// ラベル
		// =======================================================
		{
			std::vector<std::wstring> labels = {
				L"タイム", L"ランク"
			};

			for (int i = 0; i < labels.size(); i++) {
				labelObjs.push_back(AddGameObject(
					GameObjectText(
						labels[i],
						{
							CONFIG.LINE_HEIGHT,				// 文字サイズ
							"HG創英角ﾎﾟｯﾌﾟ体",				// フォント
							FONT_WEIGHT_NORMAL
						},

						// リサイズ設定
						TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,

						TEXT_ORIGIN_HORIZONTAL_CENTER,	// 横揃え設定
						TEXT_ORIGIN_VERTICAL_CENTER,	// 縦揃え設定
						{ 100.0f, CONFIG.LINE_HEIGHT, 1.0f },		// サイズ、上のリサイズ設定によって役割が違う
						{ CONFIG.LABEL_LEFT, CONFIG.LABEL_TOP - CONFIG.LINE_HEIGHT * i * 2.0f },			// ポジション
						Quaternion::Identity(),			// 回転
						{ 0.0f, 0.0f, 0.0f, 0.0f }		// 色
					),
					LAYER_LABEL // 2Dレイヤーに追加
				));
			}
		}

		nextLabel = AddGameObject(
			GameObjectText(
				GetInputLabel(L"{OK}") + L"でステージ選択に戻る",
				FONT{
					100.0f,
					"HG創英角ﾎﾟｯﾌﾟ体",
					FONT_WEIGHT_BOLD
				},
				TEXT_RESIZE_KEEP_RATIO_BY_WIDTH,
				TEXT_ORIGIN_HORIZONTAL_LEFT,
				TEXT_ORIGIN_VERTICAL_BOTTOM,
				{ screenSize.x * 0.5f, 100.0f }, { screenSize.x * -0.5f + 40.0f, screenSize.y * -0.5f + 40.0f }
			), LAYER_HINTS
		);
		nextLabel->color.w = 0.0f;


		// =======================================================
		// ランク判定
		// =======================================================

		float rankSTime = GetCommonFloat("course_rank_s_time");
		float rankATime = GetCommonFloat("course_rank_a_time");
		float rankBTime = GetCommonFloat("course_rank_b_time");

		std::wstring rankStr;
		if (resultTime <= rankSTime) {
			rank = RANK_S;
			rankStr = L"S";
			rankColor = CONFIG.RANK_S_COLOR;
			miiTexture = LoadTexture(ASSET.RESULT_MII_S_TEXTURE);
			rankSE = AddGameObject(
				GameObjectAudio(LoadAudio(ASSET.SE_RANK_S), false)
			);
		}
		else if (resultTime <= rankATime) {
			rank = RANK_A;
			rankStr = L"A";
			rankColor = CONFIG.RANK_A_COLOR;
			miiTexture = LoadTexture(ASSET.RESULT_MII_A_TEXTURE);
			rankSE = AddGameObject(
				GameObjectAudio(LoadAudio(ASSET.SE_RANK_A), false)
			);
		}
		else if (resultTime <= rankBTime) {
			rank = RANK_B;
			rankStr = L"B";
			rankColor = CONFIG.RANK_B_COLOR;
			miiTexture = LoadTexture(ASSET.RESULT_MII_B_TEXTURE);
			rankSE = AddGameObject(
				GameObjectAudio(LoadAudio(ASSET.SE_RANK_B), false)
			);
		}
		else {
			rank = RANK_C;
			rankStr = L"C";
			rankColor = CONFIG.RANK_C_COLOR;
			miiTexture = LoadTexture(ASSET.RESULT_MII_C_TEXTURE);
			rankSE = AddGameObject(
				GameObjectAudio(LoadAudio(ASSET.SE_RANK_C), false)
			);
		}

		// ランク文字をテクスチャ化する
		rankTexture = RenderText(rankStr, {
			600.0f,						// 文字サイズ
			FONT_MEIRYO,				// フォント
			FONT_WEIGHT_BOLD
			});

		// =======================================================
		// タイム表示
		// =======================================================

		timeObj = AddGameObject(
			GameObjectText(
				StringToWString(FormatTime(resultTime)),
				{
					CONFIG.LINE_HEIGHT,			// 文字サイズ
					"HG創英角ﾎﾟｯﾌﾟ体",	// フォント
					FONT_WEIGHT_NORMAL
				},

				// リサイズ設定
				TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,

				TEXT_ORIGIN_HORIZONTAL_LEFT,	// 横揃え設定
				TEXT_ORIGIN_VERTICAL_CENTER,	// 縦揃え設定
				{ 200.0f, CONFIG.LINE_HEIGHT },		// サイズ、上のリサイズ設定によって役割が違う
				{ CONFIG.LABEL_LEFT + CONFIG.MARGIN_X, CONFIG.LABEL_TOP },			// ポジション
				Quaternion::Identity(),			// 回転
				{ 0.0f, 0.0f, 0.0f, 0.0f }		// 色
			),
			LAYER_LABEL // 2Dレイヤーに追加
		);
		bestTimeObj = AddGameObject(
			GameObjectText(
				L"ベストタイム " + StringToWString(FormatTime(best)),
				{
					CONFIG.LINE_HEIGHT * 0.4f,			// 文字サイズ
					"HG創英角ﾎﾟｯﾌﾟ体",	// フォント
					FONT_WEIGHT_NORMAL
				},

				// リサイズ設定
				TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,

				TEXT_ORIGIN_HORIZONTAL_LEFT,	// 横揃え設定
				TEXT_ORIGIN_VERTICAL_CENTER,	// 縦揃え設定
				{ 200.0f, CONFIG.LINE_HEIGHT * 0.4f },		// サイズ、上のリサイズ設定によって役割が違う
				{ CONFIG.LABEL_LEFT + CONFIG.MARGIN_X, CONFIG.LABEL_TOP - CONFIG.LINE_HEIGHT * 0.8f },			// ポジション
				Quaternion::Identity(),			// 回転
				{ 0.0f, 0.0f, 0.0f, 0.0f }		// 色
			),
			LAYER_LABEL // 2Dレイヤーに追加
		);
		if (best == 0.0f) {
			bestTimeObj->SetValue(L"ベストタイム --:--.---");
		}
		newBestObj = AddGameObject(
			GameObjectText(
				L"ベスト更新！",
				{
					CONFIG.LINE_HEIGHT * 0.4f,			// 文字サイズ
					"HG創英角ﾎﾟｯﾌﾟ体",	// フォント
					FONT_WEIGHT_NORMAL
				},

				// リサイズ設定
				TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,

				TEXT_ORIGIN_HORIZONTAL_LEFT,	// 横揃え設定
				TEXT_ORIGIN_VERTICAL_BOTTOM,	// 縦揃え設定
				{ 200.0f, CONFIG.LINE_HEIGHT * 0.4f },		// サイズ、上のリサイズ設定によって役割が違う
				{ CONFIG.LABEL_LEFT + CONFIG.MARGIN_X, CONFIG.LABEL_TOP + CONFIG.LINE_HEIGHT * 0.5f },			// ポジション
				Quaternion::Identity(),			// 回転
				{ 1.0f, 0.0f, 0.0f, 0.0f }		// 色
			),
			LAYER_LABEL // 2Dレイヤーに追加
		);

		// =======================================================
		// みぃちゃんのイラスト表示
		// =======================================================
		{
			shadowMaskRenderTarget = GetRenderer()->CreateRenderTarget(miiTexture->GetWidth(), miiTexture->GetHeight());
			shadowRenderTarget = GetRenderer()->CreateRenderTarget(miiTexture->GetWidth(), miiTexture->GetHeight());

			F3 size = F3{ (float)miiTexture->GetWidth(), (float)miiTexture->GetHeight() };
			size = size * (screenSize.y / miiTexture->GetHeight());

			miiShadow = AddGameObject(
				GameObjectQuad(
					shadowRenderTarget->texture,
					size,
					F3{ (screenSize.x - size.x) * 0.5f, (-screenSize.y + size.y) * 0.5f },
					Quaternion::Identity(),
					F4{ 1.0f, 1.0f, 1.0f, 0.0f }
				), LAYER_ILLUSTRATION
			);

			mii = AddGameObject(
				GameObjectQuad(
					miiTexture,
					size,
					F3{ (screenSize.x - size.x) * 0.5f, (-screenSize.y + size.y) * 0.5f },
					Quaternion::Identity(),
					F4{ 1.0f, 1.0f, 1.0f, 0.0f }
				), LAYER_ILLUSTRATION
			);
		}

		// =======================================================
		// ランク表示
		// =======================================================
		{
			rankRenderTarget = GetRenderer()->CreateRenderTarget(rankTexture->GetWidth(), rankTexture->GetHeight());
			rankMaskRenderTarget = GetRenderer()->CreateRenderTarget(rankTexture->GetWidth(), rankTexture->GetHeight());

			F3 size = F3{ (float)rankTexture->GetWidth(), (float)rankTexture->GetHeight() };
			F3 position = { CONFIG.LABEL_LEFT + CONFIG.MARGIN_X + (float)rankTexture->GetWidth() * 0.5f, CONFIG.LABEL_TOP - CONFIG.LINE_HEIGHT - 300.0f };

			rankQuad = AddGameObject(
				GameObjectQuad(
					rankRenderTarget->texture,
					size,
					position
				), LAYER_LABEL
			);
			rankQuad->color.w = 0.0f;
			rankQuadEffect = AddGameObject(
				GameObjectQuad(
					rankRenderTarget->texture,
					size,
					position
				), LAYER_LABEL
			);
			rankQuadEffect->color.w = 0.0f;
		}

		// =======================================================
		// リザルトBGM再生
		// =======================================================

		bgmScene = (BGMScene::BGMScene*)LoadScene("bgm");
		bgmScene->SetCurrentScene(this);
		bgmScene->SetPlaylist(this, { ASSET.BGM_RESULT });
		bgmScene->Play(ASSET.BGM_RESULT);

		// =======================================================
		// ベスト更新時はファイルへ保存
		// =======================================================
		if (newBest) {
			size_t size;
			size = sizeof(best);
			char* data = new char[sizeof(best)];
			memcpy(data, &best, sizeof(best));

			MGObject mgo{
				MGOBJECT_TYPE_DATA,
				size,
				0
			};

			std::ofstream file("best_" + stageKey + ".mgd", std::ios::binary);

			file.write((const char*)&mgo, sizeof(MGObject));
			file.write((const char*)data, size);
			file.close();
		}


		// 最初は背景演出から開始
		updateFunc = [this, t = Progress{ 300.0f, false }]() mutable { UpdateBackground(t); };

	}


	// =======================================================
	// 終了処理
	// =======================================================
	void ResultScene::Uninit()
	{
		Scene::Uninit();
		if (rankRenderTarget) {
			GetRenderer()->ReleaseRenderTarget(rankRenderTarget);
			rankRenderTarget = nullptr;
		}
		if (rankMaskRenderTarget) {
			GetRenderer()->ReleaseRenderTarget(rankMaskRenderTarget);
			rankMaskRenderTarget = nullptr;
		}
		if (shadowRenderTarget) {
			GetRenderer()->ReleaseRenderTarget(shadowRenderTarget);
			shadowRenderTarget = nullptr;
		}
		if (shadowMaskRenderTarget) {
			GetRenderer()->ReleaseRenderTarget(shadowMaskRenderTarget);
			shadowMaskRenderTarget = nullptr;
		}
		labelObjs.clear();
		
	}


	// =======================================================
	// 更新
	// =======================================================
	void ResultScene::Update()
	{

		// 現在の演出ステップを更新
		if (updateFunc && !InTransition()) {
			updateFunc();
		}

		// ランク文字の反射演出用オフセット更新
		reflectionOffset.IncreaseValue(GetDeltaTime());

		// キャラ影の流れ演出用オフセット更新
		shadowOffset.IncreaseValue(GetDeltaTime());
	}

	// =======================================================
	// 背景ステート更新
	// =======================================================
	void ResultScene::UpdateBackground(Progress& t)
	{
		// 決定ボタンで演出をスキップ
		if (IsInputDown(INPUT_OK)) {
			t = 1.0f;
		}

		F2 screenSize = GetScreenSize();

		// 斜めフリップ
		rotateBackground->size.x = screenSize.x * 0.5f * t;
		rotateBackground->rotate = Quaternion::AxisZDegree(-16.0f * t);

		// 背景演出完了後、ラベル表示へ進む
		if (t == 1.0f) {
			updateFunc = [this, _t = Progress{ 500.0f, false }]() mutable { UpdateLabels(_t); };
		}

		Scene::Update();
		t.IncreaseValue(GetDeltaTime());
	}

	// =======================================================
	// ラベルステート更新
	// =======================================================
	void ResultScene::UpdateLabels(Progress& t)
	{
		// 決定ボタンで演出をスキップ
		if (IsInputDown(INPUT_OK)) {
			t = 1.0f;
		}

		// 「タイム」「ランク」などのラベルを少し下からフェードイン
		for (int i = 0; i < labelObjs.size(); i++) {
			float y = (CONFIG.LABEL_TOP - CONFIG.LINE_HEIGHT * i * 2.0f);
			labelObjs[i]->position.y = (y - 30.0f) * (1.0f - t) + y * t;
			labelObjs[i]->color.w = t;
		}

		// ラベル演出完了後、タイム表示へ進む
		if (t == 1.0f) {
			updateFunc = [this, _t = Progress{ 500.0f, false }]() mutable { UpdateTime(_t); };
		}

		Scene::Update();
		t.IncreaseValue(GetDeltaTime());
	}

	// =======================================================
	// タイムステート更新
	// =======================================================
	void ResultScene::UpdateTime(Progress& t)
	{
		// 決定ボタンで演出をスキップ
		if (IsInputDown(INPUT_OK)) {
			t = 1.0f;
		}

		F2 screenSize = GetScreenSize();

		// タイムとベストタイムをフェードイン
		timeObj->color.w = bestTimeObj->color.w = t;

		// ベスト更新時は「ベスト更新！」も表示し、今回タイムを赤くする
		if (newBest) {
			newBestObj->color.w = t;
			timeObj->color.x = 1.0f;
		}

		// 右側からスライドイン
		timeObj->position.x = bestTimeObj->position.x = 
			newBestObj->position.x = 
			CONFIG.LABEL_LEFT + CONFIG.MARGIN_X + screenSize.x * (1.0f - t);
		
		Scene::Update();

		// タイム表示完了後、ランク表示へ進む
		if (t == 1.0f) {
			updateFunc = [this, _t = Progress{ 300.0f, false }, s = Progress{ 300.0f, false }]() mutable { UpdateRank(_t, s); };
		}

		t.IncreaseValue(GetDeltaTime());
	}

	// =======================================================
	// ランクステート更新
	// =======================================================
	void ResultScene::UpdateRank(Progress& t, Progress& s)
	{
		// 決定ボタンで演出をスキップ
		if (IsInputDown(INPUT_OK)) {
			t = 1.0f;
			s = 1.0f;
		}

		// ランク文字を大きい状態から通常サイズへ縮小しながら表示
		F3 rankSize = F3{ (float)rankTexture->GetWidth(), (float)rankTexture->GetHeight() };
		rankQuad->size = Lerp(rankSize * 5.0f, rankSize, t);
		rankQuad->color.w = t;

		// ランク文字の一瞬広がるエフェクト
		if (s) {
			rankQuadEffect->size = Lerp(rankSize, rankSize * 2.0f, s);
			rankQuadEffect->color.w = 1.0f - s;
		}
		else {
			rankQuadEffect->color.w = 0.0f;
		}

		Scene::Update();

		// ランク演出完了後、SEを鳴らしてキャラ表示へ進む
		if (s == 1.0f) {
			rankSE->Play();
			updateFunc = [this, _t = Progress{ 340.0f, false }, _s = Progress{ 2000.0f, false }]() mutable { UpdateMii(_t, _s); };
		}

		// ランク本体が表示されてからエフェクトを進める
		if (t == 1.0f) {
			s.IncreaseValue(GetDeltaTime());
		}

		t.IncreaseValue(GetDeltaTime());
	}

	// =======================================================
	// キャライラストステート更新
	// =======================================================
	void ResultScene::UpdateMii(Progress& t, Progress& s)
	{
		// 決定ボタンで演出をスキップ
		if (IsInputDown(INPUT_OK)) {
			t = 1.0f;
			s = 1.0f;
		}

		F2 screenSize = GetScreenSize();

		// キャラ画像の基準位置
		F3 position = { (screenSize.x - mii->size.x) * 0.5f, (-screenSize.y + mii->size.y) * 0.5f };

		// キャライラストを右からフェードイン
		mii->position = position + F3{ 100.0f, 0.0f } * (1.0f - t);
		mii->color.w = t;

		// 影を少し遅れて表示
		miiShadow->position = position + F3{ 60.0f, -40.0f } * s;
		miiShadow->color.w = s;

		Scene::Update();

		// キャラ演出完了後、入力待ちへ移行
		if (s == 1.0f) {
			updateFunc = [this, _t = Progress{ 2000.0f, true }]() mutable { UpdateStandby(_t); };
		}
		if (t == 1.0f) {
			s.IncreaseValue(GetDeltaTime());
		}
		t.IncreaseValue(GetDeltaTime());
	}

	// =======================================================
	// 入力待ちステート更新
	// =======================================================
	void ResultScene::UpdateStandby(Progress& t)
	{
		// 戻る案内を点滅させる
		nextLabel->color.w = sinf(t * PI);

		// 決定ボタンでステージ選択へ戻る
		if (IsInputTrigger(INPUT_OK)) {
			SceneTransit("stage_select", "star");
			t = { 1.0, false };
			t = 1.0f;
		}

		Scene::Update();
		t.IncreaseValue(GetDeltaTime());
	}

	// =======================================================
	// 描画
	// =======================================================
	void ResultScene::Draw()
	{
		Renderer* renderer = GetRenderer();

		// =======================================================
		// みぃちゃんの影を作る
		// =======================================================
		if(mii && mii->enable && mii->color.w){
			F3 size = { miiTexture->GetWidth(), miiTexture->GetHeight() };

			// キャラ形状のマスクを作る
			renderer->SetRenderTarget(shadowMaskRenderTarget);
			renderer->SetViewport(miiTexture->GetWidth() * 0.5f, miiTexture->GetHeight() * 0.5f, miiTexture->GetWidth(), miiTexture->GetHeight());
			renderer->ApplyCamera(currentCamera2D);
			renderer->ClearRenderTargetView(shadowMaskRenderTarget);

			// 白背景からキャラ部分を抜く
			DrawQuad(whiteTex, {}, size);
			renderer->SetBlendState(BLEND_STATE_DEST_OUT);
			DrawQuad(miiTexture, { }, size);
			renderer->SetBlendState(BLEND_STATE_ALPHA);

			// 影用RenderTargetに背景模様を描く
			renderer->SetRenderTarget(shadowRenderTarget);
			renderer->SetViewport(miiTexture->GetWidth() * 0.5f, miiTexture->GetHeight() * 0.5f, miiTexture->GetWidth(), miiTexture->GetHeight());
			renderer->ApplyCamera(currentCamera2D);
			renderer->ClearRenderTargetView(shadowRenderTarget);

			float scale = max((float)miiTexture->GetWidth() / miiShadowTexture->GetWidth(), (float)miiTexture->GetHeight() / miiShadowTexture->GetHeight());
			
			// 流れる影模様
			DrawQuad(miiShadowTexture, {},
				F3{ (float)miiShadowTexture->GetWidth(), (float)miiShadowTexture->GetHeight() } * scale,
				Quaternion::Identity(),
				{ 1.0f, 1.0f, 1.0f, 1.0f }, 
				{ shadowOffset, 0.0f }
			);

			renderer->SetBlendState(BLEND_STATE_ADD);

			// 影の明るさを追加
			DrawQuad(whiteTex, {}, size, Quaternion::Identity(), miiShadow->color * 0.5f);

			// マスクでキャラ形状だけを残す
			renderer->SetBlendState(BLEND_STATE_DEST_OUT);
			DrawQuad(shadowMaskRenderTarget->texture, { }, size);

			renderer->SetBlendState(BLEND_STATE_ALPHA);
		}

		// =======================================================
		// ランク文字の生成
		// =======================================================
		if(rankQuad && rankQuad->enable && rankQuad->color.w){
			F3 size = { rankTexture->GetWidth(), rankTexture->GetHeight() };

			// ランク文字のマスクを作る
			renderer->SetRenderTarget(rankMaskRenderTarget);
			renderer->SetViewport(rankTexture->GetWidth() * 0.5f, rankTexture->GetHeight() * 0.5f, rankTexture->GetWidth(), rankTexture->GetHeight());
			renderer->ApplyCamera(currentCamera2D);
			renderer->ClearRenderTargetView(rankMaskRenderTarget);
			DrawQuad(whiteTex, {}, size);
			renderer->SetBlendState(BLEND_STATE_DEST_OUT);
			DrawQuad(rankTexture, { }, size);
			renderer->SetBlendState(BLEND_STATE_ALPHA);

			// ランク文字内部の色・反射演出を描画
			renderer->SetRenderTarget(rankRenderTarget);
			renderer->SetViewport(rankTexture->GetWidth() * 0.5f, rankTexture->GetHeight() * 0.5f, rankTexture->GetWidth(), rankTexture->GetHeight());
			renderer->ClearRenderTargetView(rankRenderTarget);

			// ランク色で塗る
			DrawQuad(whiteTex, {}, size, Quaternion::Identity(), rankColor);

			// 反射テクスチャを加算合成
			renderer->SetBlendState(BLEND_STATE_ADD);
			DrawQuad(reflectionTexture, {}, size, Quaternion::Identity(),
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				{ -reflectionOffset, 0.0f });

			// 文字形状以外を抜く
			renderer->SetBlendState(BLEND_STATE_DEST_OUT);
			DrawQuad(rankMaskRenderTarget->texture, { }, size);

			// 設定戻す
			renderer->SetBlendState(BLEND_STATE_ALPHA);
			
		}

		// =======================================================
		// 通常描画へ戻す
		// =======================================================

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
		
		Scene::Draw();
	}
}
