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

	void BeginDraw(const PDirectXRenderInfo& pInfo);
	bool EndDraw(const PDirectXRenderInfo& pInfo);
	void VSSetConstantBuffer(const PDirectXRenderInfo& pInfo, const int count, ID3D11Buffer** ppCBuffer);
	void GSSetConstantBuffer(const PDirectXRenderInfo& pInfo, const int count, ID3D11Buffer** ppCBuffer);
	void PSSetConstantBuffer(const PDirectXRenderInfo& pInfo, const int count, ID3D11Buffer** ppCBuffer);
	void DrawPrimitives(const PDirectXRenderInfo& pInfo, const D3D_PRIMITIVE_TOPOLOGY primitiveType, const int icount, const int ioffset, const int voffset);
	bool Draw(const PDirectXRenderInfo&);

	bool Dispose(const PDirectXRenderInfo&, const bool disposeWindow = false);

	ID3D11Buffer* CreateVertexBuffer(const PDirectXRenderInfo&, const int dataCount, const int structSize, const void* pData);
	bool SetVertexBuffer(const PDirectXRenderInfo&, ID3D11Buffer*, const int structSize);
	ID3D11Buffer* CreateIndexBuffer(const PDirectXRenderInfo&, const int dataCount, const UINT* pData);
	bool SetIndexBuffer(const PDirectXRenderInfo&, ID3D11Buffer*);

	ID3D11VertexShader* LoadVertexShader(const PDirectXRenderInfo& pInfo, const std::wstring& filepath, const std::string& entryPoint, ID3DBlob** ppBlob);
	bool SetShader(const PDirectXRenderInfo&, ID3D11VertexShader*);
	ID3D11GeometryShader* LoadGeometryShader(const PDirectXRenderInfo& pInfo, const std::wstring& filepath, const std::string& entryPoint, ID3DBlob** ppBlob);
	bool SetShader(const PDirectXRenderInfo&, ID3D11GeometryShader*);
	ID3D11PixelShader* LoadPixelShader(const PDirectXRenderInfo& pInfo, const std::wstring& filepath, const std::string& entryPoint, ID3DBlob** ppBlob);
	bool SetShader(const PDirectXRenderInfo&, ID3D11PixelShader*);

	bool SetInputLayout(const PDirectXRenderInfo& pInfo, UINT count, D3D11_INPUT_ELEMENT_DESC layout[], ID3DBlob* pBlobVS);

	bool GenerateConstantBuffer(const PDirectXRenderInfo& pInfo, const UINT size, ID3D11Buffer** ppBuffer);
	bool UpdateBuffer(const PDirectXRenderInfo& pInfo, ID3D11Buffer* pBuffer, void* pData, const int dataSize);

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
