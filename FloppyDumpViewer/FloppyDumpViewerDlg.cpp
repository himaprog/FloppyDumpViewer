
// FloppyDumpViewerDlg.cpp : 実装ファイル
//

#include "pch.h"
#include "framework.h"
#include "FloppyDumpViewer.h"
#include "FloppyDumpViewerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

// 実装
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CFloppyDumpViewerDlg ダイアログ



static LPCTSTR g_strIndexNameSet[] = { _T("全体"), _T("ＭＢＲ"), _T("ＦＡＴ １"), _T("ＦＡＴ ２"), _T("ルート・ディレクトリ"), _T("データ領域") };
static LPCTSTR g_strCharSet[] = { _T("ASCII"), _T("Shift-JIS") };

static const size_t g_nSectorSize = 512;


CFloppyDumpViewerDlg::CFloppyDumpViewerDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_FLOPPYDUMPVIEWER_DIALOG, pParent)
	, m_oDiskInfo{ 0 }
	, m_oIndexInfo { 0 }
	, m_nCharSet(CHARSET_ASCII)
	, m_pbySectorBuf(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFloppyDumpViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMB_DRIVE, m_ctrlCmbDrive);
	DDX_Control(pDX, IDC_BTN_DRIVEGET, m_ctrlBtnDriveget);
	DDX_Control(pDX, IDC_BTN_DRIVEOPEN, m_ctrlBtnDriveopen);
	DDX_Control(pDX, IDC_BTN_DRIVECLOSE, m_ctrlBtnDriveclose);
	DDX_Control(pDX, IDC_EDT_DISKTYPE, m_ctrlEdtDisktype);
	DDX_Control(pDX, IDC_EDT_SECPERTRC, m_ctrlEdtSecpertrc);
	DDX_Control(pDX, IDC_EDT_TOTALSEC, m_ctrlEdtTotalsec);
	DDX_Control(pDX, IDC_EDT_VLABEL, m_ctrlEdtVlabel);
	DDX_Control(pDX, IDC_EDT_VNUMBER, m_ctrlEdtVnumber);
	DDX_Control(pDX, IDC_TRE_INDEX, m_ctrlTreIndex);
	DDX_Control(pDX, IDC_EDT_CURSEC, m_ctrlEdtCursec);
	DDX_Control(pDX, IDC_BTN_SEC_G, m_ctrlBtnSecG);
	DDX_Control(pDX, IDC_EDT_SECRNG, m_ctrlEdtSecrng);
	DDX_Control(pDX, IDC_BTN_SEC_F, m_ctrlBtnSecF);
	DDX_Control(pDX, IDC_BTN_SEC_P, m_ctrlBtnSecP);
	DDX_Control(pDX, IDC_BTN_SEC_N, m_ctrlBtnSecN);
	DDX_Control(pDX, IDC_BTN_SEC_L, m_ctrlBtnSecL);
	DDX_Control(pDX, IDC_EDT_HEXDUMP, m_ctrlEdtHexdump);
	DDX_Control(pDX, IDC_CMB_CHARSET, m_ctrlCmbCharset);
}

BEGIN_MESSAGE_MAP(CFloppyDumpViewerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCEL, &CFloppyDumpViewerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_DRIVEGET, &CFloppyDumpViewerDlg::OnBnClickedBtnDriveget)
	ON_BN_CLICKED(IDC_BTN_DRIVEOPEN, &CFloppyDumpViewerDlg::OnBnClickedBtnDriveopen)
	ON_BN_CLICKED(IDC_BTN_DRIVECLOSE, &CFloppyDumpViewerDlg::OnBnClickedBtnDriveclose)
	ON_BN_CLICKED(IDC_BTN_SEC_G, &CFloppyDumpViewerDlg::OnBnClickedBtnSecG)
	ON_BN_CLICKED(IDC_BTN_SEC_F, &CFloppyDumpViewerDlg::OnBnClickedBtnSecF)
	ON_BN_CLICKED(IDC_BTN_SEC_P, &CFloppyDumpViewerDlg::OnBnClickedBtnSecP)
	ON_BN_CLICKED(IDC_BTN_SEC_N, &CFloppyDumpViewerDlg::OnBnClickedBtnSecN)
	ON_BN_CLICKED(IDC_BTN_SEC_L, &CFloppyDumpViewerDlg::OnBnClickedBtnSecL)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TRE_INDEX, &CFloppyDumpViewerDlg::OnTvnSelchangedTreIndex)
	ON_CBN_SELCHANGE(IDC_CMB_CHARSET, &CFloppyDumpViewerDlg::OnCbnSelchangeCmbCharset)
END_MESSAGE_MAP()


// CFloppyDumpViewerDlg メッセージ ハンドラー

BOOL CFloppyDumpViewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	// TODO: 初期化をここに追加します。
	m_oDiskInfo.hDevice = INVALID_HANDLE_VALUE;
	m_pbySectorBuf = new BYTE[g_nSectorSize];
	memset(m_pbySectorBuf, 0, g_nSectorSize);

	ASSERT(_countof(m_oIndexInfo) == _countof(g_strIndexNameSet));
	for (int i = 0; i < _countof(m_oIndexInfo); i++)
	{
		m_oIndexInfo[i].strName = g_strIndexNameSet[i];
	}

	for (int i = 0; i < _countof(g_strCharSet); i++)
	{
		m_ctrlCmbCharset.InsertString(i, g_strCharSet[i]);
	}
	m_ctrlCmbCharset.SetCurSel(0);

	m_oHexFont.CreateFont(0, 9, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE/*DEFAULT_PITCH*/,
		_T("ＭＳ ゴシック"));
	m_ctrlEdtHexdump.SetFont(&m_oHexFont);

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

void CFloppyDumpViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CFloppyDumpViewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CFloppyDumpViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CFloppyDumpViewerDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
	if (m_oDiskInfo.hDevice != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_oDiskInfo.hDevice);
	}

	if (m_pbySectorBuf != nullptr)
	{
		delete[] m_pbySectorBuf;
	}
}

void CFloppyDumpViewerDlg::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	if (MessageBox(_T("終了しますか？"), nullptr, MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		CDialog::OnCancel();
	}
}

void CFloppyDumpViewerDlg::OnBnClickedBtnDriveget()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	DWORD nDrives = GetLogicalDrives();

	for (int i = m_ctrlCmbDrive.GetCount() - 1; i >= 0; i--)
	{
		m_ctrlCmbDrive.DeleteString(i);
	}

	if ((nDrives & 0x03) != 0)
	{
		CString sDriveName;

		for (unsigned i = 0; i < 2; i++)
		{
			if (nDrives & (1 << i))
			{
				sDriveName.Format(_T("%c:"), _T('A') + i);
				m_ctrlCmbDrive.InsertString(m_ctrlCmbDrive.GetCount(), sDriveName);
			}
		}

		m_ctrlCmbDrive.EnableWindow();
		m_ctrlCmbDrive.SetCurSel(0);
		m_ctrlBtnDriveopen.EnableWindow();
	}
	else
	{
		m_ctrlCmbDrive.EnableWindow(FALSE);
		m_ctrlBtnDriveget.EnableWindow();
	}
}

void CFloppyDumpViewerDlg::OnBnClickedBtnDriveopen()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	if (ReadDiskInfo())
	{
		UpdateAllUi();
	}
}

void CFloppyDumpViewerDlg::OnBnClickedBtnDriveclose()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	if (m_oDiskInfo.hDevice != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_oDiskInfo.hDevice);
		m_oDiskInfo.hDevice = INVALID_HANDLE_VALUE;
	}
	UpdateAllUi();
}

void CFloppyDumpViewerDlg::OnBnClickedBtnSecG()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString oStr;
	int nSector;
	BOOL bReadDisk = TRUE;

	m_ctrlEdtCursec.GetWindowText(oStr);
	nSector = _tcstoul(oStr.GetString(), nullptr, 10);

	if (nSector < m_oDiskInfo.oCurrentSectorRange.nFirstSector || nSector > m_oDiskInfo.oCurrentSectorRange.nLaseSector)
	{
		MessageBox(_T("入力された値が正しくありません。"), NULL, MB_ICONERROR | MB_OK);
		oStr.Format(_T("%d"), m_oDiskInfo.nCurrentSector);
		m_ctrlEdtCursec.SetWindowText(oStr);
		return;
	}

	if (nSector == m_oDiskInfo.nCurrentSector)
	{
		bReadDisk = FALSE;
	}

	m_oDiskInfo.nCurrentSector = nSector;
	UpdateHexView(bReadDisk);
}

void CFloppyDumpViewerDlg::OnBnClickedBtnSecF()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString oStr;
	BOOL bReadDisk = TRUE;

	if (m_oDiskInfo.nCurrentSector == m_oDiskInfo.oCurrentSectorRange.nFirstSector)
	{
		bReadDisk = FALSE;
	}
	else
	{
		m_oDiskInfo.nCurrentSector = m_oDiskInfo.oCurrentSectorRange.nFirstSector;
	}

	oStr.Format(_T("%d"), m_oDiskInfo.nCurrentSector);
	m_ctrlEdtCursec.SetWindowText(oStr.GetString());

	UpdateHexView(bReadDisk);
}

void CFloppyDumpViewerDlg::OnBnClickedBtnSecP()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString oStr;
	BOOL bReadDisk = TRUE;

	if (m_oDiskInfo.nCurrentSector == m_oDiskInfo.oCurrentSectorRange.nFirstSector)
	{
		bReadDisk = FALSE;
	}
	else
	{
		m_oDiskInfo.nCurrentSector--;
	}

	oStr.Format(_T("%d"), m_oDiskInfo.nCurrentSector);
	m_ctrlEdtCursec.SetWindowText(oStr.GetString());

	UpdateHexView(bReadDisk);
}

void CFloppyDumpViewerDlg::OnBnClickedBtnSecN()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString oStr;
	BOOL bReadDisk = TRUE;

	if (m_oDiskInfo.nCurrentSector == m_oDiskInfo.oCurrentSectorRange.nLaseSector)
	{
		bReadDisk = FALSE;
	}
	else
	{
		m_oDiskInfo.nCurrentSector++;
	}

	oStr.Format(_T("%d"), m_oDiskInfo.nCurrentSector);
	m_ctrlEdtCursec.SetWindowText(oStr.GetString());

	UpdateHexView(bReadDisk);
}

void CFloppyDumpViewerDlg::OnBnClickedBtnSecL()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString oStr;
	BOOL bReadDisk = TRUE;

	if (m_oDiskInfo.nCurrentSector == m_oDiskInfo.oCurrentSectorRange.nLaseSector)
	{
		bReadDisk = FALSE;
	}
	else
	{
		m_oDiskInfo.nCurrentSector = m_oDiskInfo.oCurrentSectorRange.nLaseSector;
	}

	oStr.Format(_T("%d"), m_oDiskInfo.nCurrentSector);
	m_ctrlEdtCursec.SetWindowText(oStr.GetString());

	UpdateHexView(bReadDisk);
}

void CFloppyDumpViewerDlg::OnTvnSelchangedTreIndex(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	HTREEITEM hSelectedItem = pNMTreeView->itemNew.hItem;
	CString oItemText = m_ctrlTreIndex.GetItemText(hSelectedItem);

	int nIdx = 0;

	for (; nIdx < _countof(g_strIndexNameSet); nIdx++)
	{
		if (oItemText.Compare(g_strIndexNameSet[nIdx]) == 0)
		{
			break;
		}
	}

	switch (nIdx)
	{
		case 0:  // 全体
			m_oDiskInfo.oCurrentSectorRange.nFirstSector = m_oDiskInfo.oSectorInfoMbr.nFirstSector;
			m_oDiskInfo.oCurrentSectorRange.nLaseSector = m_oDiskInfo.oSectorInfoData.nLaseSector;
			break;

		case 1:  // ＭＢＲ
			m_oDiskInfo.oCurrentSectorRange.nFirstSector = m_oDiskInfo.oSectorInfoMbr.nFirstSector;
			m_oDiskInfo.oCurrentSectorRange.nLaseSector = m_oDiskInfo.oSectorInfoMbr.nLaseSector;
			break;

		case 2:  // ＦＡＴ １
			m_oDiskInfo.oCurrentSectorRange.nFirstSector = m_oDiskInfo.oSectorInfoFat1.nFirstSector;
			m_oDiskInfo.oCurrentSectorRange.nLaseSector = m_oDiskInfo.oSectorInfoFat1.nLaseSector;
			break;

		case 3:  // ＦＡＴ ２
			m_oDiskInfo.oCurrentSectorRange.nFirstSector = m_oDiskInfo.oSectorInfoFat2.nFirstSector;
			m_oDiskInfo.oCurrentSectorRange.nLaseSector = m_oDiskInfo.oSectorInfoFat2.nLaseSector;
			break;

		case 4:  // ルート・ディレクトリ
			m_oDiskInfo.oCurrentSectorRange.nFirstSector = m_oDiskInfo.oSectorInfoRoot.nFirstSector;
			m_oDiskInfo.oCurrentSectorRange.nLaseSector = m_oDiskInfo.oSectorInfoRoot.nLaseSector;
			break;

		case 5:  // データ領域
			m_oDiskInfo.oCurrentSectorRange.nFirstSector = m_oDiskInfo.oSectorInfoData.nFirstSector;
			m_oDiskInfo.oCurrentSectorRange.nLaseSector = m_oDiskInfo.oSectorInfoData.nLaseSector;
			break;

		default:
			m_oDiskInfo.oCurrentSectorRange.nFirstSector = 0;
			m_oDiskInfo.oCurrentSectorRange.nLaseSector = 0;
			TRACE(_T("コントロールIDC_TRE_INDEXの選択処理が正しくありません"));
			break;
	}
	m_oDiskInfo.nCurrentSector = m_oDiskInfo.oCurrentSectorRange.nFirstSector;

	CString oStr;
	oStr.Format(_T("%d"), m_oDiskInfo.nCurrentSector);
	m_ctrlEdtCursec.SetWindowText(oStr);
	oStr.Format(_T("(%d - %d)"), m_oDiskInfo.oCurrentSectorRange.nFirstSector, m_oDiskInfo.oCurrentSectorRange.nLaseSector);
	m_ctrlEdtSecrng.SetWindowText(oStr);

	UpdateHexView();

	// MFC規定の処理
	*pResult = 0;
}

void CFloppyDumpViewerDlg::OnCbnSelchangeCmbCharset()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	switch (m_ctrlCmbCharset.GetCurSel())
	{
		case 0:
			m_nCharSet = CHARSET_ASCII;
			break;

		case 1:
			m_nCharSet = CHARSET_SHIFTJIS;
			break;

		default:
			m_nCharSet = CHARSET_UNKNOWN;
			TRACE(_T("コントロールIDC_CMB_CHARSETの選択処理が正しくありません"));
			break;
	}

	UpdateHexView(FALSE);
}


void CFloppyDumpViewerDlg::UpdateAllUi()
{
	// TODO: ここに実装コードを追加します.
	UpdateDiskUi();
	UpdateIndexUi();
	UpdateSectorUi();
}

void CFloppyDumpViewerDlg::UpdateDiskUi()
{
	// TODO: ここに実装コードを追加します.
	CEdit *poEditSet[] = { &m_ctrlEdtDisktype, &m_ctrlEdtVnumber, &m_ctrlEdtVlabel, &m_ctrlEdtSecpertrc, &m_ctrlEdtTotalsec };

	if (m_ctrlCmbDrive.GetCount() == 0)
	{
		m_ctrlCmbDrive.EnableWindow(FALSE);
		m_ctrlBtnDriveget.EnableWindow();
		m_ctrlBtnDriveopen.EnableWindow(FALSE);
		m_ctrlBtnDriveclose.EnableWindow(FALSE);

		for (int i = 0; i < _countof(poEditSet); i++)
		{
			poEditSet[i]->SetWindowText(nullptr);
			poEditSet[i]->EnableWindow(FALSE);
		}
	}
	else
	{
		if (m_oDiskInfo.hDevice == INVALID_HANDLE_VALUE)
		{
			m_ctrlCmbDrive.EnableWindow();
			m_ctrlBtnDriveget.EnableWindow();
			m_ctrlBtnDriveopen.EnableWindow();
			m_ctrlBtnDriveclose.EnableWindow(FALSE);

			for (int i = 0; i < _countof(poEditSet); i++)
			{
				poEditSet[i]->SetWindowText(nullptr);
				poEditSet[i]->EnableWindow(FALSE);
			}
		}
		else
		{
			m_ctrlCmbDrive.EnableWindow(FALSE);
			m_ctrlBtnDriveget.EnableWindow(FALSE);
			m_ctrlBtnDriveopen.EnableWindow(FALSE);
			m_ctrlBtnDriveclose.EnableWindow();

			for (int i = 0; i < _countof(poEditSet); i++)
			{
				poEditSet[i]->EnableWindow();
			}

			CString sSectorsPerTrack, sTotalSecor, sVolumeNumber;
			sSectorsPerTrack.Format(_T("%d"), m_oDiskInfo.oDiskGeometry.SectorsPerTrack);
			sTotalSecor.Format(_T("%d"), m_oDiskInfo.nTotalSectors);
			sVolumeNumber.Format(_T("%08X"), m_oDiskInfo.nVolumeNumber);

			m_ctrlEdtDisktype.SetWindowText(GetDiskTypeName());
			m_ctrlEdtSecpertrc.SetWindowText(sSectorsPerTrack);
			m_ctrlEdtTotalsec.SetWindowText(sTotalSecor);
			m_ctrlEdtVlabel.SetWindowText(m_oDiskInfo.szVolumeLabel);
			m_ctrlEdtVnumber.SetWindowText(sVolumeNumber);
		}
	}
}

void CFloppyDumpViewerDlg::UpdateIndexUi()
{
	// TODO: ここに実装コードを追加します.
	bool bDiskCheckOk;

	switch (m_oDiskInfo.oDiskGeometry.MediaType)
	{
		case F3_1Pt44_512:
		case F3_720_512:
			bDiskCheckOk = true;
			break;

		default:
			bDiskCheckOk = false;
			break;
	}

	if (m_oDiskInfo.hDevice == INVALID_HANDLE_VALUE || !bDiskCheckOk)
	{
		m_ctrlTreIndex.DeleteAllItems();
		m_ctrlTreIndex.EnableWindow(FALSE);
	}
	else
	{
		HTREEITEM hTreeRoot;

		m_ctrlTreIndex.EnableWindow();

		hTreeRoot = m_ctrlTreIndex.InsertItem(m_oIndexInfo[0].strName);
		for (int i = 1; i < _countof(m_oIndexInfo); i++)
		{
			m_ctrlTreIndex.InsertItem(m_oIndexInfo[i].strName, hTreeRoot);
		}
		m_ctrlTreIndex.Expand(hTreeRoot, TVE_EXPAND);
		m_ctrlTreIndex.Select(hTreeRoot, TVGN_CARET);
	}
}

void CFloppyDumpViewerDlg::UpdateSectorUi(BOOL bReadDisk)
{
	// TODO: ここに実装コードを追加します.
	CEdit* poEditSet[] = { &m_ctrlEdtCursec, &m_ctrlEdtSecrng, &m_ctrlEdtHexdump };
	CButton* poButtonSet[] = { &m_ctrlBtnSecG, &m_ctrlBtnSecF, &m_ctrlBtnSecP, &m_ctrlBtnSecN, &m_ctrlBtnSecL };

	if (m_oDiskInfo.hDevice == INVALID_HANDLE_VALUE)
	{
		for (int i = 0; i < _countof(poEditSet); i++)
		{
			poEditSet[i]->SetWindowText(nullptr);
			poEditSet[i]->EnableWindow(FALSE);
		}

		for (int i = 0; i < _countof(poButtonSet); i++)
		{
			poButtonSet[i]->EnableWindow(FALSE);
		}
	}
	else
	{
		for (int i = 0; i < _countof(poEditSet); i++)
		{
			poEditSet[i]->EnableWindow();
		}

		for (int i = 0; i < _countof(poButtonSet); i++)
		{
			poButtonSet[i]->EnableWindow();
		}

		UpdateHexView(bReadDisk);
	}
}

BOOL CFloppyDumpViewerDlg::ReadDiskInfo()
{
	// TODO: ここに実装コードを追加します.
	BOOL bRet = FALSE;

	CString oDriveNameString, oFileNameString;
	BOOL bResult;
	DWORD junk;

	CString drvRoot;  // ?

	if (m_oDiskInfo.hDevice != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_oDiskInfo.hDevice);
		m_oDiskInfo.hDevice = INVALID_HANDLE_VALUE;
	}

	if (m_ctrlCmbDrive.GetCount() == 0)
	{
		goto L_Term;
	}

	m_ctrlCmbDrive.GetLBText(m_ctrlCmbDrive.GetCurSel(), oDriveNameString);
	if (oDriveNameString != _T("A:") && oDriveNameString != _T("B:"))
	{
		TRACE(_T("コントロールIDC_CMB_DRIVEのドライブ文字が正しくありません"));
		goto L_Term;
	}

	oFileNameString.Format(_T("\\\\.\\%s"), oDriveNameString.GetString());
	m_oDiskInfo.hDevice = ::CreateFile(oFileNameString.GetString(),
		GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if (m_oDiskInfo.hDevice == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("ディスクのハンドルが取得できませんでした"), nullptr, MB_ICONERROR | MB_OK);
		goto L_Term;
	}
	m_oDiskInfo.oDriveName = oDriveNameString;

	bResult = ::DeviceIoControl(m_oDiskInfo.hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY,
		nullptr, 0, &m_oDiskInfo.oDiskGeometry, sizeof m_oDiskInfo.oDiskGeometry, &junk, nullptr);
	if (!bResult)
	{
		MessageBox(_T("ディスク情報が取得できませんでした"), nullptr, MB_ICONERROR | MB_OK);
		goto L_Term;
	}

	drvRoot.Format(_T("%s\\"), oDriveNameString.GetString());
	ASSERT(sizeof m_oDiskInfo.szVolumeLabel >= 13);
	memset(m_oDiskInfo.szVolumeLabel, 0, sizeof m_oDiskInfo.szVolumeLabel);
	bResult = ::GetVolumeInformation(drvRoot.GetString(),
		m_oDiskInfo.szVolumeLabel, _countof(m_oDiskInfo.szVolumeLabel), &m_oDiskInfo.nVolumeNumber, nullptr, nullptr, nullptr, 0);
	if (!bResult)
	{
		//goto L_Term;
	}

	m_oDiskInfo.nTotalSectors = m_oDiskInfo.oDiskGeometry.Cylinders.LowPart * m_oDiskInfo.oDiskGeometry.TracksPerCylinder * m_oDiskInfo.oDiskGeometry.SectorsPerTrack;

	switch (m_oDiskInfo.oDiskGeometry.MediaType)
	{
		case F3_1Pt44_512:
			m_oDiskInfo.oSectorInfoMbr.nFirstSector = m_oDiskInfo.nCurrentSector = 0;
			m_oDiskInfo.oSectorInfoMbr.nLaseSector = 0;
			m_oDiskInfo.oSectorInfoFat1.nFirstSector = m_oDiskInfo.oSectorInfoMbr.nFirstSector + m_oSecInfo1440.nSectorsMbr;
			m_oDiskInfo.oSectorInfoFat1.nLaseSector = m_oDiskInfo.oSectorInfoFat1.nFirstSector + m_oSecInfo1440.nSectorsFat - 1;
			m_oDiskInfo.oSectorInfoFat2.nFirstSector = m_oDiskInfo.oSectorInfoFat1.nFirstSector + m_oSecInfo1440.nSectorsFat;
			m_oDiskInfo.oSectorInfoFat2.nLaseSector = m_oDiskInfo.oSectorInfoFat2.nFirstSector + m_oSecInfo1440.nSectorsFat - 1;
			m_oDiskInfo.oSectorInfoRoot.nFirstSector = m_oDiskInfo.oSectorInfoFat2.nFirstSector + m_oSecInfo1440.nSectorsFat;
			m_oDiskInfo.oSectorInfoRoot.nLaseSector = m_oDiskInfo.oSectorInfoRoot.nFirstSector + m_oSecInfo1440.nSectorsRootdir - 1;
			m_oDiskInfo.oSectorInfoData.nFirstSector = m_oDiskInfo.oSectorInfoRoot.nFirstSector + m_oSecInfo1440.nSectorsRootdir;
			m_oDiskInfo.oSectorInfoData.nLaseSector = m_oSecInfo1440.nTotalSectors - 1;
			m_oDiskInfo.oCurrentSectorRange.nFirstSector = m_oDiskInfo.oSectorInfoMbr.nFirstSector;
			m_oDiskInfo.oCurrentSectorRange.nLaseSector = m_oDiskInfo.oSectorInfoData.nLaseSector;
			break;

		case F3_720_512:
			m_oDiskInfo.oSectorInfoMbr.nFirstSector = m_oDiskInfo.nCurrentSector = 0;
			m_oDiskInfo.oSectorInfoMbr.nLaseSector = 0;
			m_oDiskInfo.oSectorInfoFat1.nFirstSector = m_oDiskInfo.oSectorInfoMbr.nFirstSector + m_oSecInfo720.nSectorsMbr;
			m_oDiskInfo.oSectorInfoFat1.nLaseSector = m_oDiskInfo.oSectorInfoFat1.nFirstSector + m_oSecInfo720.nSectorsFat - 1;
			m_oDiskInfo.oSectorInfoFat2.nFirstSector = m_oDiskInfo.oSectorInfoFat1.nFirstSector + m_oSecInfo720.nSectorsFat;
			m_oDiskInfo.oSectorInfoFat2.nLaseSector = m_oDiskInfo.oSectorInfoFat2.nFirstSector + m_oSecInfo720.nSectorsFat - 1;
			m_oDiskInfo.oSectorInfoRoot.nFirstSector = m_oDiskInfo.oSectorInfoFat2.nFirstSector + m_oSecInfo720.nSectorsFat;
			m_oDiskInfo.oSectorInfoRoot.nLaseSector = m_oDiskInfo.oSectorInfoRoot.nFirstSector + m_oSecInfo720.nSectorsRootdir - 1;
			m_oDiskInfo.oSectorInfoData.nFirstSector = m_oDiskInfo.oSectorInfoRoot.nFirstSector + m_oSecInfo720.nSectorsRootdir;
			m_oDiskInfo.oSectorInfoData.nLaseSector = m_oSecInfo720.nTotalSectors - 1;
			m_oDiskInfo.oCurrentSectorRange.nFirstSector = m_oDiskInfo.oSectorInfoMbr.nFirstSector;
			m_oDiskInfo.oCurrentSectorRange.nLaseSector = m_oDiskInfo.oSectorInfoData.nLaseSector;
			break;

		default:
			goto L_Term;
	}

	bRet = TRUE;

L_Term:
	if (!bRet)
	{
		if (m_oDiskInfo.hDevice != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(m_oDiskInfo.hDevice);
			m_oDiskInfo.hDevice = INVALID_HANDLE_VALUE;
		}
	}

	return bRet;
}

LPCTSTR CFloppyDumpViewerDlg::GetDiskTypeName()
{
	// TODO: ここに実装コードを追加します.
	static LPCTSTR strName;

	if (m_oDiskInfo.hDevice == INVALID_HANDLE_VALUE)
	{
		strName = nullptr;
	}
	else
	{
		switch (m_oDiskInfo.oDiskGeometry.MediaType)
		{
			case F3_1Pt44_512:
				strName = m_oSecInfo1440.strTypeName;
				break;

			case F3_720_512:
				strName = m_oSecInfo720.strTypeName;
				break;

			default:
				strName = nullptr;
				break;
		}
	}

	return strName;
}

void CFloppyDumpViewerDlg::UpdateHexView(BOOL bReadDisk)
{
	// TODO: ここに実装コードを追加します.
	CString oPageString, oLineString;
	CString oHexCharString;
	CByteArray oCharData;
	size_t nGenerateLength;
	BYTE byLast, byLastBak = 0;
	DWORD nAddress;
	const size_t nBytes16 = 16;
	LPCTSTR pstrNewLine = _T("\r\n");

	ASSERT(m_pbySectorBuf != nullptr);
	if (bReadDisk)
	{
		memset(m_pbySectorBuf, 0, g_nSectorSize);
	}

	if (m_oDiskInfo.hDevice == INVALID_HANDLE_VALUE)
	{
		m_ctrlEdtHexdump.SetWindowText(nullptr);
		return;
	}

	if (bReadDisk)
	{
		DWORD nReadBytes;

		if (::SetFilePointer(m_oDiskInfo.hDevice, m_oDiskInfo.nCurrentSector * g_nSectorSize, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			MessageBox(_T("セクタの移動に失敗しました"), nullptr, MB_ICONERROR | MB_OK);
			oPageString.Empty();
			goto L_IoDone;
		}

		if (::ReadFile(m_oDiskInfo.hDevice, m_pbySectorBuf, g_nSectorSize, &nReadBytes, nullptr) == FALSE)
		{
			MessageBox(_T("セクタの読み込みに失敗しました"), nullptr, MB_ICONERROR | MB_OK);
			oPageString.Empty();
			//goto L_IoDone;
		}
	}
L_IoDone:

	oPageString.Empty();

	// 見出し部
	oLineString.Format(_T("%*s"), (8 + 1 + 2), _T(""));
	for (int i = 0; i < nBytes16; i++)
	{
		oLineString.AppendFormat(_T("+%X%c"), i, (i == 7 ? _T('-') : _T(' ')));
	}
	oLineString += _T(' ');
	for (int i = 0; i < nBytes16; i++)
	{
		oLineString.AppendFormat(_T("%X"), i);
	}
	oLineString += pstrNewLine;
	oPageString += oLineString;

	CString oHLineString(_T('-'), (8 + 1 + 2 + (3 * 16 - 1) + 2 + 16));
	oLineString = oHLineString;
	oLineString += pstrNewLine;
	oPageString += oLineString;

	// データ部
	byLast = byLastBak = 0;
	for (int i = 0; i < g_nSectorSize; i += nBytes16)
	{
		oLineString.Empty();  // TODO: # すぐ後にFormatが有るからEmpty不要？

		nAddress = m_oDiskInfo.nCurrentSector * g_nSectorSize + i;
		oLineString.Format(_T("%04X:%04X  "), HIWORD(nAddress), LOWORD(nAddress));

		for (int j = 0; j < nBytes16; j++)
		{
			oLineString.AppendFormat(_T("%02X%c"), m_pbySectorBuf[i + j], (j == 7 ? _T('-') : _T(' ')));
		}
		oLineString.AppendFormat(_T("%*s"), (byLast == 0) ? 1 : 2, _T(""));

		oHexCharString.Empty();

		nGenerateLength = (byLast == 0) ? nBytes16 : nBytes16 - 1;
		byLast = 0;
		GenerateHexCharData(oCharData, m_nCharSet, &m_pbySectorBuf[i + (nBytes16 - nGenerateLength)], nGenerateLength, &byLast);

		if (i == 0)
		{
			if (!_ismbblead(oCharData[0]) && !_istprint(oCharData[0]))
			{
				oCharData[0] = _T('.');
			}
		}

		CStringA oTmpAString;

		if (byLast != 0)
		{
			if (i < g_nSectorSize - nBytes16)
			{
				oCharData.RemoveAt(oCharData.GetCount() - 1);
				oCharData.Add(byLast);
				oCharData.Add('\0');  // 文字列終端の処理
				oTmpAString.AppendFormat("%s", oCharData.GetData());

				byLastBak = byLast;
				GenerateHexCharData(oCharData, m_nCharSet, &m_pbySectorBuf[i + nBytes16], 1, &byLast);
				byLast = byLastBak;
			}
		}
		oCharData.Add('\0');  // 文字列終端の処理
		oTmpAString.AppendFormat("%s", oCharData.GetData());

#ifdef _UNICODE
		WCHAR szTmpStringBuf[g_nSectorSize + 5];

		memset(szTmpStringBuf, 0, sizeof szTmpStringBuf);
		::MultiByteToWideChar(CP_OEMCP, MB_COMPOSITE,
			oTmpAString.GetBuffer(), nBytes16, szTmpStringBuf, g_nSectorSize + 5);
		oHexCharString = szTmpStringBuf;
#else
		oHexCharString = oTmpAString;
#endif

		oLineString += oHexCharString;
		oLineString += pstrNewLine;
		oPageString += oLineString;
	}

	m_ctrlEdtHexdump.SetWindowText(oPageString.GetString());
}

BOOL CFloppyDumpViewerDlg::GenerateHexCharData(CByteArray& oHexCharData, CHARSET nCharSet, const BYTE* pbyData, size_t nDataSize, BYTE* pbyLast)
{
	// TODO: ここに実装コードを追加します.
	if (pbyData == nullptr || nDataSize == 0)
	{
		return FALSE;
	}
	oHexCharData.RemoveAll();

	BOOL bRet = FALSE;

	PBYTE pBuf, ppBuf;
	const size_t nBufSize = nDataSize;
	BYTE byCur, byLast;
	bool bLeadbyte;

	pBuf = ppBuf = new BYTE[nBufSize];
	byLast = 0;
	bLeadbyte = false;

	switch (nCharSet)
	{
		case CHARSET_ASCII:
			for (int i = 0; i < nDataSize; i++)
			{
				byCur = *(pbyData + i) & 0x7F;
				*ppBuf++ = _istprint(byCur) ? byCur : '.';
			}
			byLast = 0;
			break;

		case CHARSET_SHIFTJIS:
			if (pbyLast != nullptr)
			{
				byLast = *pbyLast;
			}
			
			for (int i = 0; i < nDataSize; i++)
			{
				byCur = *(pbyData + i);

				if (i == 0 && byLast != 0)
				{
					bLeadbyte = _ismbblead(byLast) ? true : false;
					if (bLeadbyte)
					{
						*ppBuf++ = byCur;
						bLeadbyte = true;
						byLast = byCur;
					}
					else
					{
						byCur &= 0x7F;
						*ppBuf++ = _istprint(byCur) ? byCur : '.';
						bLeadbyte = false;
						byLast = 0;
					}
					continue;
				}

				if (bLeadbyte)
				{
					*ppBuf++ = byCur;
					bLeadbyte = false;
					byLast = 0;
					continue;
				}

				bLeadbyte = _ismbblead(byCur) ? true : false;
				if (bLeadbyte)
				{
					*ppBuf++ = (i < nDataSize - 1) ? byCur : '.';
					byLast = byCur;
				}
				else
				{
					byCur &= 0x7F;
					*ppBuf++ = _istprint(byCur) ? byCur : '.';
					byLast = 0;
				}
			}
			break;

		default:
			memset(pBuf, 0, nBufSize);
			byLast = 0;
			goto L_Term;
	}

	bRet = TRUE;

L_Term:
	oHexCharData.SetSize(nBufSize);
	memcpy(oHexCharData.GetData(), pBuf, nBufSize);

	if (pBuf != nullptr)
	{
		delete[] pBuf;
	}

	if (pbyLast != nullptr)
	{
		*pbyLast = byLast;
	}

	return bRet;
}
