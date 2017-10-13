//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#if !defined(                                                                  \
	AFX_OBJECTIVEDLG_H__2D6B2B2E_16F8_43B2_9534_A2DC1672352F__INCLUDED_)
#define AFX_OBJECTIVEDLG_H__2D6B2B2E_16F8_43B2_9534_A2DC1672352F__INCLUDED_

//#include "objective.h"

/////////////////////////////////////////////////////////////////////////////
// ObjectiveDlg dialog

class ObjectiveDlg : public CDialog
{
	// Construction
  public:
	ObjectiveDlg(CWnd* pParent = nullptr); // standard constructor
	void LoadDialogValues(void);
	void SaveDialogValues(void);

	void UpdateTitleDisplay(void);
	void UpdateDescriptionDisplay(void);

	CObjective m_ModifiedObjective;

	CString m_TitleUnlocalizedText;
	bool m_TitleUseResourceString;
	int32_t m_TitleResourceStringID;

	CString m_DescriptionUnlocalizedText;
	bool m_DescriptionUseResourceString;
	int32_t m_DescriptionResourceStringID;

	int32_t nConditionSpeciesSelectionIndex;
	int32_t nActionSpeciesSelectionIndex;
	int32_t nFailureConditionSpeciesSelectionIndex;
	int32_t nFailureActionSpeciesSelectionIndex;

	class CConditionSpeciesList
		: public EList<condition_species_type, condition_species_type>
	{
	};
	class CActionSpeciesList
		: public EList<action_species_type, action_species_type>
	{
	};
	CConditionSpeciesList comboBoxItems;
	CActionSpeciesList actionComboBoxItems;
	CConditionSpeciesList failureConditionComboBoxItems;
	CActionSpeciesList failureActionComboBoxItems;

	/* for persistent storage of miscellaneous working data*/
	CObjectiveCondition* pNewCondition;
	CObjectiveAction* pNewAction;

	// Dialog Data
	//{{AFX_DATA(ObjectiveDlg)
	enum
	{
		IDD = IDD_OBJECTIVE
	};
	CButton m_AddFailureConditionButton;
	CListBox m_FailureActionList;
	CListBox m_FailureConditionList;
	CComboBox m_FailureActionComboBox;
	CComboBox m_FailureConditionComboBox;
	CComboBox m_modelGroup;
	CComboBox m_Mech;
	CButton m_AddButton;
	CComboBox m_ActionComboBox;
	CComboBox m_ComboBox;
	CListBox m_ActionList;
	CListBox m_List;
	int32_t m_PriorityRadioButton;
	int32_t m_ResourcePointsEdit;
	BOOL m_PreviousPrimaryObjectiveMustBeCompleteCheckButton;
	BOOL m_AllPreviousPrimaryObjectivesMustBeCompleteCheckButton;
	BOOL m_DisplayMarkerCheckButton;
	float m_XEdit;
	float m_YEdit;
	BOOL m_HiddenTriggerCheckButton;
	BOOL m_ActivateOnFlagCheckButton;
	int32_t m_ActivateFlagIndexEdit;
	CString m_TitleEdit;
	CString m_DescriptionEdit;
	CString m_TitleResourceStringIDEdit;
	CString m_DescriptionResourceStringIDEdit;
	CString m_ActivateFlagIDEdit;
	BOOL m_ResetStatusOnFlagCheckButton;
	CString m_ResetStatusFlagIDEdit;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ObjectiveDlg)
  protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
													 //}}AFX_VIRTUAL

	// Implementation
  protected:
	// Generated message map functions
	//{{AFX_MSG(ObjectiveDlg)
	virtual BOOL OnInitDialog(void);
	afx_msg void OnObjectiveAddConditionButton(void);
	afx_msg void OnObjectiveRemoveConditionButton(void);
	afx_msg void OnObjectiveAddActionButton(void);
	afx_msg void OnObjectiveRemoveActionButton(void);
	virtual void OnOK(void);
	afx_msg void OnSelchangeGroup(void);
	afx_msg void OnBaseedit2(void);
	afx_msg void OnHighilight2edit2(void);
	afx_msg void OnHighlight1edit(void);
	afx_msg void OnChangeHighlight1(void);
	afx_msg void OnChangeHighlight2(void);
	afx_msg void OnChangeBase2(void);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, uint32_t nCtlColor);
	afx_msg void OnObjectiveTitleEditButton(void);
	afx_msg void OnObjectiveDescriptionEditButton(void);
	afx_msg void OnObjectiveAddFailureConditionButton(void);
	afx_msg void OnObjectiveRemoveFailureConditionButton(void);
	afx_msg void OnObjectiveAddFailureActionButton(void);
	afx_msg void OnObjectiveRemoveFailureActionButton(void);
	afx_msg void OnObjectiveViewConditionButton(void);
	afx_msg void OnObjectiveViewActionButton(void);
	afx_msg void OnObjectiveViewFailureConditionButton(void);
	afx_msg void OnObjectiveViewFailureActionButton(void);
	afx_msg void OnObjectiveClearLogisticsModelButton(void);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void DoColorBox(CWnd* pWnd);
	void DoEditColorChange(int32_t ID);

	CBrush baseBrush;
	CBrush brush1;
	CBrush brush2;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before
// the previous line.

#endif // !defined(AFX_OBJECTIVEDLG_H__2D6B2B2E_16F8_43B2_9534_A2DC1672352F__INCLUDED_)
