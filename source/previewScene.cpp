#include "previewScene.h"
#include "stone.h"
#include "fever_piece.h"
#include "goal.h"
#include "gameObjectModel.h"
#include "CSVResource.h"

namespace PreviewScene {
	static constexpr const char* WHITE_TEXTURE = "asset\\texture\\white.png";
	static constexpr const int LAYER_PLAYER = 3;
	static constexpr const int LAYER_GALLERY = 4;
	static constexpr const int LAYER_ITEMS = 5;
	static constexpr const int LAYER_SURFACE = 6;
	static constexpr const char* INSTANCE_LIST = "asset\\instance.csv";

	// =======================================================
	// シーン登録
	// =======================================================
	static SceneName sceneName = REGISTER_SCENE("preview", PreviewScene);

	void PreviewScene::LoadCourse(const std::string& background, const std::string& rail, 
		const std::string& collision,
		const std::string& surface, const std::string& items, const std::string& camera)
	{
		// ステージ設定
		{
			this->background = LoadModel(background);

			course = AddGameObject(
				Course(
					LoadModel(surface),
					LoadAnimation(rail),
					nullptr
				), LAYER_SURFACE
			);

			cameraRoll = LoadAnimation(camera);

			// オブジェクト配置
			{
				// CSVファイルから設定を読み込む
				D_TABLE table;
				D_KVTABLE keyValuePair;
				ReadCSVFromPath(INSTANCE_LIST, table);

				// TableデータからKey-Value-Pairへ変換
				TableToKeyValuePair("instance_id", table, keyValuePair);

				MGObject mgo = LoadMGO(items.c_str());
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
									nullptr,
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
									nullptr,
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
									nullptr,
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
		t = 0.0f;
	}

	void PreviewScene::Lock()
	{
		locked.store(true, std::memory_order_release);
	}

	void PreviewScene::Unlock()
	{
		locked.store(false, std::memory_order_release);
	}

	bool PreviewScene::IsLocked() const
	{
		return locked.load(std::memory_order_relaxed);
	}

	bool PreviewScene::IsBusy() const
	{
		return busy.load(std::memory_order_relaxed);
	}

	// =======================================================
	// 初期化
	// =======================================================
	void PreviewScene::Init()
	{
		Scene::Init();
	}


	// =======================================================
	// 終了処理
	// =======================================================
	void PreviewScene::Uninit()
	{
		Scene::Uninit();
		course = nullptr;
		background = nullptr;
	}


	// =======================================================
	// 更新
	// =======================================================
	void PreviewScene::Update()
	{
		if (locked.load(std::memory_order_relaxed)) return;

		busy.store(true, std::memory_order_release);

		F3 size;
		F3 position;
		Quaternion rotate;

		if (cameraRoll && cameraRoll->modelNodeChannels.count("camera1")) {
			cameraRoll->Apply("camera1", cameraRoll->rawAnimation->frames * t, size, position, rotate);
			currentCamera->SetPosition(position);
			currentCamera->SetFront(Rotate({ 0.0f, 0.0f, 1.0f }, rotate));
		}

		Scene::Update();
		t.IncreaseValue(GetDeltaTime());
		busy.store(false, std::memory_order_release);
	}


	// =======================================================
	// 描画
	// =======================================================
	void PreviewScene::Draw()
	{
		if (locked.load(std::memory_order_relaxed)) return;

		busy.store(true, std::memory_order_release);

		Renderer* renderer = GetRenderer();

		RenderTarget* previewRenderTarget = GetRenderTarget();
		
		if (previewRenderTarget) {
			GetRenderer()->SetRenderTarget(previewRenderTarget);
			GetRenderer()->ClearRenderTargetView(previewRenderTarget);
			GetRenderer()->SetViewport(
				previewRenderTarget->texture->GetWidth() * 0.5f, previewRenderTarget->texture->GetHeight() * 0.5f,
				previewRenderTarget->texture->GetWidth(), previewRenderTarget->texture->GetHeight()
			);
		}

		// スクリーンの中心座標を取得
		F2 screenCenter = GetScreenCenter();

		// スクリーンのサイズを取得
		F2 screenSize = GetScreenSize();

		if (background) {
			// 背景
			renderer->ApplyCamera(currentCamera);
			renderer->SetDepthState(DEPTH_STATE_NO_WRITE_DRAW_ALL);
			DrawModel(background, currentCamera->GetPosition());
			renderer->SetDepthState(DEPTH_STATE_ENABLE);
		}

		Scene::Draw();
		if (previewRenderTarget) {
			GetRenderer()->SetRenderTarget(this->GetRenderTarget());
			GetRenderer()->SetViewport(
				GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f,
				GetScreenWidth(), GetScreenHeight()
			);
		}
		busy.store(false, std::memory_order_release);
	}

	// =======================================================
	// レイヤータイプの定義
	// =======================================================
	/*LAYER_TYPE PreviewScene::GetLayerType(int layer)
	{
		return Scene::GetLayerType(layer);
	}*/

	void PreviewScene::SortGameObjects()
	{
		if (locked.load(std::memory_order_relaxed)) return;

		busy.store(true, std::memory_order_release);
		gameObjects.sort([this](GameObject* a, GameObject* b) -> bool {
			if (a->layer == b->layer) {
				return DistanceSquare(currentCamera->position, a->position) > DistanceSquare(currentCamera->position, b->position);
			}
			return a->layer < b->layer;
		});
		busy.store(false, std::memory_order_release);
	}
}
