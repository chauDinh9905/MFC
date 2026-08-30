
// FileClientUIDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "FileClientUI.h"
#include "FileClientUIDlg.h"
#include "afxdialogex.h"
#include "PipeClient.h"
#include "UiStrings.h"
#include "LanguageSettings.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CFileClientUIDlg dialog



CFileClientUIDlg::CFileClientUIDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FILECLIENTUI_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFileClientUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PATH, m_editPath);
	DDX_Control(pDX, IDC_STATIC_RESULT, m_staticResult);
	DDX_Control(pDX, IDC_STATIC_FLAG, m_staticFlag);
}

BEGIN_MESSAGE_MAP(CFileClientUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDIT_PATH, &CFileClientUIDlg::OnEnChangeEditPath)
	ON_BN_CLICKED(IDC_BTN_DELETE, &CFileClientUIDlg::OnBnClickedBtnDelete)
	ON_BN_CLICKED(IDC_BTN_CREATE, &CFileClientUIDlg::OnClickedBtnCreate)
	ON_NOTIFY(BCN_DROPDOWN, IDC_BTN_DELETE, &CFileClientUIDlg::OnDropdownBtnDelete)
	ON_BN_CLICKED(IDC_BTN_LANG_EN, &CFileClientUIDlg::OnClickedBtnLangEn)
	ON_BN_CLICKED(IDC_BTN_LANG_VI, &CFileClientUIDlg::OnClickedBtnLangVi)
	ON_BN_CLICKED(IDC_BTN_OPENDIR, &CFileClientUIDlg::OnClickedBtnOpendir)
	ON_STN_CLICKED(IDC_STATIC_RESULT, &CFileClientUIDlg::OnStnClickedStaticResult)
	ON_STN_CLICKED(IDC_BTN_OPENDIR, &CFileClientUIDlg::OnStnClickedBtnOpendir)
	ON_STN_CLICKED(IDC_BTN_CLOSE, &CFileClientUIDlg::OnStnClickedBtnClose)
	ON_STN_CLICKED(IDC_ICON_FOLER, &CFileClientUIDlg::OnClickedIconFoler)
	ON_STN_CLICKED(IDC_STATIC_FLAG, &CFileClientUIDlg::OnClickedStaticFlag)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()


// CFileClientUIDlg message handlers

BOOL CFileClientUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_currentLang = LanguageSettings::Load(); // Lab 7
	ApplyLanguageToControls();

	CRect rect;
	GetWindowRect(&rect);
	HRGN hRgn = CreateRoundRectRgn(0, 0, rect.Width(), rect.Height(), 20, 20);
	SetWindowRgn(hRgn, TRUE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFileClientUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFileClientUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFileClientUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CFileClientUIDlg::OnEnChangeEditPath()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CFileClientUIDlg::OnBnClickedBtnDelete()
{
	// TODO: Add your control notification handler code here
	DoFileAction(FileAction::Delete);
}

void CFileClientUIDlg::OnClickedBtnCreate()
{
	// TODO: Add your control notification handler code here
	DoFileAction(FileAction::Create);
}

void CFileClientUIDlg::OnDropdownBtnDelete(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMBCDROPDOWN pDropDown = reinterpret_cast<LPNMBCDROPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CFileClientUIDlg::OnClickedBtnLangEn()
{
	// TODO: Add your control notification handler code here
	m_currentLang = _T("en");
	ApplyLanguageToControls();
	LanguageSettings::Save(m_currentLang);
}

void CFileClientUIDlg::OnClickedBtnLangVi()
{
	// TODO: Add your control notification handler code here
	m_currentLang = _T("vi");
	ApplyLanguageToControls();
	LanguageSettings::Save(m_currentLang);
}

void CFileClientUIDlg::OnClickedBtnOpendir()
{
	// TODO: Add your control notification handler code here
	ShellExecute(nullptr, _T("open"), _T("explorer.exe"), _T("C:\\Windows"), nullptr, SW_SHOWNORMAL);
}
void CFileClientUIDlg::DoFileAction(FileAction action)
{
	CString path;
	m_editPath.GetWindowText(path);

	FileRequest req = {};
	req.action = action;
	wcscpy_s(req.path, path.GetString());
	wcscpy_s(req.lang, m_currentLang.GetString());

	FileResponse resp = {};
	CString err;

	GetDlgItem(IDC_BTN_CREATE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_DELETE)->EnableWindow(FALSE);

	bool ok = PipeClient::SendRequest(req, resp, err);

	GetDlgItem(IDC_BTN_CREATE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_DELETE)->EnableWindow(TRUE);

	m_staticResult.SetWindowText(ok ? resp.message : err);
}

void CFileClientUIDlg::ApplyLanguageToControls() {
	GetDlgItem(IDC_BTN_CREATE)->SetWindowText(UiStrings::Get(UiKey::CreateButton, m_currentLang));
	GetDlgItem(IDC_BTN_DELETE)->SetWindowText(UiStrings::Get(UiKey::DeleteButton, m_currentLang));

	HICON hIcon = AfxGetApp()->LoadIcon(
		m_currentLang == _T("vi") ? IDI_FLAG_VN : IDI_FLAG_EN);
	m_staticFlag.SetIcon(hIcon);

	GetDlgItem(IDC_BTN_CREATE)->Invalidate();
	GetDlgItem(IDC_BTN_DELETE)->Invalidate();
}
void CFileClientUIDlg::OnStnClickedStaticResult()
{
	// TODO: Add your control notification handler code here
}

void CFileClientUIDlg::OnStnClickedBtnOpendir()
{
	// TODO: Add your control notification handler code here
}

void CFileClientUIDlg::OnStnClickedBtnClose()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void CFileClientUIDlg::OnClickedIconFoler()
{
	// TODO: Add your control notification handler code here
	ShellExecute(nullptr, _T("open"), _T("explorer.exe"), _T("C:\\Windows"), nullptr, SW_SHOWNORMAL);
}

void CFileClientUIDlg::OnClickedStaticFlag()
{
	// TODO: Add your control notification handler code here
	m_currentLang = (m_currentLang == _T("vi")) ? _T("en") : _T("vi");
	ApplyLanguageToControls();
	LanguageSettings::Save(m_currentLang);
}

BOOL CFileClientUIDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	CRect rect;
	GetClientRect(&rect);
	CBrush brush(RGB(250, 245, 240));
	pDC->FillRect(&rect, &brush);
	return TRUE;
}

void CFileClientUIDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	ReleaseCapture();
	SendMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
	CDialogEx::OnLButtonDown(nFlags, point);
}

void CFileClientUIDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO: Add your message handler code here and/or call default
	if (nIDCtl == IDC_BTN_CREATE || nIDCtl == IDC_BTN_DELETE)
	{
		CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
		CRect rect = lpDrawItemStruct->rcItem;
		bool pressed = (lpDrawItemStruct->itemState & ODS_SELECTED);

		COLORREF bgColor;
		if (nIDCtl == IDC_BTN_CREATE)
			bgColor = pressed ? RGB(200, 90, 30) : RGB(230, 110, 40);
		else
			bgColor = pressed ? RGB(120, 120, 120) : RGB(160, 160, 160);

		CBrush brush(bgColor);
		CBrush* oldBrush = pDC->SelectObject(&brush);
		CPen pen(PS_NULL, 0, bgColor);
		CPen* oldPen = pDC->SelectObject(&pen);

		pDC->RoundRect(rect, CPoint(12, 12));

		pDC->SelectObject(oldBrush);
		pDC->SelectObject(oldPen);

		CString text;
		GetDlgItem(nIDCtl)->GetWindowText(text);

		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		pDC->DrawText(text, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		return;
	}

	CDialogEx::OnDrawItem(nIDCtl, lpDrawItemStruct);
}
