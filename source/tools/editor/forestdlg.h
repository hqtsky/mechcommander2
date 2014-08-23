//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef FORESTDLG_H
#define FORESTDLG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifndef FOREST_H
#include "Forest.h"
#endif

class ForestDlg : public CDialog
{

public:
	ForestDlg(CWnd* pParent = NULL);   

	//{{AFX_DATA(ForestDlg)
	enum { IDD = IDD_FOREST_DLG };
	float	m_maxDensity;
	float	m_minDensity;
	float	m_maxHeight;
	float	m_minHeight;
	BOOL	m_randomPlacement;
	float	m_xLoc;
	float	m_yLoc;
	float	m_radius;
	CString	m_Name;
	int32_t		m_1;
	int32_t		m_2;
	int32_t		m_3;
	int32_t		m_4;
	int32_t		m_5;
	int32_t		m_6;
	int32_t		m_7;
	int32_t		m_8;
	int32_t		m_9;
	int32_t		m_10;
	int32_t		m_12;
	int32_t		m_13;
	int32_t		m_14;
	int32_t		m_15;
	//}}AFX_DATA

	Forest		forest;

	//{{AFX_VIRTUAL(ForestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	//}}AFX_VIRTUAL

	afx_msg void OnEditChanged( uint32_t nID );
	afx_msg void OnSliderChanged( uint32_t id, NMHDR * pNotifyStruct, LRESULT * result );



protected:

	//{{AFX_MSG(ForestDlg)
	afx_msg void OnSave();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnLoad();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif // 