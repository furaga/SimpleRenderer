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
		// �E�C���h�E�����
		PostQuitMessage(0);
		// Direct3D�̏I������
		if (g_pDXInfo != NULL && (*g_pDXInfo) != nullptr && g_pDXHelper != NULL)
			g_pDXHelper->Dispose(*g_pDXInfo);
		if (g_pDXInfo != NULL && (*g_pDXInfo) != nullptr)
			(*g_pDXInfo)->hWindow = NULL;
		return 0;

	case WM_SIZE:
		// �E�C���h�E �T�C�Y�̕ύX����
		if (!g_pDXInfo || !(*g_pDXInfo)->pD3DDevice || wParam == SIZE_MINIMIZED)
			break;
		g_pDXHelper->Resize(*g_pDXInfo, { LOWORD(lParam), HIWORD(lParam) });
		break;

	case WM_KEYDOWN:
		// �L�[���͂̏���
		switch (wParam)
		{
		case VK_ESCAPE:	// [ESC]�L�[�ŃE�C���h�E�����
			PostQuitMessage(0);
			break;

		case VK_F2:		// [F2]�L�[�Ő[�x�o�b�t�@�̃��[�h��؂�ւ���
			(*g_pDXInfo)->DepthMode = !(*g_pDXInfo)->DepthMode;
			break;

		case VK_F5:		// [F5]�L�[�ŉ�ʃ��[�h��؂�ւ���
			if ((*g_pDXInfo)->pSwapChain != NULL) {
				(*g_pDXInfo)->pSwapChain->GetFullscreenState(&fullscreen, NULL);
				(*g_pDXInfo)->pSwapChain->SetFullscreenState(!fullscreen, NULL);
			}
			break;
		}
		break;
	}

	// �f�t�H���g����
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
	// �f�o�C�X�̏�������
	hr = IsDeviceRemoved(pInfo);
	if (!hr)
		return false;

	// �X�^���o�C ���[�h
	if (pInfo->StandbyMode) {
		hr = pInfo->pSwapChain->Present(0, DXGI_PRESENT_TEST);
		if (hr != S_OK) {
			Sleep(100);	// 0.1�b�҂�
			return true;
		}
		pInfo->StandbyMode = false; // �X�^���o�C ���[�h����������
	}

	// ��ʂ̍X�V
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

	// �f�o�C�X�̏����m�F
	hr = pInfo->pD3DDevice->GetDeviceRemovedReason();
	switch (hr) {
	case S_OK:
		break;         // ����

	case DXGI_ERROR_DEVICE_HUNG:
	case DXGI_ERROR_DEVICE_RESET:
		DXTRACE_ERR(L"IsDeviceRemoved g_pDXInfo->pD3DDevice->GetDeviceRemovedReason", hr);
		Dispose(pInfo);   // Direct3D�̉��(�A�v���P�[�V������`)
		hr = InitializeDirect3D(pInfo);  // Direct3D�̏�����(�A�v���P�[�V������`)
		if (FAILED(hr))
			return false; // ���s�B�A�v���P�[�V�������I��
		break;

	case DXGI_ERROR_DEVICE_REMOVED:
	case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
	case DXGI_ERROR_INVALID_CALL:
	default:
		DXTRACE_ERR(L"IsDeviceRemoved g_pDXInfo->pD3DDevice->GetDeviceRemovedReason", hr);
		return false;   // �ǂ����悤���Ȃ��̂ŁA�A�v���P�[�V�������I���B
	};

	return true;
}

bool DirectXHelper::Resize(const PDirectXRenderInfo& pInfo, const SIZE size)
{
	// �`��^�[�Q�b�g����������
	pInfo->pImmediateContext->OMSetRenderTargets(0, NULL, NULL);	// �`��^�[�Q�b�g�̉���
	SAFE_RELEASE(pInfo->pRenderTargetView);					    // �`��^�[�Q�b�g �r���[�̉��
	SAFE_RELEASE(pInfo->pDepthStencilView);					// �[�x/�X�e���V�� �r���[�̉��
	SAFE_RELEASE(pInfo->pDepthStencil);						// �[�x/�X�e���V�� �e�N�X�`���̉��

	// �o�b�t�@�̕ύX
	pInfo->pSwapChain->ResizeBuffers(3, size.cx, size.cy, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	// �o�b�N �o�b�t�@�̏�����
	g_pDXHelper->InitBackBuffer(pInfo);

	return true;
}

void DirectXHelper::BeginDraw(const PDirectXRenderInfo& pInfo)
{
	// �`��^�[�Q�b�g�̃N���A
	auto ctx = pInfo->pImmediateContext;
	auto rtv = pInfo->pRenderTargetView;

	pInfo->pImmediateContext->ClearRenderTargetView(
		pInfo->pRenderTargetView, // �N���A����`��^�[�Q�b�g
		pInfo->dxconfig.ClearColor);         // �N���A����l

	// �[�x/�X�e���V���̃N���A
	pInfo->pImmediateContext->ClearDepthStencilView(
		pInfo->pDepthStencilView, // �N���A����[�x/�X�e���V���E�r���[
		D3D11_CLEAR_DEPTH,   // �[�x�l�������N���A����
		1.0f,                // �[�x�o�b�t�@���N���A����l
		0);                  // �X�e���V���E�o�b�t�@���N���A����l(���̏ꍇ�A���֌W)

	// IA�ɒ��_�o�b�t�@��ݒ�
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
		pInfo->StandbyMode = true;  // �X�^���o�C ���[�h�ɓ���
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
		icount, // �`�悷��C���f�b�N�X��(���_��)
		ioffset,  // �C���f�b�N�X�E�o�b�t�@�̍ŏ��̃C���f�b�N�X����`��J�n
		voffset); // ���_�o�b�t�@���̍ŏ��̒��_�f�[�^����g�p�J�n
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

	// �f�o�C�X�E�X�e�[�g�̃N���A
	if (pInfo->pImmediateContext)
		pInfo->pImmediateContext->ClearState();

	// �X���b�v �`�F�C�����E�C���h�E ���[�h�ɂ���
	if (pInfo->pSwapChain)
		pInfo->pSwapChain->SetFullscreenState(FALSE, NULL);

	// �擾�����C���^�[�t�F�C�X�̊J��
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

	// ���_�V�F�[�_�̍쐬
	hr = pInfo->pD3DDevice->CreateVertexShader(
		pBlob->GetBufferPointer(), // �o�C�g�E�R�[�h�ւ̃|�C���^
		pBlob->GetBufferSize(),    // �o�C�g�E�R�[�h�̒���
		NULL,
		&pShader); // ���_�V�F�[�_���󂯎��ϐ�

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

	// ���_�V�F�[�_�̍쐬
	hr = pInfo->pD3DDevice->CreateGeometryShader(
		pBlob->GetBufferPointer(), // �o�C�g�E�R�[�h�ւ̃|�C���^
		pBlob->GetBufferSize(),    // �o�C�g�E�R�[�h�̒���
		NULL,
		&pShader); // ���_�V�F�[�_���󂯎��ϐ�

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

	// ���_�V�F�[�_�̍쐬
	hr = pInfo->pD3DDevice->CreatePixelShader(
		pBlob->GetBufferPointer(), // �o�C�g�E�R�[�h�ւ̃|�C���^
		pBlob->GetBufferSize(),    // �o�C�g�E�R�[�h�̒���
		NULL,
		&pShader); // ���_�V�F�[�_���󂯎��ϐ�

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
	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;    // ���I(�_�C�i�~�b�N)�g�p�@
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // �萔�o�b�t�@
	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;     // CPU���珑������
	cBufferDesc.MiscFlags = 0;
	cBufferDesc.StructureByteStride = 0;
	cBufferDesc.ByteWidth = size; // �o�b�t�@�E�T�C�Y

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

// �E�C���h�E�����E�\��
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

HRESULT DirectXHelper::InitBackBuffer(const PDirectXRenderInfo& pInfo)
{
	HRESULT hr;

	// �X���b�v�E�`�F�C������ŏ��̃o�b�N�E�o�b�t�@���擾����
	ID3D11Texture2D *pBackBuffer;
	hr = pInfo->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitBackBuffer g_pSwapChain->GetBuffer", hr);  // ���s

	// �o�b�N�E�o�b�t�@�̏��
	D3D11_TEXTURE2D_DESC descBackBuffer;
	pBackBuffer->GetDesc(&descBackBuffer);
	hr = pInfo->pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &pInfo->pRenderTargetView);
	SAFE_RELEASE(pBackBuffer);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitBackBuffer g_pD3DDevice->CreateRenderTargetView", hr);  // ���s

	// �[�x/�X�e���V���E�e�N�X�`���̍쐬
	D3D11_TEXTURE2D_DESC descDepth = descBackBuffer;
	descDepth.MipLevels = 1;       // �~�b�v�}�b�v�E���x����
	descDepth.ArraySize = 1;       // �z��T�C�Y
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;  // �t�H�[�}�b�g(�[�x�̂�)
	descDepth.Usage = D3D11_USAGE_DEFAULT;      // �f�t�H���g�g�p�@
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL; // �[�x/�X�e���V���Ƃ��Ďg�p
	descDepth.CPUAccessFlags = 0;   // CPU����̓A�N�Z�X���Ȃ�
	descDepth.MiscFlags = 0;   // ���̑��̐ݒ�Ȃ�
	hr = pInfo->pD3DDevice->CreateTexture2D(&descDepth, NULL, &pInfo->pDepthStencil);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitBackBuffer g_pD3DDevice->CreateTexture2D", hr);  // ���s

	// �[�x/�X�e���V�� �r���[�̍쐬
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDepth.Format;            // �r���[�̃t�H�[�}�b�g
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Flags = 0;
	descDSV.Texture2D.MipSlice = 0;
	hr = pInfo->pD3DDevice->CreateDepthStencilView(pInfo->pDepthStencil, &descDSV, &pInfo->pDepthStencilView);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitBackBuffer g_pD3DDevice->CreateDepthStencilView", hr);  // ���s

	// �r���[�|�[�g�̐ݒ�
	pInfo->ViewPort[0].TopLeftX = 0.0f;    // �r���[�|�[�g�̈�̍���X���W�B
	pInfo->ViewPort[0].TopLeftY = 0.0f;    // �r���[�|�[�g�̈�̍���Y���W�B
	pInfo->ViewPort[0].Width = (FLOAT)descBackBuffer.Width;   // �r���[�|�[�g�̈�̕�
	pInfo->ViewPort[0].Height = (FLOAT)descBackBuffer.Height;  // �r���[�|�[�g�̈�̍���
	pInfo->ViewPort[0].MinDepth = 0.0f; // �r���[�|�[�g�̈�̐[�x�l�̍ŏ��l
	pInfo->ViewPort[0].MaxDepth = 1.0f; // �r���[�|�[�g�̈�̐[�x�l�̍ő�l

	//// �萔�o�b�t�@�@���X�V
	//// �ˉe�ϊ��s��(�p�[�X�y�N�e�B�u(�����@)�ˉe)
	//XMMATRIX mat = XMMatrixPerspectiveFovLH(
	//	XMConvertToRadians(30.0f),		// ����p30��
	//	(float)descBackBuffer.Width / (float)descBackBuffer.Height,	// �A�X�y�N�g��
	//	1.0f,							// �O�����e�ʂ܂ł̋���
	//	20.0f);							// ������e�ʂ܂ł̋���
	//mat = XMMatrixTranspose(mat);
	//XMStoreFloat4x4(&pInfo->cbNeverChanges.Projection, mat);
	//UpdateBuffer(pInfo->pImmediateContext, pInfo->pCBuffer[0], &pInfo->cbNeverChanges, sizeof(cbNeverChanges));

	//�T�C�Y��ۑ�
	pInfo->sizeWindow.cx = descBackBuffer.Width;
	pInfo->sizeWindow.cy = descBackBuffer.Height;

	return S_OK;
}

HRESULT DirectXHelper::UpdateBuffer(ID3D11DeviceContext* pContext, ID3D11Buffer* pBuffer, void* pData, UINT dataSize)
{
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	HRESULT hr = pContext->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	if (FAILED(hr))
		return DXTRACE_ERR(L"InitBackBuffer  g_pImmediateContext->Map", hr);  // ���s
	CopyMemory(MappedResource.pData, pData, dataSize);
	pContext->Unmap(pBuffer, 0);
	return S_OK;
}
