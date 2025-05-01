// =======================================================
// sceneTransition.h
// 
// SceneTransition�N���X�y�уV�[���J�ڐ���
// 
// ��ҁF鰕��r�i�K�C�@�}���`�����j�@2024/08/06
// =======================================================
#ifndef _SCENE_TRANSITAION_H
#define _SCENE_TRANSITAION_H

#include "MGCommon.h"
#include "scene.h"


// =======================================================
// �V�[���J�ړo�^�p�̒�^��
// REGISTER_TRANSITION(�V�[���J�ږ�, �N���X��)
// =======================================================
#define REGISTER_TRANSITION(name, className) \
		MG::RegisterTransition(name, []() -> MG::SceneTransition* { return (MG::SceneTransition*)new className(); })


namespace MG {
	typedef std::string TransitionName;
	constexpr const char* TRANSITION_DEFAULT = "default";

	class SceneTransition {
	protected:
		std::list<Scene*>* m_runningScenes;
		SceneName m_src;
		SceneName m_dest;
		bool inTransition = true;
	public:

		virtual void Update();

		virtual void Draw();

		void SetRunningScenes(std::list<Scene*>* runningScenes);

		void SetSrc(SceneName src);

		void SetDest(SceneName dest);

		bool InTransition();
	};

	TransitionName RegisterTransition(string name, SceneTransition* (*function)());

	SceneTransition* CreateTransition(const string& name);

	void UnregisterAllTransition();

} // namespace MG

#endif