#include "scene.h"
#include "rendererDX.h"
#include "resourceToolDX.h"
#include <mfapi.h>       // MFStartup
#include <mfidl.h>        // Media Foundation Interfaces
#include <mfreadwrite.h>  // Media Foundation Reader/Writer
#include <mftransform.h>
#include <mferror.h>
#include <evr.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;
using namespace MG;

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "strmiids.lib")

namespace Mp4Scene {
	// =======================================================
	// �N���X��`
	// =======================================================
	class Mp4Scene : public Scene {
	private:
		
		
	public:
		void Init() override;
		//void Uninit() override;
		void Update() override;
		void Draw() override;
	};


	// =======================================================
	// �V�[���o�^
	// =======================================================
	static SceneName sceneName = REGISTER_SCENE("mp4", Mp4Scene);

	static GUID mSubtype;
	static IMFTransform* decoder;
	ID3D11Texture2D* videoTexture;
	ID3D11ShaderResourceView* videoSRV;
	IMFSourceReader* sourceReader;
	RendererDX* renderer = ((RendererDX*)GetRenderer());

	void EnumerateMediaTypes(IMFSourceReader* sourceReader) {
		ComPtr<IMFMediaType> mediaType;
		for (DWORD i = 0; ; i++) {
			HRESULT hr = sourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, i, &mediaType);
			if (FAILED(hr)) break;

			GUID majorType, subtype;
			mediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
			mediaType->GetGUID(MF_MT_SUBTYPE, &subtype);

			if (majorType == MFMediaType_Video) {
				wprintf(L"Subtype: %s\n", subtype == MFVideoFormat_RGB32 ? L"RGB32" : L"Other");
				mSubtype = subtype;
			}
		}
	}

	void TransferToTexture(ID3D11DeviceContext* context, ID3D11Texture2D* texture, BYTE* frameData, UINT frameWidth, UINT frameHeight) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		context->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		for (UINT row = 0; row < frameHeight; ++row) {
			memcpy(reinterpret_cast<BYTE*>(mappedResource.pData) + row * mappedResource.RowPitch,
				frameData + row * frameWidth * 4, // RGBA �̏ꍇ
				frameWidth * 4);
		}

		context->Unmap(texture, 0);
	}

	void DecodeFrame(ComPtr<IMFSample> sample) {
		// �T���v�����f�R�[�_�[�ɑ��M
		HRESULT hr = decoder->ProcessInput(0, sample.Get(), 0);
		if (FAILED(hr)) return;

		// �f�R�[�h�ς݃f�[�^���擾
		MFT_OUTPUT_DATA_BUFFER outputDataBuffer = { 0 };
		DWORD status = 0;
		ComPtr<IMFSample> decodedSample;

		hr = MFCreateSample(&decodedSample);
		if (FAILED(hr)) return;

		ComPtr<IMFMediaBuffer> buffer;
		MFCreateMemoryBuffer(1920 * 1080 * 4, &buffer); // �K�؂ȃT�C�Y��ݒ�
		decodedSample->AddBuffer(buffer.Get());

		outputDataBuffer.pSample = decodedSample.Get();
		hr = decoder->ProcessOutput(0, 1, &outputDataBuffer, &status);
		if (FAILED(hr)) return;

		// �f�R�[�h�ς݃t���[���� `decodedSample` �Ɋi�[����Ă��܂��B

		hr = decodedSample->ConvertToContiguousBuffer(&buffer);
		if (FAILED(hr)) return;

		BYTE* data = nullptr;
		DWORD dataLength = 0;
		hr = buffer->Lock(&data, nullptr, &dataLength);
		if (FAILED(hr)) return;

		TransferToTexture(renderer->GetDeviceContext(), videoTexture, data, 1920, 1080);
		buffer->Unlock();
	}

	

	// =======================================================
	// ������
	// =======================================================
	void Mp4Scene::Init()
	{
		Scene::Init();
		HRESULT hr = MFStartup(MF_VERSION);
		if (FAILED(hr)) return;

		// ����\�[�X���[�_�[�̍쐬
		hr = MFCreateSourceReaderFromURL(L"asset\\test.mp4", nullptr, &sourceReader);
		if (FAILED(hr)) return;

		IMFMediaType* nativeType;
		hr = sourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &nativeType);
		if (FAILED(hr)) return;

		EnumerateMediaTypes(sourceReader);

		// �o�̓t�H�[�}�b�g��ݒ� (RGB32)
		IMFMediaType* videoType;
		MFCreateMediaType(&videoType);
		videoType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		videoType->SetGUID(MF_MT_SUBTYPE, mSubtype);
		MFSetAttributeSize(videoType, MF_MT_FRAME_SIZE, 1920, 1080);
		MFSetAttributeRatio(videoType, MF_MT_FRAME_RATE, 60, 1);
		hr = sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, videoType);
		if (FAILED(hr)) return;

		
		hr = CoCreateInstance(
			CLSID_MSH264DecoderMFT, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&decoder));
		if (FAILED(hr)) return;

		// ���̓��f�B�A�^�C�v�̐ݒ�
		hr = decoder->SetInputType(0, nativeType, 0);
		if (FAILED(hr)) return;

		// �o�̓��f�B�A�^�C�v�̐ݒ�
		IMFMediaType* outputType;
		MFCreateMediaType(&outputType);
		outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		outputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);

		hr = decoder->SetOutputType(0, outputType, 0);
		if (FAILED(hr)) return;

		// �e�N�X�`���ݒ�
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = 1920;
		textureDesc.Height = 1080;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DYNAMIC;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		renderer->GetDevice()->CreateTexture2D(&textureDesc, nullptr, &videoTexture);

		// �V�F�[�_�[���\�[�X�r���[���쐬
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		renderer->GetDevice()->CreateShaderResourceView(videoTexture, &srvDesc, &videoSRV);
	}


	// =======================================================
	// �I������
	// =======================================================
	/*void Mp4Scene::Uninit()
	{
		Scene::Uninit();
	}*/


	// =======================================================
	// �X�V
	// =======================================================
	void Mp4Scene::Update()
	{
		Scene::Update();
		ComPtr<IMFSample> sample;
		DWORD streamIndex, flags;
		LONGLONG timestamp;

		HRESULT hr = sourceReader->ReadSample(
			MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &flags, &timestamp, &sample);
		if (FAILED(hr) || !sample) return;

		DecodeFrame(sample.Get());

		//ComPtr<IMFMediaBuffer> buffer;
		//sample->ConvertToContiguousBuffer(&buffer);

		//BYTE* data = nullptr;
		//DWORD maxLength, currentLength;
		//buffer->Lock(&data, &maxLength, &currentLength);

		//// Direct3D �e�N�X�`���Ƀf�[�^��]��
		//D3D11_MAPPED_SUBRESOURCE mappedResource;
		//renderer->GetDeviceContext()->Map(videoTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		//memcpy(mappedResource.pData, data, currentLength);
		//renderer->GetDeviceContext()->Unmap(videoTexture, 0);

		//buffer->Unlock();
	}


	// =======================================================
	// �`��
	// =======================================================
	void Mp4Scene::Draw()
	{
		Scene::Draw();

		static TextureDX tex(videoSRV);
		DrawQuad(&tex, {}, {10, 10});
	}
}
