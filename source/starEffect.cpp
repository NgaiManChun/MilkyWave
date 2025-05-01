#include "starEffect.h"
#include "progress.h"
#include "camera.h"
#include "scene.h"
#include "renderer.h"

static Camera3D* g_Camera;

static constexpr const char* TEXTURE_STAR = "asset\\texture\\star.png";

StarEffect::StarEffect(F3 position, float span, float duration, float maxRadius)
	: GameObject({}, position), spanT({ span, false }), duration(duration), maxRadius(maxRadius), texture(LoadTexture(TEXTURE_STAR))
{
}

void StarEffect::Update()
{
	int deltaTime = GetDeltaTime();
	std::remove_if(stars.begin(), stars.end(), [deltaTime](StarEffect::Star& star) {
		if (star.t == 1.0f) {
			return true;
		}
		star.t.IncreaseValue(deltaTime);
		return false;
	});

	if (started) {
		if (spanT == 1.0f) {
			// ¶¬
			stars.push_back(
				{
					position,
					position + F3{
						((float)rand() / RAND_MAX) * maxRadius * ((rand() % 2) ? 1.0f : -1.0f),
						((float)rand() / RAND_MAX) * maxRadius * ((rand() % 2) ? 1.0f : -1.0f),
						((float)rand() / RAND_MAX) * maxRadius * ((rand() % 2) ? 1.0f : -1.0f)
					},
					Progress{ duration, false }
				}
			);
 			spanT = 0.0f;
		}
		spanT.IncreaseValue(GetDeltaTime());
	}
}

float easeOutQuint(float t) {
	double t2 = (--t) * t;
	return 1 + t * t2 * t2;
}

void StarEffect::Draw()
{

	GetRenderer()->SetDepthState(DEPTH_STATE_NO_WRITE);
	GetRenderer()->SetBlendState(BLEND_STATE_ADD);

	for (auto& star : stars) {
		DrawQuad(texture, Lerp(star.startPosition, star.endPosition, star.t), F3{0.1f, 0.1f, 0.1f}, scene->GetCurrentCamera()->GetRotate(),
			{ color.x, color.y, color.z, color.w * (1.0f - easeOutQuint(star.t)) } );
	}

	GetRenderer()->SetBlendState(BLEND_STATE_ALPHA);
	GetRenderer()->SetDepthState(DEPTH_STATE_ENABLE);

}

void StarEffect::SetStarted(bool started)
{
	this->started = started;
	if (!started) {
		spanT = 0.0f;
	}
}
