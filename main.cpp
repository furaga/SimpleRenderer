#include "stdafx.h"
#include "VertexPositionColorTexture.h"
#include "DirectXHelper.h"
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

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam);

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

DirectXHelper* g_pDXHelper = NULL;
PDirectXRenderInfo g_pDXInfo;
DirectXConfiguration g_dxConfig;

HRESULT InitDirect3D()
{
	// �E�C���h�E�̃N���C�A���g �G���A
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
	bool res = g_pDXHelper->Draw(g_pDXInfo, g_dxConfig);
	return res ? S_OK : !S_OK;
}

/*-------------------------------------------
Direct3D�̏I������
--------------------------------------------*/
bool CleanupDirect3D(void)
{
	return g_pDXHelper->Dispose(g_pDXInfo);
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
		if (!g_pDXInfo || !g_pDXInfo->pD3DDevice || wParam == SIZE_MINIMIZED)
			break;

		// �`��^�[�Q�b�g����������
		g_pDXInfo->pImmediateContext->OMSetRenderTargets(0, NULL, NULL);	// �`��^�[�Q�b�g�̉���
		SAFE_RELEASE(g_pDXInfo->pRenderTargetView);					    // �`��^�[�Q�b�g �r���[�̉��
		SAFE_RELEASE(g_pDXInfo->pDepthStencilView);					// �[�x/�X�e���V�� �r���[�̉��
		SAFE_RELEASE(g_pDXInfo->pDepthStencil);						// �[�x/�X�e���V�� �e�N�X�`���̉��

		// �o�b�t�@�̕ύX
		g_pDXInfo->pSwapChain->ResizeBuffers(3, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

		// �o�b�N �o�b�t�@�̏�����
		g_pDXHelper->InitBackBuffer(g_pDXInfo);
		break;

	case WM_KEYDOWN:
		// �L�[���͂̏���
		switch (wParam)
		{
		case VK_ESCAPE:	// [ESC]�L�[�ŃE�C���h�E�����
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		case VK_F2:		// [F2]�L�[�Ő[�x�o�b�t�@�̃��[�h��؂�ւ���
			g_pDXInfo->DepthMode = !g_pDXInfo->DepthMode;
			break;

		case VK_F5:		// [F5]�L�[�ŉ�ʃ��[�h��؂�ւ���
			if (g_pDXInfo->pSwapChain != NULL) {
				g_pDXInfo->pSwapChain->GetFullscreenState(&fullscreen, NULL);
				g_pDXInfo->pSwapChain->SetFullscreenState(!fullscreen, NULL);
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
	hr = g_pDXInfo->pD3DDevice->GetDeviceRemovedReason();
	switch (hr) {
	case S_OK:
		break;         // ����

	case DXGI_ERROR_DEVICE_HUNG:
	case DXGI_ERROR_DEVICE_RESET:
		DXTRACE_ERR(L"IsDeviceRemoved g_pDXInfo->pD3DDevice->GetDeviceRemovedReason", hr);
		CleanupDirect3D();   // Direct3D�̉��(�A�v���P�[�V������`)
		hr = InitDirect3D();  // Direct3D�̏�����(�A�v���P�[�V������`)
		if (FAILED(hr))
			return hr; // ���s�B�A�v���P�[�V�������I��
		break;

	case DXGI_ERROR_DEVICE_REMOVED:
	case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
	case DXGI_ERROR_INVALID_CALL:
	default:
		DXTRACE_ERR(L"IsDeviceRemoved g_pDXInfo->pD3DDevice->GetDeviceRemovedReason", hr);
		return hr;   // �ǂ����悤���Ȃ��̂ŁA�A�v���P�[�V�������I���B
	};

	return S_OK;         // ����
}

/*--------------------------------------------
�A�C�h�����̏���
--------------------------------------------*/
bool AppIdle(void)
{
	if (!g_pDXInfo->pD3DDevice)
		return false;

	HRESULT hr;
	// �f�o�C�X�̏�������
	hr = IsDeviceRemoved();
	if (FAILED(hr))
		return false;

	// �X�^���o�C ���[�h
	if (g_pDXInfo->StandbyMode) {
		hr = g_pDXInfo->pSwapChain->Present(0, DXGI_PRESENT_TEST);
		if (hr != S_OK) {
			Sleep(100);	// 0.1�b�҂�
			return true;
		}
		g_pDXInfo->StandbyMode = false; // �X�^���o�C ���[�h����������
	}

	// ��ʂ̍X�V
	Update();
	hr = Render();
	if (hr == DXGI_STATUS_OCCLUDED) {
		g_pDXInfo->StandbyMode = true;  // �X�^���o�C ���[�h�ɓ���
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
