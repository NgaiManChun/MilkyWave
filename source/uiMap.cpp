#include "uiMap.h"
#include "progress.h"
#include "config.h"

static const UIMap::_CONFIG CONFIG = LoadConfig<UIMap::_CONFIG>("asset\\config.csv", [](const D_KVTABLE& table) -> UIMap::_CONFIG {
	return {
		TABLE_FLOAT_VALUE(table, "UI_MAP_BAR_WIDTH", 50.0f),
		TABLE_FLOAT_VALUE(table, "UI_MAP_BAR_HEIGHT", 600.0f),
		TABLE_FLOAT_VALUE(table, "UI_MAP_ICON_WIDTH", 400.0f),
		TABLE_FLOAT_VALUE(table, "UI_MAP_ICON_HEIGHT", 313.0f),
		TABLE_FLOAT_VALUE(table, "UI_MAP_RIGHT", 80.0f),
		TABLE_FLOAT_VALUE(table, "UI_MAP_BOTTOM", 40.0f)
	};
});

static const UIMap::_ASSET ASSET = LoadConfig<UIMap::_ASSET>("asset\\asset_list.csv", [](const D_KVTABLE& table) -> UIMap::_ASSET {
	return {
		TABLE_STR_VALUE(table, "UI_MAP_ICON", "asset\\texture\\map_icon.png"),
		TABLE_STR_VALUE(table, "UI_MAP_BAR", "asset\\texture\\map_bar.png")
	};
});


UIMap::UIMap() : GameObject()
{
	F2 screenSize = GetScreenSize();
	size = { CONFIG.UI_MAP_ICON_WIDTH, CONFIG.UI_MAP_BAR_HEIGHT + CONFIG.UI_MAP_ICON_HEIGHT, 0.0f };
	position = { screenSize.x * 0.5f - CONFIG.UI_MAP_RIGHT, screenSize.y * -0.5f + CONFIG.UI_MAP_BOTTOM , 0.0f };
	iconTexture = LoadTexture(ASSET.UI_MAP_ICON);
	barTexture = LoadTexture(ASSET.UI_MAP_BAR);
	currentT = 0.0f;
}

void UIMap::Update()
{
	if (player) {
		currentT = max(min(player->GetGravityT(), 1.0f), 0.0f);
	}
}

void UIMap::Draw()
{
	DrawQuad(barTexture, { position.x - CONFIG.UI_MAP_BAR_WIDTH * 0.5f, position.y + CONFIG.UI_MAP_BAR_HEIGHT * 0.5f } , { CONFIG.UI_MAP_BAR_WIDTH, CONFIG.UI_MAP_BAR_HEIGHT }, rotate, color);

	DrawQuad(iconTexture, { position.x, position.y + CONFIG.UI_MAP_BAR_HEIGHT * currentT }, { CONFIG.UI_MAP_ICON_WIDTH, CONFIG.UI_MAP_ICON_HEIGHT }, rotate, color);

}

void UIMap::SetPlayer(const Player* player)
{
	this->player = player;
}
