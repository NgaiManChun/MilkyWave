// =======================================================
// flipTransitaion.cpp
// 
// �t���b�v�V�[���J��
// 
// ��ҁF鰕��r�i�K�C�@�}���`�����j�@2024/08/29
// =======================================================
#include "sceneTransitaion.h"
#include "progress.h"
using namespace MG;

#define TEXTURE_WHITE "asset\\texture\\white.png"

namespace FlipTransitaion {
	// =======================================================
	// �N���X��`
	// 
	// SceneTransition�N���X�Ɍp��
	// =======================================================
	class FlipTransitaion : public SceneTransition {
	private:
		Progress progress = Progress(1000.0f, false);
	public:
		void Update() override;
		void Draw() override;
	};

	// =======================================================
	// �J�ڌ��ʓo�^
	// =======================================================
	static TransitionName name = REGISTER_TRANSITION("flip", FlipTransitaion);


	// =======================================================
	// �X�V
	// 
	// �e�V�[����Update���Ăяo�����O�ɂ������̏������s��
	// �J�ڏ������I�������AinTransition��FALSE�ɂ��Ă�������
	// ���̃I�u�W�F�N�g�͎����I�ɏ��ł����
	// =======================================================
	void FlipTransitaion::Update()
	{
		inTransition = true;
		if (progress == 0.0f) {
			// �ړI�V�[�����J�n����
			StartScene(m_dest);
		}
		else if (progress == 1.0f) {
			// �J�ڌ��̃V�[�����I������
			EndScene(m_src);
			inTransition = false;
		}

		// ���Ԍo�ߏ���
		progress.IncreaseValue(GetDeltaTime());
	}


	// =======================================================
	// �`��
	// =======================================================
	void FlipTransitaion::Draw()
	{
		// �R�s�[�����A���X�g�ϓ��h�~
		std::list<Scene*> runningScenes(*m_runningScenes);

		// �X�N���[���̒��S���W���擾
		F2 screenCenter = GetScreenCenter();

		// �X�N���[���̃T�C�Y���擾
		F2 screenSize = GetScreenSize();

		float position_x = (screenCenter.x - screenSize.x)* (1 - progress) +
			(screenCenter.x + screenSize.x) * progress;

		

		// ���̎��A�ғ����̃V�[���͓�i�J�ڐ�ƑJ�ڌ��j����̂Ń��[�v
		for (auto itr = runningScenes.begin(); itr != runningScenes.end(); itr++) {
			Scene* scene = *itr;

			if (scene->sceneName == m_dest) {
				// �J�ڐ�
				if (progress > 0.5f) {
					scene->Draw();
				}
				
			}
			else if (scene->sceneName == m_src) {
				// �J�ڌ�
				if (progress < 0.5f) {
					scene->Draw();
				}
			}
		}

		DrawQuad(
			LoadTexture(TEXTURE_WHITE, name),
			{ position_x, screenCenter.y, 0 },
			{ screenSize.x, screenSize.y, 0 },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f }
		);
	}

} // namespace FlipTransitaion