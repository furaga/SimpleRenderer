#pragma once

#include "stdafx.h"
#include "ConstBuffer.h"
#include "Configuration.h"

// directxのコンテキストやバッファなど、描画に必要なデータ
struct DirectXRenderInfo
{
	DirectXConfiguration dxconfig;

	HINSTANCE hInst = NULL;
	HWND hWindow = NULL;

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

	SIZE sizeWindow;

public:
	DirectXRenderInfo()
	{
		for (int i = 0; i < 2; i++)
			pVerBuffer[i] = NULL;
		for (int i = 0; i < 3; i++)
			pCBuffer[i] = NULL;
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