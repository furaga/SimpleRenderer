#include "stdafx.h"
#include "VertexPositionColorTexture.h"
#include "DirectXHelper.h"
#include "Configuration.h"
#include "CGame.h"

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

	auto pHelper = new DirectXHelper();
	auto pInfo = pHelper->Initialize(hInst);
	auto pGame = new CGame();
	pGame->Initialize(pHelper, pInfo);

	bool res = pHelper->Run(pInfo, pGame);

	pGame->Dispose(pHelper, pInfo);
	pHelper->Dispose(pInfo);
	SAFE_DELETE(pGame);
	SAFE_DELETE(pHelper);

	return 0;
}
