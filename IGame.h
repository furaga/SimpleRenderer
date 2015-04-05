#pragma once
#include "stdafx.h"
#include <memory>
class DirectXHelper;
struct DirectXRenderInfo;
typedef std::shared_ptr<DirectXRenderInfo> PDirectXRenderInfo;

class IGame
{
public:
	virtual bool Initialize(DirectXHelper*, const PDirectXRenderInfo&) = 0;
	virtual bool Dispose(DirectXHelper*, const PDirectXRenderInfo&) = 0;
	virtual bool Update(DirectXHelper*, const PDirectXRenderInfo&) = 0;
	virtual bool Draw(DirectXHelper*, const PDirectXRenderInfo&) = 0;
};