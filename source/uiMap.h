#ifndef _UIMAP_H
#define _UIMAP_H
#include "gameObject.h"
#include "progress.h"
#include "player.h"
using namespace MG;


class UIMap : public GameObject
{
public:
	struct _CONFIG {
		float UI_MAP_BAR_WIDTH = 50.0f;
		float UI_MAP_BAR_HEIGHT = 600.0f;
		float UI_MAP_ICON_WIDTH = 400.0f;
		float UI_MAP_ICON_HEIGHT = 313.0f;
		float UI_MAP_RIGHT = 80.0f;
		float UI_MAP_BOTTOM = 40.0f;
	};
	struct _ASSET {
		std::string UI_MAP_ICON;
		std::string UI_MAP_BAR;
	};
private:
	float currentT;
	const Player* player;

	Texture* barTexture;
	Texture* iconTexture;
public:
	UIMap();

	void Update() override;

	void Draw()override;

	void SetPlayer(const Player* player);
};

#endif