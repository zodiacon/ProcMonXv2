#include "pch.h"
#include "FiltersDlg.h"
#include "DialogHelper.h"
#include "FilterFactory.h"

CFiltersDlg::CFiltersDlg(FilterConfiguration& fc) : m_Config(fc) {
}

CString CFiltersDlg::CompareTypeToString(CompareType compare) {
	static PCWSTR types[] = {
		L"Equals",
		L"Not Equal",
		L"Contains",
		L"Does Not Contain",
		L"Greater Than",
		L"Less Than",
	};
	return types[(int)compare];
}

CString CFiltersDlg::GetComboText(CComboBox& cb) {
	CString text;
	cb.GetWindowText(text);
	return text;
}

bool CFiltersDlg::SwapItems(CListViewCtrl& lv, int i1, int i2) {
	if (i1 < 0 || i2 < 0)
		return false;

	LVITEM item1, item2;
	WCHAR text1[128], text2[128];
	item1.pszText = text1;
	item2.pszText = text2;
	item1.cchTextMax = item2.cchTextMax = _countof(text1);
	auto mask = LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM | LVIF_STATE;
	item1.mask = mask;
	item1.iItem = i1;
	item2.mask = mask;
	item2.iItem = i2;
	lv.GetItem(&item1);
	lv.GetItem(&item2);
	item1.iItem = i2;
	item2.iItem = i1;
	lv.SetItem(&item1);
	lv.SetItem(&item2);

	for (int c = 1;; c++) {
		if (!lv.GetItemText(i1, c, text1, _countof(text1)))
			break;
		lv.GetItemText(i2, c, text2, _countof(text2));
		lv.SetItemText(i1, c, text2);
		lv.SetItemText(i2, c, text1);
	}

	return true;
}

void CFiltersDlg::InitList(const FilterConfiguration& config) {
	m_List.DeleteAllItems();

	for (int i = 0; i < config.GetFilterCount(); ++i) {
		auto filter = config.GetFilter(i);
		int n = m_List.InsertItem(m_List.GetItemCount(), filter->Name.c_str(), filter->Action == FilterAction::Include ? 0 : 1);
		m_List.SetItemText(n, 3, filter->Action == FilterAction::Include ? L"Include" : L"Exclude");
		m_List.SetItemText(n, 2, filter->Parameters.c_str());
		m_List.SetItemText(n, 1, CompareTypeToString(filter->Compare));
		m_List.SetCheckState(n, filter->Enabled);
	}
}

void CFiltersDlg::UpdateUI() {
	int selected = m_List.GetSelectedIndex();
	int count = m_List.GetItemCount();

	GetDlgItem(IDC_UP).EnableWindow(selected > 0);
	GetDlgItem(IDC_DOWN).EnableWindow(selected >= 0 && selected < m_List.GetItemCount() - 1);
	GetDlgItem(IDC_SAVE).EnableWindow(count > 0);
	GetDlgItem(IDC_DELETE).EnableWindow(selected >= 0);
	GetDlgItem(IDC_ADD).EnableWindow(m_Text.GetWindowTextLength() > 0);
	GetDlgItem(IDC_EDIT).EnableWindow(selected >= 0);
}

void CFiltersDlg::UpdateConfig(FilterConfiguration& config) {
	config.Clear();
	FilterDescription desc;
	CString text;
	for (int i = 0; i < m_List.GetItemCount(); i++) {
		m_List.GetItemText(i, 0, text);
		desc.Name = text;
		desc.Enabled = m_List.GetCheckState(i) ? true : false;
		m_List.GetItemText(i, 1, text);
		m_CompareTypes.SelectString(-1, text);
		desc.Compare = (CompareType)m_CompareTypes.GetItemData(m_CompareTypes.GetCurSel());
		m_List.GetItemText(i, 2, text);
		desc.Parameters = text;
		m_List.GetItemText(i, 3, text);
		m_IncExc.SelectString(-1, text);
		desc.Action = (FilterAction)m_IncExc.GetItemData(m_IncExc.GetCurSel());
		config.AddFilter(desc);
	}
}

LRESULT CFiltersDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	DialogHelper::AdjustOKCancelButtons(this);

	SetIcon(AtlLoadIconImage(IDI_FILTER, 0, 16, 16), FALSE);
	SetIcon(AtlLoadIconImage(IDI_FILTER, 0, 32, 32), TRUE);

	m_List.Attach(GetDlgItem(IDC_LIST));
	m_List.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 2, 2);
	images.AddIcon(AtlLoadIconImage(IDI_OK2, 0, 16, 16));
	images.AddIcon(AtlLoadIconImage(IDI_CANCEL2, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	struct {
		UINT button, icon;
	} buttons[] = {
		{ IDC_UP, IDI_UP_ARROW },
		{ IDC_DOWN, IDI_DOWN_ARROW },
		{ IDC_SAVE, IDI_FILE },
		{ IDC_LOAD, IDI_OPEN },
		{ IDC_DELETE, IDI_FILTER_DELETE },
		{ IDC_ADD, IDI_FILTER_ADD },
		{ IDC_EDIT, IDI_FILTER_EDIT },
	};

	for (auto& b : buttons) {
		((CButton)GetDlgItem(b.button)).SetIcon(AtlLoadIconImage(b.icon, 0, 24, 24));
	}

	m_List.InsertColumn(0, L"Name", LVCFMT_LEFT, 100);
	m_List.InsertColumn(1, L"Compare Type", LVCFMT_LEFT, 100);
	m_List.InsertColumn(2, L"Parameters", LVCFMT_LEFT, 150);
	m_List.InsertColumn(3, L"Action", LVCFMT_LEFT, 80);

	// populate combos

	m_Text.Attach(GetDlgItem(IDC_VALUE));

	m_FilterNames.Attach(GetDlgItem(IDC_NAMES));
	for (auto name : FilterFactory::GetFilterNames()) {
		m_FilterNames.AddString(name);
	}
	m_FilterNames.SetCurSel(0);

	// compare types combo

	struct {
		PCWSTR text;
		CompareType type;
	} compare[] = {
		{ L"Equals", CompareType::Equals },
		{ L"Not Equals", CompareType::NotEqual },
		{ L"Contains", CompareType::Contains },
		{ L"Does Not Contain", CompareType::NotContains },
	};

	m_CompareTypes.Attach(GetDlgItem(IDC_COMPARE));
	for (auto& c : compare) {
		auto n = m_CompareTypes.AddString(c.text);
		m_CompareTypes.SetItemData(n, (DWORD_PTR)c.type);
	}
	m_CompareTypes.SetCurSel(0);

	// include / exclude combo

	m_IncExc.Attach(GetDlgItem(IDC_INC_EXC));
	auto n = m_IncExc.AddString(L"Include");
	m_IncExc.SetItemData(n, (DWORD_PTR)FilterAction::Include);
	n = m_IncExc.AddString(L"Exclude");
	m_IncExc.SetItemData(n, (DWORD_PTR)FilterAction::Exclude);
	m_IncExc.SetCurSel(0);

	InitList(m_Config);

	UpdateUI();

	return 0;
}

LRESULT CFiltersDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	if (wID == IDOK) {
		UpdateConfig(m_Config);
	}

	EndDialog(wID);
	return 0;
}

LRESULT CFiltersDlg::OnAdd(WORD, WORD wID, HWND, BOOL&) {
	auto incexc = GetComboText(m_IncExc);
	int n = m_List.InsertItem(m_List.GetItemCount(), GetComboText(m_FilterNames), incexc == L"Include" ? 0 : 1);
	m_List.SetItemText(n, 1, GetComboText(m_CompareTypes));
	m_List.SetItemText(n, 2, GetComboText(m_Text));
	m_List.SetItemText(n, 3, incexc);
	m_List.SetCheckState(n, TRUE);
	m_Text.SetWindowText(L"");
	UpdateUI();

	return 0;
}

LRESULT CFiltersDlg::OnSave(WORD, WORD wID, HWND, BOOL&) {
	CSimpleFileDialog dlg(FALSE, L"ini", nullptr, OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_ENABLESIZING, SaveLoadIniFilter, *this);
	if (dlg.DoModal() == IDOK) {
		FilterConfiguration config;
		UpdateConfig(config);
		config.Save(dlg.m_szFileName);
	}
	return 0;
}

LRESULT CFiltersDlg::OnLoad(WORD, WORD wID, HWND, BOOL&) {
	CSimpleFileDialog dlg(TRUE, L"ini", nullptr, OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, SaveLoadIniFilter, *this);
	if (dlg.DoModal() == IDOK) {
		FilterConfiguration config;
		config.Load(dlg.m_szFileName);
		InitList(config);
		UpdateUI();
	}
	return 0;
}

LRESULT CFiltersDlg::OnEdit(WORD, WORD wID, HWND, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	ATLASSERT(selected >= 0);
	CString text;
	m_List.GetItemText(selected, 0, text);
	m_FilterNames.SelectString(-1, text);

	m_List.GetItemText(selected, 1, text);
	m_CompareTypes.SelectString(-1, text);

	m_List.GetItemText(selected, 2, text);
	m_Text.SetWindowText(text);

	m_List.GetItemText(selected, 3, text);
	m_IncExc.SelectString(-1, text);

	m_List.DeleteItem(selected);

	return 0;
}

LRESULT CFiltersDlg::OnMoveUp(WORD, WORD wID, HWND, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	ATLASSERT(selected > 0);
	SwapItems(m_List, selected, selected - 1);
	m_List.SelectItem(selected - 1);

	return 0;
}

LRESULT CFiltersDlg::OnMoveDown(WORD, WORD wID, HWND, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	ATLASSERT(selected < m_List.GetItemCount() - 1);
	SwapItems(m_List, selected, selected + 1);
	m_List.SelectItem(selected + 1);

	return 0;
}

LRESULT CFiltersDlg::OnDelete(WORD, WORD wID, HWND, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	ATLASSERT(selected >= 0);
	m_List.DeleteItem(selected);

	return 0;
}

LRESULT CFiltersDlg::OnValueChanged(WORD, WORD wID, HWND, BOOL&) {
	((CButton)GetDlgItem(IDC_ADD)).EnableWindow(m_Text.GetWindowTextLength() > 0);

	return 0;
}

LRESULT CFiltersDlg::OnItemChanged(int, LPNMHDR, BOOL&) {
	UpdateUI();
	return 0;
}
