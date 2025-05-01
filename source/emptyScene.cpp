#include "scene.h"
using namespace MG;

namespace EmptyScene {
	// =======================================================
	// クラス定義
	// =======================================================
	class EmptyScene : public Scene {
	public:
		//void Init() override;
		//void Uninit() override;
		//void Update() override;
		//void Draw() override;
		//LAYER_TYPE GetLayerType(int layer) override;
	};


	// =======================================================
	// シーン登録
	// =======================================================
	static SceneName sceneName = REGISTER_SCENE("empty", EmptyScene);

	// =======================================================
	// 初期化
	// =======================================================
	/*void EmptyScene::Init()
	{
		Scene::Init();
	}*/
	

	// =======================================================
	// 終了処理
	// =======================================================
	/*void EmptyScene::Uninit()
	{
		Scene::Uninit();
	}*/
	

	// =======================================================
	// 更新
	// =======================================================
	/*void EmptyScene::Update()
	{
		Scene::Update();
	}*/
	

	// =======================================================
	// 描画
	// =======================================================
	/*void EmptyScene::Draw()
	{
		Scene::Draw();
	}*/

	// =======================================================
	// レイヤータイプの定義
	// =======================================================
	/*LAYER_TYPE EmptyScene::GetLayerType(int layer)
	{
		return Scene::GetLayerType(layer);
	}*/
}
