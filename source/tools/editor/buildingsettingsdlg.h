//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#if !defined(                                                                  \
	AFX_BUILDINGSETTINGSDLG_H__66842972_D1C0_48E8_8DF6_930ED43A3896__INCLUDED_)
#define AFX_BUILDINGSETTINGSDLG_H__66842972_D1C0_48E8_8DF6_930ED43A3896__INCLUDED_

/////////////////////////////////////////////////////////////////////////////
// BuildingSettingsDlg dialog

//#include <elist.h>
//#include "action.h"

class EditorObject;

class BuildingSettingsDlg : public CDialog
{
  public:
	BuildingSettingsDlg(EList<EditorObject*, EditorObject*>&,
		ActionUndoMgr& undoMgr); // standard constructor

	//{{AFX_DATA(BuildingSettingsDlg)
	enum
	{
		IDD = IDD_BUILDINGSETTINGS
	};
	CComboBox m_Mech;
	CComboBox m_Group;
	int32_t m_Alignment;
	float m_x;
	float m_y;
	int32_t m_partID;
	CString m_forestName;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(BuildingSettingsDlg)
  protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

	// Implementation
  protected:
	void DoColorBox(CWnd* pWnd);

	//{{AFX_MSG(BuildingSettingsDlg)
	afx_msg void OnSelchangeGroup(void);
	virtual void OnOK(void);
	virtual BOOL OnInitDialog(void);
	afx_msg void OnSelchangeMech(void);
	virtual void OnCancel(void);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void applyChanges(void);
	ModifyBuildingAction* pAction;

	typedef EList<EditorObject*, EditorObject*> EDITOROBJECT_LIST;
	EDITOROBJECT_LIST& units;

	void updateMemberVariables(void);
	ActionUndoMgr* pUndoMgr;
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_BUILDINGSETTINGSDLG_H__66842972_D1C0_48E8_8DF6_930ED43A3896__INCLUDED_)
