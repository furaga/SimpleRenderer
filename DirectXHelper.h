#pragma once
#include "stdafx.h"
#include "IGame.h"
#include "ConstBuffer.h"
#include <string>
#include <memory>

class DirectXConfiguration;
struct DirectXRenderInfo;

typedef std::shared_ptr<DirectXRenderInfo> PDirectXRenderInfo;

class DirectXHelper
{
public:
	DirectXHelper();
    virtual ~DirectXHelper();

	PDirectXRenderInfo Initialize(HINSTANCE hInst);

	bool Run(const PDirectXRenderInfo& pInfo, IGame* game);

	bool IsDeviceRemoved(const PDirectXRenderInfo& pInfo);

	bool Resize(const PDirectXRenderInfo&, const SIZE);

	bool Draw(const PDirectXRenderInfo&);

	bool Dispose(const PDirectXRenderInfo&, const bool disposeWindow = false);

	ID3D11Buffer* CreateVertexBuffer(const PDirectXRenderInfo&, const int dataCount, const int structSize, const void* pData);
	ID3D11Buffer* CreateIndexBuffer(const PDirectXRenderInfo&, const int dataCount, const UINT* pData);
	bool SetVertexBuffer(const PDirectXRenderInfo&, ID3D11Buffer*);
	bool SetIndexBuffer(const PDirectXRenderInfo&, ID3D11Buffer*);

	ID3D11VertexShader* LoadVertexShader(const PDirectXRenderInfo& pInfo, const std::wstring& filepath, const std::string& entryPoint, ID3DBlob** ppBlob);
	ID3D11GeometryShader* LoadGeometryShader(const PDirectXRenderInfo& pInfo, const std::wstring& filepath, const std::string& entryPoint, ID3DBlob** ppBlob);
	ID3D11PixelShader* LoadPixelShader(const PDirectXRenderInfo& pInfo, const std::wstring& filepath, const std::string& entryPoint, ID3DBlob** ppBlob);
	bool SetShader(const PDirectXRenderInfo&, ID3D11VertexShader*);
	bool SetShader(const PDirectXRenderInfo&, ID3D11GeometryShader*);
	bool SetShader(const PDirectXRenderInfo&, ID3D11PixelShader*);

	ID3D11InputLayout* SetInputLayout(const PDirectXRenderInfo& pInfo, UINT count, D3D11_INPUT_ELEMENT_DESC layout[], ID3DBlob* pBlobVS);

private:
	bool AppIdle(const PDirectXRenderInfo& pInfo, IGame* game);

	HRESULT InitializeWindow(HINSTANCE hInst, HWND* phWindow);
	HRESULT GenerateWindow(const HINSTANCE hInst, const WNDPROC OnWidowEvent, const std::wstring wndTitle, const std::wstring wndName, const SIZE wndSize, HWND* pWnd);

	HRESULT InitializeDirect3D(const PDirectXRenderInfo& pInfo);
	HRESULT GenerateDeviceSwapChain(const DirectXConfiguration& dxconfig, const SIZE& bufferSize, const HWND& hWindow, const PDirectXRenderInfo& pinfo);
	HRESULT GenerateConstantBuffers(const UINT bufferCount, const UINT* sizes, ID3D11Device* pD3DDevice, ID3D11Buffer** pBuffers);

	HRESULT SetBlendMode(ID3D11Device* pD3DDevice, ID3D11BlendState** ppBlendState);
	HRESULT SetRasterMode(ID3D11Device* pD3DDevice, ID3D11RasterizerState** ppRasterizerState);
	HRESULT SetDepthTest(ID3D11Device* pD3DDevice, ID3D11DepthStencilState** ppDepthStencilState);

	HRESULT InitBackBuffer(const PDirectXRenderInfo&);

	HRESULT UpdateBuffer(ID3D11DeviceContext* pContext, ID3D11Buffer* pBuffer, void* pData, UINT dataSize);
};
