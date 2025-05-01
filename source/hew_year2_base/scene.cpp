// =======================================================
// scene.cpp
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
// =======================================================
#define NOMINMAX
#include "scene.h"
#include "sceneTransitaion.h"
#include "gameObject.h"
#include "renderer.h"
#include "resourceTool.h"
#include <algorithm>

namespace MG {
	void Scene::Init() {}

	LAYER_TYPE Scene::GetLayerType(int layer)
	{
		if (layer < 0 || layer >= LAYER_2D) {
			return LAYER_TYPE_2D;
		}
		return LAYER_TYPE_3D;
	}

	void Scene::SortGameObjects()
	{
		gameObjects.sort([](GameObject* a, GameObject* b) -> bool {
			return a->layer < b->layer;
		});
	}

	std::list<GameObject*>& Scene::GetGameObjects()
	{
		return gameObjects;
	}

	Camera3D* Scene::GetCurrentCamera() const
	{
		return currentCamera;
	}

	Camera2D* Scene::GetCurrentCamera2D() const
	{
		return currentCamera2D;
	}

	void Scene::DoGameObjectCommands()
	{
		while (!gameObjectCommands.empty()) {
			GameObjectCommand command = gameObjectCommands.front();
			gameObjectCommands.pop_front();
			DoGameObjectCommand(command);
		}
	}

	void Scene::DoGameObjectCommand(const GameObjectCommand& command)
	{
		if (command.type == UPDATE) {
			auto itr = std::find(gameObjects.begin(), gameObjects.end(), command.gameObject);
			if (itr != gameObjects.end() && command.gameObject->enable) {
				lockedGameObject = command.gameObject;
				command.gameObject->Update();
				lockedGameObject = nullptr;
			}
		}
		else if (command.type == DRAW) {
			auto itr = std::find(gameObjects.begin(), gameObjects.end(), command.gameObject);
			if (itr != gameObjects.end() && command.gameObject->enable) {
				LAYER_TYPE type = GetLayerType(command.gameObject->layer);
				if (currentLayerType != type) {
					currentLayerType = type;
					if (type == LAYER_TYPE_2D) {
						GetRenderer()->ApplyCamera(currentCamera2D);
					}
					else {
						GetRenderer()->ApplyCamera(currentCamera);
					}
				}
				lockedGameObject = command.gameObject;
				command.gameObject->Draw();
				lockedGameObject = nullptr;
			}
		}
		else if (command.type == ADD) {
			command.gameObject->scene = this;
			command.gameObject->layer = command.layer;
			gameObjects.push_back(command.gameObject);
		}
		else if (command.type == INSERT) {
			int index = command.index;
			index = std::max(index, 0);
			index = std::min(index, (int)gameObjects.size());
			command.gameObject->scene = this;
			command.gameObject->layer = command.layer;
			gameObjects.insert(std::next(gameObjects.begin(), index), command.gameObject);
		}
		else if (command.type == REMOVE) {
			gameObjects.remove(command.gameObject);
			delete command.gameObject;
		}
	}


	// =======================================================
	// �Q�[���I�u�W�F�N�g���ғ����X�g�ɒǉ�
	// =======================================================
	GameObject* Scene::AddGameObjectPtr(GameObject* gameObject, const int layer)
	{
		DoGameObjectCommand({ gameObject, ADD, layer });
		return gameObject;
	}

	// =======================================================
	// �Q�[���I�u�W�F�N�g���ғ����X�g�ɑ}��
	// =======================================================
	GameObject* Scene::InsertGameObjectPtr(GameObject* gameObject, const unsigned int index, const int layer)
	{
		DoGameObjectCommand({ gameObject, INSERT, layer, index });
		return gameObject;
	}

	// =======================================================
	// �Q�[���I�u�W�F�N�g���ғ����X�g���珜�O
	// =======================================================
	void Scene::DeleteGameObject(GameObject* gameObject)
	{
		if (lockedGameObject != nullptr && lockedGameObject == gameObject) {
			gameObjectCommands.push_front({ gameObject, REMOVE });
		}
		else {
			DoGameObjectCommand({ gameObject, REMOVE });
		}
	}


	// =======================================================
	// �I������
	// 
	// �ғ����X�g�̒��̃Q�[���I�u�W�F�N�g�͂��ׂĔj��
	// ���̃V�[���Ŏ擾�������\�[�X�����ׂĉ��
	// �i���̃X�R�[�v�Ŏg�p���̃��\�[�X�͏��O�j
	// =======================================================
	void Scene::Uninit() {
		for (auto itr = gameObjects.begin(); itr != gameObjects.end(); itr++) {
			GameObject* gameObject = *itr;
			delete gameObject;
		}
		gameObjects.clear();
		gameObjectCommands.clear();
		ReleaseResource(sceneName);
	}


	// =======================================================
	// �X�V
	// 
	// �ғ����X�g�̒��̃Q�[���I�u�W�F�N�g��Update�֐���
	// ����Ăяo��
	// =======================================================
	void Scene::Update()
	{
		for (auto itr = gameObjects.begin(); itr != gameObjects.end(); itr++) {
			GameObject* gameObject = *itr;
			gameObjectCommands.push_back({ gameObject, UPDATE });
		}
		DoGameObjectCommands();
	}


	// =======================================================
	// �`��
	// 
	// �ғ����X�g�̒��̃Q�[���I�u�W�F�N�g��Draw�֐���
	// ����Ăяo��
	// =======================================================
	void Scene::Draw() {
		for (auto itr = gameObjects.begin(); itr != gameObjects.end(); itr++) {
			gameObjectCommands.push_back({ *itr, DRAW });
		}
		currentLayerType = LAYER_TYPE_NONE;
		Renderer* renderer = GetRenderer();
		renderer->SetAmbient(ambientLight);
		renderer->SetLight(driectLightPosition, driectLight);
		DoGameObjectCommands();
	}

	// =======================================================
	// �e�N�X�`�����\�[�X���擾
	// =======================================================
	Texture* Scene::LoadTexture(const std::string& path)
	{
		return GetResourceTool()->LoadTexture(path, sceneName);
	}


	// =======================================================
	// �e�N�X�`�����\�[�X���擾�A�X�R�[�v�w��
	// =======================================================
	Texture* Scene::LoadTexture(const std::string& path, const std::string& scope)
	{
		return GetResourceTool()->LoadTexture(path, scope);
	}

	Texture* Scene::RenderText(const std::wstring& text, const FONT& font)
	{
		return GetResourceTool()->RenderText(text, font, sceneName);
	}

	Texture* Scene::RenderText(const std::wstring& text, const FONT& font, const std::string& scope)
	{
		return GetResourceTool()->RenderText(text, font, scope);
	}


	// =======================================================
	// �I�[�f�B�I���\�[�X���擾
	// =======================================================
	Audio* Scene::LoadAudio(const std::string& path)
	{
		return GetResourceTool()->LoadAudio(path, sceneName);
	}


	// =======================================================
	// �I�[�f�B�I���\�[�X���擾�A�X�R�[�v�w��
	// =======================================================
	Audio* Scene::LoadAudio(const std::string& path, const std::string& scope)
	{
		return GetResourceTool()->LoadAudio(path, scope);
	}


	// =======================================================
	// ���f�����\�[�X���擾
	// =======================================================
	Model* Scene::LoadModel(const std::string& path)
	{
		return GetResourceTool()->LoadModel(path, sceneName);
	}

	
	// =======================================================
	// ���f�����\�[�X���擾�A�X�R�[�v�w��
	// =======================================================
	Model* Scene::LoadModel(const std::string& path, const std::string& scope)
	{
		return GetResourceTool()->LoadModel(path, scope);
	}


	// =======================================================
	// �V�[���J�ڊJ�n
	// 
	// �����F�J�ڐ�
	// =======================================================
	void Scene::SceneTransit(SceneName dest)
	{
		MG::SceneTransit(dest, sceneName);
	}


	// =======================================================
	// �V�[���J�ڊJ�n
	// 
	// �����F�J�ڐ�A�J�ڌ��ʖ�
	// =======================================================
	void Scene::SceneTransit(SceneName dest, std::string transitionName)
	{
		MG::SceneTransit(dest, sceneName, transitionName);
	}


	// =======================================================
	// �V�[���J�ڊJ�n
	// 
	// �����F�J�ڐ�A�J�ڌ��ʖ��A�J�ڌ�
	// �J�ڌ������̃V�[������Ȃ��ꍇ�����g�p
	// =======================================================
	void Scene::SceneTransit(SceneName dest, std::string transitionName, SceneName src)
	{
		MG::SceneTransit(dest, src, transitionName);
	}

	void Scene::SetRenderTarget(RenderTarget* renderTarget)
	{
		this->renderTarget = renderTarget;
	}

	RenderTarget* Scene::GetRenderTarget() const
	{
		return renderTarget;
	}


	// =======================================================
	// �ȉ��V�[������֘A
	// =======================================================

	static map<string, Scene* (*)()>* __sceneInstanceFunctions;
	static map<string, Scene*>* __instances;
	static list<Scene*>* __runningScenes;
	static Scene* lockedScene = nullptr;
	static list<SceneCommand> __sceneCommands;
	static SceneTransition* sceneTransition;


	// =======================================================
	// �V�[���o�^�֐�
	// 
	// �����P�̓V�[����
	// �����Q�̓C���X�^���X�쐬��CALLBACK�֐�
	// 
	// ��^���@REGISTER_SCENE(�V�[����, �N���X��)
	// �ŗ��p����̂����X�X��
	// =======================================================
	SceneName RegisterScene(string name, Scene* (*function)())
	{
		if (!__sceneInstanceFunctions) {
			__sceneInstanceFunctions = new map<string, Scene* (*)()>;
		}
		if (!__instances) {
			__instances = new map<string, Scene*>;
		}
		if (!__runningScenes) {
			__runningScenes = new list<Scene*>;
		}
		(*__sceneInstanceFunctions)[name] = function;
		(*__instances)[name] = nullptr;

		return name;
	}

	void DoSceneCommands()
	{
		while (!__sceneCommands.empty()) {
			SceneCommand command = __sceneCommands.front();
			__sceneCommands.pop_front();
			DoSceneCommand(command);
		}
	}

	void DoSceneCommand(SceneCommand command)
	{
		if (command.type == UPDATE) {
			if ((*__instances)[command.sceneName]) {
				Scene* scene = (*__instances)[command.sceneName];
				auto itr = std::find(__runningScenes->begin(), __runningScenes->end(), scene);
				if (itr != __runningScenes->end()) {
					lockedScene = scene;
					scene->Update();
					scene->SortGameObjects();
					lockedScene = nullptr;
				}
			}
		}
		else if (command.type == DRAW) {
			if ((*__instances)[command.sceneName]) {
				Scene* scene = (*__instances)[command.sceneName];
				auto itr = std::find(__runningScenes->begin(), __runningScenes->end(), scene);
				if (itr != __runningScenes->end()) {
					lockedScene = scene;
					scene->Draw();
					lockedScene = nullptr;
				}
			}
		}
		else if (command.type == START) {
			Scene* scene = LoadScene(command.sceneName);
			if (scene) {
				auto itr = std::find(__runningScenes->begin(), __runningScenes->end(), scene);
				if (itr != __runningScenes->end()) {
					__runningScenes->erase(itr);
				}
				lockedScene = scene;
				scene->Init();
				lockedScene = nullptr;
				__runningScenes->push_front(scene);
			}
		}
		else if (command.type == END) {
			if ((*__instances)[command.sceneName]) {
				Scene* scene = (*__instances)[command.sceneName];
				auto itr = std::find(__runningScenes->begin(), __runningScenes->end(), scene);
				if (itr != __runningScenes->end()) {
					lockedScene = scene;
					scene->Uninit();
					lockedScene = nullptr;
					__runningScenes->erase(itr);
				}
			}
		}
		else if (command.type == LOAD) {
			if (!(*__instances)[command.sceneName]) {
				if ((*__sceneInstanceFunctions)[command.sceneName]) {
					(*__instances)[command.sceneName] = (*__sceneInstanceFunctions)[command.sceneName]();
				}
			}
		}
		else if (command.type == RELEASE) {
			Scene* scene = (*__instances)[command.sceneName];
			auto itr = std::find(__runningScenes->begin(), __runningScenes->end(), scene);
			if (scene && itr != __runningScenes->end()) {
				__runningScenes->erase(itr);
				delete scene;
				__instances->erase(command.sceneName);
			}
		}
	}

	void UninitScene()
	{
		for (const auto& pair : (*__instances)) {
			if ((*__instances)[pair.first]) {
				(*__instances)[pair.first]->Uninit();
			}
			delete (*__instances)[pair.first];
			(*__instances)[pair.first] = nullptr;
		}
		for (auto& pair : *__sceneInstanceFunctions) {
			(*__sceneInstanceFunctions)[pair.first] = nullptr;
		}
		__sceneInstanceFunctions->clear();
		__instances->clear();
		__runningScenes->clear();
		delete __sceneInstanceFunctions;
		delete __instances;
		delete __runningScenes;
	}


	// =======================================================
	// �V�[���̃C���X�^���X����
	// 
	// �w�肵���V�[���̃C���X�^���X�𐶐�����
	// �����V�[���̃C���X�^���X�͓����ɕ������Ȃ�
	// Init�֐��͌Ăяo����Ȃ�
	// =======================================================
	Scene* LoadScene(const string& name)
	{
		DoSceneCommand({ name, LOAD });
		return (*__instances)[name];
	}


	// =======================================================
	// �V�[���̃C���X�^���X�j��
	// 
	// �w�肵���V�[���̃C���X�^���X��j������
	// Uninit�֐��͌Ăяo����Ȃ�
	// =======================================================
	void ReleaseScene(const string& name)
	{
		if (lockedScene != nullptr && lockedScene == (*__instances)[name]) {
			__sceneCommands.push_front({ name, RELEASE });
		}
		else {
			DoSceneCommand({ name, RELEASE });
		}
	}


	// =======================================================
	// �V�[���J�n
	// 
	// �w�肵���V�[�����ғ��V�[���s��̍őO�ɒǉ�����
	// �V�[����Init�֐��͌Ăяo�����
	// �C���X�^���X���܂���������ĂȂ��V�[���͐��������
	// =======================================================
	void StartScene(const string& name)
	{
		DoSceneCommand({ name, START });
	}


	// =======================================================
	// �V�[���I��
	// 
	// �w�肵���V�[�����ғ��V�[���s�񂩂珜�O
	// �V�[����Uninit�֐��͌Ăяo�����
	// �C���X�^���X�͔j������Ȃ�
	// =======================================================
	void EndScene(const string& name)
	{
		DoSceneCommand({ name, END });
	}


	// =======================================================
	// �V�[���X�V
	// 
	// �ғ��V�[���s��̒��̃V�[����Update�֐�������Ăяo��
	// �V�[���J�ڒ��͐�ɃV�[���J�ڍX�V����
	// =======================================================
	void UpdateScene()
	{
		if (sceneTransition) {
			if (!sceneTransition->InTransition()) {
				delete sceneTransition;
				sceneTransition = nullptr;
			}
		}
		if (sceneTransition) {
			sceneTransition->Update();
		}
		for (auto itr = __runningScenes->begin(); itr != __runningScenes->end(); itr++) {
			Scene* scene = *itr;
			__sceneCommands.push_back({ scene->sceneName, UPDATE });
		}
		DoSceneCommands();
	}


	// =======================================================
	// �V�[���`��
	// 
	// �ғ��V�[���s��̒��̃V�[����Draw�֐�������Ăяo��
	// �V�[���J�ڒ��̓V�[���J�ڂ�Draw�֐����������s����
	// =======================================================
	void DrawScene()
	{
		if (sceneTransition) {
			sceneTransition->Draw();
		}
		else {
			for (auto itr = __runningScenes->begin(); itr != __runningScenes->end(); itr++) {
				Scene* scene = *itr;
				__sceneCommands.push_back({ scene->sceneName, DRAW });
			}
		}
		DoSceneCommands();
	}

	void SceneTransit(SceneName dest, SceneName src)
	{
		SceneTransit(dest, src, TRANSITION_DEFAULT);
	}


	// =======================================================
	// �V�[���J�ڊJ�n
	// =======================================================
	void SceneTransit(SceneName dest, SceneName src, std::string transitionName)
	{
		if (!sceneTransition) {
			sceneTransition = CreateTransition(transitionName);
			sceneTransition->SetRunningScenes(__runningScenes);
			sceneTransition->SetDest(dest);
			sceneTransition->SetSrc(src);
		}
	}


	// =======================================================
	// �V�[���J�ڒ����ǂ������`�F�b�N
	// =======================================================
	bool InTransition()
	{
		if (sceneTransition) {
			return sceneTransition->InTransition();
		}
		return false;
	}
}