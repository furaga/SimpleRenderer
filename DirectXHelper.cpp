#include "stdafx.h"
#include <stdio.h>
#include "DirectXHelper.h"
#include "ConstBuffer.h"
#include "Configuration.h"

// directx�̃R���e�L�X�g��o�b�t�@�ȂǁA�`��ɕK�v�ȃf�[�^
struct DirectXHelper::DirectXRenderInfo
{
	D3D_FEATURE_LEVEL FeatureLevelsSupported; // �f�o�C�X�쐬���ɕԂ����@�\���x��

	ID3D11Device*           pD3DDevice = NULL;
	ID3D11DeviceContext*    pImmediateContext = NULL;
	IDXGISwapChain*         pSwapChain = NULL;

	ID3D11RenderTargetView* pRenderTargetView = NULL;
	D3D11_VIEWPORT          ViewPort[1];

	ID3D11Texture2D*          pDepthStencil = NULL;
	ID3D11DepthStencilView*   pDepthStencilView = NULL;

	ID3D11InputLayout*        pInputLayout = NULL;
	ID3D11Buffer*             pVerBuffer[2];
	ID3D11Buffer*             pIdxBuffer = NULL;

	ID3D11VertexShader*       pVertexShader = NULL;
	ID3D11GeometryShader*     pGeometryShader = NULL;
	ID3D11PixelShader*        pPixelShader = NULL;
	ID3D11Buffer*             pCBuffer[3];

	ID3D11BlendState*         pBlendState = NULL;
	ID3D11RasterizerState*	  pRasterizerState = NULL;
	ID3D11DepthStencilState*  pDepthStencilState = NULL;

	// �[�x�o�b�t�@�̃��[�h
	bool DepthMode = true;

	// �X�^���o�C���[�h
	bool StandbyMode = false;

	XMFLOAT3 LightPos;

	cbNeverChanges       cbNeverChanges;       // �����ϊ��s��
	cbChangesEveryFrame  cbChangesEveryFrame;  // �r���[�ϊ��s��@�������W
	cbChangesEveryObject cbChangesEveryObject; // ���[���h�ϊ��s��

public:
	DirectXRenderInfo()
	{
	}
	void SetDeviceSwapChain(ID3D11Device* device, ID3D11DeviceContext* context, IDXGISwapChain* swapchain, D3D_FEATURE_LEVEL features)
	{
		FeatureLevelsSupported = features;
		pImmediateContext = context;
		pD3DDevice = device;
		pSwapChain = swapchain;
	}
	void SetConstBuffer(const ::cbNeverChanges& data)
	{
		cbNeverChanges = data;
	}
	void SetConstBuffer(const ::cbChangesEveryFrame& data)
	{
		cbChangesEveryFrame = data;
	}
	void SetConstBuffer(const ::cbChangesEveryObject& data)
	{
		cbChangesEveryObject = data;
	}
	::cbNeverChanges GetcbNeverChanges() const
	{
		return cbNeverChanges;
	}
	::cbChangesEveryFrame GetcbChangesEveryFrame() const
	{
		return cbChangesEveryFrame;
	}
	::cbChangesEveryObject GetcbChangesEveryObject() const
	{
		return cbChangesEveryObject;
	}
};

typedef std::unique_ptr<DirectXHelper::DirectXRenderInfo> PRenderInfo;


DirectXHelper::DirectXHelper()
{
}

DirectXHelper::~DirectXHelper()
{
}

HRESULT DirectXHelper::GenerateDeviceSwapChain(const DirectXConfiguration& dxconfig, const SIZE& bufferSize, const HWND& hWindow, PRenderInfo& pinfo)
{
	// �f�o�C�X�ƃX���b�v �`�F�C���̍쐬
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));	// �\���̂̒l��������
	sd.BufferCount = dxconfig.BufferCount;		// �o�b�N �o�b�t�@��
	sd.BufferDesc.Width = bufferSize.cx;	// �o�b�N �o�b�t�@�̕�
	sd.BufferDesc.Height = bufferSize.cy;	// �o�b�N �o�b�t�@�̍���
	sd.BufferDesc.Format = dxconfig.BufferFormat;  // �t�H�[�}�b�g
	sd.BufferDesc.RefreshRate.Numerator = dxconfig.RefreshRateNumerator;  // ���t���b�V�� ���[�g(���q)
	sd.BufferDesc.RefreshRate.Denominator = dxconfig.RefreshRateDenominator; // ���t���b�V�� ���[�g(����)
	sd.BufferDesc.ScanlineOrdering = dxconfig.BufferScanOrder;	// �X�L�������C��
	sd.BufferDesc.Scaling = dxconfig.BufferScanScaling;			// �X�P�[�����O
	sd.BufferUsage = dxconfig.BufferUsage; // �o�b�N �o�b�t�@�̎g�p�@
	sd.OutputWindow = hWindow;	// �֘A�t����E�C���h�E
	sd.SampleDesc.Count = dxconfig.SampleCount;		// �}���` �T���v���̐�
	sd.SampleDesc.Quality = dxconfig.SampleQuality;		// �}���` �T���v���̃N�I���e�B
	sd.Windowed = dxconfig.IsWindowMode;				// �E�C���h�E ���[�h
	sd.Flags = dxconfig.SwapChainFlg; // ���[�h�����؂�ւ�

#if defined(DEBUG) || defined(_DEBUG)
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#else
	UINT createDeviceFlags = 0;
#endif

	D3D_FEATURE_LEVEL FeatureLevelsSupported;
	ID3D11Device*           pD3DDevice = NULL;
	ID3D11DeviceContext*    pImmediateContext = NULL;
	IDXGISwapChain*         pSwapChain = NULL;

	// �n�[�h�E�F�A�E�f�o�C�X���쐬
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		// configs
		NULL, dxconfig.DriverType, NULL, createDeviceFlags,
		dxconfig.FeatureLevels, dxconfig.FeatureLevelCount, D3D11_SDK_VERSION, &sd,
		// outputs
		&pSwapChain,
		&pD3DDevice,
		&FeatureLevelsSupported,
		&pImmediateContext);

	if (FAILED(hr))
	{
		// WARP�f�o�C�X���쐬
		hr = D3D11CreateDeviceAndSwapChain(
			// configs
			NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags,
			dxconfig.FeatureLevels, dxconfig.FeatureLevelCount, D3D11_SDK_VERSION, &sd,
			// outputs
			&pSwapChain,
			&pD3DDevice,
			&FeatureLevelsSupported,
			&pImmediateContext);

		if (FAILED(hr))
		{
			// ���t�@�����X�E�f�o�C�X���쐬
			hr = D3D11CreateDeviceAndSwapChain(
				// configs
				NULL, D3D_DRIVER_TYPE_REFERENCE, NULL, createDeviceFlags,
				dxconfig.FeatureLevels, dxconfig.FeatureLevelCount, D3D11_SDK_VERSION, &sd,
				// outputs
				&pSwapChain,
				&pD3DDevice,
				&FeatureLevelsSupported,
				&pImmediateContext);

			if (FAILED(hr))
			{
				return DXTRACE_ERR(L"InitDirect3D D3D11CreateDeviceAndSwapChain", hr);
			}
		}
	}

	pinfo->SetDeviceSwapChain(pD3DDevice, pImmediateContext, pSwapChain, FeatureLevelsSupported);

	return S_OK;
}

PRenderInfo DirectXHelper::Initialize(HWND hWindow, SIZE wndSize, DirectXConfiguration dxconfig)
{
	PRenderInfo pinfo(new DirectXHelper::DirectXRenderInfo());
	HRESULT hr = GenerateDeviceSwapChain(dxconfig, wndSize, hWindow, pinfo);
	if (FAILED(hr))
	{
		DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateBuffer", hr);
		return NULL;
	}
	return pinfo;
}

HRESULT DirectXHelper::GenerateVertexBuffer(const int dataCount, const int structSize, const void* pData, ID3D11Device* pD3DDevice, ID3D11Buffer** ppBuffer)
{
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;      // �f�t�H���g�g�p�@
	bufferDesc.ByteWidth = dataCount * structSize;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // ���_�o�b�t�@
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = pData;  // �o�b�t�@�E�f�[�^�̏����l
	subData.SysMemPitch = 0;
	subData.SysMemSlicePitch = 0;

	HRESULT hr = pD3DDevice->CreateBuffer(&bufferDesc, &subData, ppBuffer);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateBuffer", hr);

	return S_OK;
}

HRESULT DirectXHelper::GenerateIndexBuffer(const int dataCount, const UINT* pData, ID3D11Device* pD3DDevice, ID3D11Buffer** ppBuffer)
{
	// �C���f�b�N�X�E�o�b�t�@�̒�`
	D3D11_BUFFER_DESC idxBufferDesc;
	idxBufferDesc.Usage = D3D11_USAGE_DEFAULT;     // �f�t�H���g�g�p�@
	idxBufferDesc.ByteWidth = sizeof(UINT) * dataCount;       // 12�~3���_
	idxBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // �C���f�b�N�X�E�o�b�t�@
	idxBufferDesc.CPUAccessFlags = 0;
	idxBufferDesc.MiscFlags = 0;
	idxBufferDesc.StructureByteStride = 0;

	// �C���f�b�N�X�E�o�b�t�@�̃T�u���\�[�X�̒�`
	D3D11_SUBRESOURCE_DATA idxSubData;
	idxSubData.pSysMem = pData;  // �o�b�t�@�E�f�[�^�̏����l
	idxSubData.SysMemPitch = 0;
	idxSubData.SysMemSlicePitch = 0;

	// �C���f�b�N�X�E�o�b�t�@�̍쐬
	HRESULT hr = pD3DDevice->CreateBuffer(&idxBufferDesc, &idxSubData, ppBuffer);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateBuffer", hr);

	return S_OK;
}

HRESULT DirectXHelper::LoadVertexShader(
	const std::wstring& filepath, const std::string& entryPoint, const DirectXConfiguration& dxconfig, ID3D11Device* pD3DDevice,
	ID3DBlob** ppBlobVS, ID3D11VertexShader** ppShader)
{
	ID3DBlob* pBlobVS = NULL;
	HRESULT hr = D3DX11CompileFromFile(
		filepath.c_str(), NULL, NULL,
		"VS", entryPoint.c_str(), dxconfig.FlagCompile, 0, NULL,
		&pBlobVS, NULL, NULL);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitDirect3D D3DX11CompileShaderFromFile", hr);

	*ppBlobVS = pBlobVS;

	// ���_�V�F�[�_�̍쐬
	hr = pD3DDevice->CreateVertexShader(
		pBlobVS->GetBufferPointer(), // �o�C�g�E�R�[�h�ւ̃|�C���^
		pBlobVS->GetBufferSize(),    // �o�C�g�E�R�[�h�̒���
		NULL,
		ppShader); // ���_�V�F�[�_���󂯎��ϐ�

	if (FAILED(hr)) {
		SAFE_RELEASE(pBlobVS);
		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateVertexShader", hr);
	}

	return S_OK;
}

HRESULT DirectXHelper::LoadPixelShader(
	const std::wstring& filepath, const std::string& entryPoint, const DirectXConfiguration& dxconfig,
	ID3D11Device* pD3DDevice, ID3D11PixelShader** ppShader)
{
	ID3DBlob* pBlobVS = NULL;
	HRESULT hr = D3DX11CompileFromFile(
		filepath.c_str(), NULL, NULL,
		"PS", entryPoint.c_str(), dxconfig.FlagCompile, 0, NULL,
		&pBlobVS, NULL, NULL);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitDirect3D D3DX11CompileShaderFromFile", hr);

	// ���_�V�F�[�_�̍쐬
	hr = pD3DDevice->CreatePixelShader(
		pBlobVS->GetBufferPointer(), // �o�C�g�E�R�[�h�ւ̃|�C���^
		pBlobVS->GetBufferSize(),    // �o�C�g�E�R�[�h�̒���
		NULL,
		ppShader); // ���_�V�F�[�_���󂯎��ϐ�
	if (FAILED(hr)) {
		SAFE_RELEASE(pBlobVS);
		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateVertexShader", hr);
	}

	return S_OK;
}

HRESULT DirectXHelper::LoadGeometryShader(
	const std::wstring& filepath, const std::string& entryPoint, const DirectXConfiguration& dxconfig,
	ID3D11Device* pD3DDevice, ID3D11GeometryShader** ppShader)
{
	ID3DBlob* pBlobVS = NULL;
	HRESULT hr = D3DX11CompileFromFile(
		filepath.c_str(), NULL, NULL,
		"GS", entryPoint.c_str(), dxconfig.FlagCompile, 0, NULL,
		&pBlobVS, NULL, NULL);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitDirect3D D3DX11CompileShaderFromFile", hr);

	// ���_�V�F�[�_�̍쐬
	hr = pD3DDevice->CreateGeometryShader(
		pBlobVS->GetBufferPointer(), // �o�C�g�E�R�[�h�ւ̃|�C���^
		pBlobVS->GetBufferSize(),    // �o�C�g�E�R�[�h�̒���
		NULL,
		ppShader); // ���_�V�F�[�_���󂯎��ϐ�
	if (FAILED(hr)) {
		SAFE_RELEASE(pBlobVS);
		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateVertexShader", hr);
	}

	return S_OK;
}

HRESULT DirectXHelper::GenerateConstantBuffers(const UINT bufferCount, const UINT* sizes, ID3D11Device* pD3DDevice, ID3D11Buffer** pBuffers)
{
	// �萔�o�b�t�@�̒�`
	D3D11_BUFFER_DESC cBufferDesc;
	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;    // ���I(�_�C�i�~�b�N)�g�p�@
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // �萔�o�b�t�@
	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;     // CPU���珑������
	cBufferDesc.MiscFlags = 0;
	cBufferDesc.StructureByteStride = 0;

	for (int i = 0; i < bufferCount; i++)
	{
		cBufferDesc.ByteWidth = sizes[i]; // �o�b�t�@�E�T�C�Y
		HRESULT hr = pD3DDevice->CreateBuffer(&cBufferDesc, NULL, &pBuffers[i]);
		if (FAILED(hr))
			return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateBuffer", hr);
	}
}

HRESULT DirectXHelper::SetBlendMode(ID3D11Device* pD3DDevice, ID3D11BlendState** ppBlendState)
{
	// �u�����h�E�X�e�[�g�E�I�u�W�F�N�g�̍쐬
	D3D11_BLEND_DESC BlendState;
	ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC));
	BlendState.AlphaToCoverageEnable = FALSE;
	BlendState.IndependentBlendEnable = FALSE;
	BlendState.RenderTarget[0].BlendEnable = FALSE;
	BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	HRESULT hr = pD3DDevice->CreateBlendState(&BlendState, ppBlendState);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateBlendState", hr);
	return S_OK;
}

HRESULT DirectXHelper::SetRasterMode(ID3D11Device* pD3DDevice, ID3D11RasterizerState** ppRasterizerState)
{
	// ���X�^���C�U�E�X�e�[�g�E�I�u�W�F�N�g�̍쐬
	D3D11_RASTERIZER_DESC RSDesc;
	RSDesc.FillMode = D3D11_FILL_SOLID;   // ���ʂɕ`�悷��
	RSDesc.CullMode = D3D11_CULL_NONE;    // ���ʂ�`�悷��
	RSDesc.FrontCounterClockwise = FALSE; // ���v��肪�\��
	RSDesc.DepthBias = 0;
	RSDesc.DepthBiasClamp = 0;
	RSDesc.SlopeScaledDepthBias = 0;
	RSDesc.DepthClipEnable = TRUE;
	RSDesc.ScissorEnable = FALSE;
	RSDesc.MultisampleEnable = FALSE;
	RSDesc.AntialiasedLineEnable = FALSE;
	HRESULT hr = pD3DDevice->CreateRasterizerState(&RSDesc, ppRasterizerState);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateRasterizerState", hr);
	return S_OK;
}

HRESULT DirectXHelper::SetDepthTest(ID3D11Device* pD3DDevice, ID3D11DepthStencilState** ppDepthStencilState)
{
	// �[�x/�X�e���V���E�X�e�[�g�E�I�u�W�F�N�g�̍쐬
	D3D11_DEPTH_STENCIL_DESC DepthStencil;
	DepthStencil.DepthEnable = TRUE; // �[�x�e�X�g����
	DepthStencil.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // ��������
	DepthStencil.DepthFunc = D3D11_COMPARISON_LESS; // ��O�̕��̂�`��
	DepthStencil.StencilEnable = FALSE; // �X�e���V���E�e�X�g�Ȃ�
	DepthStencil.StencilReadMask = 0;     // �X�e���V���ǂݍ��݃}�X�N�B
	DepthStencil.StencilWriteMask = 0;     // �X�e���V���������݃}�X�N�B
	// �ʂ��\�������Ă���ꍇ�̃X�e���V���E�e�X�g�̐ݒ�
	DepthStencil.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;  // �ێ�
	DepthStencil.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;  // �ێ�
	DepthStencil.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;  // �ێ�
	DepthStencil.FrontFace.StencilFunc = D3D11_COMPARISON_NEVER; // ��Ɏ��s
	// �ʂ����������Ă���ꍇ�̃X�e���V���E�e�X�g�̐ݒ�
	DepthStencil.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;   // �ێ�
	DepthStencil.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;   // �ێ�
	DepthStencil.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;   // �ێ�
	DepthStencil.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS; // ��ɐ���
	HRESULT hr = pD3DDevice->CreateDepthStencilState(&DepthStencil, ppDepthStencilState);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateDepthStencilState", hr);
	return S_OK;
}

bool DirectXHelper::Draw(std::unique_ptr<DirectXHelper::DirectXRenderInfo> pInfo)
{
	return true;
}

bool DirectXHelper::Dispose(std::unique_ptr<DirectXHelper::DirectXRenderInfo> pInfo)
{
	return true;
}
