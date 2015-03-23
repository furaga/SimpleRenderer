#pragma once
#include "stdafx.h"
#include <string>
#include <memory>

class DirectXConfiguration;

class DirectXHelper
{
public:
	class DirectXRenderInfo;
	DirectXHelper();
    ~DirectXHelper();
	std::unique_ptr<DirectXRenderInfo> Initialize(HWND hWindow, SIZE wndSize, DirectXConfiguration dxconfig);
	bool Draw(std::unique_ptr<DirectXRenderInfo>);
	bool Dispose(std::unique_ptr<DirectXRenderInfo>);
private:
	HRESULT GenerateDeviceSwapChain(const DirectXConfiguration& dxconfig, const SIZE& bufferSize, const HWND& hWindow, std::unique_ptr<DirectXRenderInfo>& pinfo);
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

};
