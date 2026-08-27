
// FileClientUIDlg.h : header file
//

#pragma once
#include "SharedProtocol.h"
// CFileClientUIDlg dialog
class CFileClientUIDlg : public CDialogEx
{
// Construction
public:
	CFileClientUIDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILECLIENTUI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEditPath();
	afx_msg void OnBnClickedBtnDelete();
	CEdit m_editPath;
	afx_msg void OnClickedBtnCreate();
	afx_msg void OnDropdownBtnDelete(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickedBtnLangEn();
	afx_msg void OnClickedBtnLangVi();
	afx_msg void OnClickedBtnOpendir();
	CStatic m_staticResult;
	CString m_currentLang; // "vi" hoặc "en"

	void DoFileAction(FileAction action);
	void ApplyLanguageToControls();
};
