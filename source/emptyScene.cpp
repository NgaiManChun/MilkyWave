#include "scene.h"
using namespace MG;

namespace EmptyScene {
	// =======================================================
	// �N���X��`
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
	// �V�[���o�^
	// =======================================================
	static SceneName sceneName = REGISTER_SCENE("empty", EmptyScene);

	// =======================================================
	// ������
	// =======================================================
	/*void EmptyScene::Init()
	{
		Scene::Init();
	}*/
	

	// =======================================================
	// �I������
	// =======================================================
	/*void EmptyScene::Uninit()
	{
		Scene::Uninit();
	}*/
	

	// =======================================================
	// �X�V
	// =======================================================
	/*void EmptyScene::Update()
	{
		Scene::Update();
	}*/
	

	// =======================================================
	// �`��
	// =======================================================
	/*void EmptyScene::Draw()
	{
		Scene::Draw();
	}*/

	// =======================================================
	// ���C���[�^�C�v�̒�`
	// =======================================================
	/*LAYER_TYPE EmptyScene::GetLayerType(int layer)
	{
		return Scene::GetLayerType(layer);
	}*/
}
