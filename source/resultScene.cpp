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
	// �V�[���o�^
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
	// ������
	// =======================================================
	void ResultScene::Init()
	{
		Scene::Init();

		whiteTex = LoadTexture(WHITE_TEXTURE);
		reflectionTexture = LoadTexture(ASSET.REFLECTION_TEXTURE);
		miiShadowTexture = LoadTexture(ASSET.RESULT_MII_SHADOW_TEXTURE);

		F2 screenSize = GetScreenSize();

		float resultTime = GetCommonFloat("result_time");
		
		std::string stageKey = GetCommonString("stage_key");
		std::string filename = "best_" + stageKey + ".mgd";
		float best = 0.0f;
		MGObject mgo = LoadMGO(filename.c_str());
		if (mgo.size != 0) {
			memcpy(&best, mgo.data, sizeof(best));
		}
		if (resultTime < best || best == 0.0f) {
			best = resultTime;
			newBest = true;
		}
		else {
			newBest = false;
		}
		

		// �w�i
		{
			AddGameObject(
				GameObjectQuad(whiteTex, { screenSize.x, screenSize.y }),
				LAYER_BACKGROUND
			)->color = { 1.0f, 0.87f, 0.35f, 1.0f };

			rotateBackground = AddGameObject(
				GameObjectQuad(whiteTex, { 0.0f, screenSize.y * 2.0f }, { screenSize.x * -0.5f }),
				LAYER_BACKGROUND
			);
			rotateBackground->color = { 1.0f, 0.74f, 0.35f, 1.0f };
		}

		// ���x��
		{
			std::vector<std::wstring> labels = {
				L"�^�C��", L"�����N"
			};

			for (int i = 0; i < labels.size(); i++) {
				labelObjs.push_back(AddGameObject(
					GameObjectText(
						labels[i],
						{
							CONFIG.LINE_HEIGHT,				// �����T�C�Y
							"HG�n�p�p�߯�ߑ�",				// �t�H���g
							FONT_WEIGHT_NORMAL
						},

						// ���T�C�Y�ݒ�
						TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,

						TEXT_ORIGIN_HORIZONTAL_CENTER,	// �������ݒ�
						TEXT_ORIGIN_VERTICAL_CENTER,	// �c�����ݒ�
						{ 100.0f, CONFIG.LINE_HEIGHT, 1.0f },		// �T�C�Y�A��̃��T�C�Y�ݒ�ɂ���Ė������Ⴄ
						{ CONFIG.LABEL_LEFT, CONFIG.LABEL_TOP - CONFIG.LINE_HEIGHT * i * 2.0f },			// �|�W�V����
						Quaternion::Identity(),			// ��]
						{ 0.0f, 0.0f, 0.0f, 0.0f }		// �F
					),
					LAYER_LABEL // 2D���C���[�ɒǉ�
				));
			}
		}

		nextLabel = AddGameObject(
			GameObjectText(
				GetInputLabel(L"{OK}") + L"�ŃX�e�[�W�I���ɖ߂�",
				FONT{
					100.0f,
					"HG�n�p�p�߯�ߑ�",
					FONT_WEIGHT_BOLD
				},
				TEXT_RESIZE_KEEP_RATIO_BY_WIDTH,
				TEXT_ORIGIN_HORIZONTAL_LEFT,
				TEXT_ORIGIN_VERTICAL_BOTTOM,
				{ screenSize.x * 0.5f, 100.0f }, { screenSize.x * -0.5f + 40.0f, screenSize.y * -0.5f + 40.0f }
			), LAYER_HINTS
		);
		nextLabel->color.w = 0.0f;

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

		rankTexture = RenderText(rankStr, {
			600.0f,						// �����T�C�Y
			FONT_MEIRYO,				// �t�H���g
			FONT_WEIGHT_BOLD
			});

		// �^�C��
		timeObj = AddGameObject(
			GameObjectText(
				StringToWString(FormatTime(resultTime)),
				{
					CONFIG.LINE_HEIGHT,			// �����T�C�Y
					"HG�n�p�p�߯�ߑ�",	// �t�H���g
					FONT_WEIGHT_NORMAL
				},

				// ���T�C�Y�ݒ�
				TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,

				TEXT_ORIGIN_HORIZONTAL_LEFT,	// �������ݒ�
				TEXT_ORIGIN_VERTICAL_CENTER,	// �c�����ݒ�
				{ 200.0f, CONFIG.LINE_HEIGHT },		// �T�C�Y�A��̃��T�C�Y�ݒ�ɂ���Ė������Ⴄ
				{ CONFIG.LABEL_LEFT + CONFIG.MARGIN_X, CONFIG.LABEL_TOP },			// �|�W�V����
				Quaternion::Identity(),			// ��]
				{ 0.0f, 0.0f, 0.0f, 0.0f }		// �F
			),
			LAYER_LABEL // 2D���C���[�ɒǉ�
		);
		bestTimeObj = AddGameObject(
			GameObjectText(
				L"�x�X�g�^�C�� " + StringToWString(FormatTime(best)),
				{
					CONFIG.LINE_HEIGHT * 0.4f,			// �����T�C�Y
					"HG�n�p�p�߯�ߑ�",	// �t�H���g
					FONT_WEIGHT_NORMAL
				},

				// ���T�C�Y�ݒ�
				TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,

				TEXT_ORIGIN_HORIZONTAL_LEFT,	// �������ݒ�
				TEXT_ORIGIN_VERTICAL_CENTER,	// �c�����ݒ�
				{ 200.0f, CONFIG.LINE_HEIGHT * 0.4f },		// �T�C�Y�A��̃��T�C�Y�ݒ�ɂ���Ė������Ⴄ
				{ CONFIG.LABEL_LEFT + CONFIG.MARGIN_X, CONFIG.LABEL_TOP - CONFIG.LINE_HEIGHT * 0.8f },			// �|�W�V����
				Quaternion::Identity(),			// ��]
				{ 0.0f, 0.0f, 0.0f, 0.0f }		// �F
			),
			LAYER_LABEL // 2D���C���[�ɒǉ�
		);
		if (best == 0.0f) {
			bestTimeObj->SetValue(L"�x�X�g�^�C�� --:--.---");
		}
		newBestObj = AddGameObject(
			GameObjectText(
				L"�x�X�g�X�V�I",
				{
					CONFIG.LINE_HEIGHT * 0.4f,			// �����T�C�Y
					"HG�n�p�p�߯�ߑ�",	// �t�H���g
					FONT_WEIGHT_NORMAL
				},

				// ���T�C�Y�ݒ�
				TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,

				TEXT_ORIGIN_HORIZONTAL_LEFT,	// �������ݒ�
				TEXT_ORIGIN_VERTICAL_BOTTOM,	// �c�����ݒ�
				{ 200.0f, CONFIG.LINE_HEIGHT * 0.4f },		// �T�C�Y�A��̃��T�C�Y�ݒ�ɂ���Ė������Ⴄ
				{ CONFIG.LABEL_LEFT + CONFIG.MARGIN_X, CONFIG.LABEL_TOP + CONFIG.LINE_HEIGHT * 0.5f },			// �|�W�V����
				Quaternion::Identity(),			// ��]
				{ 1.0f, 0.0f, 0.0f, 0.0f }		// �F
			),
			LAYER_LABEL // 2D���C���[�ɒǉ�
		);

		// �݂������̃C���X�g
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


		// �����N
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

		bgmScene = (BGMScene::BGMScene*)LoadScene("bgm");
		bgmScene->SetCurrentScene(this);
		bgmScene->SetPlaylist(this, { ASSET.BGM_RESULT });
		bgmScene->Play(ASSET.BGM_RESULT);

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

		updateFunc = [this, t = Progress{ 300.0f, false }]() mutable { UpdateBackground(t); };

	}


	// =======================================================
	// �I������
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
	// �X�V
	// =======================================================
	void ResultScene::Update()
	{

		if (updateFunc && !InTransition()) {
			updateFunc();
		}
		reflectionOffset.IncreaseValue(GetDeltaTime());
		shadowOffset.IncreaseValue(GetDeltaTime());
	}


	void ResultScene::UpdateBackground(Progress& t)
	{
		if (IsInputDown(INPUT_OK)) {
			t = 1.0f;
		}
		F2 screenSize = GetScreenSize();
		rotateBackground->size.x = screenSize.x * 0.5f * t;
		rotateBackground->rotate = Quaternion::AxisZDegree(-16.0f * t);
		if (t == 1.0f) {
			updateFunc = [this, _t = Progress{ 500.0f, false }]() mutable { UpdateLabels(_t); };
		}
		Scene::Update();
		t.IncreaseValue(GetDeltaTime());
	}

	void ResultScene::UpdateLabels(Progress& t)
	{
		if (IsInputDown(INPUT_OK)) {
			t = 1.0f;
		}
		for (int i = 0; i < labelObjs.size(); i++) {
			float y = (CONFIG.LABEL_TOP - CONFIG.LINE_HEIGHT * i * 2.0f);
			labelObjs[i]->position.y = (y - 30.0f) * (1.0f - t) + y * t;
			labelObjs[i]->color.w = t;
		}
		if (t == 1.0f) {
			updateFunc = [this, _t = Progress{ 500.0f, false }]() mutable { UpdateTime(_t); };
		}
		Scene::Update();
		t.IncreaseValue(GetDeltaTime());
	}

	void ResultScene::UpdateTime(Progress& t)
	{
		if (IsInputDown(INPUT_OK)) {
			t = 1.0f;
		}
		F2 screenSize = GetScreenSize();
		timeObj->color.w = bestTimeObj->color.w = t;
		if (newBest) {
			newBestObj->color.w = t;
			timeObj->color.x = 1.0f;
		}
		timeObj->position.x = bestTimeObj->position.x = 
			newBestObj->position.x = 
			CONFIG.LABEL_LEFT + CONFIG.MARGIN_X + screenSize.x * (1.0f - t);
		
		Scene::Update();
		if (t == 1.0f) {
			updateFunc = [this, _t = Progress{ 300.0f, false }, s = Progress{ 300.0f, false }]() mutable { UpdateRank(_t, s); };
		}
		t.IncreaseValue(GetDeltaTime());
	}

	void ResultScene::UpdateRank(Progress& t, Progress& s)
	{
		if (IsInputDown(INPUT_OK)) {
			t = 1.0f;
			s = 1.0f;
		}
		F3 rankSize = F3{ (float)rankTexture->GetWidth(), (float)rankTexture->GetHeight() };
		rankQuad->size = Lerp(rankSize * 5.0f, rankSize, t);
		rankQuad->color.w = t;

		if (s) {
			rankQuadEffect->size = Lerp(rankSize, rankSize * 2.0f, s);
			rankQuadEffect->color.w = 1.0f - s;
		}
		else {
			rankQuadEffect->color.w = 0.0f;
		}
		Scene::Update();
		if (s == 1.0f) {
			rankSE->Play();
			updateFunc = [this, _t = Progress{ 340.0f, false }, _s = Progress{ 2000.0f, false }]() mutable { UpdateMii(_t, _s); };
		}
		if (t == 1.0f) {
			s.IncreaseValue(GetDeltaTime());
		}
		t.IncreaseValue(GetDeltaTime());
	}

	void ResultScene::UpdateMii(Progress& t, Progress& s)
	{
		if (IsInputDown(INPUT_OK)) {
			t = 1.0f;
			s = 1.0f;
		}
		F2 screenSize = GetScreenSize();
		F3 position = { (screenSize.x - mii->size.x) * 0.5f, (-screenSize.y + mii->size.y) * 0.5f };

		mii->position = position + F3{ 100.0f, 0.0f } * (1.0f - t);
		mii->color.w = t;
		miiShadow->position = position + F3{ 60.0f, -40.0f } * s;
		miiShadow->color.w = s;
		Scene::Update();
		if (s == 1.0f) {
			updateFunc = [this, _t = Progress{ 2000.0f, true }]() mutable { UpdateStandby(_t); };
		}
		if (t == 1.0f) {
			s.IncreaseValue(GetDeltaTime());
		}
		t.IncreaseValue(GetDeltaTime());
	}

	void ResultScene::UpdateStandby(Progress& t)
	{
		nextLabel->color.w = sinf(t * PI);

		if (IsInputTrigger(INPUT_OK)) {
			SceneTransit("stage_select", "star");
			t = { 1.0, false };
			t = 1.0f;
		}

		Scene::Update();
		t.IncreaseValue(GetDeltaTime());
	}

	// =======================================================
	// �`��
	// =======================================================
	void ResultScene::Draw()
	{
		Renderer* renderer = GetRenderer();

		// �݂������̉e�����
		if(mii && mii->enable && mii->color.w){
			F3 size = { miiTexture->GetWidth(), miiTexture->GetHeight() };

			// �}�X�N�`��
			renderer->SetRenderTarget(shadowMaskRenderTarget);
			renderer->SetViewport(miiTexture->GetWidth() * 0.5f, miiTexture->GetHeight() * 0.5f, miiTexture->GetWidth(), miiTexture->GetHeight());
			renderer->ApplyCamera(currentCamera2D);
			renderer->ClearRenderTargetView(shadowMaskRenderTarget);
			DrawQuad(whiteTex, {}, size);
			renderer->SetBlendState(BLEND_STATE_DEST_OUT);
			DrawQuad(miiTexture, { }, size);
			renderer->SetBlendState(BLEND_STATE_ALPHA);

			// �e�`��
			renderer->SetRenderTarget(shadowRenderTarget);
			renderer->SetViewport(miiTexture->GetWidth() * 0.5f, miiTexture->GetHeight() * 0.5f, miiTexture->GetWidth(), miiTexture->GetHeight());
			renderer->ApplyCamera(currentCamera2D);
			renderer->ClearRenderTargetView(shadowRenderTarget);

			float scale = max((float)miiTexture->GetWidth() / miiShadowTexture->GetWidth(), (float)miiTexture->GetHeight() / miiShadowTexture->GetHeight());
			DrawQuad(miiShadowTexture, {},
				F3{ (float)miiShadowTexture->GetWidth(), (float)miiShadowTexture->GetHeight() } * scale,
				Quaternion::Identity(),
				{ 1.0f, 1.0f, 1.0f, 1.0f }, 
				{ shadowOffset, 0.0f }
			);
			renderer->SetBlendState(BLEND_STATE_ADD);
			DrawQuad(whiteTex, {}, size, Quaternion::Identity(), miiShadow->color * 0.5f);
			renderer->SetBlendState(BLEND_STATE_DEST_OUT);
			DrawQuad(shadowMaskRenderTarget->texture, { }, size);
			renderer->SetBlendState(BLEND_STATE_ALPHA);
		}

		// �����N
		if(rankQuad && rankQuad->enable && rankQuad->color.w){
			F3 size = { rankTexture->GetWidth(), rankTexture->GetHeight() };

			// �}�X�N�`��
			renderer->SetRenderTarget(rankMaskRenderTarget);
			renderer->SetViewport(rankTexture->GetWidth() * 0.5f, rankTexture->GetHeight() * 0.5f, rankTexture->GetWidth(), rankTexture->GetHeight());
			renderer->ApplyCamera(currentCamera2D);
			renderer->ClearRenderTargetView(rankMaskRenderTarget);
			DrawQuad(whiteTex, {}, size);
			renderer->SetBlendState(BLEND_STATE_DEST_OUT);
			DrawQuad(rankTexture, { }, size);
			renderer->SetBlendState(BLEND_STATE_ALPHA);

			// �w�i�`��
			renderer->SetRenderTarget(rankRenderTarget);
			renderer->SetViewport(rankTexture->GetWidth() * 0.5f, rankTexture->GetHeight() * 0.5f, rankTexture->GetWidth(), rankTexture->GetHeight());
			renderer->ClearRenderTargetView(rankRenderTarget);
			DrawQuad(whiteTex, {}, size, Quaternion::Identity(), rankColor);
			renderer->SetBlendState(BLEND_STATE_ADD);
			DrawQuad(reflectionTexture, {}, size, Quaternion::Identity(),
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				{ -reflectionOffset, 0.0f });
			renderer->SetBlendState(BLEND_STATE_DEST_OUT);
			DrawQuad(rankMaskRenderTarget->texture, { }, size);
			renderer->SetBlendState(BLEND_STATE_ALPHA);
			
		}

		// �ʏ탌���_�[�^�[�Q�b�g�ɖ߂�
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
