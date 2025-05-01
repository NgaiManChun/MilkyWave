// =======================================================
// sampleTransitaion.cpp
// 
// スターシーン遷移
// 
// 作者：宮下雅友　2024/12/23
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
	// クラス定義
	// 
	// SceneTransitionクラスに継承
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
	// 遷移効果登録
	// 
	// REGISTER_TRANSITION( 遷移効果名, クラス名 )
	// 「static TransitionName name = 」の部分は必須
	// =======================================================
	static TransitionName name = REGISTER_TRANSITION("star", StarTransition);

	StarTransition::StarTransition() {
		// スクリーンの中心座標を取得
		F2 screenCenter = GetScreenCenter();

		// スクリーンのサイズを取得
		F2 screenSize = GetScreenSize();

		maxLength = Distance({}, screenSize);
		angle = atanf(-screenSize.y / screenSize.x);
		rotate = Quaternion::AxisZRadian(angle);
	}

	// =======================================================
	// 更新
	// 
	// 各シーンのUpdateが呼び出される前にこっちの処理を行う
	// 遷移処理が終わったら、inTransitionをFALSEにしてください
	// このオブジェクトは自動的に消滅される
	// =======================================================
	void StarTransition::Update()
	{
		inTransition = true;
		if (progress == 0.0f) {
			// 目的シーンを開始する
			StartScene(m_dest);
		}
		else if (progress == 1.0f) {
			// 遷移元のシーンを終了する
			EndScene(m_src);
			inTransition = false;
		}

		starRotate *= Quaternion::AxisZDegree(-720.0f * GetDeltaTime() * 0.001f);

		// 時間経過処理
		progress.IncreaseValue(GetDeltaTime());
	}


	// =======================================================
	// 描画
	// =======================================================
	void StarTransition::Draw()
	{
		// スクリーンの中心座標を取得
		F2 screenCenter = GetScreenCenter();

		// スクリーンのサイズを取得
		F2 screenSize = GetScreenSize();

		for (auto scene : *m_runningScenes) {
			if (scene->sceneName == m_src) {

				// サブ描画ターゲット設定
				RenderTarget* subRenderTarget = renderer->GetSubRenderTarget();
				renderer->SetRenderTarget(subRenderTarget);
				renderer->ClearRenderTargetView(subRenderTarget);
				renderer->SetViewport(screenCenter.x, screenCenter.y, screenSize.x, screenSize.y);

				scene->SetRenderTarget(subRenderTarget);

				// シーンの描画処理
				DoSceneCommand({ scene->sceneName, DRAW });

				renderer->SetViewport(screenCenter.x, screenCenter.y, screenSize.x, screenSize.y);
				renderer->ApplyCamera(&camera);

				// くりぬき画像の描画
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

				// メイン描画ターゲットに切り替える
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

		// メイン描画ターゲットに切り替える
		renderer->SetRenderTarget(nullptr);
		renderer->SetViewport(screenCenter.x, screenCenter.y, screenSize.x, screenSize.y);
		renderer->ApplyCamera(&camera);
		// 星の描画
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