#include "stdafx.h"
#include "VertexPositionColorTexture.h"
#include "DirectXHelper.h"
#include "Configuration.h"
#include "CGame.h"

/*--------------------------------------------
���C��
---------------------------------------------*/
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int)
{
	// �f�o�b�O �q�[�v �}�l�[�W���ɂ�郁�������蓖�Ă̒ǐՕ��@��ݒ�
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// XNA Math���C�u�����̃T�|�[�g �`�F�b�N
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
