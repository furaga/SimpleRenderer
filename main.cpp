#include "stdafx.h"
#include "VertexPositionColorTexture.h"
#include "DirectXHelper.h"
#include "Configuration.h"

// 頂点・インデックスバッファの初期化
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
// 入力要素
D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(XMFLOAT3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(XMFLOAT3) + sizeof(XMFLOAT3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

/*-------------------------------------------
グローバル変数(アプリケーション関連)
--------------------------------------------*/
HINSTANCE	g_hInstance = NULL;	// インスタンス ハンドル
HWND		g_hWindow = NULL;	// ウインドウ ハンドル

WCHAR		g_szAppTitle[] = L"Direct3D 11 Sample05";
WCHAR		g_szWndClass[] = L"D3D11S05";

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam);

/*-------------------------------------------
アプリケーション初期化
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

DirectXHelper* g_pDXHelper = NULL;
PDirectXRenderInfo g_pDXInfo;
DirectXConfiguration g_dxConfig;

HRESULT InitDirect3D()
{
	// ウインドウのクライアント エリア
	RECT rc;
	GetClientRect(g_hWindow, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	g_pDXHelper = new DirectXHelper();
	g_pDXInfo = g_pDXHelper->Initialize(g_hWindow, { width, height }, g_dxConfig);

	return S_OK;
}

void Update(void)
{
//	XMVECTORF32 eyePosition = { 0.0f, 5.0f, -5.0f, 1.0f };  // 視点(カメラの位置)
//	XMVECTORF32 focusPosition = { 0.0f, 0.0f, 0.0f, 1.0f };  // 注視点
//	XMVECTORF32 upDirection = { 0.0f, 1.0f, 0.0f, 1.0f };  // カメラの上方向
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
	bool res = g_pDXHelper->Draw(g_pDXInfo, g_dxConfig);
	return res ? S_OK : !S_OK;
}

/*-------------------------------------------
Direct3Dの終了処理
--------------------------------------------*/
bool CleanupDirect3D(void)
{
	return g_pDXHelper->Dispose(g_pDXInfo);
}

/*-------------------------------------------
アプリケーションの終了処理
--------------------------------------------*/
bool CleanupApp(void)
{
	// ウインドウ クラスの登録解除
	UnregisterClass(g_szWndClass, g_hInstance);
	return true;
}

/*-------------------------------------------
ウィンドウ処理
--------------------------------------------*/
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam)
{
	HRESULT hr = S_OK;
	BOOL fullscreen;

	switch (msg)
	{
	case WM_DESTROY:
		// Direct3Dの終了処理
		CleanupDirect3D();
		// ウインドウを閉じる
		PostQuitMessage(0);
		g_hWindow = NULL;
		return 0;

		// ウインドウ サイズの変更処理
	case WM_SIZE:
		if (!g_pDXInfo || !g_pDXInfo->pD3DDevice || wParam == SIZE_MINIMIZED)
			break;

		// 描画ターゲットを解除する
		g_pDXInfo->pImmediateContext->OMSetRenderTargets(0, NULL, NULL);	// 描画ターゲットの解除
		SAFE_RELEASE(g_pDXInfo->pRenderTargetView);					    // 描画ターゲット ビューの解放
		SAFE_RELEASE(g_pDXInfo->pDepthStencilView);					// 深度/ステンシル ビューの解放
		SAFE_RELEASE(g_pDXInfo->pDepthStencil);						// 深度/ステンシル テクスチャの解放

		// バッファの変更
		g_pDXInfo->pSwapChain->ResizeBuffers(3, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

		// バック バッファの初期化
		g_pDXHelper->InitBackBuffer(g_pDXInfo);
		break;

	case WM_KEYDOWN:
		// キー入力の処理
		switch (wParam)
		{
		case VK_ESCAPE:	// [ESC]キーでウインドウを閉じる
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		case VK_F2:		// [F2]キーで深度バッファのモードを切り替える
			g_pDXInfo->DepthMode = !g_pDXInfo->DepthMode;
			break;

		case VK_F5:		// [F5]キーで画面モードを切り替える
			if (g_pDXInfo->pSwapChain != NULL) {
				g_pDXInfo->pSwapChain->GetFullscreenState(&fullscreen, NULL);
				g_pDXInfo->pSwapChain->SetFullscreenState(!fullscreen, NULL);
			}
			break;
		}
		break;
	}

	// デフォルト処理
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

/*--------------------------------------------
デバイスの消失処理
--------------------------------------------*/
HRESULT IsDeviceRemoved(void)
{
	HRESULT hr;

	// デバイスの消失確認
	hr = g_pDXInfo->pD3DDevice->GetDeviceRemovedReason();
	switch (hr) {
	case S_OK:
		break;         // 正常

	case DXGI_ERROR_DEVICE_HUNG:
	case DXGI_ERROR_DEVICE_RESET:
		DXTRACE_ERR(L"IsDeviceRemoved g_pDXInfo->pD3DDevice->GetDeviceRemovedReason", hr);
		CleanupDirect3D();   // Direct3Dの解放(アプリケーション定義)
		hr = InitDirect3D();  // Direct3Dの初期化(アプリケーション定義)
		if (FAILED(hr))
			return hr; // 失敗。アプリケーションを終了
		break;

	case DXGI_ERROR_DEVICE_REMOVED:
	case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
	case DXGI_ERROR_INVALID_CALL:
	default:
		DXTRACE_ERR(L"IsDeviceRemoved g_pDXInfo->pD3DDevice->GetDeviceRemovedReason", hr);
		return hr;   // どうしようもないので、アプリケーションを終了。
	};

	return S_OK;         // 正常
}

/*--------------------------------------------
アイドル時の処理
--------------------------------------------*/
bool AppIdle(void)
{
	if (!g_pDXInfo->pD3DDevice)
		return false;

	HRESULT hr;
	// デバイスの消失処理
	hr = IsDeviceRemoved();
	if (FAILED(hr))
		return false;

	// スタンバイ モード
	if (g_pDXInfo->StandbyMode) {
		hr = g_pDXInfo->pSwapChain->Present(0, DXGI_PRESENT_TEST);
		if (hr != S_OK) {
			Sleep(100);	// 0.1秒待つ
			return true;
		}
		g_pDXInfo->StandbyMode = false; // スタンバイ モードを解除する
	}

	// 画面の更新
	Update();
	hr = Render();
	if (hr == DXGI_STATUS_OCCLUDED) {
		g_pDXInfo->StandbyMode = true;  // スタンバイ モードに入る
	}

	return true;
}

/*--------------------------------------------
メイン
---------------------------------------------*/
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int)
{
	// デバッグ ヒープ マネージャによるメモリ割り当ての追跡方法を設定
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// XNA Mathライブラリのサポート チェック
	if (XMVerifyCPUSupport() != TRUE)
	{
		DXTRACE_MSG(L"WinMain XMVerifyCPUSupport");
		return 0;
	}

	// アプリケーションに関する初期化
	HRESULT hr = InitializeWindow(hInst);
	if (FAILED(hr))
	{
		DXTRACE_ERR(L"WinMain InitializeWindow", hr);
		return 0;
	}

	// Direct3Dの初期化
	hr = InitDirect3D();
	if (FAILED(hr)) {
		DXTRACE_ERR(L"WinMain InitDirect3D", hr);
		CleanupDirect3D();
		CleanupApp();
		return 0;
	}

	// メッセージ ループ
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
			// アイドル処理
			if (!AppIdle())
				// エラーがある場合，アプリケーションを終了する
				DestroyWindow(g_hWindow);
		}
	} while (msg.message != WM_QUIT);

	// アプリケーションの終了処理
	CleanupApp();

	return (int)msg.wParam;
}
