#include "stdafx.h"
#include "VertexPositionColorTexture.h"
#include "DirectXRenderInfo.h"
#include "Configuration.h"

// ���_�E�C���f�b�N�X�o�b�t�@�̏�����
struct VertexPositionColorTexture posVertex[] = {
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
};
UINT idxVertexID[] = {
	0, 1, 3, 1, 2, 3, 1, 5, 2, 5, 6, 2, 5, 4, 6, 4, 7, 6,
	4, 5, 0, 5, 1, 0, 4, 0, 7, 0, 3, 7, 3, 2, 7, 2, 6, 7
};
// ���͗v�f
D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(XMFLOAT3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(XMFLOAT3) + sizeof(XMFLOAT3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

/*-------------------------------------------
�O���[�o���ϐ�(�A�v���P�[�V�����֘A)
--------------------------------------------*/
HINSTANCE	g_hInstance = NULL;	// �C���X�^���X �n���h��
HWND		g_hWindow = NULL;	// �E�C���h�E �n���h��

WCHAR		g_szAppTitle[] = L"Direct3D 11 Sample05";
WCHAR		g_szWndClass[] = L"D3D11S05";

// �N�����̕`��̈�T�C�Y
SIZE		g_sizeWindow = { 640, 480 };		// �E�C���h�E�̃N���C�A���g�̈�

/*-------------------------------------------
�O���[�o���ϐ�(Direct3D�֘A)
--------------------------------------------*/

//�@�\���x���̔z��
D3D_FEATURE_LEVEL g_pFeatureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
UINT              g_FeatureLevels = 3;   // �z��̗v�f��
D3D_FEATURE_LEVEL g_FeatureLevelsSupported; // �f�o�C�X�쐬���ɕԂ����@�\���x��

// �C���^�[�t�F�C�X
ID3D11Device*           g_pD3DDevice = NULL;        // �f�o�C�X
ID3D11DeviceContext*    g_pImmediateContext = NULL; // �f�o�C�X�E�R���e�L�X�g
IDXGISwapChain*         g_pSwapChain = NULL;        // �X���b�v�E�`�F�C��

ID3D11RenderTargetView* g_pRenderTargetView = NULL; // �`��^�[�Q�b�g�E�r���[
D3D11_VIEWPORT          g_ViewPort[1];				// �r���[�|�[�g

ID3D11Texture2D*          g_pDepthStencil = NULL;		// �[�x/�X�e���V��
ID3D11DepthStencilView*   g_pDepthStencilView = NULL;	// �[�x/�X�e���V���E�r���[

ID3D11InputLayout*        g_pInputLayout = NULL;            // ���̓��C�A�E�g�E�I�u�W�F�N�g
ID3D11Buffer*             g_pVerBuffer[2] = { NULL, NULL }; // ���_�o�b�t�@�̃C���^�[�t�F�C�X
ID3D11Buffer*             g_pIdxBuffer = NULL;              // �C���f�b�N�X�E�o�b�t�@�̃C���^�[�t�F�C�X

ID3D11VertexShader*       g_pVertexShader = NULL;   // ���_�V�F�[�_
ID3D11GeometryShader*     g_pGeometryShader = NULL; // �W�I���g���E�V�F�[�_
ID3D11PixelShader*        g_pPixelShader = NULL;    // �s�N�Z���E�V�F�[�_
ID3D11Buffer*             g_pCBuffer[3] = { NULL, NULL, NULL }; // �萔�o�b�t�@

ID3D11BlendState*         g_pBlendState = NULL;			// �u�����h�E�X�e�[�g�E�I�u�W�F�N�g
ID3D11RasterizerState*	  g_pRasterizerState = NULL;	// ���X�^���C�U�E�X�e�[�g�E�I�u�W�F�N�g
ID3D11DepthStencilState*  g_pDepthStencilState = NULL;	// �[�x/�X�e���V���E�X�e�[�g�E�I�u�W�F�N�g

// �V�F�[�_�̃R���p�C�� �I�v�V����
#if defined(DEBUG) || defined(_DEBUG)
UINT g_flagCompile = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION
| D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR;
#else
UINT g_flagCompile = D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR;
#endif

// �[�x�o�b�t�@�̃��[�h
bool g_bDepthMode = true;

// �X�^���o�C���[�h
bool g_bStandbyMode = false;

// �`��^�[�Q�b�g���N���A����l(RGBA)
float g_ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };

XMFLOAT3 g_vLightPos(3.0f, 3.0f, -3.0f);   // �������W(���[���h���W�n)

/*-------------------------------------------
�֐���`
--------------------------------------------*/

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam);
HRESULT InitBackBuffer(void);

/*-------------------------------------------
�A�v���P�[�V����������
--------------------------------------------*/

HRESULT GenerateWindow(const HINSTANCE hInst, const WNDPROC OnWidowEvent, const std::wstring wndTitle, const std::wstring wndName, const SIZE wndSize, HWND* pWnd)
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = OnWidowEvent;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = wndName.c_str();

	if (!RegisterClass(&wc))
	{
		return DXTRACE_ERR(L"GenerateWindow", GetLastError());
	}

	RECT rect;
	rect.top = 0;
	rect.left = 0;
	rect.right = wndSize.cx;
	rect.bottom = wndSize.cy;
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, TRUE);

	*pWnd = CreateWindow(
		wndName.c_str(), wndTitle.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInst, NULL);
	if (*pWnd == NULL)
		return DXTRACE_ERR(L"GenerateWindow", GetLastError());

	return S_OK;
}

HRESULT InitializeWindow(HINSTANCE hInst)
{
	g_hInstance = hInst;
	HRESULT hr = GenerateWindow(hInst, MainWndProc, WindowConfiguration::WindowTitle, WindowConfiguration::WindowName, WindowConfiguration::WindowSize, &g_hWindow);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitializeWindow", hr);
	ShowWindow(g_hWindow, SW_SHOWNORMAL);
	UpdateWindow(g_hWindow);
	return S_OK;
}
//
//HRESULT GenerateDeviceSwapChain(const DirectXConfiguration& dxconfig, const SIZE& bufferSize, const HWND& hWindow, DirectXRenderInfo& dxinfo)
//{
//	// �f�o�C�X�ƃX���b�v �`�F�C���̍쐬
//	DXGI_SWAP_CHAIN_DESC sd;
//	ZeroMemory(&sd, sizeof(sd));	// �\���̂̒l��������
//	sd.BufferCount = dxconfig.BufferCount;		// �o�b�N �o�b�t�@��
//	sd.BufferDesc.Width = bufferSize.cx;	// �o�b�N �o�b�t�@�̕�
//	sd.BufferDesc.Height = bufferSize.cy;	// �o�b�N �o�b�t�@�̍���
//	sd.BufferDesc.Format = dxconfig.BufferFormat;  // �t�H�[�}�b�g
//	sd.BufferDesc.RefreshRate.Numerator = dxconfig.RefreshRateNumerator;  // ���t���b�V�� ���[�g(���q)
//	sd.BufferDesc.RefreshRate.Denominator = dxconfig.RefreshRateDenominator; // ���t���b�V�� ���[�g(����)
//	sd.BufferDesc.ScanlineOrdering = dxconfig.BufferScanOrder;	// �X�L�������C��
//	sd.BufferDesc.Scaling = dxconfig.BufferScanScaling;			// �X�P�[�����O
//	sd.BufferUsage = dxconfig.BufferUsage; // �o�b�N �o�b�t�@�̎g�p�@
//	sd.OutputWindow = hWindow;	// �֘A�t����E�C���h�E
//	sd.SampleDesc.Count = dxconfig.SampleCount;		// �}���` �T���v���̐�
//	sd.SampleDesc.Quality = dxconfig.SampleQuality;		// �}���` �T���v���̃N�I���e�B
//	sd.Windowed = dxconfig.IsWindowMode;				// �E�C���h�E ���[�h
//	sd.Flags = dxconfig.SwapChainFlg; // ���[�h�����؂�ւ�
//
//#if defined(DEBUG) || defined(_DEBUG)
//	UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
//#else
//	UINT createDeviceFlags = 0;
//#endif
//
//
//	D3D_FEATURE_LEVEL FeatureLevelsSupported;
//	ID3D11Device*           pD3DDevice = NULL;
//	ID3D11DeviceContext*    pImmediateContext = NULL;
//	IDXGISwapChain*         pSwapChain = NULL;
//
//	// �n�[�h�E�F�A�E�f�o�C�X���쐬
//	HRESULT hr = D3D11CreateDeviceAndSwapChain(
//		// configs
//		NULL, dxconfig.DriverType, NULL, createDeviceFlags,
//		dxconfig.FeatureLevels, dxconfig.FeatureLevelCount, D3D11_SDK_VERSION, &sd,
//		// outputs
//		&pSwapChain,
//		&pD3DDevice,
//		&FeatureLevelsSupported,
//		&pImmediateContext);
//
//	if (FAILED(hr))
//	{
//		// WARP�f�o�C�X���쐬
//		hr = D3D11CreateDeviceAndSwapChain(
//			// configs
//			NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags,
//			dxconfig.FeatureLevels, dxconfig.FeatureLevelCount, D3D11_SDK_VERSION, &sd,
//			// outputs
//			&pSwapChain,
//			&pD3DDevice,
//			&FeatureLevelsSupported,
//			&pImmediateContext);
//
//		if (FAILED(hr))
//		{
//			// ���t�@�����X�E�f�o�C�X���쐬
//			hr = D3D11CreateDeviceAndSwapChain(
//				// configs
//				NULL, D3D_DRIVER_TYPE_REFERENCE, NULL, createDeviceFlags,
//				dxconfig.FeatureLevels, dxconfig.FeatureLevelCount, D3D11_SDK_VERSION, &sd,
//				// outputs
//				&pSwapChain,
//				&pD3DDevice,
//				&FeatureLevelsSupported,
//				&pImmediateContext);
//
//			if (FAILED(hr))
//			{
//				return DXTRACE_ERR(L"InitDirect3D D3D11CreateDeviceAndSwapChain", hr);
//			}
//		}
//	}
//
//	dxinfo.SetDeviceSwapChain(pD3DDevice, pImmediateContext, pSwapChain, FeatureLevelsSupported);
//
//	return S_OK;
//}
//
//HRESULT GenerateVertexBuffer(const int dataCount, const int structSize, const void* pData, ID3D11Buffer** ppBuffer)
//{
//	D3D11_BUFFER_DESC bufferDesc;
//	bufferDesc.Usage = D3D11_USAGE_DEFAULT;      // �f�t�H���g�g�p�@
//	bufferDesc.ByteWidth = dataCount * structSize;
//	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // ���_�o�b�t�@
//	bufferDesc.CPUAccessFlags = 0;
//	bufferDesc.MiscFlags = 0;
//	bufferDesc.StructureByteStride = 0;
//
//	D3D11_SUBRESOURCE_DATA subData;
//	subData.pSysMem = pData;  // �o�b�t�@�E�f�[�^�̏����l
//	subData.SysMemPitch = 0;
//	subData.SysMemSlicePitch = 0;
//
//	HRESULT hr = g_pD3DDevice->CreateBuffer(&bufferDesc, &subData, ppBuffer);
//	if (FAILED(hr))
//		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateBuffer", hr);
//
//	return S_OK;
//}
//
//HRESULT GenerateIndexBuffer(const int dataCount, const UINT* pData, ID3D11Buffer** ppBuffer)
//{
//	// �C���f�b�N�X�E�o�b�t�@�̒�`
//	D3D11_BUFFER_DESC idxBufferDesc;
//	idxBufferDesc.Usage = D3D11_USAGE_DEFAULT;     // �f�t�H���g�g�p�@
//	idxBufferDesc.ByteWidth = sizeof(UINT) * dataCount;       // 12�~3���_
//	idxBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // �C���f�b�N�X�E�o�b�t�@
//	idxBufferDesc.CPUAccessFlags = 0;
//	idxBufferDesc.MiscFlags = 0;
//	idxBufferDesc.StructureByteStride = 0;
//
//	// �C���f�b�N�X�E�o�b�t�@�̃T�u���\�[�X�̒�`
//	D3D11_SUBRESOURCE_DATA idxSubData;
//	idxSubData.pSysMem = pData;  // �o�b�t�@�E�f�[�^�̏����l
//	idxSubData.SysMemPitch = 0;
//	idxSubData.SysMemSlicePitch = 0;
//
//	// �C���f�b�N�X�E�o�b�t�@�̍쐬
//	HRESULT hr = g_pD3DDevice->CreateBuffer(&idxBufferDesc, &idxSubData, ppBuffer);
//	if (FAILED(hr))
//		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateBuffer", hr);
//
//	return S_OK;
//}
//
//HRESULT LoadVertexShader(
//	const std::wstring& filepath, const std::string& entryPoint, const DirectXConfiguration& dxconfig, ID3D11Device* pD3DDevice, 
//	ID3DBlob** ppBlobVS, ID3D11VertexShader** ppShader)
//{
//	ID3DBlob* pBlobVS = NULL;
//	HRESULT hr = D3DX11CompileFromFile(
//		filepath.c_str(), NULL, NULL,
//		"VS", entryPoint.c_str(), dxconfig.FlagCompile, 0, NULL,
//		&pBlobVS, NULL, NULL);
//	if (FAILED(hr))
//		return DXTRACE_ERR(L"InitDirect3D D3DX11CompileShaderFromFile", hr);
//
//	*ppBlobVS = pBlobVS;
//
//	// ���_�V�F�[�_�̍쐬
//	hr = pD3DDevice->CreateVertexShader(
//		pBlobVS->GetBufferPointer(), // �o�C�g�E�R�[�h�ւ̃|�C���^
//		pBlobVS->GetBufferSize(),    // �o�C�g�E�R�[�h�̒���
//		NULL,
//		ppShader); // ���_�V�F�[�_���󂯎��ϐ�
//
//	if (FAILED(hr)) {
//		SAFE_RELEASE(pBlobVS);
//		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateVertexShader", hr);
//	}
//
//	return S_OK;
//}
//
//HRESULT LoadPixelShader(
//	const std::wstring& filepath, const std::string& entryPoint, const DirectXConfiguration& dxconfig,
//	ID3D11Device* pD3DDevice, ID3D11PixelShader** ppShader)
//{
//	ID3DBlob* pBlobVS = NULL;
//	HRESULT hr = D3DX11CompileFromFile(
//		filepath.c_str(), NULL, NULL,
//		"PS", entryPoint.c_str(), dxconfig.FlagCompile, 0, NULL,
//		&pBlobVS, NULL, NULL);
//	if (FAILED(hr))
//		return DXTRACE_ERR(L"InitDirect3D D3DX11CompileShaderFromFile", hr);
//
//	// ���_�V�F�[�_�̍쐬
//	hr = pD3DDevice->CreatePixelShader(
//		pBlobVS->GetBufferPointer(), // �o�C�g�E�R�[�h�ւ̃|�C���^
//		pBlobVS->GetBufferSize(),    // �o�C�g�E�R�[�h�̒���
//		NULL,
//		ppShader); // ���_�V�F�[�_���󂯎��ϐ�
//	if (FAILED(hr)) {
//		SAFE_RELEASE(pBlobVS);
//		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateVertexShader", hr);
//	}
//
//	return S_OK;
//}
//HRESULT LoadGeometryShader(
//	const std::wstring& filepath, const std::string& entryPoint, const DirectXConfiguration& dxconfig,
//	ID3D11Device* pD3DDevice, ID3D11GeometryShader** ppShader)
//{
//	ID3DBlob* pBlobVS = NULL;
//	HRESULT hr = D3DX11CompileFromFile(
//		filepath.c_str(), NULL, NULL,
//		"GS", entryPoint.c_str(), dxconfig.FlagCompile, 0, NULL,
//		&pBlobVS, NULL, NULL);
//	if (FAILED(hr))
//		return DXTRACE_ERR(L"InitDirect3D D3DX11CompileShaderFromFile", hr);
//
//	// ���_�V�F�[�_�̍쐬
//	hr = pD3DDevice->CreateGeometryShader(
//		pBlobVS->GetBufferPointer(), // �o�C�g�E�R�[�h�ւ̃|�C���^
//		pBlobVS->GetBufferSize(),    // �o�C�g�E�R�[�h�̒���
//		NULL,
//		ppShader); // ���_�V�F�[�_���󂯎��ϐ�
//	if (FAILED(hr)) {
//		SAFE_RELEASE(pBlobVS);
//		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateVertexShader", hr);
//	}
//
//	return S_OK;
//}
//
//HRESULT GenerateConstantBuffers(const UINT bufferCount, const UINT* sizes, ID3D11Device* pD3DDevice, ID3D11Buffer** pBuffers)
//{
//	// �萔�o�b�t�@�̒�`
//	D3D11_BUFFER_DESC cBufferDesc;
//	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;    // ���I(�_�C�i�~�b�N)�g�p�@
//	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // �萔�o�b�t�@
//	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;     // CPU���珑������
//	cBufferDesc.MiscFlags = 0;
//	cBufferDesc.StructureByteStride = 0;
//
//	for (int i = 0; i < bufferCount; i++)
//	{
//		cBufferDesc.ByteWidth = sizes[i]; // �o�b�t�@�E�T�C�Y
//		HRESULT hr = pD3DDevice->CreateBuffer(&cBufferDesc, NULL, &pBuffers[i]);
//		if (FAILED(hr))
//			return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateBuffer", hr);
//	}
//}
//HRESULT SetBlendMode(ID3D11Device* pD3DDevice, ID3D11BlendState** ppBlendState)
//{
//	// �u�����h�E�X�e�[�g�E�I�u�W�F�N�g�̍쐬
//	D3D11_BLEND_DESC BlendState;
//	ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC));
//	BlendState.AlphaToCoverageEnable = FALSE;
//	BlendState.IndependentBlendEnable = FALSE;
//	BlendState.RenderTarget[0].BlendEnable = FALSE;
//	BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
//	HRESULT hr = pD3DDevice->CreateBlendState(&BlendState, ppBlendState);
//	if (FAILED(hr))
//		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateBlendState", hr);
//	return S_OK;
//}
//HRESULT SetRasterMode(ID3D11Device* pD3DDevice, ID3D11RasterizerState** ppRasterizerState)
//{
//	// ���X�^���C�U�E�X�e�[�g�E�I�u�W�F�N�g�̍쐬
//	D3D11_RASTERIZER_DESC RSDesc;
//	RSDesc.FillMode = D3D11_FILL_SOLID;   // ���ʂɕ`�悷��
//	RSDesc.CullMode = D3D11_CULL_NONE;    // ���ʂ�`�悷��
//	RSDesc.FrontCounterClockwise = FALSE; // ���v��肪�\��
//	RSDesc.DepthBias = 0;
//	RSDesc.DepthBiasClamp = 0;
//	RSDesc.SlopeScaledDepthBias = 0;
//	RSDesc.DepthClipEnable = TRUE;
//	RSDesc.ScissorEnable = FALSE;
//	RSDesc.MultisampleEnable = FALSE;
//	RSDesc.AntialiasedLineEnable = FALSE;
//	HRESULT hr = pD3DDevice->CreateRasterizerState(&RSDesc, ppRasterizerState);
//	if (FAILED(hr))
//		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateRasterizerState", hr);
//	return S_OK;
//}
//HRESULT SetDepthTest(ID3D11Device* pD3DDevice, ID3D11DepthStencilState** ppDepthStencilState)
//{
//	// �[�x/�X�e���V���E�X�e�[�g�E�I�u�W�F�N�g�̍쐬
//	D3D11_DEPTH_STENCIL_DESC DepthStencil;
//	DepthStencil.DepthEnable = TRUE; // �[�x�e�X�g����
//	DepthStencil.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // ��������
//	DepthStencil.DepthFunc = D3D11_COMPARISON_LESS; // ��O�̕��̂�`��
//	DepthStencil.StencilEnable = FALSE; // �X�e���V���E�e�X�g�Ȃ�
//	DepthStencil.StencilReadMask = 0;     // �X�e���V���ǂݍ��݃}�X�N�B
//	DepthStencil.StencilWriteMask = 0;     // �X�e���V���������݃}�X�N�B
//	// �ʂ��\�������Ă���ꍇ�̃X�e���V���E�e�X�g�̐ݒ�
//	DepthStencil.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;  // �ێ�
//	DepthStencil.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;  // �ێ�
//	DepthStencil.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;  // �ێ�
//	DepthStencil.FrontFace.StencilFunc = D3D11_COMPARISON_NEVER; // ��Ɏ��s
//	// �ʂ����������Ă���ꍇ�̃X�e���V���E�e�X�g�̐ݒ�
//	DepthStencil.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;   // �ێ�
//	DepthStencil.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;   // �ێ�
//	DepthStencil.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;   // �ێ�
//	DepthStencil.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS; // ��ɐ���
//	HRESULT hr = pD3DDevice->CreateDepthStencilState(&DepthStencil, ppDepthStencilState);
//	if (FAILED(hr))
//		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateDepthStencilState", hr);
//	return S_OK;
//}

HRESULT InitDirect3D()
{
	// �E�C���h�E�̃N���C�A���g �G���A
	//RECT rc;
	//GetClientRect(g_hWindow, &rc);
	//UINT width = rc.right - rc.left;
	//UINT height = rc.bottom - rc.top;

	//// �f�o�C�X���̐���
	//DirectXConfiguration dxconfig;
	//DirectXRenderInfo dxinfo;

	//HRESULT hr = GenerateDeviceSwapChain(dxconfig, { width, height }, g_hWindow, dxinfo);
	//if (FAILED(hr))
	//	return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateBuffer", hr);

	//g_pSwapChain = dxinfo.pSwapChain;
	//g_pD3DDevice = dxinfo.pD3DDevice;
	//g_FeatureLevelsSupported = dxinfo.FeatureLevelsSupported;
	//g_pImmediateContext = dxinfo.pImmediateContext;

	//// ���_�o�b�t�@����
	//hr = GenerateVertexBuffer(sizeof(posVertex) / sizeof(VertexPositionColorTexture), sizeof(VertexPositionColorTexture), posVertex, &g_pVerBuffer[0]);
	//if (FAILED(hr))
	//	return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateBuffer", hr);	
	//hr = GenerateIndexBuffer(sizeof(idxVertexID) / sizeof(UINT), idxVertexID, &g_pIdxBuffer);
	//if (FAILED(hr))
	//	return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateBuffer", hr);

	//// �V�F�[�_���R���p�C��
	//ID3DBlob* pBlobVS = NULL;
	//hr = LoadVertexShader(L"..\\misc\\D3D11Sample05.sh", "vs_4_0", dxconfig, g_pD3DDevice, &pBlobVS, &g_pVertexShader);
	//if (FAILED(hr))
	//	return DXTRACE_ERR(L"InitDirect3D LoadVertexShader", hr);
	//hr = LoadGeometryShader(L"..\\misc\\D3D11Sample05.sh", "gs_4_0", dxconfig, g_pD3DDevice, &g_pGeometryShader);
	//if (FAILED(hr))
	//	return DXTRACE_ERR(L"InitDirect3D LoadGeometryShader", hr);
	//hr = LoadPixelShader(L"..\\misc\\D3D11Sample05.sh", "ps_4_0", dxconfig, g_pD3DDevice, &g_pPixelShader);
	//if (FAILED(hr))
	//	return DXTRACE_ERR(L"InitDirect3D LoadPixelShader", hr);

	//// ���̓��C�A�E�g�E�I�u�W�F�N�g�̍쐬
	//hr = g_pD3DDevice->CreateInputLayout(layout, _countof(layout), pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), &g_pInputLayout);
	//SAFE_RELEASE(pBlobVS);
	//if (FAILED(hr))
	//	return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateInputLayout", hr);

	//// �萔�o�b�t�@
	//UINT cBufferSizes[] = { sizeof(cbNeverChanges), sizeof(cbChangesEveryFrame), sizeof(cbChangesEveryObject), };
	//hr = GenerateConstantBuffers(sizeof(cBufferSizes) / sizeof(UINT), cBufferSizes, g_pD3DDevice, g_pCBuffer);
	//if (FAILED(hr))
	//	return DXTRACE_ERR(L"InitDirect3D GenerateConstantBuffers", hr);

	//// �e��ݒ�
	//SetBlendMode(g_pD3DDevice, &g_pBlendState);
	//SetRasterMode(g_pD3DDevice, &g_pRasterizerState);
	//SetDepthTest(g_pD3DDevice, &g_pDepthStencilState);

	//// �o�b�N �o�b�t�@�̏�����
	//hr = InitBackBuffer();
	//if (FAILED(hr))
	//	return DXTRACE_ERR(L"InitDirect3D InitBackBuffer", hr);

//	return hr;

	return S_OK;
}

HRESULT UpdateBuffer(ID3D11DeviceContext* pContext, ID3D11Buffer* pBuffer, void* pData, UINT dataSize)
{
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	HRESULT hr = pContext->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitBackBuffer  g_pImmediateContext->Map", hr);  // ���s
	CopyMemory(MappedResource.pData, pData, dataSize);
	g_pImmediateContext->Unmap(g_pCBuffer[0], 0);
}

/*-------------------------------------------
�o�b�N �o�b�t�@�̏�����(�o�b�N �o�b�t�@��`��^�[�Q�b�g�ɐݒ�)
--------------------------------------------*/
HRESULT InitBackBuffer(void)
{
	//HRESULT hr;

	//// �X���b�v�E�`�F�C������ŏ��̃o�b�N�E�o�b�t�@���擾����
	//ID3D11Texture2D *pBackBuffer;
	//hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	//if (FAILED(hr))
	//	return DXTRACE_ERR(L"InitBackBuffer g_pSwapChain->GetBuffer", hr);  // ���s

	//// �o�b�N�E�o�b�t�@�̏��
	//D3D11_TEXTURE2D_DESC descBackBuffer;
	//pBackBuffer->GetDesc(&descBackBuffer);
	//hr = g_pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
	//SAFE_RELEASE(pBackBuffer);
	//if (FAILED(hr))
	//	return DXTRACE_ERR(L"InitBackBuffer g_pD3DDevice->CreateRenderTargetView", hr);  // ���s

	//// �[�x/�X�e���V���E�e�N�X�`���̍쐬
	//D3D11_TEXTURE2D_DESC descDepth = descBackBuffer;
	//descDepth.MipLevels = 1;       // �~�b�v�}�b�v�E���x����
	//descDepth.ArraySize = 1;       // �z��T�C�Y
	//descDepth.Format = DXGI_FORMAT_D32_FLOAT;  // �t�H�[�}�b�g(�[�x�̂�)
	//descDepth.Usage = D3D11_USAGE_DEFAULT;      // �f�t�H���g�g�p�@
	//descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL; // �[�x/�X�e���V���Ƃ��Ďg�p
	//descDepth.CPUAccessFlags = 0;   // CPU����̓A�N�Z�X���Ȃ�
	//descDepth.MiscFlags = 0;   // ���̑��̐ݒ�Ȃ�
	//hr = g_pD3DDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
	//if (FAILED(hr))
	//	return DXTRACE_ERR(L"InitBackBuffer g_pD3DDevice->CreateTexture2D", hr);  // ���s

	//// �[�x/�X�e���V�� �r���[�̍쐬
	//D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	//descDSV.Format = descDepth.Format;            // �r���[�̃t�H�[�}�b�g
	//descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	//descDSV.Flags = 0;
	//descDSV.Texture2D.MipSlice = 0;
	//hr = g_pD3DDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
	//if (FAILED(hr))
	//	return DXTRACE_ERR(L"InitBackBuffer g_pD3DDevice->CreateDepthStencilView", hr);  // ���s

	//// �r���[�|�[�g�̐ݒ�
	//g_ViewPort[0].TopLeftX = 0.0f;    // �r���[�|�[�g�̈�̍���X���W�B
	//g_ViewPort[0].TopLeftY = 0.0f;    // �r���[�|�[�g�̈�̍���Y���W�B
	//g_ViewPort[0].Width = (FLOAT)descBackBuffer.Width;   // �r���[�|�[�g�̈�̕�
	//g_ViewPort[0].Height = (FLOAT)descBackBuffer.Height;  // �r���[�|�[�g�̈�̍���
	//g_ViewPort[0].MinDepth = 0.0f; // �r���[�|�[�g�̈�̐[�x�l�̍ŏ��l
	//g_ViewPort[0].MaxDepth = 1.0f; // �r���[�|�[�g�̈�̐[�x�l�̍ő�l

	//// �萔�o�b�t�@�@���X�V
	//// �ˉe�ϊ��s��(�p�[�X�y�N�e�B�u(�����@)�ˉe)
	//XMMATRIX mat = XMMatrixPerspectiveFovLH(
	//	XMConvertToRadians(30.0f),		// ����p30��
	//	(float)descBackBuffer.Width / (float)descBackBuffer.Height,	// �A�X�y�N�g��
	//	1.0f,							// �O�����e�ʂ܂ł̋���
	//	20.0f);							// ������e�ʂ܂ł̋���
	//mat = XMMatrixTranspose(mat);
	//XMStoreFloat4x4(&g_cbNeverChanges.Projection, mat);
	//UpdateBuffer(g_pImmediateContext, g_pCBuffer[0], &g_cbNeverChanges, sizeof(cbNeverChanges));

	////�T�C�Y��ۑ�
	//g_sizeWindow.cx = descBackBuffer.Width;
	//g_sizeWindow.cy = descBackBuffer.Height;

	return S_OK;
}

void Update(void)
{
//	XMVECTORF32 eyePosition = { 0.0f, 5.0f, -5.0f, 1.0f };  // ���_(�J�����̈ʒu)
//	XMVECTORF32 focusPosition = { 0.0f, 0.0f, 0.0f, 1.0f };  // �����_
//	XMVECTORF32 upDirection = { 0.0f, 1.0f, 0.0f, 1.0f };  // �J�����̏����
//	XMMATRIX mat = XMMatrixLookAtLH(eyePosition, focusPosition, upDirection);
//	XMStoreFloat4x4(&g_cbChangesEveryFrame.View, XMMatrixTranspose(mat));
//
//	XMVECTOR vec = XMVector3TransformCoord(XMLoadFloat3(&g_vLightPos), mat);
//	XMStoreFloat3(&g_cbChangesEveryFrame.Light, vec);
//
//	XMMATRIX matY, matX;
//	FLOAT rotate = (FLOAT)(XM_PI * (timeGetTime() % 3000)) / 1500.0f;
//	matY = XMMatrixRotationY(rotate);
//	rotate = (FLOAT)(XM_PI * (timeGetTime() % 1500)) / 750.0f;
//	matX = XMMatrixRotationX(rotate);
//	XMStoreFloat4x4(&g_cbChangesEveryObject.World, XMMatrixTranspose(matY * matX));
}

HRESULT Render(void)
{
	HRESULT hr;

	// �`��^�[�Q�b�g�̃N���A
	g_pImmediateContext->ClearRenderTargetView(
		g_pRenderTargetView, // �N���A����`��^�[�Q�b�g
		g_ClearColor);         // �N���A����l

	// �[�x/�X�e���V���̃N���A
	g_pImmediateContext->ClearDepthStencilView(
		g_pDepthStencilView, // �N���A����[�x/�X�e���V���E�r���[
		D3D11_CLEAR_DEPTH,   // �[�x�l�������N���A����
		1.0f,                // �[�x�o�b�t�@���N���A����l
		0);                  // �X�e���V���E�o�b�t�@���N���A����l(���̏ꍇ�A���֌W)

	// �����̂̕`��
	hr = UpdateBuffer(g_pImmediateContext, g_pCBuffer[1], &g_cbChangesEveryFrame, sizeof(cbChangesEveryFrame));
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitBackBuffer  UpdateBuffer", hr);  // ���s
	hr = UpdateBuffer(g_pImmediateContext, g_pCBuffer[2], &g_cbChangesEveryObject, sizeof(cbChangesEveryObject));
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitBackBuffer  UpdateBuffer", hr);  // ���s

	// IA�ɒ��_�o�b�t�@��ݒ�
	UINT strides[1] = { sizeof(VertexPositionColorTexture) };
	UINT offsets[1] = { 0 };
	g_pImmediateContext->IASetVertexBuffers(0, 1, g_pVerBuffer, strides, offsets);
	g_pImmediateContext->IASetIndexBuffer(g_pIdxBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_pImmediateContext->IASetInputLayout(g_pInputLayout);
	g_pImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 3, g_pCBuffer);

	g_pImmediateContext->GSSetShader(g_pGeometryShader, NULL, 0);
	g_pImmediateContext->GSSetConstantBuffers(0, 3, g_pCBuffer);

	g_pImmediateContext->RSSetViewports(1, g_ViewPort);
	g_pImmediateContext->RSSetState(g_pRasterizerState);

	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 3, g_pCBuffer);

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_bDepthMode ? g_pDepthStencilView : NULL);
	FLOAT BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	g_pImmediateContext->OMSetBlendState(g_pBlendState, BlendFactor, 0xffffffff);
	g_pImmediateContext->OMSetDepthStencilState(g_pDepthStencilState, 0);

	g_pImmediateContext->DrawIndexed(
		36, // �`�悷��C���f�b�N�X��(���_��)
		0,  // �C���f�b�N�X�E�o�b�t�@�̍ŏ��̃C���f�b�N�X����`��J�n
		0); // ���_�o�b�t�@���̍ŏ��̒��_�f�[�^����g�p�J�n

	hr = g_pSwapChain->Present(0,	// ��ʂ𒼂��ɍX�V����
		0);	// ��ʂ����ۂɍX�V����

	return hr;
}

/*-------------------------------------------
Direct3D�̏I������
--------------------------------------------*/
bool CleanupDirect3D(void)
{
	// �f�o�C�X�E�X�e�[�g�̃N���A
	if (g_pImmediateContext)
		g_pImmediateContext->ClearState();

	// �X���b�v �`�F�C�����E�C���h�E ���[�h�ɂ���
	if (g_pSwapChain)
		g_pSwapChain->SetFullscreenState(FALSE, NULL);

	// �擾�����C���^�[�t�F�C�X�̊J��
	SAFE_RELEASE(g_pDepthStencilState);
	SAFE_RELEASE(g_pBlendState);
	SAFE_RELEASE(g_pRasterizerState);

	SAFE_RELEASE(g_pCBuffer[2]);
	SAFE_RELEASE(g_pCBuffer[1]);
	SAFE_RELEASE(g_pCBuffer[0]);

	SAFE_RELEASE(g_pInputLayout);

	SAFE_RELEASE(g_pPixelShader);
	SAFE_RELEASE(g_pGeometryShader);
	SAFE_RELEASE(g_pVertexShader);

	SAFE_RELEASE(g_pIdxBuffer);
	SAFE_RELEASE(g_pVerBuffer[1]);
	SAFE_RELEASE(g_pVerBuffer[0]);

	SAFE_RELEASE(g_pDepthStencilView);
	SAFE_RELEASE(g_pDepthStencil);

	SAFE_RELEASE(g_pRenderTargetView);
	SAFE_RELEASE(g_pSwapChain);

	SAFE_RELEASE(g_pImmediateContext);
	SAFE_RELEASE(g_pD3DDevice);

	return true;
}

/*-------------------------------------------
�A�v���P�[�V�����̏I������
--------------------------------------------*/
bool CleanupApp(void)
{
	// �E�C���h�E �N���X�̓o�^����
	UnregisterClass(g_szWndClass, g_hInstance);
	return true;
}

/*-------------------------------------------
�E�B���h�E����
--------------------------------------------*/
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam)
{
	HRESULT hr = S_OK;
	BOOL fullscreen;

	switch (msg)
	{
	case WM_DESTROY:
		// Direct3D�̏I������
		CleanupDirect3D();
		// �E�C���h�E�����
		PostQuitMessage(0);
		g_hWindow = NULL;
		return 0;

		// �E�C���h�E �T�C�Y�̕ύX����
	case WM_SIZE:
		if (!g_pD3DDevice || wParam == SIZE_MINIMIZED)
			break;

		// �`��^�[�Q�b�g����������
		g_pImmediateContext->OMSetRenderTargets(0, NULL, NULL);	// �`��^�[�Q�b�g�̉���
		SAFE_RELEASE(g_pRenderTargetView);					    // �`��^�[�Q�b�g �r���[�̉��
		SAFE_RELEASE(g_pDepthStencilView);					// �[�x/�X�e���V�� �r���[�̉��
		SAFE_RELEASE(g_pDepthStencil);						// �[�x/�X�e���V�� �e�N�X�`���̉��

		// �o�b�t�@�̕ύX
		g_pSwapChain->ResizeBuffers(3, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

		// �o�b�N �o�b�t�@�̏�����
		InitBackBuffer();
		break;

	case WM_KEYDOWN:
		// �L�[���͂̏���
		switch (wParam)
		{
		case VK_ESCAPE:	// [ESC]�L�[�ŃE�C���h�E�����
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		case VK_F2:		// [F2]�L�[�Ő[�x�o�b�t�@�̃��[�h��؂�ւ���
			g_bDepthMode = !g_bDepthMode;
			break;

		case VK_F5:		// [F5]�L�[�ŉ�ʃ��[�h��؂�ւ���
			if (g_pSwapChain != NULL) {
				g_pSwapChain->GetFullscreenState(&fullscreen, NULL);
				g_pSwapChain->SetFullscreenState(!fullscreen, NULL);
			}
			break;
		}
		break;
	}

	// �f�t�H���g����
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

/*--------------------------------------------
�f�o�C�X�̏�������
--------------------------------------------*/
HRESULT IsDeviceRemoved(void)
{
	HRESULT hr;

	// �f�o�C�X�̏����m�F
	hr = g_pD3DDevice->GetDeviceRemovedReason();
	switch (hr) {
	case S_OK:
		break;         // ����

	case DXGI_ERROR_DEVICE_HUNG:
	case DXGI_ERROR_DEVICE_RESET:
		DXTRACE_ERR(L"IsDeviceRemoved g_pD3DDevice->GetDeviceRemovedReason", hr);
		CleanupDirect3D();   // Direct3D�̉��(�A�v���P�[�V������`)
		hr = InitDirect3D();  // Direct3D�̏�����(�A�v���P�[�V������`)
		if (FAILED(hr))
			return hr; // ���s�B�A�v���P�[�V�������I��
		break;

	case DXGI_ERROR_DEVICE_REMOVED:
	case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
	case DXGI_ERROR_INVALID_CALL:
	default:
		DXTRACE_ERR(L"IsDeviceRemoved g_pD3DDevice->GetDeviceRemovedReason", hr);
		return hr;   // �ǂ����悤���Ȃ��̂ŁA�A�v���P�[�V�������I���B
	};

	return S_OK;         // ����
}

/*--------------------------------------------
�A�C�h�����̏���
--------------------------------------------*/
bool AppIdle(void)
{
	if (!g_pD3DDevice)
		return false;

	HRESULT hr;
	// �f�o�C�X�̏�������
	hr = IsDeviceRemoved();
	if (FAILED(hr))
		return false;

	// �X�^���o�C ���[�h
	if (g_bStandbyMode) {
		hr = g_pSwapChain->Present(0, DXGI_PRESENT_TEST);
		if (hr != S_OK) {
			Sleep(100);	// 0.1�b�҂�
			return true;
		}
		g_bStandbyMode = false; // �X�^���o�C ���[�h����������
	}

	// ��ʂ̍X�V
	Update();
	hr = Render();
	if (hr == DXGI_STATUS_OCCLUDED) {
		g_bStandbyMode = true;  // �X�^���o�C ���[�h�ɓ���
	}

	return true;
}

/*--------------------------------------------
���C��
---------------------------------------------*/
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int)
{
	// �f�o�b�O �q�[�v �}�l�[�W���ɂ�郁�������蓖�Ă̒ǐՕ��@��ݒ�
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// XNA Math���C�u�����̃T�|�[�g �`�F�b�N
	if (XMVerifyCPUSupport() != TRUE)
	{
		DXTRACE_MSG(L"WinMain XMVerifyCPUSupport");
		return 0;
	}

	// �A�v���P�[�V�����Ɋւ��鏉����
	HRESULT hr = InitializeWindow(hInst);
	if (FAILED(hr))
	{
		DXTRACE_ERR(L"WinMain InitializeWindow", hr);
		return 0;
	}

	// Direct3D�̏�����
	hr = InitDirect3D();
	if (FAILED(hr)) {
		DXTRACE_ERR(L"WinMain InitDirect3D", hr);
		CleanupDirect3D();
		CleanupApp();
		return 0;
	}

	// ���b�Z�[�W ���[�v
	MSG msg;
	do
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// �A�C�h������
			if (!AppIdle())
				// �G���[������ꍇ�C�A�v���P�[�V�������I������
				DestroyWindow(g_hWindow);
		}
	} while (msg.message != WM_QUIT);

	// �A�v���P�[�V�����̏I������
	CleanupApp();

	return (int)msg.wParam;
}
