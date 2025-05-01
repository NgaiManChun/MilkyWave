#include "feverTime.h"
#include "cmath"
#include "scene.h"

#define PI (3.141592f)

constexpr const char* TEXTURE_STAR_FRAGMENT = "asset\\texture\\star_fragment.png";
constexpr const char* TEXTURE_LIGHT = "asset\\texture\\star_fragment_light.png";
constexpr const char* TEXTURE_STAR = "asset\\texture\\star.png";
 
void FragmentUI::SetCurrentSize(float size)
{
	currentSize = size;
}

float FragmentUI::GetCurrentSize()
{
	return currentSize;
}

FeverTimeUI::FeverTimeUI() :GameObject({ 1.0f,1.0f,1.0f })
{
	fragmentTexture = LoadTexture(TEXTURE_STAR_FRAGMENT);
	lightTexture = LoadTexture(TEXTURE_LIGHT);
	starTexture = LoadTexture(TEXTURE_STAR);

	for (int i = 0; i < FRAGMENT_COUNT; i++) {
		fragment[i].rotate = Quaternion::AxisZDegree(-i * 72.0f);
		F3 pos;
		float angle = PI / 2 - 2.0f * PI * i / FRAGMENT_COUNT;
		pos.x = centerPosition.x + (radius + 10.0f) * std::cos(angle);
		pos.y = centerPosition.y + (radius + 10.0f) * std::sin(angle);
		pos.z = 0.0f;
		fragment[i].position = pos;
		fragment[i].size = { 0.0f, 0.0f, 0.0f };
	}
}

void FeverTimeUI::Update()
{
	bool noCharage = true;
	auto& feverStocks = player->GetFeverStocks();
	for (auto& feverStock : feverStocks) {
		if (feverStock.hold == 1.0f) {
			charage.IncreaseValue(GetDeltaTime());
			noCharage = false;
		}
	}
	if (noCharage) {
		charage.IncreaseValue(-GetDeltaTime());
	}
}

float easeOutExpo(float t) {
	return 1.0f - pow(2.0f, -8.0f * t);
}

void FeverTimeUI::Draw()
{
	auto& feverStocks = player->GetFeverStocks();
	for (auto& feverStock : feverStocks) {
		if (feverStock.hold < 1.0f) {
			for (int i = 0; i < 3; i++) {
				const F2& p1 = feverStock.effectControls[i];
				DrawQuad(
					lightTexture,
					Bezier({ feverStock.p0.x, feverStock.p0.y }, { p1.x, p1.y }, position, feverStock.hold),
					F3{ FRAGMENT_MAX_WIDTH, FRAGMENT_MAX_HEIGHT, 0.0f },
					Quaternion::AxisZDegree(((int)p1.x%10) * 1080.0f * feverStock.hold)
				);
			}
		}
	}

    // UI‚Ì”wŒi‚Ì•`ŽÊ
    for (int i = 0; i < FRAGMENT_COUNT; i++) {
        DrawQuad(
            fragmentTexture,
            position + fragment[i].position,
            F3{ FRAGMENT_MAX_WIDTH, FRAGMENT_MAX_HEIGHT, 0.0f },
            fragment[i].rotate,
			F4{ color.x, color.y, color.z, color.w * 0.3f }
        );
    }

	if (player) {
		float currentAmount = player->GetFeverAmount();
		int i = 0;
		while (currentAmount > 0.0f) {

			float scale = min(currentAmount / (1.0f / FRAGMENT_COUNT), 1.0f);
			
			if (charage && scale > 0.01f && currentAmount < (1.0f / FRAGMENT_COUNT)) {
				DrawQuad(
					lightTexture,
					position + fragment[i].position,
					F3{ FRAGMENT_MAX_WIDTH, FRAGMENT_MAX_HEIGHT, 0.0f } * 2.5f,
					fragment[i].rotate,
					{ color.x, color.y, color.z, color.w * charage }
				);
			}

			DrawQuad(
				fragmentTexture,
				position + fragment[i].position,
				F3{ FRAGMENT_MAX_WIDTH * scale, FRAGMENT_MAX_HEIGHT * scale, 0.0f },
				fragment[i].rotate,
				color
			);
			i++;
			currentAmount -= 1.0f / FRAGMENT_COUNT;
		}
		
	}
}

void FeverTimeUI::SetCurrentGauge(float gauge)
{
	currentGauge = gauge;
}

void FeverTimeUI::SetPlayer(const Player* player)
{
	this->player = player;
}
