#include "stdafx.h"
#include <stdio.h>
#include "DirectXHelper.h"
#include "ConstBuffer.h"
#include "Configuration.h"
#include "VertexPositionColorTexture.h"
#include "DirectXRenderInfo.h"
#include "IGame.h"

DirectXHelper* g_pDXHelper = NULL;
PDirectXRenderInfo* g_pDXInfo = NULL;

LRESULT CALLBACK OnWndProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam)
{
	if (g_pDXHelper == NULL || g_pDXInfo == NULL)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	HRESULT hr = S_OK;
	BOOL fullscreen;

	switch (msg)
	{
	case WM_DESTROY:
		// ウインドウを閉じる
		PostQuitMessage(0);
		// Direct3Dの終了処理
		if (g_pDXInfo != NULL && (*g_pDXInfo) != nullptr && g_pDXHelper != NULL)
			g_pDXHelper->Dispose(*g_pDXInfo);
		if (g_pDXInfo != NULL && (*g_pDXInfo) != nullptr)
			(*g_pDXInfo)->hWindow = NULL;
		return 0;

	case WM_SIZE:
		// ウインドウ サイズの変更処理
		if (!g_pDXInfo || !(*g_pDXInfo)->pD3DDevice || wParam == SIZE_MINIMIZED)
			break;
		g_pDXHelper->Resize(*g_pDXInfo, { LOWORD(lParam), HIWORD(lParam) });
		break;

	case WM_KEYDOWN:
		// キー入力の処理
		switch (wParam)
		{
		case VK_ESCAPE:	// [ESC]キーでウインドウを閉じる
			PostQuitMessage(0);
			break;

		case VK_F2:		// [F2]キーで深度バッファのモードを切り替える
			(*g_pDXInfo)->DepthMode = !(*g_pDXInfo)->DepthMode;
			break;

		case VK_F5:		// [F5]キーで画面モードを切り替える
			if ((*g_pDXInfo)->pSwapChain != NULL) {
				(*g_pDXInfo)->pSwapChain->GetFullscreenState(&fullscreen, NULL);
				(*g_pDXInfo)->pSwapChain->SetFullscreenState(!fullscreen, NULL);
			}
			break;
		}
		break;
	}

	// デフォルト処理
	return DefWindowProc(hWnd, msg, wParam, lParam);
}


//==================================================================================================
//
// Public methods
//
//==================================================================================================

DirectXHelper::DirectXHelper()
{
}

DirectXHelper::~DirectXHelper() = default;

PDirectXRenderInfo DirectXHelper::Initialize(HINSTANCE hInst)
{
	PDirectXRenderInfo pInfo = std::make_shared<DirectXRenderInfo>();
	pInfo->hInst = hInst;

	HWND hWindow = NULL;
	HRESULT hr = InitializeWindow(hInst, &hWindow);
	if (FAILED(hr))
	{
		DXTRACE_ERR(L"DirectXHelper::Initialize InitializeWindow", hr);
		return pInfo;
	}
	pInfo->hWindow = hWindow;

	hr = InitializeDirect3D(pInfo);

	g_pDXHelper = this;
	g_pDXInfo = &pInfo;

	/*UINT sizes[] = { sizeof(::cbNeverChanges), sizeof(::cbChangesEveryFrame), sizeof(::cbChangesEveryObject), };
	GenerateConstantBuffers(3, sizes, pinfo->pD3DDevice, pinfo->pCBuffer);*/

	InitBackBuffer(pInfo);
	return pInfo;
}

bool DirectXHelper::AppIdle(const PDirectXRenderInfo& pInfo, IGame* game)
{
	if (!pInfo->pD3DDevice)
		return false;

	HRESULT hr;
	// デバイスの消失処理
	hr = IsDeviceRemoved(pInfo);
	if (!hr)
		return false;

	// スタンバイ モード
	if (pInfo->StandbyMode) {
		hr = pInfo->pSwapChain->Present(0, DXGI_PRESENT_TEST);
		if (hr != S_OK) {
			Sleep(100);	// 0.1秒待つ
			return true;
		}
		pInfo->StandbyMode = false; // スタンバイ モードを解除する
	}

	// 画面の更新
	bool res = game->Update(this, pInfo);
	if (!res)
		return false;

	res = game->Draw(this, pInfo);
	if (!res)
		return false;

	return true;
}

bool DirectXHelper::Run( const PDirectXRenderInfo& pInfo, IGame* game)
{
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
			if (!AppIdle(pInfo, game))
				DestroyWindow(pInfo->hWindow);
		}
	}
	while (msg.message != WM_QUIT);

	return true;
}

bool DirectXHelper::IsDeviceRemoved(const PDirectXRenderInfo& pInfo)
{
	HRESULT hr;

	// デバイスの消失確認
	hr = pInfo->pD3DDevice->GetDeviceRemovedReason();
	switch (hr) {
	case S_OK:
		break;         // 正常

	case DXGI_ERROR_DEVICE_HUNG:
	case DXGI_ERROR_DEVICE_RESET:
		DXTRACE_ERR(L"IsDeviceRemoved g_pDXInfo->pD3DDevice->GetDeviceRemovedReason", hr);
		Dispose(pInfo);   // Direct3Dの解放(アプリケーション定義)
		hr = InitializeDirect3D(pInfo);  // Direct3Dの初期化(アプリケーション定義)
		if (FAILED(hr))
			return false; // 失敗。アプリケーションを終了
		break;

	case DXGI_ERROR_DEVICE_REMOVED:
	case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
	case DXGI_ERROR_INVALID_CALL:
	default:
		DXTRACE_ERR(L"IsDeviceRemoved g_pDXInfo->pD3DDevice->GetDeviceRemovedReason", hr);
		return false;   // どうしようもないので、アプリケーションを終了。
	};

	return true;
}

bool DirectXHelper::Resize(const PDirectXRenderInfo& pInfo, const SIZE size)
{
	// 描画ターゲットを解除する
	pInfo->pImmediateContext->OMSetRenderTargets(0, NULL, NULL);	// 描画ターゲットの解除
	SAFE_RELEASE(pInfo->pRenderTargetView);					    // 描画ターゲット ビューの解放
	SAFE_RELEASE(pInfo->pDepthStencilView);					// 深度/ステンシル ビューの解放
	SAFE_RELEASE(pInfo->pDepthStencil);						// 深度/ステンシル テクスチャの解放

	// バッファの変更
	pInfo->pSwapChain->ResizeBuffers(3, size.cx, size.cy, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	// バック バッファの初期化
	g_pDXHelper->InitBackBuffer(pInfo);

	return true;
}

void DirectXHelper::BeginDraw(const PDirectXRenderInfo& pInfo)
{
	// 描画ターゲットのクリア
	auto ctx = pInfo->pImmediateContext;
	auto rtv = pInfo->pRenderTargetView;

	pInfo->pImmediateContext->ClearRenderTargetView(
		pInfo->pRenderTargetView, // クリアする描画ターゲット
		pInfo->dxconfig.ClearColor);         // クリアする値

	// 深度/ステンシルのクリア
	pInfo->pImmediateContext->ClearDepthStencilView(
		pInfo->pDepthStencilView, // クリアする深度/ステンシル・ビュー
		D3D11_CLEAR_DEPTH,   // 深度値だけをクリアする
		1.0f,                // 深度バッファをクリアする値
		0);                  // ステンシル・バッファをクリアする値(この場合、無関係)

	// IAに頂点バッファを設定
	UINT strides[1] = { pInfo->verStride };
	UINT offsets[1] = { 0 };
	pInfo->pImmediateContext->IASetVertexBuffers(0, 1, pInfo->pVerBuffers, strides, offsets);
	pInfo->pImmediateContext->IASetIndexBuffer(pInfo->pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pInfo->pImmediateContext->IASetInputLayout(pInfo->pInputLayout);
	pInfo->pImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pInfo->pImmediateContext->VSSetShader(pInfo->pVertexShader, NULL, 0);
//	pInfo->pImmediateContext->VSSetConstantBuffers(0, 3, pInfo->pCBuffer);

	pInfo->pImmediateContext->GSSetShader(pInfo->pGeometryShader, NULL, 0);
//	pInfo->pImmediateContext->GSSetConstantBuffers(0, 3, pInfo->pCBuffer);

	pInfo->pImmediateContext->RSSetViewports(1, pInfo->ViewPort);
	pInfo->pImmediateContext->RSSetState(pInfo->pRasterizerState);

	pInfo->pImmediateContext->PSSetShader(pInfo->pPixelShader, NULL, 0);
//	pInfo->pImmediateContext->PSSetConstantBuffers(0, 3, pInfo->pCBuffer);

	pInfo->pImmediateContext->OMSetRenderTargets(1, &pInfo->pRenderTargetView, pInfo->DepthMode ? pInfo->pDepthStencilView : NULL);
	FLOAT BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pInfo->pImmediateContext->OMSetBlendState(pInfo->pBlendState, BlendFactor, 0xffffffff);
	pInfo->pImmediateContext->OMSetDepthStencilState(pInfo->pDepthStencilState, 0);
}

bool DirectXHelper::EndDraw(const PDirectXRenderInfo& pInfo)
{
	HRESULT hr = pInfo->pSwapChain->Present(0, 0);

	if (hr == DXGI_STATUS_OCCLUDED) {
		pInfo->StandbyMode = true;  // スタンバイ モードに入る
	}

	return SUCCEEDED(hr);
}

void DirectXHelper::VSSetConstantBuffer(const PDirectXRenderInfo& pInfo, const int count, ID3D11Buffer** ppCBuffer)
{
	pInfo->pImmediateContext->VSSetConstantBuffers(0, count, ppCBuffer);
}

void DirectXHelper::GSSetConstantBuffer(const PDirectXRenderInfo& pInfo, const int count, ID3D11Buffer** ppCBuffer)
{
	pInfo->pImmediateContext->GSSetConstantBuffers(0, count, ppCBuffer);
}

void DirectXHelper::PSSetConstantBuffer(const PDirectXRenderInfo& pInfo, const int count, ID3D11Buffer** ppCBuffer)
{
	pInfo->pImmediateContext->PSSetConstantBuffers(0, count, ppCBuffer);
}

void DirectXHelper::DrawPrimitives(const PDirectXRenderInfo& pInfo, const D3D_PRIMITIVE_TOPOLOGY primitiveType, const int icount, const int ioffset, const int voffset)
{
	pInfo->pImmediateContext->IASetPrimitiveTopology(primitiveType);
	pInfo->pImmediateContext->DrawIndexed(
		icount, // 描画するインデックス数(頂点数)
		ioffset,  // インデックス・バッファの最初のインデックスから描画開始
		voffset); // 頂点バッファ内の最初の頂点データから使用開始
}

bool DirectXHelper::Draw(const PDirectXRenderInfo& pInfo)
{
	BeginDraw(pInfo);


	return EndDraw(pInfo);
}

bool DirectXHelper::Dispose(const PDirectXRenderInfo& pInfo, const bool disposeWindow)
{
	if (pInfo == nullptr)
		return false;

	// デバイス・ステートのクリア
	if (pInfo->pImmediateContext)
		pInfo->pImmediateContext->ClearState();

	// スワップ チェインをウインドウ モードにする
	if (pInfo->pSwapChain)
		pInfo->pSwapChain->SetFullscreenState(FALSE, NULL);

	// 取得したインターフェイスの開放
	if (!pInfo->IsDisposed())
		pInfo->Dispose();

	if (disposeWindow)
	{
		g_pDXHelper = NULL;
		g_pDXInfo = NULL;
		UnregisterClass(WindowConfiguration::WindowTitle.c_str(), pInfo->hInst);
	}

	return true;
}

ID3D11Buffer* DirectXHelper::CreateVertexBuffer(const PDirectXRenderInfo& pInfo, const int dataCount, const int structSize, const void* pData)
{
	ID3D11Buffer* pBuffer = NULL;

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

	HRESULT hr = pInfo->pD3DDevice->CreateBuffer(&bufferDesc, &subData, &pBuffer);
	if (FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(L"InitDirect3D g_pD3DDevice->CreateBuffer", hr);
		return NULL;
	}

	return pBuffer;
}

ID3D11Buffer* DirectXHelper::CreateIndexBuffer(const PDirectXRenderInfo& pInfo, const int dataCount, const UINT* pData)
{
	ID3D11Buffer* pBuffer = NULL;

	D3D11_BUFFER_DESC idxBufferDesc;
	idxBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	idxBufferDesc.ByteWidth = sizeof(UINT) * dataCount;
	idxBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	idxBufferDesc.CPUAccessFlags = 0;
	idxBufferDesc.MiscFlags = 0;
	idxBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA idxSubData;
	idxSubData.pSysMem = pData;
	idxSubData.SysMemPitch = 0;
	idxSubData.SysMemSlicePitch = 0;

	HRESULT hr = pInfo->pD3DDevice->CreateBuffer(&idxBufferDesc, &idxSubData, &pBuffer);
	if (FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(L"InitDirect3D g_pD3DDevice->CreateBuffer", hr);
		return NULL;
	}

	return pBuffer;
}

bool DirectXHelper::SetVertexBuffer(const PDirectXRenderInfo& pInfo, ID3D11Buffer* verBuffer, const int structSize)
{
	pInfo->pVerBuffers[0] = verBuffer;
	pInfo->verStride = structSize;
	return true;
}

bool DirectXHelper::SetIndexBuffer(const PDirectXRenderInfo& pInfo, ID3D11Buffer* idxBuffer)
{
	pInfo->pIndexBuffer = idxBuffer;
	return true;
}

ID3D11VertexShader* DirectXHelper::LoadVertexShader(const PDirectXRenderInfo& pInfo, const std::wstring& filepath, const std::string& entryPoint, ID3DBlob** ppBlob)
{
	ID3D11VertexShader* pShader = NULL;

	if (ppBlob != NULL)
		*ppBlob = NULL;

	ID3DBlob* pBlob = NULL;
	HRESULT hr = D3DX11CompileFromFile(filepath.c_str(), NULL, NULL, entryPoint.c_str(), "vs_4_0", pInfo->dxconfig.FlagCompile, 0, NULL, &pBlob, NULL, NULL);
	if (FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(L"InitDirect3D D3DX11CompileShaderFromFile", hr);
		return NULL;
	}

	// 頂点シェーダの作成
	hr = pInfo->pD3DDevice->CreateVertexShader(
		pBlob->GetBufferPointer(), // バイト・コードへのポインタ
		pBlob->GetBufferSize(),    // バイト・コードの長さ
		NULL,
		&pShader); // 頂点シェーダを受け取る変数

	if (FAILED(hr))
	{
		SAFE_RELEASE(pBlob);
		DXTRACE_ERR_MSGBOX(L"InitDirect3D g_pD3DDevice->CreateVertexShader", hr);
		return NULL;
	}

	if (ppBlob != NULL)
		*ppBlob = pBlob;

	return pShader;
}

ID3D11GeometryShader* DirectXHelper::LoadGeometryShader(const PDirectXRenderInfo& pInfo, const std::wstring& filepath, const std::string& entryPoint, ID3DBlob** ppBlob)
{
	ID3D11GeometryShader* pShader = NULL;

	if (ppBlob != NULL)
		*ppBlob = NULL;

	ID3DBlob* pBlob = NULL;
	HRESULT hr = D3DX11CompileFromFile(filepath.c_str(), NULL, NULL, entryPoint.c_str(), "gs_4_0", pInfo->dxconfig.FlagCompile, 0, NULL, &pBlob, NULL, NULL);
	if (FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(L"InitDirect3D D3DX11CompileShaderFromFile", hr);
		return NULL;
	}

	// 頂点シェーダの作成
	hr = pInfo->pD3DDevice->CreateGeometryShader(
		pBlob->GetBufferPointer(), // バイト・コードへのポインタ
		pBlob->GetBufferSize(),    // バイト・コードの長さ
		NULL,
		&pShader); // 頂点シェーダを受け取る変数

	if (FAILED(hr))
	{
		SAFE_RELEASE(pBlob);
		DXTRACE_ERR_MSGBOX(L"InitDirect3D g_pD3DDevice->CreateVertexShader", hr);
		return NULL;
	}

	if (ppBlob != NULL)
		*ppBlob = pBlob;

	return pShader;
}

ID3D11PixelShader* DirectXHelper::LoadPixelShader(const PDirectXRenderInfo& pInfo, const std::wstring& filepath, const std::string& entryPoint, ID3DBlob** ppBlob)
{
	ID3D11PixelShader* pShader = NULL;

	if (ppBlob != NULL)
		*ppBlob = NULL;

	ID3DBlob* pBlob = NULL;
	HRESULT hr = D3DX11CompileFromFile(filepath.c_str(), NULL, NULL, entryPoint.c_str(), "ps_4_0", pInfo->dxconfig.FlagCompile, 0, NULL, &pBlob, NULL, NULL);
	if (FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(L"InitDirect3D D3DX11CompileShaderFromFile", hr);
		return NULL;
	}

	// 頂点シェーダの作成
	hr = pInfo->pD3DDevice->CreatePixelShader(
		pBlob->GetBufferPointer(), // バイト・コードへのポインタ
		pBlob->GetBufferSize(),    // バイト・コードの長さ
		NULL,
		&pShader); // 頂点シェーダを受け取る変数

	if (FAILED(hr))
	{
		SAFE_RELEASE(pBlob);
		DXTRACE_ERR_MSGBOX(L"InitDirect3D g_pD3DDevice->CreateVertexShader", hr);
		return NULL;
	}

	if (ppBlob != NULL)
		*ppBlob = pBlob;

	return pShader;
}

bool DirectXHelper::SetShader(const PDirectXRenderInfo& pInfo, ID3D11VertexShader* pShader)
{
	pInfo->pVertexShader = pShader;
	return true;
}

bool DirectXHelper::SetShader(const PDirectXRenderInfo& pInfo, ID3D11GeometryShader* pShader)
{
	pInfo->pGeometryShader = pShader;
	return true;
}

bool DirectXHelper::SetShader(const PDirectXRenderInfo& pInfo, ID3D11PixelShader* pShader)
{
	pInfo->pPixelShader = pShader;
	return true;
}

bool DirectXHelper::SetInputLayout(const PDirectXRenderInfo& pInfo, UINT layoutCount, D3D11_INPUT_ELEMENT_DESC* layout, ID3DBlob* pBlobVS)
{
	ID3D11InputLayout* pInputLayout = NULL;

	HRESULT hr = pInfo->pD3DDevice->CreateInputLayout(
		layout,                            
		layoutCount,
		pBlobVS->GetBufferPointer(),
		pBlobVS->GetBufferSize(),
		&pInputLayout);

	SAFE_RELEASE(pBlobVS);

	if (FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(L"DirectXHelper::SetInputLayout", hr);
		return false;
	}

	pInfo->pInputLayout = pInputLayout;
	return true;
}

bool DirectXHelper::GenerateConstantBuffer(const PDirectXRenderInfo& pInfo, const UINT size, ID3D11Buffer** ppBuffer)
{
	D3D11_BUFFER_DESC cBufferDesc;
	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;    // 動的(ダイナミック)使用法
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // 定数バッファ
	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;     // CPUから書き込む
	cBufferDesc.MiscFlags = 0;
	cBufferDesc.StructureByteStride = 0;
	cBufferDesc.ByteWidth = size; // バッファ・サイズ

	HRESULT hr = pInfo->pD3DDevice->CreateBuffer(&cBufferDesc, NULL, ppBuffer);
	if (FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(L"InitDirect3D g_pD3DDevice->CreateBuffer", hr);
		return false;
	}

	return true;
}

bool DirectXHelper::UpdateBuffer(const PDirectXRenderInfo& pInfo, ID3D11Buffer* pBuffer, void* pData, const int dataSize)
{
	HRESULT hr = UpdateBuffer(pInfo->pImmediateContext, pBuffer, pData, static_cast<UINT>(dataSize));
	return SUCCEEDED(hr);
}

//==================================================================================================
//
// Private methods
//
//==================================================================================================

// ウインドウ生成・表示
HRESULT DirectXHelper::InitializeWindow(HINSTANCE hInst, HWND* phWindow)
{
	HRESULT hr = GenerateWindow(hInst, OnWndProc, WindowConfiguration::WindowTitle, WindowConfiguration::WindowName, WindowConfiguration::WindowSize, phWindow);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitializeWindow", hr);
	ShowWindow(*phWindow, SW_SHOWNORMAL);
	UpdateWindow(*phWindow);
	return S_OK;
}

HRESULT  DirectXHelper::GenerateWindow(const HINSTANCE hInst, const WNDPROC OnWidowEvent, const std::wstring wndTitle, const std::wstring wndName, const SIZE wndSize, HWND* pWnd)
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


HRESULT DirectXHelper::InitializeDirect3D(const PDirectXRenderInfo& pInfo)
{
	RECT rc;
	GetClientRect(pInfo->hWindow, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	HRESULT hr = GenerateDeviceSwapChain(pInfo->dxconfig, { width, height }, pInfo->hWindow, pInfo);
	if (FAILED(hr))
	{
		return DXTRACE_ERR(L"DirectXHelper::Initialize GenerateDeviceSwapChain", hr);
	}
	SetBlendMode(pInfo->pD3DDevice, &pInfo->pBlendState);
	SetRasterMode(pInfo->pD3DDevice, &pInfo->pRasterizerState);
	SetDepthTest(pInfo->pD3DDevice, &pInfo->pDepthStencilState);

	return S_OK;
}

HRESULT DirectXHelper::GenerateDeviceSwapChain(const DirectXConfiguration& dxconfig, const SIZE& bufferSize, const HWND& hWindow, const PDirectXRenderInfo& pinfo)
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

HRESULT DirectXHelper::InitBackBuffer(const PDirectXRenderInfo& pInfo)
{
	HRESULT hr;

	// スワップ・チェインから最初のバック・バッファを取得する
	ID3D11Texture2D *pBackBuffer;
	hr = pInfo->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitBackBuffer g_pSwapChain->GetBuffer", hr);  // 失敗

	// バック・バッファの情報
	D3D11_TEXTURE2D_DESC descBackBuffer;
	pBackBuffer->GetDesc(&descBackBuffer);
	hr = pInfo->pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &pInfo->pRenderTargetView);
	SAFE_RELEASE(pBackBuffer);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitBackBuffer g_pD3DDevice->CreateRenderTargetView", hr);  // 失敗

	// 深度/ステンシル・テクスチャの作成
	D3D11_TEXTURE2D_DESC descDepth = descBackBuffer;
	descDepth.MipLevels = 1;       // ミップマップ・レベル数
	descDepth.ArraySize = 1;       // 配列サイズ
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;  // フォーマット(深度のみ)
	descDepth.Usage = D3D11_USAGE_DEFAULT;      // デフォルト使用法
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL; // 深度/ステンシルとして使用
	descDepth.CPUAccessFlags = 0;   // CPUからはアクセスしない
	descDepth.MiscFlags = 0;   // その他の設定なし
	hr = pInfo->pD3DDevice->CreateTexture2D(&descDepth, NULL, &pInfo->pDepthStencil);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitBackBuffer g_pD3DDevice->CreateTexture2D", hr);  // 失敗

	// 深度/ステンシル ビューの作成
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDepth.Format;            // ビューのフォーマット
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Flags = 0;
	descDSV.Texture2D.MipSlice = 0;
	hr = pInfo->pD3DDevice->CreateDepthStencilView(pInfo->pDepthStencil, &descDSV, &pInfo->pDepthStencilView);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitBackBuffer g_pD3DDevice->CreateDepthStencilView", hr);  // 失敗

	// ビューポートの設定
	pInfo->ViewPort[0].TopLeftX = 0.0f;    // ビューポート領域の左上X座標。
	pInfo->ViewPort[0].TopLeftY = 0.0f;    // ビューポート領域の左上Y座標。
	pInfo->ViewPort[0].Width = (FLOAT)descBackBuffer.Width;   // ビューポート領域の幅
	pInfo->ViewPort[0].Height = (FLOAT)descBackBuffer.Height;  // ビューポート領域の高さ
	pInfo->ViewPort[0].MinDepth = 0.0f; // ビューポート領域の深度値の最小値
	pInfo->ViewPort[0].MaxDepth = 1.0f; // ビューポート領域の深度値の最大値

	//// 定数バッファ①を更新
	//// 射影変換行列(パースペクティブ(透視法)射影)
	//XMMATRIX mat = XMMatrixPerspectiveFovLH(
	//	XMConvertToRadians(30.0f),		// 視野角30°
	//	(float)descBackBuffer.Width / (float)descBackBuffer.Height,	// アスペクト比
	//	1.0f,							// 前方投影面までの距離
	//	20.0f);							// 後方投影面までの距離
	//mat = XMMatrixTranspose(mat);
	//XMStoreFloat4x4(&pInfo->cbNeverChanges.Projection, mat);
	//UpdateBuffer(pInfo->pImmediateContext, pInfo->pCBuffer[0], &pInfo->cbNeverChanges, sizeof(cbNeverChanges));

	//サイズを保存
	pInfo->sizeWindow.cx = descBackBuffer.Width;
	pInfo->sizeWindow.cy = descBackBuffer.Height;

	return S_OK;
}

HRESULT DirectXHelper::UpdateBuffer(ID3D11DeviceContext* pContext, ID3D11Buffer* pBuffer, void* pData, UINT dataSize)
{
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	HRESULT hr = pContext->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitBackBuffer  g_pImmediateContext->Map", hr);  // 失敗
	CopyMemory(MappedResource.pData, pData, dataSize);
	pContext->Unmap(pBuffer, 0);
	return S_OK;
}
