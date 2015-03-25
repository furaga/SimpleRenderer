#include "stdafx.h"
#include "CGame.h"

class CGame::Impl
{
public:
	Impl() { }
	~Impl() { }

	bool Initialize(DirectXHelper* pHelper, const PDirectXRenderInfo& pInfo)
	{

		return true;
	}

	bool Dispose(DirectXHelper* pHelper, const PDirectXRenderInfo& pInfo)
	{
		return true;
	}

	bool Update(DirectXHelper* pHelper, const PDirectXRenderInfo& pInfo)
	{
		return true;
	}

};


#pragma region pImpl wrappers
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
#pragma endregion pImpl wrappers