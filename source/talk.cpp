#include "talk.h"
#include "keyboard.h"
#include "input.h"
#include <string>
#include <sstream>
#include <iostream>

#define SCREEN_WIDTH  (1920.0f)
#define SCREEN_HEIGHT (1080.0f)


constexpr const char* TEXTURE_TALK_LEFT = "asset\\texture\\talkLeft.png";
constexpr const char* TEXTURE_TALK_RIGHT = "asset\\texture\\talkRight.png";

std::wstring GetReplacedLabel(std::wstring str)
{
	ReplaceAll(str, L"{AnalogLeft}", GetInputLabel(L"{AnalogLeft}"));
	ReplaceAll(str, L"{OK}", GetInputLabel(L"{OK}"));
	return str;
}

Talk::Talk(const std::string scenario) : 
	csvCount(4),
	retouchTexture(0),
	textTalkSize(45.0f),
	PositionTalk({ 0.0f,  -SCREEN_HEIGHT / 2 + 200.0f, 0.0f }),
	PositionPlayerLeft({ -SCREEN_WIDTH / 2 + 210.0f, -260.0f, 0.0f }),
	PositionPlayerRight({ SCREEN_WIDTH / 2 - 210.0f, -260.0f, 0.0f }),
	SizeTalk({ 1400.0f,  300.0f, 0.0f }),
	SizePlayerLeft({ 650.0f,  500.0f, 0.0f }),
	SizePlayerRight({ 650.0f,  500.0f, 0.0f }),
	ColorPlayerLeft({ 1.0f, 1.0f, 1.0f, 1.0f }),
	ColorPlayerRight({ 1.0f, 1.0f, 1.0f, 1.0f }),
	PlayerNamePos({ 1.0f, 1.0f, 1.0f }),
	talkFirstLinePos({ -500.0f,-383.0f,0.0f }),
	talkSecondLinePos({ -500.0f,-448.0f,0.0f }),
	textureTag(false),
	GameObject()
{

	D_KVTABLE keyValuePair;
	//const char* CSV_FILE = "asset\\talk.csv";
	ReadCSVFromPath(scenario, table);

	int rowNum = table.size(); // 行の数
	int colNum = table[0].size(); // 列の数

	//===============================================================

	csvTextureLeft = table[1][3]; // 0行1列の内容（文字列）
	csvTextureRight = table[1][4]; // 0行1列の内容（文字列）
	csvRetouchName = table[4][7];



	csv_Left_or_Right = table[4][1];
	Left_or_Right = std::stoi(csv_Left_or_Right);
	csvRetouchTexture = table[4][5];
	if (csvRetouchTexture == "1") {
		retouchTexture = std::stoi(csvRetouchTexture);
		if (csv_Left_or_Right == "1") {
			csvTextureLeft = table[csvCount][6];
			texturePlayerLeft = LoadTexture(csvTextureLeft);
		}
		else if (csv_Left_or_Right == "2") {
			csvTextureRight = table[csvCount][6];
			texturePlayerRight = LoadTexture(csvTextureRight);
		}
	}



	if (Left_or_Right == 1) {
		playerName = StringToWString(table[1][1]);
		ColorPlayerRight = { 0.4f,0.4f,0.4f,1.0f };
		textureTalkFrame = LoadTexture(TEXTURE_TALK_LEFT);
		PlayerNamePos = { -425.0f,-245.0f,0.0f };
	}
	else {
		playerName = StringToWString(table[1][2]);
		ColorPlayerLeft = { 0.4f,0.4f,0.4f,1.0f };
		textureTalkFrame = LoadTexture(TEXTURE_TALK_RIGHT);
		PlayerNamePos = { 425.0f,-245.0f,0.0f };
	}
	talkFirstLine = StringToWString(table[4][2]);
	talkSecondLine = StringToWString(table[4][3]);



	if (table[4][7] != "")
	{
		playerName = StringToWString(table[4][7]);
	}







	//===============================================================

	texturePlayerLeft = LoadTexture(csvTextureLeft);
	texturePlayerRight = LoadTexture(csvTextureRight);

	textTalkFirstLine = new GameObjectText(
		talkFirstLine,
		{
			textTalkSize,
			FONT_MEIRYO,
			FONT_WEIGHT_BOLD
		},
		TEXT_RESIZE_USE_RAW,
		TEXT_ORIGIN_HORIZONTAL_LEFT,
		TEXT_ORIGIN_VERTICAL_BOTTOM,
		//{ 100.0f, 50.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },//size
		talkFirstLinePos,//position
		Quaternion::Identity(),
		{ 1.0f, 1.0f, 1.0f, 1.0f }//color
	);
	textTalkSecondLine = new GameObjectText(
		talkSecondLine,
		{
			textTalkSize,
			FONT_MEIRYO,
			FONT_WEIGHT_BOLD
		},
		TEXT_RESIZE_USE_RAW,
		TEXT_ORIGIN_HORIZONTAL_LEFT,
		TEXT_ORIGIN_VERTICAL_BOTTOM,
		//{ 100.0f, 50.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },//size
		talkSecondLinePos,//position
		Quaternion::Identity(),
		{ 1.0f, 1.0f, 1.0f, 1.0f }//color
	);

	textPlayerName = new GameObjectText(
		playerName,
		{
			100.0f,
			FONT_MEIRYO,
			FONT_WEIGHT_BOLD
		},
		TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,
		TEXT_ORIGIN_HORIZONTAL_CENTER,
		TEXT_ORIGIN_VERTICAL_CENTER,
		{ 80.0f, 80.0f, 1.0f },
		PlayerNamePos,
		Quaternion::Identity(),
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	);




	csvTextTalkSize = table[csvCount][4];
	if (csvTextTalkSize == "大") {

		textTalkFirstLine->position.y;

		textTalkFirstLine->SetFont(
			{
				60.0f,
				FONT_MEIRYO,
				FONT_WEIGHT_BOLD
			}
		);

		textTalkSecondLine->position.y = talkSecondLinePos.y - 20.0f;

		textTalkSecondLine->SetFont(
			{
				60.0f,
				FONT_MEIRYO,
				FONT_WEIGHT_BOLD
			}
		);
	}
	else if (csvTextTalkSize == "小") {
		textTalkFirstLine->position.y = talkSecondLinePos.y + 70.0f;

		textTalkFirstLine->SetFont(
			{
				35.0f,
				FONT_MEIRYO,
				FONT_WEIGHT_BOLD
			}
		);

		textTalkSecondLine->position.y = talkSecondLinePos.y + 5.0f;

		textTalkSecondLine->SetFont(
			{
				35.0f,
				FONT_MEIRYO,
				FONT_WEIGHT_BOLD
			}
		);
	}
	else {
		textTalkFirstLine->position.y = talkFirstLinePos.y;

		textTalkFirstLine->SetFont(
			{
				45.0f,
				FONT_MEIRYO,
				FONT_WEIGHT_BOLD
			}
		);

		textTalkSecondLine->position.y = talkSecondLinePos.y;

		textTalkSecondLine->SetFont(
			{
				45.0f,
				FONT_MEIRYO,
				FONT_WEIGHT_BOLD
			}
		);
	}


	if (table[csvCount][3] == "") {
		if (csvTextTalkSize == "大") {

			textTalkFirstLine->position.y = (talkFirstLinePos.y + talkSecondLinePos.y) / 2 - 10.0f;
		}
		else if (csvTextTalkSize == "小") {
			textTalkFirstLine->position.y = (talkFirstLinePos.y + talkSecondLinePos.y) / 2 + 5.0f;
		}
		else {
			textTalkFirstLine->position.y = (talkFirstLinePos.y + talkSecondLinePos.y) / 2;
		}
		
		textTalkFirstLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][2])));
		textTalkSecondLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][3])));
	}
	else {
		textTalkFirstLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][2])));
		textTalkSecondLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][3])));
	}

	F2 screenSize = GetScreenSize();
	hints = GameObjectText(
		GetInputLabel(L"{OK}") + L"次へ　" + GetInputLabel(L"{Start}") + L"スキップ",
		{
			40.0f,
			"HG創英角ﾎﾟｯﾌﾟ体",
			FONT_WEIGHT_BOLD
		},
		TEXT_RESIZE_KEEP_RATIO_BY_HEIGHT,
		TEXT_ORIGIN_HORIZONTAL_RIGHT,
		TEXT_ORIGIN_VERTICAL_TOP,
		{ 80.0f, 40.0f, 1.0f },
		{ screenSize.x * 0.5f - 40.0f, screenSize.y * 0.5f - 40.0f },
		Quaternion::Identity(),
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	);
}

Talk::~Talk()
{
	delete textTalkFirstLine;
	delete textTalkSecondLine;
	delete textPlayerName;
	textTalkFirstLine = nullptr;
	textTalkSecondLine = nullptr;
	textPlayerName = nullptr;
}



void Talk::Update()
{

	textTalkFirstLine->Update();
	textTalkSecondLine->Update();

	textPlayerName->Update();
	hints.Update();
}

void Talk::Draw()
{
	DrawQuad(textureTalkFrame, PositionTalk, SizeTalk, rotate, { 1.0f,1.0f,1.0f,0.7f });
	DrawQuad(texturePlayerLeft, PositionPlayerLeft, SizePlayerLeft, rotate, ColorPlayerLeft);
	DrawQuad(texturePlayerRight, PositionPlayerRight, SizePlayerRight, rotate, ColorPlayerRight);

	textTalkFirstLine->Draw();
	textTalkSecondLine->Draw();

	textPlayerName->Draw();

	// 手紙とか画像表示させるときやつ
	if (textureTag == true) {
		DrawQuad(texture, { 0.0f, 0.0f, 0.0f }, { 800.0f, 800.0f,0.0f }, rotate, { 1.0f,1.0f,1.0f,1.0f });
	}

	hints.Draw();

}

void Talk::NextLine()
{
	csvCount++;

	if (table.size() > csvCount) {
		textureTag = false;

		csvTextTalkSize = table[csvCount][4];
		if (csvTextTalkSize == "大") {

			textTalkFirstLine->position.y;

			textTalkFirstLine->SetFont(
				{
					60.0f,
					FONT_MEIRYO,
					FONT_WEIGHT_BOLD
				}
			);

			textTalkSecondLine->position.y = talkSecondLinePos.y - 20.0f;

			textTalkSecondLine->SetFont(
				{
					60.0f,
					FONT_MEIRYO,
					FONT_WEIGHT_BOLD
				}
			);
		}
		else if (csvTextTalkSize == "小") {
			textTalkFirstLine->position.y = talkSecondLinePos.y + 70.0f;

			textTalkFirstLine->SetFont(
				{
					35.0f,
					FONT_MEIRYO,
					FONT_WEIGHT_BOLD
				}
			);

			textTalkSecondLine->position.y = talkSecondLinePos.y + 5.0f;

			textTalkSecondLine->SetFont(
				{
					35.0f,
					FONT_MEIRYO,
					FONT_WEIGHT_BOLD
				}
			);
		}
		else {
			textTalkFirstLine->position.y = talkFirstLinePos.y;

			textTalkFirstLine->SetFont(
				{
					45.0f,
					FONT_MEIRYO,
					FONT_WEIGHT_BOLD
				}
			);

			textTalkSecondLine->position.y = talkSecondLinePos.y;

			textTalkSecondLine->SetFont(
				{
					45.0f,
					FONT_MEIRYO,
					FONT_WEIGHT_BOLD
				}
			);
		}


		csvRetouchTexture = table[csvCount][5];
		csv_Left_or_Right = table[csvCount][1];
		Left_or_Right = std::stoi(csv_Left_or_Right);
		if (Left_or_Right == 1) {
			textPlayerName->SetValue(StringToWString(table[1][1]));
			if (table[csvCount][3] == "") {
				if (csvTextTalkSize == "大") {

					textTalkFirstLine->position.y = (talkFirstLinePos.y + talkSecondLinePos.y) / 2 - 10.0f;
				}
				else if (csvTextTalkSize == "小") {
					textTalkFirstLine->position.y = (talkFirstLinePos.y + talkSecondLinePos.y) / 2 + 5.0f;
				}
				else {
					textTalkFirstLine->position.y = (talkFirstLinePos.y + talkSecondLinePos.y) / 2;
				}
				textTalkFirstLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][2])));
				textTalkSecondLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][3])));
			}
			else {
				textTalkFirstLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][2])));
				textTalkSecondLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][3])));
			}
			textPlayerName->color = { 1.0f,1.0f,1.0f,1.0f };
			ColorPlayerRight = { 0.4f,0.4f,0.4f,1.0f };
			ColorPlayerLeft = { 1.0f,1.0f,1.0f,1.0f };

			textureTalkFrame = LoadTexture(TEXTURE_TALK_LEFT);

			textPlayerName->position = { -425.0f,-245.0f,0.0f };

			if (csvRetouchTexture == "") {
				csvTextureLeft = table[1][3];
				texturePlayerLeft = LoadTexture(csvTextureLeft);
			}
			else if (csvRetouchTexture == "1") {
				retouchTexture = std::stoi(csvRetouchTexture);
				csvTextureLeft = table[csvCount][6];
				texturePlayerLeft = LoadTexture(csvTextureLeft);
			}

		}
		else if (Left_or_Right == 2) {
			textPlayerName->SetValue(StringToWString(table[1][2]));
			if (table[csvCount][3] == "") {
				if (csvTextTalkSize == "大") {
					textTalkFirstLine->position.y = (talkFirstLinePos.y + talkSecondLinePos.y) / 2 - 10.0f;
				}
				else if (csvTextTalkSize == "小") {
					textTalkFirstLine->position.y = (talkFirstLinePos.y + talkSecondLinePos.y) / 2 + 5.0f;
				}
				else {
					textTalkFirstLine->position.y = (talkFirstLinePos.y + talkSecondLinePos.y) / 2;
				}
				textTalkFirstLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][2])));
				textTalkSecondLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][3])));
			}
			else {
				textTalkFirstLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][2])));
				textTalkSecondLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][3])));
			}

			textPlayerName->color = { 1.0f,1.0f,1.0f,1.0f };
			ColorPlayerRight = { 1.0f,1.0f,1.0f,1.0f };
			ColorPlayerLeft = { 0.4f,0.4f,0.4f,1.0f };
			textureTalkFrame = LoadTexture(TEXTURE_TALK_RIGHT);

			textPlayerName->position = { 425.0f,-245.0f,0.0f };


			if (csvRetouchTexture == "") {
				csvTextureRight = table[1][4];
				texturePlayerRight = LoadTexture(csvTextureRight);
			}
			else if (csvRetouchTexture == "1") {
				retouchTexture = std::stoi(csvRetouchTexture);
				csvTextureRight = table[csvCount][6];
				texturePlayerRight = LoadTexture(csvTextureRight);
			}
		}

		else if (table[csvCount][7] != "")
		{
			textPlayerName->SetValue(StringToWString(table[csvCount][7]));
		}
		else if (Left_or_Right == 3) {
			if (table[csvCount][3] == "") {
				if (csvTextTalkSize == "大") {
					textTalkFirstLine->position.y = (talkFirstLinePos.y + talkSecondLinePos.y) / 2 - 10.0f;
				}
				else if (csvTextTalkSize == "小") {
					textTalkFirstLine->position.y = (talkFirstLinePos.y + talkSecondLinePos.y) / 2 + 5.0f;
				}
				else {
					textTalkFirstLine->position.y = (talkFirstLinePos.y + talkSecondLinePos.y) / 2;
				}
				textTalkFirstLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][2])));
				textTalkSecondLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][3])));
			}
			else {
				textTalkFirstLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][2])));
				textTalkSecondLine->SetValue(GetReplacedLabel(StringToWString(table[csvCount][3])));
			}
			textPlayerName->color = { 0.4f,0.4f,0.4f,1.0f };
			ColorPlayerRight = { 0.4f,0.4f,0.4f,1.0f };
			ColorPlayerLeft = { 0.4f,0.4f,0.4f,1.0f };

			csvTexture = table[csvCount][6];
			texture = LoadTexture(csvTexture);
			textureTag = true;
		}
		if (table[csvCount][7] != "")
		{
			textPlayerName->SetValue(StringToWString(table[csvCount][7]));
		}

	}

	textTalkFirstLine->Update();
	textTalkSecondLine->Update();

	textPlayerName->Update();
}

void Talk::Skip()
{
	csvCount = table.size();
}

bool Talk::IsEnd() const
{
	return csvCount >= table.size();
}


