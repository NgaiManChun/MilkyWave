// =======================================================
// main.cpp
// 
// ��ҁF鰕��r�i�K�C�@�}���`�����j�@2024/08/06
// =======================================================
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG 
int AllocBreakPoint() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(12290);
	
	return 0;
}
static int _AllocBreakPoint = AllocBreakPoint();
#endif

#include "keyboard.h"
#include "CSVResource.h"
#include "MGCommon.h"
#include "rendererDX.h"
#include "resourceToolDX.h"
#include "drawToolDX.h"
#include "audioToolDX.h"
#include "scene.h"
#include "sceneTransitaion.h"
#include "input.h"
using namespace MG;

// =======================================================
// �}�N����`
// =======================================================
#define CLASS_NAME "Milky Wave"
#define WINDOW_CAPTION "Milky Wave"
#define INIT_SCENE "title"
#define SCREEN_WIDTH (1920.0f)
#define SCREEN_HEIGHT (1080.0f)
#define SCREEN_CENTER_X (SCREEN_WIDTH * 0.5f)
#define SCREEN_CENTER_Y (SCREEN_HEIGHT * 0.5f)
#define INIT_SCREEN_WIDTH (960.0f)
#define INIT_SCREEN_HEIGHT (540.0f)
#define ASSET_LIST_FILE "asset\\asset_list.csv"
constexpr const char* CONFIG_FILE = "asset\\crusade\\config.csv";





// =======================================================
// �v���g�^�C�v�錾
// =======================================================
LRESULT	CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT	Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow);
void Uninit(void);
void Update(void);
void Draw(void);
HRESULT InitDirectInput(HINSTANCE hInstance);
HRESULT InitGamepad(HWND hWnd);
BOOL CALLBACK EnumGamepadsCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
void UpdateGamepad();


// =======================================================
// �O���[�o���ϐ�
// =======================================================
static RendererDX* renderer;
static ResourceToolDX* resourceTool;
static DrawToolDX* drawTool;
static AudioToolDX* audioTool;
static int joyDevices;

// =======================================================
// ���C���֐�
// =======================================================
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance, LPSTR lpCmd, int nCmdShow)
{
	// �t���[�����[�g�v���p�ϐ�
	DWORD dwExecLastTime;
	DWORD dwFPSLastTime;
	DWORD dwCurrentTime;
	DWORD dwFrameCount;

	//COM�R���|�[�l���g�̏����i�@�\�𕔕i�����ĊO���̃v���O�������狤�L���p����d�g�݁j
	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	//�E�B���h�E�N���X�̓o�^�i�E�B���h�E�̎d�l�I�ȕ������߂�Windows�փZ�b�g����j
	WNDCLASS	wc;	//�\���̂�����
	ZeroMemory(&wc, sizeof(WNDCLASS));//���e���O�ŏ�����
	wc.lpfnWndProc = WndProc;	//�R�[���o�b�N�֐��̃|�C���^�[
	wc.lpszClassName = CLASS_NAME;	//���̎d�l���̖��O
	wc.hInstance = hInstance;	//���̃A�v���P�[�V�����̂���
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);//�J�[�\���̎��
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);//�E�B���h�E�̔w�i�F
	RegisterClass(&wc);	//�\���̂�Windows�փZ�b�g

	//�E�B���h�E�T�C�Y�̒���
	RECT rc = { 0, 0, INIT_SCREEN_WIDTH, INIT_SCREEN_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZE ^ WS_THICKFRAME, FALSE);

	//�E�B���h�E�̍쐬
	HWND	hWnd = CreateWindow(
		CLASS_NAME,	//��肽���E�B���h�E
		WINDOW_CAPTION,	//�E�B���h�E�ɕ\������^�C�g��
		WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZE ^ WS_THICKFRAME,	//�W���I�Ȍ`��̃E�B���h�E �T�C�Y�ύX�֎~
		CW_USEDEFAULT,		//�f�t�H���g�ݒ�ł��܂���
		CW_USEDEFAULT,
		rc.right - rc.left,//CW_USEDEFAULT,//�E�B���h�E�̕�
		rc.bottom - rc.top,//CW_USEDEFAULT,//�E�B���h�E�̍���
		NULL,
		NULL,
		hInstance,		//�A�v���P�[�V�����̃n���h��
		NULL
	);

	//����������
	if (FAILED(Init(hInstance, hWnd, true)))
	{
		return -1;//���������s
	}

	//�쐬�����E�B���h�E��\������
	ShowWindow(hWnd, nCmdShow);//�����ɏ]���ĕ\���A�܂��͔�\��
	//�E�B���h�E�̓��e�������\��
	UpdateWindow(hWnd);

	//���b�Z�[�W���[�v
	MSG	msg;
	ZeroMemory(&msg, sizeof(MSG));//���b�Z�[�W�\���̂��쐬���ď�����

	//joyDevices = JslConnectDevices();

	// �t���[�����[�g�v��������
	timeBeginPeriod(1); // �^�C�}�[�̕���\��ݒ�
	dwExecLastTime = dwFPSLastTime = timeGetTime();
	dwCurrentTime = dwFrameCount = 0;

	//�I�����b�Z�[�W������܂Ń��[�v����
	//�Q�[�����[�v
	while (1)
	{	//���b�Z�[�W�̗L�����`�F�b�N
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{ //Windows���烁�b�Z�[�W�������I
			if (msg.message == WM_QUIT)//���S�I�����܂������b�Z�[�W
			{
				break;	//while���[�v����ʂ���
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);	//WndProc���Ăяo�����
			}
	
		}
		dwCurrentTime = timeGetTime();
		if (dwCurrentTime - dwExecLastTime >= 1000.0f / 60.0f)
		{
			MG::SetDeltaTime(dwCurrentTime - dwExecLastTime);

			Update();	//�X�V����

			Draw();		//�`�揈��

			keycopy();
			dwExecLastTime = dwCurrentTime;
		}

	}//while

	//�I������
	Uninit();

	//�I������
	return (int)msg.wParam;

}

// =======================================================
// �E�B���h�E�v���V�[�W��
// ���b�Z�[�W���[�v���ŌĂяo�����
// =======================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATEAPP:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:

	case WM_SYSKEYUP:
	case WM_KEYDOWN:	//�L�[�������ꂽ
		if (wParam == VK_MENU) {
			return 0;  // �����ς݂ɂ���
		}
		else if (wParam == VK_ESCAPE)//�����ꂽ�̂�ESC�L�[
		{
			//�E�B���h�E����������N�G�X�g��Windows�ɑ���
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
		
		Keyboard_ProcessMessage(uMsg, wParam, lParam);
		break;
	case WM_CLOSE:	//�E�B���h�E����Ȃ�������				
		if (
			MessageBox(hWnd, "�{���ɏI�����Ă�낵���ł����H",
				"�m�F", MB_OKCANCEL | MB_DEFBUTTON2) == IDOK
			)
		{//OK�������ꂽ�Ƃ�
			DestroyWindow(hWnd);//�I������葱����Windows�փ��N�G�X�g
		}
		else
		{
			return 0;	//����ς�I���Ȃ�
		}

		break;
	case WM_DESTROY:	//�I������OK�ł���
		PostQuitMessage(0);		//�����̃��b�Z�[�W�ɂO�𑗂�
		break;

	}

	//�K�p�̖������b�Z�[�W�͓K���ɏ��������ďI��
	return DefWindowProc(hWnd, uMsg, wParam, lParam);

}

// =======================================================
// ������
// =======================================================
HRESULT	Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow)
{

	SetScreen(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_CENTER_X, SCREEN_CENTER_Y);

	renderer = new RendererDX(SCREEN_WIDTH, SCREEN_HEIGHT);
	renderer->Init(hInstance, hWnd, bWindow);
	resourceTool = new ResourceToolDX(renderer);
	drawTool = new DrawToolDX(renderer);
	audioTool = new AudioToolDX();

	SetRenderer(renderer);
	SetResourceTool(resourceTool);
	SetDrawTool(drawTool);
	SetAudioTool(audioTool);
	
	InitInput();

	// �o�b�N�O���E���h�ŉғ�
	StartScene("bgm");

	// �ŏ��̃V�[��
	StartScene(INIT_SCENE);

	return	S_OK;
}

// =======================================================
// �I������
// =======================================================
void Uninit(void)
{
	UninitScene();
	UnregisterAllTransition();
	resourceTool->ReleaseAllResource();
	renderer->Uninit();
	delete renderer;
	delete resourceTool;
	delete drawTool;
	delete audioTool;
}

// =======================================================
// �X�V����
// =======================================================
void Update(void)
{
	UpdateInput();
	//UpdateGamepad();
	UpdateScene();
}

// =======================================================
// �`�揈��
// =======================================================
void	Draw(void)
{
	renderer->SetRenderTarget(nullptr);
	renderer->ClearRenderTargetView();
	renderer->SetViewport(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_WIDTH, SCREEN_HEIGHT);


	DrawScene();

	renderer->Present();
}
