#include "stdafx.h"
#include "VertexPositionColorTexture.h"
#include "DirectXHelper.h"
#include "Configuration.h"
#include "CGame.h"

/*--------------------------------------------
メイン
---------------------------------------------*/
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int)
{
	// デバッグ ヒープ マネージャによるメモリ割り当ての追跡方法を設定
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// XNA Mathライブラリのサポート チェック
	if (XMVerifyCPUSupport() != TRUE)
	{
		DXTRACE_MSG(L"WinMain XMVerifyCPUSupport");
		return 0;
	}

	auto pHelper = new DirectXHelper();
	auto pInfo = pHelper->Initialize(hInst);
	auto pGame = new CGame();
	pGame->Initialize(pHelper, pInfo);

	bool res = pHelper->Run(pInfo, pGame);

	pGame->Dispose(pHelper, pInfo);
	pHelper->Dispose(pInfo);
	SAFE_DELETE(pGame);
	SAFE_DELETE(pHelper);

	return 0;
}
