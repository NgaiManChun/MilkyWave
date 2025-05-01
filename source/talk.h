#ifndef _TALK_H
#define _TALK_H
#include "gameObject.h"
#include "progress.h"
#include "gameObjectText.h"
#include "CSVResource.h"

#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iostream>

using namespace MG;


class Talk : public GameObject
{
private:

	GameObjectText* textTalkFirstLine;  // 会話の１行目の文字
	GameObjectText* textTalkSecondLine; // 会話の２行目の文字
	GameObjectText* textPlayerName;     // キャラクター名
	GameObjectText hints;
	
	int csvCount; // csvの何行何列目を進めるためのカウント

	Texture* textureTalkFrame;   // 会話文の枠
	Texture* texturePlayerLeft;  // 左のキャラクターの画像
	Texture* texturePlayerRight; // 右のキャラクターの画像

	Texture* texture; // なんか表示させたいとき画像
	std::string csvTexture;
	bool textureTag;

	/* キャラクターと会話文の枠の情報 */
	F3 PositionTalk;		// 会話文の枠の位置
	F3 SizeTalk;			// 会話文の枠のサイズ
	F3 PositionPlayerLeft;  // 左のキャラクターの画像の位置
	F3 PositionPlayerRight; // 右のキャラクターの画像の位置
	F3 SizePlayerLeft;      // 左のキャラクターの画像のサイズ
	F3 SizePlayerRight;		// 右のキャラクターの画像のサイズ
	F4 ColorPlayerLeft;		// 左のキャラクターの画像の色
	F4 ColorPlayerRight;	// 右のキャラクターの画像の色
	F3 PlayerNamePos;		// プレイヤーの名前の位置
	F3 talkFirstLinePos;
	F3 talkSecondLinePos;

	/* csvから読み取るやつ */
	std::string csvTextureTalk;  // 会話文の枠の画像
	std::string csvTextureLeft;  // 左のキャラクター画像
	std::string csvTextureRight; // 右のキャラクター画像
	std::string csv_Left_or_Right; // 左か右どちらのキャラクターが話しているかの判定
	int Left_or_Right;
	std::string csvRetouchTexture; // キャラクター画像を変えるためのやつ
	int retouchTexture;
	std::string csvTextTalkSize; // 文字サイズ変更
	float textTalkSize;
	std::string csvRetouchName; // 名前変更
	float RetouchName;

	/* 文字表示に使う */
	std::wstring playerName;
	std::wstring talkFirstLine;
	std::wstring talkSecondLine;

	D_TABLE table;

public:
	Talk(const std::string scenario);
	~Talk();

	void Update() override;

	void Draw()override;

	void NextLine();
	void Skip();
	bool IsEnd() const;
};





#endif