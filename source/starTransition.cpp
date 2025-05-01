// =======================================================
// sampleTransitaion.cpp
// 
// �X�^�[�V�[���J��
// 
// ��ҁF�{����F�@2024/12/23
// =======================================================
#include "sceneTransitaion.h"
#include "progress.h"
#include "rendererDX.h"
#include "resourceToolDX.h"
using namespace MG;

namespace StarTransition {
	static constexpr const char* TEXTURE_WHITE = "asset\\texture\\white.png";
	static constexpr const char* TEXTURE_STAR = "asset\\texture\\star.png";
	static constexpr const int STAR_MAX = 15;

	// =======================================================
	// �N���X��`
	// 
	// SceneTransition�N���X�Ɍp��
	// =======================================================
	class StarTransition : public SceneTransition {
	private:
		Progress progress = Progress(1000.0f, false);
		Camera2D camera;
		Renderer* renderer = GetRenderer();
		F3 starSize = { 300.0f,300.0f,0.0f };
		Texture* whiteTexture = LoadTexture(TEXTURE_WHITE);
		Texture* starTexture = LoadTexture(TEXTURE_STAR);
		float maxLength;
		float angle;
		Quaternion rotate;
		Quaternion starRotate = Quaternion::Identity();
	public:
		StarTransition();
		void Update() override;
		void Draw() override;
	};

	// =======================================================
	// �J�ڌ��ʓo�^
	// 
	// REGISTER_TRANSITION( �J�ڌ��ʖ�, �N���X�� )
	// �ustatic TransitionName name = �v�̕����͕K�{
	// =======================================================
	static TransitionName name = REGISTER_TRANSITION("star", StarTransition);

	StarTransition::StarTransition() {
		// �X�N���[���̒��S���W���擾
		F2 screenCenter = GetScreenCenter();

		// �X�N���[���̃T�C�Y���擾
		F2 screenSize = GetScreenSize();

		maxLength = Distance({}, screenSize);
		angle = atanf(-screenSize.y / screenSize.x);
		rotate = Quaternion::AxisZRadian(angle);
	}

	// =======================================================
	// �X�V
	// 
	// �e�V�[����Update���Ăяo�����O�ɂ������̏������s��
	// �J�ڏ������I�������AinTransition��FALSE�ɂ��Ă�������
	// ���̃I�u�W�F�N�g�͎����I�ɏ��ł����
	// =======================================================
	void StarTransition::Update()
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

		starRotate *= Quaternion::AxisZDegree(-720.0f * GetDeltaTime() * 0.001f);

		// ���Ԍo�ߏ���
		progress.IncreaseValue(GetDeltaTime());
	}


	// =======================================================
	// �`��
	// =======================================================
	void StarTransition::Draw()
	{
		// �X�N���[���̒��S���W���擾
		F2 screenCenter = GetScreenCenter();

		// �X�N���[���̃T�C�Y���擾
		F2 screenSize = GetScreenSize();

		for (auto scene : *m_runningScenes) {
			if (scene->sceneName == m_src) {

				// �T�u�`��^�[�Q�b�g�ݒ�
				RenderTarget* subRenderTarget = renderer->GetSubRenderTarget();
				renderer->SetRenderTarget(subRenderTarget);
				renderer->ClearRenderTargetView(subRenderTarget);
				renderer->SetViewport(screenCenter.x, screenCenter.y, screenSize.x, screenSize.y);

				scene->SetRenderTarget(subRenderTarget);

				// �V�[���̕`�揈��
				DoSceneCommand({ scene->sceneName, DRAW });

				renderer->SetViewport(screenCenter.x, screenCenter.y, screenSize.x, screenSize.y);
				renderer->ApplyCamera(&camera);

				// ����ʂ��摜�̕`��
				{
					GetRenderer()->SetBlendState(BLEND_STATE_DEST_OUT);

					DrawQuad(
						whiteTexture,
						{ screenSize.x * -0.5f, screenSize.y * 0.5f, 0 },
						{ maxLength * progress * 2.0f, maxLength, 0 },
						rotate
					);

					GetRenderer()->SetBlendState(BLEND_STATE_ALPHA);
				}

				scene->SetRenderTarget(nullptr);

				// ���C���`��^�[�Q�b�g�ɐ؂�ւ���
				renderer->SetRenderTarget(nullptr);
				renderer->SetViewport(screenCenter.x, screenCenter.y, screenSize.x, screenSize.y);

				renderer->ApplyCamera(&camera);
				DrawQuad(
					subRenderTarget->texture,
					{ 0.0f, 0.0f, 0.0f },
					{ screenSize.x, screenSize.y, 0.0f }
				);
			}
			else {
				DoSceneCommand({ scene->sceneName, DRAW });
			}
		}

		// ���C���`��^�[�Q�b�g�ɐ؂�ւ���
		renderer->SetRenderTarget(nullptr);
		renderer->SetViewport(screenCenter.x, screenCenter.y, screenSize.x, screenSize.y);
		renderer->ApplyCamera(&camera);
		// ���̕`��
		float starRawX = maxLength * progress;
		for (int i = 0; i < STAR_MAX; i++)
		{
			F3 position = Rotate({ starRawX , maxLength * 0.5f - starSize.y * i * 0.7f , 0.0f }, rotate) + F3{ screenSize.x * -0.5f, screenSize.y * 0.5f, 0};

			DrawQuad(
				starTexture,
				position,
				starSize,
				starRotate
			);

		}
	}

} // namespace SampleTransition