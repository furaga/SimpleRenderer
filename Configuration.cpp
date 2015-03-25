#include "stdafx.h"
#include "Configuration.h"

const std::wstring WindowConfiguration::WindowTitle = L"Direct3D 11 Sample05";
const std::wstring WindowConfiguration::WindowName = L"D3D11S05";
const SIZE WindowConfiguration::WindowSize = { 640, 480 };

const D3D_FEATURE_LEVEL DirectXConfiguration::FeatureLevels[3] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
const float DirectXConfiguration::ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
