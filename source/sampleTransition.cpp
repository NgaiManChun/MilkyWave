// =======================================================
// sampleTransitaion.cpp
// 
// �V�[���J�ڂ̃T���v��
// 
// ��ҁF鰕��r�i�K�C�@�}���`�����j�@2024/08/03
// =======================================================
#include "sceneTransitaion.h"
#include "progress.h"
#include "rendererDX.h"
#include "resourceToolDX.h"
using namespace MG;

namespace SampleTransition {
	// =======================================================
	// �N���X��`
	// 
	// SceneTransition�N���X�Ɍp��
	// =======================================================
	class SampleTransition : public SceneTransition {
	private:
		Progress progress = Progress(1000.0f, false);
		Camera2D camera;
	public:
		void Update() override;
		void Draw() override;
	};

	// =======================================================
	// �J�ڌ��ʓo�^
	// 
	// REGISTER_TRANSITION( �J�ڌ��ʖ�, �N���X�� )
	// �ustatic TransitionName name = �v�̕����͕K�{
	// =======================================================
	static TransitionName name = REGISTER_TRANSITION("sample_transition", SampleTransition);


	// =======================================================
	// �X�V
	// 
	// �e�V�[����Update���Ăяo�����O�ɂ������̏������s��
	// �J�ڏ������I�������AinTransition��FALSE�ɂ��Ă�������
	// ���̃I�u�W�F�N�g�͎����I�ɏ��ł����
	// =======================================================
	void SampleTransition::Update()
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
	void SampleTransition::Draw()
	{
		// �R�s�[�����A���X�g�ϓ��h�~
		std::list<Scene*> runningScenes(*m_runningScenes);

		// �X�N���[���̒��S���W���擾
		F2 screenCenter = GetScreenCenter();

		// �X�N���[���̃T�C�Y���擾
		F2 screenSize = GetScreenSize();

		Renderer* renderer = GetRenderer();

		// ���̎��A�ғ����̃V�[���͓�i�J�ڐ�ƑJ�ڌ��j����̂Ń��[�v
		for (auto itr = runningScenes.begin(); itr != runningScenes.end(); itr++) {
			Scene* scene = *itr;

			// �T�u�`��^�[�Q�b�g�ݒ�
			RenderTarget* subRenderTarget = renderer->GetSubRenderTarget();
			renderer->SetRenderTarget(subRenderTarget);
			renderer->ClearRenderTargetView(subRenderTarget);
			renderer->SetViewport(screenCenter.x, screenCenter.y, screenSize.x, screenSize.y);
			scene->SetRenderTarget(subRenderTarget);

			// �V�[���̕`�揈��
			DoSceneCommand({ scene->sceneName, DRAW });

			scene->SetRenderTarget(nullptr);

			// ���C���`��^�[�Q�b�g�ɐ؂�ւ���
			renderer->SetRenderTarget(nullptr);
			renderer->SetViewport(screenCenter.x, screenCenter.y, screenSize.x, screenSize.y);

			// �T�u�`��^�[�Q�b�g�̃e�N�X�`����\��t����
			if (scene->sceneName == m_dest) {
				renderer->ApplyCamera(&camera);
				// �J�ڐ�
				DrawQuad(
					subRenderTarget->texture,
					{ 0.0f, 0.0f, 0.0f },
					{ screenSize.x, screenSize.y, 0.0f },
					{ 0.0f, 0.0f, 0.0f },
					{ 1.0f, 1.0f, 1.0f, progress }
				);
			}
			else if (scene->sceneName == m_src) {
				renderer->ApplyCamera(&camera);
				// �J�ڌ�
				DrawQuad(
					subRenderTarget->texture,
					{ 0.0f, 0.0f, 0.0f },
					{ screenSize.x, screenSize.y, 0.0f },
					{ 0.0f, 0.0f, 0.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f - progress }
				);
			}

		}
	}

} // namespace SampleTransition