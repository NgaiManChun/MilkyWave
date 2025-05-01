#include "scene.h"
#include "CSVResource.h"
#include "progress.h"
using namespace MG;

namespace DemoScene {
	// =======================================================
	// �N���X��`
	// =======================================================
	class DemoScene : public Scene {
	public:
		void Init() override;
		//void Uninit() override;
		//void Update() override;
		//void Draw() override;
		//LAYER_TYPE GetLayerType(int layer) override;
	};


	// =======================================================
	// �V�[���o�^
	// �o�^����΁uSceneTransit(�V�[�����A�J�ږ�)�v�ŌĂяo����
	// ���͊֐��ł͂Ȃ���^���}�N��
	// �ustatic SceneName xxx = �v�̕����͕K�{
	// 
	// �����P	�V�[����
	// �����Q	�V�[���̃N���X��
	// =======================================================
	static SceneName sceneName = REGISTER_SCENE("empty", DemoScene);

	// =======================================================
	// ������
	// �V�[�����[�h���ꂽ���ɌĂяo�����
	// 
	// ����x�Ăяo���ꂽ�V�[���̃C���X�^���X��
	// �V�[�����I���������ۊǂ����̂ŁA
	// Init()�̓V�[���J�ڂ��邽�тɌĂяo�����ɑ΂���
	// �R���X�g���N�^�͏��񂾂��B
	// �p�r�ɂ���Ďg�������Ă��������B
	// =======================================================
	void DemoScene::Init()
	{
		Scene::Init();
		
		// =======================================================
		// GameObject�̎g�p���@
		// 
		// GameObject���̂͂����̊��N���X�Ȃ̂�
		// Update()��Draw()����
		// ���ۂ͌p�����Ďg���Ă�������
		// =======================================================
		if (false) {
			// GameObject���V�[���ɒǉ�����
			// �V�[����Update()��Draw()���鎞�A
			// GameObject��Update()��Draw()������ɉ񂳂��
			AddGameObject(GameObject());

			// �������͏��Ԃ��w�肵�ăV�[���ɒǉ�����
			// ���̏��Ԃ�Update()��Draw()�̏��ԂɂȂ�
			// ������Draw()����layer�����D�悳���
			InsertGameObject(GameObject(), 2);

			// ���C���[�w�肵�Ēǉ�
			// �f�t�H���g��0�`9�܂ł�3D����ȊO��2D
			// �ǂ̃��C���[��2D��3D���͎����Œ�`�ł���A
			// GetLayerType(int layer)���Q�Ƃ��Ă�������
			const int ANY_LAYER_NUMBER = 1;
			AddGameObject(GameObject(), ANY_LAYER_NUMBER);
			InsertGameObject(GameObject(), 2, ANY_LAYER_NUMBER);
			AddGameObject(GameObject(), LAYER_2D); // ��O��UI�Ɏg���Ƃ�
			AddGameObject(GameObject(), -1); // �t�ɉ���2D�I�u�W�F�N�g��\�肽����

			// �ǉ��������̂̃|�C���^�͕Ԃ����
			GameObject* ptr0 = AddGameObject(GameObject());
			GameObject* ptr1 = InsertGameObject(GameObject(), 3);

			ptr0->position = { 0.0f, 0.0f, 0.0f }; // x, y, z
			ptr0->size = { 1.0f, 1.0f, 1.0f }; // ���m�̓T�C�Y�̔{���A������x, y, z
			ptr0->rotate = Quaternion::Identity(); // ����͖���]

			// F2, F3, F4�̌v�Z ���ꕔ�������ĂȂ�
			ptr0->position += { 10.0f, -20.0f, 0.0f}; // ���Z������
			ptr0->size *= 2.0f; // �{�ɂ�����
			ptr0->size *= F3{ 1.0f, 2.0f, 3.0f }; // ��������w��
			
			// �N�H�[�^�j�I���̍쐬�֐�����
			Quaternion::AxisRadian({ 0.5f, 0.5f, 0.0f }, 0.1f);
			Quaternion::AxisXRadian(0.1f);
			Quaternion::AxisYRadian(0.1f);
			Quaternion::AxisZRadian(0.1f);
			Quaternion::AxisDegree({ 0.5f, 0.5f, 0.0f }, 1.0f);
			Quaternion::AxisXDegree(1.0f);
			Quaternion::AxisYDegree(1.0f);
			Quaternion::AxisZDegree(1.0f);

			// �|���Z�ɂ���΁A�u���̉�]��Ԃ��炳��ɉ�]����v���Ă����Ӗ��ɂȂ�
			ptr0->rotate *= Quaternion::AxisZRadian(0.5f);

			// GameObject��enable��false�ɂ����Update()��Draw()���̓X�L�b�v�����
			ptr0->enable = false;

			// GameObject���V�[������폜
			// �V�[���ƈ���đ����C���X�^���X�����̂Œ���
			// �Ȃ��AAddGameObject�Œǉ�����GameObject��
			// �V�[���I���i�ʂ̃V�[���Ɉڍs�����Ƃ��j����
			// ����ɏ��������̂ŁA
			// ��n���̈Ӗ��Œǈ�DeleteGameObject()����K�v�̓i�V
			DeleteGameObject(ptr0);
			ptr0 = nullptr;

		}


		// =======================================================
		// ���\�[�X
		// 
		// �����p�X�̃t�@�C���̃��\�[�X�͏d�����ă��[�h���Ȃ����A
		// ���\�[�X�v�[�����瑖�������Ԃ�������̂�
		// Update()��Draw()�̒��ɖ��t���[���Ăяo���͔̂����������g
		// 
		// ���̃V�[����Load�������\�[�X�́A
		// ���̃V�[�����I�����ɏ���ɍ폜�����
		// �������ʂ̃V�[���Ŏg�p���̃��\�[�X�͏���
		// =======================================================
		if (false) {

			// =======================================================
			// ���\�[�X���[�h
			// =======================================================
			const char* TEXTURE_FILE = "asset\crusade\white.png";
			const char* AUDIO_FILE = "asset\crusade\bgm.wav";
			const char* MODEL_FILE = "asset\crusade\model.mgo";
			Texture* texture = LoadTexture(TEXTURE_FILE);
			Audio* audio = LoadAudio(AUDIO_FILE);
			Model* model = LoadModel(MODEL_FILE);

			// �X�R�[�v���w�肵�ă��[�h
			// �V�[���̎�����n������O�����
			// �����d���āA�e�V�[�����ʂ��Ďg�����\�[�X��
			// ���̕��@�őO�����ă��[�h����Ƃ��Ɏg��
			LoadTexture(TEXTURE_FILE, "anyScope");
			LoadAudio(AUDIO_FILE, "anyScope");
			LoadModel(MODEL_FILE, "anyScope");

			// �X�R�[�v�w�肵�Ȃ��ꍇ�́A
			// ���̃V�[����sceneName���X�R�[�v�Ƃ��Ďg�p����̂�
			// �ȉ��̓�͓����Ӗ��ɂȂ�
			LoadTexture(TEXTURE_FILE);
			LoadTexture(TEXTURE_FILE, sceneName);


			// =======================================================
			// ���\�[�X���
			// 
			// ��n���̓V�[���I���������ł���̂�
			// �Ǝ��̃X�R�[�v�w�肵�Ă�ꍇ���A
			// �Ȃɂ��f�J�����\�[�X���V�[�����ɉ���������ꍇ�ȊO�͎g��Ȃ��Ă���
			// =======================================================
			ReleaseTexture(sceneName);	// ���̃V�[���̑S�e�N�X�`��
			ReleaseAudio(sceneName);	// ���̃V�[���̑S����
			ReleaseModel(sceneName);	// ���̃V�[���̑S���f��
			ReleaseResource(sceneName);	// ���̃V�[���̑S���\�[�X�i��ޖ�킸�j
			ReleaseTexture(TEXTURE_FILE, sceneName);	// ���̃V�[���̓���̃e�N�X�`��
			ReleaseTexture(TEXTURE_FILE, "anyScope");	// "anyScope"�Ƃ����X�R�[�v�̓���̃e�N�X�`��


			// =======================================================
			// �e���\�[�X�̎g�p��
			// =======================================================

			// ���GameObject
			AddGameObject(
				GameObjectQuad(
					LoadTexture(TEXTURE_FILE),
					{ 1.0f, 1.0f, 1.0f },		// size
					{ 0.0f, 0.0f, 0.0f }		// position
				)
			);

			// GameObject������ڂ�Draw
			// 
			// uv�̍ŏ��l = uvOffset
			// uv�̍ő�l = uvOffset + uvRange
			DrawQuad(
				LoadTexture(TEXTURE_FILE),
				{ 0.0f, 0.0f, 0.0f },		// position
				{ 1.0f, 1.0f, 1.0f },		// size, 2D�̏ꍇ�̓s�N�Z��
				Quaternion::Identity(),		// rotate
				{ 1.0f, 1.0f, 1.0f, 1.0f }, // color
				{ 0.0f, 0.0f },				// uvOffset
				{ 1.0f, 1.0f }				// uvRange
			);

			// �ŏ����̈���
			DrawQuad(
				LoadTexture(TEXTURE_FILE),
				{ 0.0f, 0.0f, 0.0f },		// position
				{ 1.0f, 1.0f, 1.0f }		// size
			);

			// Cube��Draw
			// 
			// �e�N�X�`����uv�l�͓V��A���́i�l�ʋ��ʁj�A��̏��ň��ɕ���
			// �Ǝ���uv�l���g�������ꍇ��DrawModel()���g���Ă�������
			DrawCube(
				LoadTexture(TEXTURE_FILE),
				{ 0.0f, 0.0f, 0.0f },		// position
				{ 1.0f, 1.0f, 1.0f },		// size
				Quaternion::Identity(),		// rotate
				{ 1.0f, 1.0f, 1.0f, 1.0f }  // color
			);


			// ���f����Draw
			DrawModel(
				LoadModel(MODEL_FILE),
				{ 0.0f, 0.0f, 0.0f },		// position
				{ 1.0f, 1.0f, 1.0f },		// size
				Quaternion::Identity(),		// rotate
				{ 1.0f, 1.0f, 1.0f, 1.0f }  // color
			);

			// Model��Mesh���Ƃ̃e�N�X�`���͎蓮�ŕς��邱�Ƃ��\
			// �������A���f���ƃe�N�X�`���̉���̃^�C�~���O���H���Ⴄ�ꍇ������̂Œ���
			model->meshTextures[model->rawModel->meshes] = LoadTexture(TEXTURE_FILE);

			// �����̓v���C���[����������K�v����̂�
			// GameObjectAudio���V�[���ɒǉ����邱�Ƃ����X�X��
			GameObjectAudio* audioObject = AddGameObject(GameObjectAudio(audio, true));
			audioObject->Play();
			audioObject->SetVolume(0.5f);
			audioObject->SetLoop(false);
			audioObject->Stop();
		}
		
		// =======================================================
		// �V�[���̈ڍs
		// 
		// �V�[���J�ڒ��A��ȏ�̃V�[���������ɉғ����Ă邱�Ƃ�����
		// �V�[�����X�^�[�g�E�G���h�����^�C�~���O��
		// �ǂ̃V�[����Draw����邩��
		// �J�ڃv���O�����Ɉˑ����Ă�̂Œ���
		// =======================================================
		if (false) {

			// �����P�@�ڍs��̃V�[����
			// �����Q�@�J�ږ��i�G�t�F�N�g�Ȃǁj
			SceneTransit("test", "sample_transition");

			// �J�ږ��L�ڂ��Ȃ��ꍇ�͂ς��Ɛ؂�ւ���
			SceneTransit("test");

			// ���݂̓V�[���J�ڂ̓r�����ǂ������`�F�b�N
			// �u�V�[���J�ڂ���������܂ł͓����Ȃ��v�Ƃ��Ɏg��
			bool inTransition = InTransition();
		}


		// =======================================================
		// �t�@�C���Ԃ̃f�[�^����
		// 
		// �������O������Εʂ̃t�@�C���ł��Z�b�g�����l���擾���邱�Ƃ��ł���
		// �V�[����������Ȃ��Ċ�{�ǂ��ł��g����̂�
		// �ꍇ�ɂ���ăV�[���J�ڂ�I�u�W�F�N�g�̐ݒ�Ɏg�p���邱�Ƃ��\
		// =======================================================
		if (false) {
			SetCommonBool("any_bool", true);
			bool anyBool = GetCommonBool("any_bool");

			SetCommonFloat("any_float", 0.5f);
			bool anyFloat = GetCommonFloat("any_float");

			SetCommonInt("any_int", 30);
			int anyInt = GetCommonInt("any_int");

			SetCommonString("any_string", "�C�ӂ̕�����");
			std::string anyString = GetCommonString("any_string");

			// �C�ӂ̃|�C���^����邱�Ƃ��ł��邪������Ɗ댯
			GameObject* ptr0 = AddGameObject(GameObject());
			SetCommonPointer("any_pointer", ptr0);
			GameObject* anyPointer = (GameObject*)GetCommonPointer("any_pointer");
		}


		// =======================================================
		// CSV�t�@�C���̓ǂݍ���
		// 
		// #include "CSVResource.h"��������K�v����
		// =======================================================
		if (false) {
			D_TABLE table;
			D_KVTABLE keyValuePair;
			const char* CSV_FILE = "asset\\config.csv";
			ReadCSVFromPath(CSV_FILE, table);

			int rowNum = table.size(); // �s�̐�
			int colNum = table[0].size(); // ��̐�
			std::string row0Col1 = table[0][1]; // 0�s1��̓��e�i������j

			// Table�f�[�^����Key-Value-Pair�֕ϊ�
			// �����P�@�L�[�̕�����
			// 
			// ��s�ڂ��̖��O�Ɍ����Ă�
			// ����ɃL�[�̕����񂪍ڂ��Ă����L�[��Ƃ���
			// keyValuePair[�L�[][��]�Ƃ����\���ɕϊ�����
			TableToKeyValuePair("key", table, keyValuePair);

			// �Ⴆ�Έȉ���csv�f�[�^������Ƃ���
			// key,				value
			// TEXTURE_WHITE,	asset\crusade\white.png
			// TEXTURE_RED,		asset\crusade\red.png
			// TEXTURE_GREEN,	asset\crusade\green.png
			// ENEMY_NUM,		100
			// PLAYER_SPEED,	50.0

			std::string textureWhitePath = keyValuePair["TEXTURE_WHITE"]["value"]; //�@asset\crusade\white.png
			std::string textureRedPath = keyValuePair["TEXTURE_RED"]["value"]; //�@asset\crusade\red.png
			std::string textureGreenPath = keyValuePair["TEXTURE_GREEN"]["value"]; //�@asset\crusade\green.png
			int enemyNum = std::stof(keyValuePair["ENEMY_NUM"]["value"]); // 100
			float playerSpeed = std::stof(keyValuePair["PLAYER_SPEED"]["value"]); // 50.0f
		}
		
		// =======================================================
		// �v���O���X
		// 
		// ���w�Ō���t�̑���ɂȂ�֗��N���X
		// #include "progress.h"��������K�v����
		// =======================================================
		if (false) {
			// �����P�@�ő��Βl
			// �����Q�@���[�v���邩
			Progress t(1000.0f, false);

			int deltaTime = GetDeltaTime(); // �O�t���[������̌o�ߎ���
			t.IncreaseValue(deltaTime);		// t�ɐ�Βl��ݐς���

			// ���̗Ⴞ�ƁA�ő��Βl��1000.0f��
			// ����GetDeltaTime()�Ŏ擾�����l��17�Ƃ��āi1000�~�j�b��60�t���[���j
			// ���݂�t�́u17.0f / 1000.0f�v�����ɂȂ�
			// �܂�0.017
			// �l��0.0f�`1.0f�͈̔͂���͂ݏo�����Ƃ͂Ȃ��i���؂�A���ł��j
			F3 startPosition = { 10.0f, 20.0f, 30.0f }; // �o���_
			F3 endPosition = { 30.0f, 0.0f, 10.0f };	// �ړI�n

			// ���ݒn
			F3 currentPosition = startPosition * (1.0f - t) + endPosition * t;

			// ���ɂ��F�̕ϐF��t�F�C�h�C���A�E�g��
			// �N�[���^�C���̔���ȂǂɎg����
			Progress coolTime(5000.0f, false);
			if (coolTime == 1.0f) {
				coolTime.SetValue(0.0f); // ��Βl��0.0f�ɖ߂�
			}
			coolTime.IncreaseValue(GetDeltaTime());
			coolTime.IncreaseValue(GetDeltaTime() * 2.0f); // �{���ɂ�����

			// �Ȃɂ��̃J�E���^�[�Ɏg���Ƃ�
			Progress counter(10.0f, false);
			if (counter == 1.0f) {
				// ����
			}
			counter.IncreaseValue(1);

			// ���[�v�ݒ�
			// ��Βl�����ł��ɂȂ����玩����0.0f�ɖ߂�
			Progress alpha(500.0f, true); // ������true�ŃI��
			F4 color = { 1.0f, 0.0f, 0.0f, alpha }; // 500�~�j�b�̎����œ_�ł���
			alpha.IncreaseValue(GetDeltaTime());
		}


		
	}


	// =======================================================
	// �I������
	// �V�[���G���h�̎��ɌĂяo�����
	// 
	// ��Init()�Ɠ��l�AUninit()�V�[���J�ڂ��邽�тɌĂяo����邪
	// �C���X�^���X���͕̂ۊǂ����̂ŁA
	// �f�X�g���N�^�͂��̎��_�ł͎��s���Ȃ��B
	// ReleaseScene()�ŋ����I�ɃC���X�^���X�������Ƃ��\�����A
	// ���ɗ��R�͂Ȃ����肻������K�v�Ȃ����A���X�X�����Ȃ��B
	// =======================================================
	/*void DemoScene::Uninit()
	{
		Scene::Uninit();
	}*/


	// =======================================================
	// �X�V
	// ���N���X��Scene::Update()���Ăяo����
	// AddGameObject()�Œǉ������SGameObjecct��Update()�����s����
	// ������enable�����o��fasle�ɂȂ��Ă�GameObjecct�͏���
	// =======================================================
	/*void DemoScene::Update()
	{
		Scene::Update();
	}*/


	// =======================================================
	// �`��
	// Update()�Ɠ�����
	// =======================================================
	/*void DemoScene::Draw()
	{
		Scene::Draw();
	}*/


	// =======================================================
	// ���C���[�^�C�v�̒�`
	// �`�悷�鎞�AGameObject�ɐݒ肵��layer�ɉ�����
	// 2D�̃J������3D�̃J�����ǂ�����K�p����̂��𕪂���B
	// ���̊֐����p�����āu�R�ԂȂ�2D�ŁA�S�Ԃ�3D�v�݂����ɁA
	// �J�X�^�}�C�Y�ł���B
	// �f�t�H���g��0�`�X�܂ł�3D����ȊO��2D
	// 
	// ��layer��2D��3D�𕪂���ȊO�ɕ`�揇�ł�����A
	// ���邢�́u�����layer�̃I�u�W�F�N�g�����`�v
	// ���Ă����g����������ł���
	// =======================================================
	/*LAYER_TYPE DemoScene::GetLayerType(int layer)
	{
		return Scene::GetLayerType(layer);
	}*/
}
