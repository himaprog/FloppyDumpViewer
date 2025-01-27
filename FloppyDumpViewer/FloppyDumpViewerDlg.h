
// FloppyDumpViewerDlg.h : ヘッダー ファイル
//

#pragma once


typedef struct _SECTORSINFO
{
	LPCTSTR strTypeName;
	const int nSectorsMbr;
	const int nSectorsFat;
	const int nSectorsRootdir;
	const int nSectorsData;
	const int nTotalSectors;
} SECTORSINFO, *PSECTORSINFO;

typedef struct _INDEXINFO
{
	LPCTSTR strName;
	int nSectors;
	int nFirstSector;
	int nLaseSector;
} INDEXINFO, *PINDEXINFO;

typedef struct _DISKINFO
{
	HANDLE hDevice;
	CString oDriveName;
	DISK_GEOMETRY oDiskGeometry;
	int nTotalSectors;
	TCHAR szVolumeLabel[13];
	DWORD nVolumeNumber;
	INDEXINFO oSectorInfoMbr;
	INDEXINFO oSectorInfoFat1;
	INDEXINFO oSectorInfoFat2;
	INDEXINFO oSectorInfoRoot;
	INDEXINFO oSectorInfoData;
	INDEXINFO oCurrentSectorRange;
	int nCurrentSector;
} DISKINFO, *PDISKINFO;

enum CHARSET
{
	CHARSET_UNKNOWN,
	CHARSET_ASCII,
	CHARSET_SHIFTJIS,
};


// CFloppyDumpViewerDlg ダイアログ
class CFloppyDumpViewerDlg : public CDialog
{
// コンストラクション
public:
	CFloppyDumpViewerDlg(CWnd* pParent = nullptr);	// 標準コンストラクター

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FLOPPYDUMPVIEWER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート


// 実装
protected:
	HICON m_hIcon;

	CComboBox m_ctrlCmbDrive;
	CButton m_ctrlBtnDriveget;
	CButton m_ctrlBtnDriveopen;
	CButton m_ctrlBtnDriveclose;
	CEdit m_ctrlEdtDisktype;
	CEdit m_ctrlEdtSecpertrc;
	CEdit m_ctrlEdtTotalsec;
	CEdit m_ctrlEdtVlabel;
	CEdit m_ctrlEdtVnumber;
	CTreeCtrl m_ctrlTreIndex;
	CEdit m_ctrlEdtCursec;
	CButton m_ctrlBtnSecG;
	CEdit m_ctrlEdtSecrng;
	CButton m_ctrlBtnSecF;
	CButton m_ctrlBtnSecP;
	CButton m_ctrlBtnSecN;
	CButton m_ctrlBtnSecL;
	CEdit m_ctrlEdtHexdump;
	CComboBox m_ctrlCmbCharset;

	INDEXINFO m_oIndexInfo[6];
	CHARSET m_nCharSet;

	SECTORSINFO m_oSecInfo1440 = { _T("3.5\", 1.44MB"), 1, 9, 14, 2847, 2880 };
	SECTORSINFO m_oSecInfo720 = { _T("3.5\", 720KB"), 1, 3, 7, 1426, 1400 };
	DISKINFO m_oDiskInfo;
	PBYTE m_pbySectorBuf;

	CFont m_oHexFont;


protected:
	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedBtnDriveget();
	afx_msg void OnBnClickedBtnDriveopen();
	afx_msg void OnBnClickedBtnDriveclose();
	afx_msg void OnBnClickedBtnSecG();
	afx_msg void OnBnClickedBtnSecF();
	afx_msg void OnBnClickedBtnSecP();
	afx_msg void OnBnClickedBtnSecN();
	afx_msg void OnBnClickedBtnSecL();
	afx_msg void OnTvnSelchangedTreIndex(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeCmbCharset();

protected:
	// 独自の関数
	void UpdateAllUi();
	void UpdateDiskUi();
	void UpdateIndexUi();
	void UpdateSectorUi(BOOL bReadDisk = TRUE);
	BOOL ReadDiskInfo();
	LPCTSTR GetDiskTypeName();
	void UpdateHexView(BOOL bReadDisk = TRUE);
	BOOL GenerateHexCharData(CByteArray& oHexCharData, CHARSET nCharSet, const BYTE* pbyData, size_t nDataSize, BYTE* pbyNextOne = NULL);
};
