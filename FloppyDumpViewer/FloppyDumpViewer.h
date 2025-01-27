
// FloppyDumpViewer.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CFloppyDumpViewerApp:
// このクラスの実装については、FloppyDumpViewer.cpp を参照してください
//

class CFloppyDumpViewerApp : public CWinApp
{
public:
	CFloppyDumpViewerApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CFloppyDumpViewerApp theApp;
