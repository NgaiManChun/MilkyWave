// =======================================================
// scene.h
// 
// Scene�N���X�y��Scene����
// 
// ��ҁF鰕��r�i�K�C�@�}���`�����j�@2024/07/06
// 
// 2024/09/05:
// UninitScene() �S�C���X�^���X��Uninit���Ăяo���ǉ�
// DoGameObjectCommands() Insert�ł��Ȃ��o�O�C��
// 
// 2024/11/18:
// AddGameObject��InsertGameObject���}�N����
// 
// 2024/01/09:
// SortGameObjects()��ǉ��A
// �V�[�����Ƃ�GameObject�̃\�[�g���@��`�ł���
// =======================================================
#ifndef _SCENE_H
#define _SCENE_H

#include "MGCommon.h"
#include "gameObject.h"
#include "gameObjectQuad.h"
#include "gameObjectAudio.h"
#include "camera.h"
#include "resourceTool.h"
#include "collision.h"
#include "renderer.h"


// =======================================================
// �V�[���o�^�p�̒�^��
// REGISTER_SCENE(�V�[����, �N���X��)
// =======================================================
#define REGISTER_SCENE(name, className) \
MG::RegisterScene(name, []() -> MG::Scene* { MG::Scene* scene = (MG::Scene*)new className(); scene->sceneName = name; return scene; }); \
																       

#define AddGameObject(constructor, ...)					\
([this, ptr = new constructor]() {						\
	AddGameObjectPtr(ptr, ##__VA_ARGS__);				\
	return ptr;											\
})()													\

#define InsertGameObject(constructor, index, ...)		\
([this, ptr = new constructor]() {						\
	InsertGameObjectPtr(ptr, index, ##__VA_ARGS__);		\
	return ptr;											\
})()													\

namespace MG {
	typedef std::string SceneName;
	typedef int LAYER_TYPE;
	constexpr const int LAYER_2D = 10;
	constexpr const LAYER_TYPE LAYER_TYPE_NONE = 0;
	constexpr const LAYER_TYPE LAYER_TYPE_3D = 1;
	constexpr const LAYER_TYPE LAYER_TYPE_2D = 2;

	enum SCENE_COMMAND_TYPE {
		START,
		END,
		LOAD,
		RELEASE,
		UPDATE,
		DRAW
	};

	/*enum LAYER_TYPE {
		LAYER_TYPE_NONE,
		LAYER_TYPE_3D,
		LAYER_TYPE_2D
	};*/

	struct SceneCommand {
		SceneName sceneName;
		SCENE_COMMAND_TYPE type;
	};

	class Scene {
		enum GAME_OBJECT_LIST_COMMAND_TYPE {
			ADD,
			INSERT,
			REMOVE,
			UPDATE,
			DRAW
		};
		struct GameObjectCommand {
			GameObject* gameObject;
			GAME_OBJECT_LIST_COMMAND_TYPE type;
			int layer;
			unsigned int index;
		};
		
	private:
		Camera2D defaultCamera2D;
		Camera3D defaultCamera;
		GameObject* lockedGameObject = nullptr;
		LAYER_TYPE currentLayerType = LAYER_TYPE_NONE;
		std::list<GameObjectCommand> gameObjectCommands;
		void DoGameObjectCommands();
		void DoGameObjectCommand(const GameObjectCommand& command);
	protected:
		Camera2D* currentCamera2D = &defaultCamera2D;
		Camera3D* currentCamera = &defaultCamera;
		F4 ambientLight = { 1.0f, 1.0f, 1.0f, 0.7f };
		F4 driectLight = { 1.0f, 1.0f, 1.0f, 0.3f };
		F3 driectLightPosition = { 5.0f, 5.0f, -1.0f };
		std::list<GameObject*> gameObjects;
		RenderTarget* renderTarget = nullptr;
	public:
		SceneName sceneName;

		virtual ~Scene() = default;

		virtual void Init();

		// =======================================================
		// �I������
		// 
		// �ғ����X�g�̒��̃Q�[���I�u�W�F�N�g�͂��ׂĔj��
		// ���̃V�[���Ŏ擾�������\�[�X�����ׂĉ��
		// �i���̃X�R�[�v�Ŏg�p���̃��\�[�X�͏��O�j
		// =======================================================
		virtual void Uninit();

		// =======================================================
		// �X�V
		// 
		// �ғ����X�g�̒��̃Q�[���I�u�W�F�N�g��Update�֐���
		// ����Ăяo��
		// =======================================================
		virtual void Update();


		// =======================================================
		// �`��
		// 
		// �ғ����X�g�̒��̃Q�[���I�u�W�F�N�g��Draw�֐���
		// ����Ăяo��
		// =======================================================
		virtual void Draw();

		virtual LAYER_TYPE GetLayerType(int layer);

		virtual void SortGameObjects();

		virtual std::list<GameObject*>& GetGameObjects();

		virtual Camera3D* GetCurrentCamera() const;

		virtual Camera2D* GetCurrentCamera2D() const;

		// �Q�[���I�u�W�F�N�g���ғ����X�g�ɒǉ�
		GameObject* AddGameObjectPtr(GameObject* gameObject, const int layer = 0);

		// �Q�[���I�u�W�F�N�g���ғ����X�g�ɑ}��
		GameObject* InsertGameObjectPtr(GameObject* gameObject, const unsigned int index, const int layer = 0);

		// �Q�[���I�u�W�F�N�g���ғ����X�g���珜�O
		void DeleteGameObject(GameObject* gameObject);

		// �e�N�X�`�����\�[�X���擾
		Texture* LoadTexture(const std::string& path);

		// �e�N�X�`�����\�[�X���擾�A�X�R�[�v�w��
		Texture* LoadTexture(const std::string& path, const std::string& scope);

		Texture* RenderText(const std::wstring& text, const FONT& font = {});

		Texture* RenderText(const std::wstring& text, const FONT& font, const std::string& scope);

		// �I�[�f�B�I���\�[�X���擾
		Audio* LoadAudio(const std::string& path);

		// �I�[�f�B�I���\�[�X���擾�A�X�R�[�v�w��
		Audio* LoadAudio(const std::string& path, const std::string& scope);

		// ���f�����\�[�X���擾
		Model* LoadModel(const std::string& path);

		// ���f�����\�[�X���擾�A�X�R�[�v�w��
		Model* LoadModel(const std::string& path, const std::string& scope);

		// �V�[���J�ڊJ�n
		void SceneTransit(SceneName dest);

		// �V�[���J�ڊJ�n
		void SceneTransit(SceneName dest, std::string transitionName);

		// �V�[���J�ڊJ�n
		void SceneTransit(SceneName dest, std::string transitionName, SceneName src);

		void SetRenderTarget(RenderTarget* renderTarget);

		RenderTarget* GetRenderTarget() const;
	};


	// =======================================================
	// �V�[������֐�
	// =======================================================
	
	void UninitScene();
	SceneName RegisterScene(std::string name, Scene* (*function)());
	void DoSceneCommands();
	void DoSceneCommand(SceneCommand command);
	Scene* LoadScene(const std::string& name);
	void ReleaseScene(const std::string& name);
	void StartScene(const std::string& name);
	void EndScene(const std::string& name);
	void UpdateScene();
	void DrawScene();
	void SceneTransit(SceneName dest, SceneName src);
	void SceneTransit(SceneName dest, SceneName src, std::string transitionName);
	bool InTransition();

} // namespace MG

#endif