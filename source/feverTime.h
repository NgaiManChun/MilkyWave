#ifndef _FEVERTIME_H
#define _FEVERTIME_H

#include "gameObject.h"
#include "player.h"
#include "progress.h"
#include <list>
using namespace MG;

#define FRAGMENT_MAX_WIDTH  (1240 / 19)
#define FRAGMENT_MAX_HEIGHT (2765 / 19)
#define FRAGMENT_COUNT  (5)

class FragmentUI :public GameObject {
private:
	float currentSize = 0.0f;

public:
	void SetCurrentSize(float size);
	float GetCurrentSize();

};

class FeverTimeUI :public GameObject {
private:
	Texture* fragmentTexture;
	Texture* lightTexture;
	Texture* starTexture;

	FragmentUI fragment[FRAGMENT_COUNT];
	F3 centerPosition = { 0.0f, 0.0f, 0.0f };
	float radius = FRAGMENT_MAX_HEIGHT / 2;

	float currentGauge = 0.0f;

	const Player* player;
	Progress charage{ 300.0f, false };

public:
	FeverTimeUI();
	void Update() override;
	void Draw() override;

	void SetCurrentGauge(float gauge);
	void SetPlayer(const Player* player);
};

#endif