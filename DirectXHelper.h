#pragma once
#include "stdafx.h"
#include "ConstBuffer.h"
#include <string>
#include <memory>

class DirectXConfiguration;
struct DirectXRenderInfo;

typedef std::unique_ptr<DirectXRenderInfo, std::default_delete<DirectXRenderInfo> > PDirectXRenderInfo;

class Shader;

class DirectXHelper
{
public:
	DirectXHelper();
    virtual ~DirectXHelper();
	PDirectXRenderInfo Initialize(HINSTANCE hInst);
	bool Resize(const PDirectXRenderInfo&, const SIZE);
	bool Draw(const PDirectXRenderInfo&);
	bool Dispose(const PDirectXRenderInfo&);

	ID3D11Buffer* CreateVertexBuffer(const PDirectXRenderInfo&, const int dataCount, const int structSize, const void* pData);
	ID3D11Buffer* CreateIndexBuffer(const PDirectXRenderInfo&, const int dataCount, const UINT* pData);
	bool SetVertexBuffer(const ID3D11Buffer*);
	bool SetIndexBuffer(const ID3D11Buffer*);

	ID3D11VertexShader* LoadVertexShader(const PDirectXRenderInfo&, const std::wstring& filepath, const std::string& entryPoint);
	ID3D11GeometryShader* LoadGeometryShader(const PDirectXRenderInfo&, const std::wstring& filepath, const std::string& entryPoint);
	ID3D11PixelShader* LoadPixelShader(const PDirectXRenderInfo&, const std::wstring& filepath, const std::string& entryPoint);
	bool SetShader(const PDirectXRenderInfo&, const ID3D11VertexShader*);
	bool SetShader(const PDirectXRenderInfo&, const ID3D11GeometryShader*);
	bool SetShader(const PDirectXRenderInfo&, const ID3D11PixelShader*);

	bool SetInputLayout(const PDirectXRenderInfo&, const int count, UINT sizes[]);

	bool SetVertices(
		const PDirectXRenderInfo& pInfo,
		const int vertCount, const int structSize, const void* vertData,
		const int idxCount, const UINT* idxData);

private:
	HRESULT InitBackBuffer(const PDirectXRenderInfo&);

	HRESULT InitializeWindow(HINSTANCE hInst, HWND* phWindow);
	HRESULT GenerateWindow(const HINSTANCE hInst, const WNDPROC OnWidowEvent, const std::wstring wndTitle, const std::wstring wndName, const SIZE wndSize, HWND* pWnd);
	
	HRESULT GenerateDeviceSwapChain(const DirectXConfiguration& dxconfig, const SIZE& bufferSize, const HWND& hWindow, PDirectXRenderInfo& pinfo);
	HRESULT GenerateVertexBuffer(const int dataCount, const int structSize, const void* pData, ID3D11Device* pD3DDevice, ID3D11Buffer** ppBuffer);
	HRESULT GenerateIndexBuffer(const int dataCount, const UINT* pData, ID3D11Device* pD3DDevice, ID3D11Buffer** ppBuffer);
	HRESULT LoadVertexShader(
		const std::wstring& filepath, const std::string& entryPoint, const DirectXConfiguration& dxconfig, ID3D11Device* pD3DDevice,
		ID3DBlob** ppBlobVS, ID3D11VertexShader** ppShader);
	HRESULT LoadPixelShader(
		const std::wstring& filepath, const std::string& entryPoint, const DirectXConfiguration& dxconfig,
		ID3D11Device* pD3DDevice, ID3D11PixelShader** ppShader);
	HRESULT LoadGeometryShader(
		const std::wstring& filepath, const std::string& entryPoint, const DirectXConfiguration& dxconfig,
		ID3D11Device* pD3DDevice, ID3D11GeometryShader** ppShader);
	HRESULT GenerateConstantBuffers(const UINT bufferCount, const UINT* sizes, ID3D11Device* pD3DDevice, ID3D11Buffer** pBuffers);
	HRESULT SetBlendMode(ID3D11Device* pD3DDevice, ID3D11BlendState** ppBlendState);
	HRESULT SetRasterMode(ID3D11Device* pD3DDevice, ID3D11RasterizerState** ppRasterizerState);
	HRESULT SetDepthTest(ID3D11Device* pD3DDevice, ID3D11DepthStencilState** ppDepthStencilState);
	HRESULT UpdateBuffer(ID3D11DeviceContext* pContext, ID3D11Buffer* pBuffer, void* pData, UINT dataSize);
};
