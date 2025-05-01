#include "scene.h"
#include "player.h"
#include "keyboard.h"
#include "renderer.h"
using namespace MG;

namespace NewScene {
	constexpr const char* MODEL_STAR = "asset\\model\\star.fbx.mgo";


	// =======================================================
	// クラス定義
	// =======================================================
	class NewScene : public Scene {
	private:
		Player* player;
	public:
		void Init() override;
		void Uninit() override;
		void Update() override;
		void Draw() override;
	};


	// =======================================================
	// シーン登録
	// =======================================================
	static SceneName sceneName = REGISTER_SCENE("newScene", NewScene);

	// =======================================================
	// 初期化
	// =======================================================
	void NewScene::Init()
	{
		Scene::Init();

		/*AddGameObject(
			Player(LoadModel(MODEL_STAR))
		);*/

		Player* temp = AddGameObject(
			Player(LoadModel(MODEL_STAR)), LAYER_2D
		);

		temp->size = { 300.0f, 300.0f, 1.0f };

		player = AddGameObject(
			Player(LoadModel(MODEL_STAR))
		);

		player->position.z = 2.0f;
		player->position.x = 3.0f;

		driectLight = { 0.0f, 0.0f, 1.0f, 0.3f };

	}


	// =======================================================
	// 終了処理
	// =======================================================
	void NewScene::Uninit()
	{
		Scene::Uninit();
	}


	// =======================================================
	// 更新
	// =======================================================
	void NewScene::Update()
	{
		Scene::Update();

		if (Keyboard_IsKeyDownTrigger(KK_SPACE)) {
			int score = 100;
			SetCommonInt("abc", score);
			SceneTransit("test", "sample_transition");
		}

		if (Keyboard_IsKeyDown(KK_W)) {
			currentCamera->Position().z += 0.1f;
		}
		if (Keyboard_IsKeyDown(KK_S)) {
			currentCamera->Position().z -= 0.1f;
		}
		if (Keyboard_IsKeyDown(KK_A)) {
			currentCamera->Position().x -= 20.0f * GetDeltaTime() / 1000.0f;
		}
		if (Keyboard_IsKeyDown(KK_D)) {
			currentCamera->Position().x += 20.0f * GetDeltaTime() / 1000.0f;
		}

		//player->position += F3{ 0.01f, 0.01f, 0.0f };

		player->rotate *= Quaternion::AxisDegree({ 0.0f, 0.0f, 1.0f }, 1.0f);
		player->rotate *= Quaternion::AxisDegree({ 1.0f, 0.0f, 0.0f }, 2.0f);
		//player->rotate *= Quaternion::AxisRadian({ 0.0f, 0.0f, 1.0f }, 0.01f);
	}


	// =======================================================
	// 描画
	// =======================================================
	void NewScene::Draw()
	{
		Scene::Draw();
	}
}
