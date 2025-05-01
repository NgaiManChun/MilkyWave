// =======================================================
// title.cpp
// 
// タイトルシーン
// 
// 作者：関口颯太（セキグチ　ソウタ）　2024/12/02
// =======================================================
#include "scene.h"
#include "keyboard.h"
#include "renderer.h"
#include "BGMScene.h"
#include "config.h"
#include "gameObjectAudio.h"
#include "input.h"
#include "progress.h"
#include "gameObjectModel.h"
#include "gameObjectText.h"
#include "gameObjectAudio.h"
#include <ctime> 
#include <functional>
using namespace MG;


namespace TitleScene {
	constexpr const char* TEXTURE_WHITE = "asset\\texture\\white.png";

	// =======================================================
	// クラス定義
	// =======================================================
	class TitleScene : public Scene {
	public:
		static struct _ASSET {
			std::string TEXTURE_LOGO;
			std::string BOAT_MODEL;
			std::string EYE_BLINK_ANIMATION;
			std::string TURBO_MODEL;
			std::string TURBO_ROTATE;
			std::string BACKGROUND_MODEL;
			std::string BGM_TITLE;
			std::string SE_STAR;
			std::string SE_BOAT;
			std::string SE_TITLE_PRESSED;
		};

	private:
		BGMScene::BGMScene* bgmScene;
		Model* background;
		Texture* textureWhite;
		GameObjectModel* boat;
		GameObjectModel* turbo;
		GameObjectQuad* title;
		GameObjectText* startLabel;
		GameObjectAudio* SEStar;
		GameObjectAudio* SEBoat;
		GameObjectAudio* SETitle;
		Progress startLabelProgress;
		Progress fadeProgress;
		Quaternion backgroundRotate;
		F3 range;
		F3 p0;
		F3 p1;
		F3 p2;
		F3 currentPosition;
		Quaternion q0;
		Quaternion q1;
		Progress posT;
		bool pressed = false;
		bool SEBoatPlayed = false;

		std::function<void()> updateFunc;

		void UpdateOpenning(Progress& t, Progress& s, Progress& r);
		void UpdateStandby();
	public:
		void Init() override;
		void Uninit() override;
		void Update() override;
		void Draw() override;
	};

	static const TitleScene::_ASSET ASSET = LoadConfig<TitleScene::_ASSET>("asset\\asset_list.csv", [](const D_KVTABLE& table) -> TitleScene::_ASSET {
		return {
			TABLE_STR_VALUE(table, "TEXTURE_LOGO", "asset\\texture\\logo.png"),
			TABLE_STR_VALUE(table, "BOAT_MODEL", "asset\texture\\model\\boat.mgm"),
			TABLE_STR_VALUE(table, "EYE_BLINK_ANIMATION", "asset\\model\\boat_eye_blink.mga"),
			TABLE_STR_VALUE(table, "TURBO_MODEL", "asset\\model\\boat_turbo.mgm"),
			TABLE_STR_VALUE(table, "TURBO_ROTATE", "asset\\model\\boat_turbo_rotate.mga"),
			TABLE_STR_VALUE(table, "BACKGROUND_MODEL", "asset\\model\\background1.mgm"),
			TABLE_STR_VALUE(table, "BGM_TITLE", "asset\\sound\\title.wav"),
			TABLE_STR_VALUE(table, "SE_STAR", "asset\\sound\\Opening_star.wav"),
			TABLE_STR_VALUE(table, "SE_BOAT", "asset\\sound\\Opening_boat_SE.wav"),
			TABLE_STR_VALUE(table, "SE_TITLE_PRESSED", "asset\\sound\\title_pressed.wav")
		};
	});

	// =======================================================
	// シーン登録
	// =======================================================
	static SceneName sceneName = REGISTER_SCENE("title", TitleScene);

	// =======================================================
	// 初期化
	// =======================================================
	void TitleScene::Init()
	{
		Scene::Init();

		srand(std::time(nullptr));

		// スクリーンの中心座標を取得
		F2 screenCenter = GetScreenCenter();

		// スクリーンのサイズを取得
		F2 screenSize = GetScreenSize();

		background = LoadModel(ASSET.BACKGROUND_MODEL);
		textureWhite = LoadTexture(TEXTURE_WHITE);

		boat = AddGameObject(
			GameObjectModel(LoadModel(ASSET.BOAT_MODEL), LoadAnimation(ASSET.EYE_BLINK_ANIMATION), { 700.0f, false })
		);

		turbo = AddGameObject(
			GameObjectModel(LoadModel(ASSET.TURBO_MODEL), LoadAnimation(ASSET.TURBO_ROTATE), { 1000.0f, true })
		);


		F3 titleSize = F3{ 522.0f, 289.0f } * 1.5f;
		title = AddGameObject(
			GameObjectQuad(
				LoadTexture(ASSET.TEXTURE_LOGO),
				titleSize,
				F3{ screenSize.x * -0.5f + titleSize.x * 0.5f + 40.0f, screenSize.y * 0.5f - titleSize.y * 0.5f - 40.0f }
			), LAYER_2D
		);

		startLabel = AddGameObject(
			GameObjectText(
				GetInputLabel(L"{Start}") + L"でスタート",
				FONT{
					100.0f,
					"HG創英角ﾎﾟｯﾌﾟ体",
					FONT_WEIGHT_BOLD
				},
				TEXT_RESIZE_USE_SCALE,
				TEXT_ORIGIN_HORIZONTAL_CENTER,
				TEXT_ORIGIN_VERTICAL_CENTER,
				{ titleSize.x * 0.7f, 60.0f }, { screenSize.x * -0.5f + titleSize.x * 0.5f + 40.0f, screenSize.y * 0.5f - titleSize.y - 40.0f - 200.0f }
			), LAYER_2D
		);

		bgmScene = (BGMScene::BGMScene*)LoadScene("bgm");
		bgmScene->SetCurrentScene(this);
		bgmScene->SetPlaylist(this, { ASSET.BGM_TITLE });
		bgmScene->Stop();

		SEStar = AddGameObject(
			GameObjectAudio(LoadAudio(ASSET.SE_STAR), false)
		);
		SEBoat = AddGameObject(
			GameObjectAudio(LoadAudio(ASSET.SE_BOAT), false)
		);
		SEBoatPlayed = false;
		SETitle = AddGameObject(
			GameObjectAudio(LoadAudio(ASSET.SE_TITLE_PRESSED), false)
		);


		currentCamera->position = {  };
		currentCamera->SetFront({ 1.0f ,0.0f, 0.0f });
		backgroundRotate = Quaternion::AxisYRadian(PI);
		range = { 1.0f, 0.2f, 1.0f };
		p0 = { -1.0f, 0.0f, 3.0f };
		p2 = F3{
			((float)rand() / RAND_MAX) * ((rand() % 2) ? 1.0f : -1.0f),
			((float)rand() / RAND_MAX) * ((rand() % 2) ? 1.0f : -1.0f),
			((float)rand() / RAND_MAX) * ((rand() % 2) ? 1.0f : -1.0f)
		} * range;
		p1 = (p0 + p2) * 0.5f;
		q0 = Quaternion::Identity();
		q1 = Quaternion::AxisZRadian((p0.x - p2.x) / (range.x * 2.0f) * 0.5f);
		posT = { 3000.0f, false };
		startLabelProgress = { 2000.0f, true };
		fadeProgress = { 2000.0f, false };
		pressed = false;

		updateFunc = [
			this, 
			t = Progress{ 600.0f, false }, 
			s = Progress{ 2000.0f, false },
			r = Progress{ 1500.0f, false }
		]() mutable { 
			UpdateOpenning(t, s, r); 
		};
	}


	// =======================================================
	// 終了処理
	// =======================================================
	void TitleScene::Uninit()
	{
		Scene::Uninit();
	}


	// =======================================================
	// 更新
	// =======================================================
	void TitleScene::Update()
	{
		if (updateFunc) {
			updateFunc();
		}
	}

	float easeInOutSine(float t) {
		return 0.5f * (1.0f + sinf(3.1415926f * (t - 0.5f)));
	}

	float easeOutExpo(float t) {
		return 1.0f - pow(2.0f, -8.0f * t);
	}

	void TitleScene::UpdateOpenning(Progress& t, Progress& s, Progress& r)
	{
		if (!pressed) {
			title->enable = false;
			startLabel->enable = false;
			boat->enable = false;
			turbo->enable = false;
			static F3 start;

			if (fadeProgress == 1.0f) {
				// 流れ星
				
				turbo->enable = true;
				turbo->rotate = Quaternion::AxisXRadian(0.2f);
				turbo->size = F3{ 1.0f, 3.0f * sinf(t * PI), 50.0f } *sinf(t * PI);
				turbo->position = currentCamera->position + (Rotate(Lerp(F3{ 0.0f, 0.0f, -0.5f }, F3{ 0.0f, 0.0f, 0.5f }, t), turbo->rotate) + currentCamera->GetFront()) * 100.0f;
				start = turbo->position;

				if (t == 1.0f) {
					// 流れ星終わった後いったん止まる

					if (s == 1.0f) {
						// ホールド終わった後向かって飛んでくる
						boat->enable = turbo->enable = true;
						boat->size = turbo->size = { 1.0f, 1.0f, 1.0f };
						boat->rotate = turbo->rotate = Quaternion(currentCamera->position - start, Rotate({ 0.0f , 1.0f, 0.0f }, Quaternion::AxisXDegree(90 * r)));
						boat->position = turbo->position = Lerp(start, currentCamera->position + F3{ -3.0f, -0.1f, -2.0f }, easeOutExpo(r));
						if (r == 1.0f) {
							pressed = true;
						}
						r.IncreaseValue(GetDeltaTime());
					}
					else if (s >= 0.9f && !SEBoatPlayed) {
						SEBoat->Play();
						SEBoatPlayed = true;
					}
					s.IncreaseValue(GetDeltaTime());
				}
				else if(t == 0.0f){
					SEStar->Play();
				}
				t.IncreaseValue(GetDeltaTime());
			}

			// フェードイン
			fadeProgress.IncreaseValue(GetDeltaTime());
		}
		else {
			// フェードアウト

			if (fadeProgress == 0.0f) {
				// タイトルに移行
				pressed = false;
				boat->enable = turbo->enable = true;
				boat->rotate = turbo->rotate = Quaternion::Identity();
				turbo->size = { 1.0f, 1.0f, 1.0f };
				title->enable = true;
				startLabel->enable = true;
				currentCamera->position = { -1.3f, 0.37f, 1.34f };
				currentCamera->SetFront(F3{ 0.0f, 0.4f, 0.3f } - currentCamera->position);
				bgmScene->Play(ASSET.BGM_TITLE);
				updateFunc = [this]() -> void { UpdateStandby(); };
			}
			fadeProgress.IncreaseValue(-GetDeltaTime());
		}
		Scene::Update();

		if (IsInputTrigger(INPUT_START) || IsInputTrigger(INPUT_OK)) {
			// スキップ
			pressed = true;
			fadeProgress = 0.0f;
		}
	}

	void TitleScene::UpdateStandby()
	{
		if (pressed) {
			title->color.w = max(1.0f - posT * 3.0f, 0.0f);
			F3 front = Lerp(F3{ 0.0f, 0.4f, 0.3f }, F3{ 0.0f, 0.0f, 20.0f }, posT) - currentCamera->position;
			currentCamera->SetFront(front);
			if (!InTransition() && posT > 0.5f) {
				SceneTransit("stage_select", "star");
			}
		}
		else {
			startLabel->color.w = cosf(startLabelProgress * PI * 2.0f);
			startLabel->SetValue(GetInputLabel(L"{Start}") + L"でスタート");
			backgroundRotate *= Quaternion::AxisYRadian(0.0002f);

			if (IsInputTrigger(INPUT_START) || IsInputTrigger(INPUT_OK)) {
				SETitle->Play();
				p0 = currentPosition;
				p1 = p0 + Normalize((p0 - p1)) * F3 { 1.0f, 0.2f, 1.0f };
				p2 = F3{ 0.0f, 0.0f, 500.0f };
				q0 = Lerp(q0, q1, posT);
				q1 = Quaternion::Identity();
				posT = { 3000.0f, false };
				fadeProgress = 1.0f;
				startLabel->color.w = 0.0f;
				pressed = true;
			}
			if (posT == 1.0f) {
				p0 = p2;
				p1 = p0 + Normalize((p0 - p1)) * F3 { 1.0f, 0.2f, 1.0f };
				p2 = F3{
					((float)rand() / RAND_MAX) * ((rand() % 2) ? 1.0f : -1.0f),
					((float)rand() / RAND_MAX) * ((rand() % 2) ? 1.0f : -1.0f),
					((float)rand() / RAND_MAX) * ((rand() % 2) ? 1.0f : -1.0f)
				} * range;

				q0 = q1;
				q1 = Quaternion::AxisZRadian((p0.x - p2.x) / (range.x * 2.0f) * 0.5f);
				posT = { ((float)rand() / RAND_MAX) * 2000.0f + 4000.0f, false };
			}
		}


		currentPosition = Bezier(p0, p1, p2, posT);
		boat->position = currentPosition;
		turbo->position = currentPosition;
		boat->rotate = turbo->rotate = Lerp(q0, q1, posT);

		Scene::Update();

		static Progress eyeS{ ((float)rand() / RAND_MAX) * 4500.0f, false };
		if (eyeS == 1.0f) {
			boat->t = 0.0f;
			eyeS = { ((float)rand() / RAND_MAX) * 4500.0f, false };
		}
		eyeS.IncreaseValue(GetDeltaTime());

		fadeProgress.IncreaseValue(GetDeltaTime());
		startLabelProgress.IncreaseValue(GetDeltaTime());
		posT.IncreaseValue(GetDeltaTime());
	}

	// =======================================================
	// 描画
	// =======================================================
	void TitleScene::Draw()
	{
		// 背景
		Renderer* renderer = GetRenderer();
		renderer->ApplyCamera(currentCamera);
		renderer->SetDepthState(DEPTH_STATE_NO_WRITE_DRAW_ALL);
		DrawModel(background, currentCamera->GetPosition(), { 1.0f, 1.0f, 1.0f }, backgroundRotate);
		renderer->SetDepthState(DEPTH_STATE_ENABLE);

		if (boat->enable) {
			boat->Draw();
		}
		
		if (turbo->enable) {
			renderer->SetDepthState(DEPTH_STATE_NO_WRITE);
			renderer->SetBlendState(BLEND_STATE_ADD);
			turbo->Draw();
			renderer->SetBlendState(BLEND_STATE_ALPHA);
			renderer->SetDepthState(DEPTH_STATE_ENABLE);
		}
		
		//Scene::Draw();
		renderer->ApplyCamera(currentCamera2D);
		if (title->enable) {
			title->Draw();
		}
		if (startLabel->enable) {
			startLabel->Draw();
		}
		
		
		F2 screenSize = GetScreenSize();
		
		DrawQuad(textureWhite, {}, { screenSize.x, screenSize.y }, Quaternion::Identity(), { 1.0f, 1.0f, 1.0f, (1.0f - fadeProgress) });
	}
}
