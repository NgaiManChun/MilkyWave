#include "uiPause.h"
#include "input.h"

static constexpr const char* TEXTURE_BACKGROUND = "asset\\texture\\white.png";

std::wstring UIPause::GetLabelText(PAUSE_OPTION option) const
{
	if (option == PAUSE_OPTION_RESUME) {
		return L"再開";
	}
	else if (option == PAUSE_OPTION_STAGE) {
		return L"ステージ選択に戻る";
	}
	else if (option == PAUSE_OPTION_TITLE) {
		return L"タイトルに戻る";
	}
	else if (option == PAUSE_OPTION_GYRO) {
		return std::wstring(L"ジャイロ：") + ((GetCommonBool("gyro")) ? L"On" : L"Off");
	}
	
	return std::wstring();
}

UIPause::UIPause(const std::string& scope) : GameObject()
{
	F2 screenSize = GetScreenSize();
	size = { screenSize.x, screenSize.y };
	textureBackground = LoadTexture(TEXTURE_BACKGROUND, scope);

	float lineHeight = 100.0f;
	float lineMargin = 20.0f;
	float totalHeight = PAUSE_OPTION_MAX * lineHeight;
	for (unsigned int i = 0; i < PAUSE_OPTION_MAX; i++) {
		labels[i] = GameObjectText(
			GetLabelText((PAUSE_OPTION)i),
			FONT{
				100.0f,
				"HG創英角ﾎﾟｯﾌﾟ体",
				FONT_WEIGHT_BOLD
			},
			TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,
			TEXT_ORIGIN_HORIZONTAL_CENTER,
			TEXT_ORIGIN_VERTICAL_CENTER,
			{ 300.0f, 100.0f }, { 0.0f, totalHeight * 0.5f - (lineHeight + lineMargin) * i }, Quaternion::Identity(),
			{ 0.0f, 0.0f, 0.0f, 1.0f }
		);
	}

	std::wstring hintsStr = GetInputLabel(L"↑") + GetInputLabel(L"↓") + L"選択\n";
	hintsStr += GetInputLabel(L"{OK}") + L"決定\n";
	hintsStr += GetInputLabel(L"{Start}") + L"再開";
	hintsLabel = GameObjectText(
		hintsStr,
		FONT{
			40.0f,
			"HG創英角ﾎﾟｯﾌﾟ体",
			FONT_WEIGHT_BOLD
		},
		TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,
		TEXT_ORIGIN_HORIZONTAL_LEFT,
		TEXT_ORIGIN_VERTICAL_BOTTOM,
		{ 300.0f, 40.0f * 3.0f },
		{ screenSize.x * -0.5f + 20.0f, screenSize.y * -0.5f + 20.0f, 0.0f }
	);
	hintsLabel.color = { 0.0f, 0.0f, 0.0f, 1.0f };

	SetOption(PAUSE_OPTION_RESUME);
	
}

UIPause::~UIPause()
{

}

void UIPause::Update()
{
	std::wstring hintsStr = GetInputLabel(L"↑") + GetInputLabel(L"↓") + L"選択\n";
	hintsStr += GetInputLabel(L"{OK}") + L"決定\n";
	hintsStr += GetInputLabel(L"{Start}") + L"再開";
	hintsLabel.SetValue(hintsStr);


	if (currentOption == PAUSE_OPTION_GYRO) {
		labels[PAUSE_OPTION_GYRO].SetValue(L"→" + GetLabelText(PAUSE_OPTION_GYRO));
		labels[PAUSE_OPTION_GYRO].color = { 1.0f, 0.7843f, 0.0431f, 1.0f };
	}
	else {
		labels[PAUSE_OPTION_GYRO].SetValue(L"　" + GetLabelText(PAUSE_OPTION_GYRO));
		labels[PAUSE_OPTION_GYRO].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	}
	
	if (!HasProcon() && !HasDualSense()) {
		labels[PAUSE_OPTION_GYRO].color += F4{ 0.5f, 0.5f, 0.5f, 0.0f };
	}
	for (unsigned int i = 0; i < PAUSE_OPTION_MAX; i++) {
		labels[i].Update();
	}
	hintsLabel.Update();
}

void UIPause::Draw()
{
	DrawQuad(textureBackground, position, size, rotate, { color.x, color.y, color.z, 0.5f });

	for (unsigned int i = 0; i < PAUSE_OPTION_MAX; i++) {
		labels[i].Draw();
	}
	hintsLabel.Draw();
}

void UIPause::SetOption(PAUSE_OPTION option)
{
	currentOption = option;
	
	for (unsigned int i = 0; i < PAUSE_OPTION_MAX; i++) {
		if (currentOption == i) {
			labels[i].SetValue(L"→" + GetLabelText((PAUSE_OPTION)i));
			labels[i].color = { 1.0f, 0.7843f, 0.0431f, 1.0f };
		}
		else {
			labels[i].SetValue(L"　" + GetLabelText((PAUSE_OPTION)i));
			labels[i].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		}
	}
	if (!HasProcon() && !HasDualSense()) {
		labels[PAUSE_OPTION_GYRO].color += F4{ 0.5f, 0.5f, 0.5f, 0.0f };
	}
}

UIPause::PAUSE_OPTION UIPause::GetOption() const
{
	return currentOption;
}

void UIPause::Up()
{
	int i = currentOption - 1;
	if (i < 0) {
		i = PAUSE_OPTION_MAX - 1;
	}
	SetOption((PAUSE_OPTION)i);
}

void UIPause::Down()
{
	int i = currentOption + 1;
	if (i >= PAUSE_OPTION_MAX) {
		i = 0;
	}
	SetOption((PAUSE_OPTION)i);
}
