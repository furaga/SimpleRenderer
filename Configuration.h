#include "stdafx.h"

#include <string>
#include "ConstBuffer.h"

// 各種設定を格納するクラス
struct Configuration
{
};


struct WindowConfiguration : public Configuration
{
	static const std::wstring WindowTitle;
	static const std::wstring WindowName;
	static const SIZE WindowSize;
};

const std::wstring WindowConfiguration::WindowTitle = L"Direct3D 11 Sample05";
const std::wstring WindowConfiguration::WindowName = L"D3D11S05";
const SIZE WindowConfiguration::WindowSize = { 640, 480 };


struct DirectXConfiguration : public Configuration
{
	static const D3D_FEATURE_LEVEL FeatureLevels[3];
	static const UINT FeatureLevelCount = 3;
#if defined(DEBUG) || defined(_DEBUG)
	static const UINT FlagCompile = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION | D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR;
#else
	static const UINT FlagCompile = D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR;
#endif
	static const float ClearColor[4];

	static const UINT BufferCount = 3;
	// サイズはウインドウサイズに合わせる（Auto fitting）
	// static const SIZE BufferSize;
	static const DXGI_FORMAT BufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const UINT RefreshRateNumerator = 60;
	static const UINT RefreshRateDenominator = 1;
	static const DXGI_MODE_SCANLINE_ORDER BufferScanOrder = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	static const DXGI_MODE_SCALING BufferScanScaling = DXGI_MODE_SCALING_CENTERED;
	static const DXGI_USAGE BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	static const UINT SampleCount = 1;
	static const UINT SampleQuality = 0;
	static const UINT IsWindowMode = TRUE;
	static const DXGI_SWAP_CHAIN_FLAG SwapChainFlg = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	static const D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;
};


const D3D_FEATURE_LEVEL DirectXConfiguration::FeatureLevels[3] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
const float DirectXConfiguration::ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
