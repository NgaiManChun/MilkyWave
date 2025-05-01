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

	GameObjectText* textTalkFirstLine;  // ��b�̂P�s�ڂ̕���
	GameObjectText* textTalkSecondLine; // ��b�̂Q�s�ڂ̕���
	GameObjectText* textPlayerName;     // �L�����N�^�[��
	GameObjectText hints;
	
	int csvCount; // csv�̉��s����ڂ�i�߂邽�߂̃J�E���g

	Texture* textureTalkFrame;   // ��b���̘g
	Texture* texturePlayerLeft;  // ���̃L�����N�^�[�̉摜
	Texture* texturePlayerRight; // �E�̃L�����N�^�[�̉摜

	Texture* texture; // �Ȃ񂩕\�����������Ƃ��摜
	std::string csvTexture;
	bool textureTag;

	/* �L�����N�^�[�Ɖ�b���̘g�̏�� */
	F3 PositionTalk;		// ��b���̘g�̈ʒu
	F3 SizeTalk;			// ��b���̘g�̃T�C�Y
	F3 PositionPlayerLeft;  // ���̃L�����N�^�[�̉摜�̈ʒu
	F3 PositionPlayerRight; // �E�̃L�����N�^�[�̉摜�̈ʒu
	F3 SizePlayerLeft;      // ���̃L�����N�^�[�̉摜�̃T�C�Y
	F3 SizePlayerRight;		// �E�̃L�����N�^�[�̉摜�̃T�C�Y
	F4 ColorPlayerLeft;		// ���̃L�����N�^�[�̉摜�̐F
	F4 ColorPlayerRight;	// �E�̃L�����N�^�[�̉摜�̐F
	F3 PlayerNamePos;		// �v���C���[�̖��O�̈ʒu
	F3 talkFirstLinePos;
	F3 talkSecondLinePos;

	/* csv����ǂݎ���� */
	std::string csvTextureTalk;  // ��b���̘g�̉摜
	std::string csvTextureLeft;  // ���̃L�����N�^�[�摜
	std::string csvTextureRight; // �E�̃L�����N�^�[�摜
	std::string csv_Left_or_Right; // �����E�ǂ���̃L�����N�^�[���b���Ă��邩�̔���
	int Left_or_Right;
	std::string csvRetouchTexture; // �L�����N�^�[�摜��ς��邽�߂̂��
	int retouchTexture;
	std::string csvTextTalkSize; // �����T�C�Y�ύX
	float textTalkSize;
	std::string csvRetouchName; // ���O�ύX
	float RetouchName;

	/* �����\���Ɏg�� */
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