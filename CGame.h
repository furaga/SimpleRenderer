#pragma once

#include "stdafx.h"
#include "IGame.h"

class CGame : public IGame
{
public:
	CGame();
	~CGame();
	bool Initialize(DirectXHelper*, const PDirectXRenderInfo&);
	bool Dispose(DirectXHelper*, const PDirectXRenderInfo&);
	bool Update(DirectXHelper*, const PDirectXRenderInfo&);
private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};
