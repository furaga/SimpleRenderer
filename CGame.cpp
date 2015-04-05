// TODO:
// FBX�̓ǂݍ���
// �R�[�l���{�b�N�X�̃����_�����O
// GI�̎���

#include "stdafx.h"
#include "CGame.h"
#include "VertexPositionColorTexture.h"
#include "DirectXHelper.h"
#include "DirectXRenderInfo.h"

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

class CGame::Impl
{
	ID3D11Buffer* pCBuffers[3];
	cbNeverChanges cbNC;
	cbChangesEveryFrame cbEF;
	cbChangesEveryObject cbEO;

public:
	Impl() { }
	~Impl() { }

	bool Initialize(DirectXHelper* pHelper, const PDirectXRenderInfo& pInfo)
	{
		int vsize = sizeof(VertexPositionColorTexture);
		int vcount = sizeof(posVertex) / vsize;
		auto vbuf = pHelper->CreateVertexBuffer(pInfo, vcount, vsize, posVertex);
		pHelper->SetVertexBuffer(pInfo, vbuf, vsize);

		int icount = sizeof(idxVertexID) / sizeof(UINT);
		auto ibuf = pHelper->CreateIndexBuffer(pInfo, icount, idxVertexID);
		pHelper->SetIndexBuffer(pInfo, ibuf);

		ID3DBlob* vBlob = NULL;
		auto vshader = pHelper->LoadVertexShader(pInfo, L"../misc/D3D11Sample05.sh", "VS", &vBlob);
		auto gshader = pHelper->LoadGeometryShader(pInfo, L"../misc/D3D11Sample05.sh", "GS", NULL);
		auto pshader = pHelper->LoadPixelShader(pInfo, L"../misc/D3D11Sample05.sh", "PS", NULL);
		pHelper->SetShader(pInfo, vshader);
		pHelper->SetShader(pInfo, gshader);
		pHelper->SetShader(pInfo, pshader);

		int lcount = sizeof(layout) / sizeof(D3D11_INPUT_ELEMENT_DESC);
		pHelper->SetInputLayout(pInfo, lcount, layout, vBlob);

		// �萔�o�b�t�@�̏�����
		pHelper->GenerateConstantBuffer(pInfo, sizeof(cbNeverChanges), &pCBuffers[0]);
		pHelper->GenerateConstantBuffer(pInfo, sizeof(cbChangesEveryFrame), &pCBuffers[1]);
		pHelper->GenerateConstantBuffer(pInfo, sizeof(cbChangesEveryObject), &pCBuffers[2]);
		
		XMMATRIX mat = XMMatrixPerspectiveFovLH(
			XMConvertToRadians(30.0f),		// ����p30��
			(float)pInfo->sizeWindow.cx / (float)pInfo->sizeWindow.cy,	// �A�X�y�N�g��
			1.0f,							// �O�����e�ʂ܂ł̋���
			20.0f);							// ������e�ʂ܂ł̋���
		mat = XMMatrixTranspose(mat);
		XMStoreFloat4x4(&cbNC.Projection, mat);

		pHelper->UpdateBuffer(pInfo, pCBuffers[0], &cbNC, sizeof(cbNC));

		return true;
	}

	bool Dispose(DirectXHelper* pHelper, const PDirectXRenderInfo& pInfo)
	{

		return true;
	}

	bool Update(DirectXHelper* pHelper, const PDirectXRenderInfo& pInfo)
	{
		// �萔�o�b�t�@�A���X�V
		// �r���[�ϊ��s��
		XMVECTORF32 eyePosition = { 0.0f, 5.0f, -5.0f, 1.0f };  // ���_(�J�����̈ʒu)
		XMVECTORF32 focusPosition = { 0.0f, 0.0f, 0.0f, 1.0f };  // �����_
		XMVECTORF32 upDirection = { 0.0f, 1.0f, 0.0f, 1.0f };  // �J�����̏����
		XMMATRIX mat = XMMatrixLookAtLH(eyePosition, focusPosition, upDirection);
		XMStoreFloat4x4(&cbEF.View, XMMatrixTranspose(mat));

		// �_�������W
		XMFLOAT3 g_vLightPos(3.0f, 3.0f, -3.0f);
		XMVECTOR vec = XMVector3TransformCoord(XMLoadFloat3(&g_vLightPos), mat);
		XMStoreFloat3(&cbEF.Light, vec);

		// �萔�o�b�t�@�B���X�V
		// ���[���h�ϊ��s��
		XMMATRIX matY, matX;
		FLOAT rotate = (FLOAT)(XM_PI * (timeGetTime() % 3000)) / 1500.0f;
		matY = XMMatrixRotationY(rotate);
		rotate = (FLOAT)(XM_PI * (timeGetTime() % 1500)) / 750.0f;
		matX = XMMatrixRotationX(rotate);
		XMStoreFloat4x4(&cbEO.World, XMMatrixTranspose(matY * matX));

		return true;
	}

	bool Draw(DirectXHelper* pHelper, const PDirectXRenderInfo& pInfo)
	{
		pHelper->BeginDraw(pInfo);

		pHelper->UpdateBuffer(pInfo, pCBuffers[1], &cbEF, sizeof(cbEF));
		pHelper->UpdateBuffer(pInfo, pCBuffers[2], &cbEO, sizeof(cbEO));

		pHelper->VSSetConstantBuffer(pInfo, 3, pCBuffers);
		pHelper->GSSetConstantBuffer(pInfo, 3, pCBuffers);
		pHelper->PSSetConstantBuffer(pInfo, 3, pCBuffers);

		pHelper->DrawPrimitives(pInfo, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, 36, 0, 0);

		bool res = pHelper->EndDraw(pInfo);
		return res;
	}

private:



};


CGame::CGame()
	: pImpl(new CGame::Impl())
{

}

CGame::~CGame() { }

bool CGame::Initialize(DirectXHelper* pHelper, const PDirectXRenderInfo& pInfo)
{
	return pImpl->Initialize(pHelper, pInfo);
}

bool CGame::Dispose(DirectXHelper* pHelper, const PDirectXRenderInfo& pInfo)
{
	return pImpl->Dispose(pHelper, pInfo);
}

bool CGame::Update(DirectXHelper* pHelper, const PDirectXRenderInfo& pInfo)
{
	return pImpl->Update(pHelper, pInfo);
}

bool CGame::Draw(DirectXHelper* pHelper, const PDirectXRenderInfo& pInfo) 
{
    return pImpl->Draw(pHelper, pInfo);
}