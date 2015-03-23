#include "stdafx.h"
#include <stdio.h>
#include "DirectXHelper.h"
#include "ConstBuffer.h"
#include "Configuration.h"

// directxのコンテキストやバッファなど、描画に必要なデータ
struct DirectXHelper::DirectXRenderInfo
{
	D3D_FEATURE_LEVEL FeatureLevelsSupported; // デバイス作成時に返される機能レベル

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

	// 深度バッファのモード
	bool DepthMode = true;

	// スタンバイモード
	bool StandbyMode = false;

	XMFLOAT3 LightPos;

	cbNeverChanges       cbNeverChanges;       // 透視変換行列
	cbChangesEveryFrame  cbChangesEveryFrame;  // ビュー変換行列　光源座標
	cbChangesEveryObject cbChangesEveryObject; // ワールド変換行列

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
	// デバイスとスワップ チェインの作成
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));	// 構造体の値を初期化
	sd.BufferCount = dxconfig.BufferCount;		// バック バッファ数
	sd.BufferDesc.Width = bufferSize.cx;	// バック バッファの幅
	sd.BufferDesc.Height = bufferSize.cy;	// バック バッファの高さ
	sd.BufferDesc.Format = dxconfig.BufferFormat;  // フォーマット
	sd.BufferDesc.RefreshRate.Numerator = dxconfig.RefreshRateNumerator;  // リフレッシュ レート(分子)
	sd.BufferDesc.RefreshRate.Denominator = dxconfig.RefreshRateDenominator; // リフレッシュ レート(分母)
	sd.BufferDesc.ScanlineOrdering = dxconfig.BufferScanOrder;	// スキャンライン
	sd.BufferDesc.Scaling = dxconfig.BufferScanScaling;			// スケーリング
	sd.BufferUsage = dxconfig.BufferUsage; // バック バッファの使用法
	sd.OutputWindow = hWindow;	// 関連付けるウインドウ
	sd.SampleDesc.Count = dxconfig.SampleCount;		// マルチ サンプルの数
	sd.SampleDesc.Quality = dxconfig.SampleQuality;		// マルチ サンプルのクオリティ
	sd.Windowed = dxconfig.IsWindowMode;				// ウインドウ モード
	sd.Flags = dxconfig.SwapChainFlg; // モード自動切り替え

#if defined(DEBUG) || defined(_DEBUG)
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#else
	UINT createDeviceFlags = 0;
#endif

	D3D_FEATURE_LEVEL FeatureLevelsSupported;
	ID3D11Device*           pD3DDevice = NULL;
	ID3D11DeviceContext*    pImmediateContext = NULL;
	IDXGISwapChain*         pSwapChain = NULL;

	// ハードウェア・デバイスを作成
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
		// WARPデバイスを作成
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
			// リファレンス・デバイスを作成
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
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;      // デフォルト使用法
	bufferDesc.ByteWidth = dataCount * structSize;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // 頂点バッファ
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = pData;  // バッファ・データの初期値
	subData.SysMemPitch = 0;
	subData.SysMemSlicePitch = 0;

	HRESULT hr = pD3DDevice->CreateBuffer(&bufferDesc, &subData, ppBuffer);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateBuffer", hr);

	return S_OK;
}

HRESULT DirectXHelper::GenerateIndexBuffer(const int dataCount, const UINT* pData, ID3D11Device* pD3DDevice, ID3D11Buffer** ppBuffer)
{
	// インデックス・バッファの定義
	D3D11_BUFFER_DESC idxBufferDesc;
	idxBufferDesc.Usage = D3D11_USAGE_DEFAULT;     // デフォルト使用法
	idxBufferDesc.ByteWidth = sizeof(UINT) * dataCount;       // 12×3頂点
	idxBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // インデックス・バッファ
	idxBufferDesc.CPUAccessFlags = 0;
	idxBufferDesc.MiscFlags = 0;
	idxBufferDesc.StructureByteStride = 0;

	// インデックス・バッファのサブリソースの定義
	D3D11_SUBRESOURCE_DATA idxSubData;
	idxSubData.pSysMem = pData;  // バッファ・データの初期値
	idxSubData.SysMemPitch = 0;
	idxSubData.SysMemSlicePitch = 0;

	// インデックス・バッファの作成
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

	// 頂点シェーダの作成
	hr = pD3DDevice->CreateVertexShader(
		pBlobVS->GetBufferPointer(), // バイト・コードへのポインタ
		pBlobVS->GetBufferSize(),    // バイト・コードの長さ
		NULL,
		ppShader); // 頂点シェーダを受け取る変数

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

	// 頂点シェーダの作成
	hr = pD3DDevice->CreatePixelShader(
		pBlobVS->GetBufferPointer(), // バイト・コードへのポインタ
		pBlobVS->GetBufferSize(),    // バイト・コードの長さ
		NULL,
		ppShader); // 頂点シェーダを受け取る変数
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

	// 頂点シェーダの作成
	hr = pD3DDevice->CreateGeometryShader(
		pBlobVS->GetBufferPointer(), // バイト・コードへのポインタ
		pBlobVS->GetBufferSize(),    // バイト・コードの長さ
		NULL,
		ppShader); // 頂点シェーダを受け取る変数
	if (FAILED(hr)) {
		SAFE_RELEASE(pBlobVS);
		return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateVertexShader", hr);
	}

	return S_OK;
}

HRESULT DirectXHelper::GenerateConstantBuffers(const UINT bufferCount, const UINT* sizes, ID3D11Device* pD3DDevice, ID3D11Buffer** pBuffers)
{
	// 定数バッファの定義
	D3D11_BUFFER_DESC cBufferDesc;
	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;    // 動的(ダイナミック)使用法
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // 定数バッファ
	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;     // CPUから書き込む
	cBufferDesc.MiscFlags = 0;
	cBufferDesc.StructureByteStride = 0;

	for (int i = 0; i < bufferCount; i++)
	{
		cBufferDesc.ByteWidth = sizes[i]; // バッファ・サイズ
		HRESULT hr = pD3DDevice->CreateBuffer(&cBufferDesc, NULL, &pBuffers[i]);
		if (FAILED(hr))
			return DXTRACE_ERR(L"InitDirect3D g_pD3DDevice->CreateBuffer", hr);
	}
}

HRESULT DirectXHelper::SetBlendMode(ID3D11Device* pD3DDevice, ID3D11BlendState** ppBlendState)
{
	// ブレンド・ステート・オブジェクトの作成
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
	// ラスタライザ・ステート・オブジェクトの作成
	D3D11_RASTERIZER_DESC RSDesc;
	RSDesc.FillMode = D3D11_FILL_SOLID;   // 普通に描画する
	RSDesc.CullMode = D3D11_CULL_NONE;    // 両面を描画する
	RSDesc.FrontCounterClockwise = FALSE; // 時計回りが表面
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
	// 深度/ステンシル・ステート・オブジェクトの作成
	D3D11_DEPTH_STENCIL_DESC DepthStencil;
	DepthStencil.DepthEnable = TRUE; // 深度テストあり
	DepthStencil.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // 書き込む
	DepthStencil.DepthFunc = D3D11_COMPARISON_LESS; // 手前の物体を描画
	DepthStencil.StencilEnable = FALSE; // ステンシル・テストなし
	DepthStencil.StencilReadMask = 0;     // ステンシル読み込みマスク。
	DepthStencil.StencilWriteMask = 0;     // ステンシル書き込みマスク。
	// 面が表を向いている場合のステンシル・テストの設定
	DepthStencil.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;  // 維持
	DepthStencil.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;  // 維持
	DepthStencil.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;  // 維持
	DepthStencil.FrontFace.StencilFunc = D3D11_COMPARISON_NEVER; // 常に失敗
	// 面が裏を向いている場合のステンシル・テストの設定
	DepthStencil.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;   // 維持
	DepthStencil.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;   // 維持
	DepthStencil.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;   // 維持
	DepthStencil.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS; // 常に成功
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
