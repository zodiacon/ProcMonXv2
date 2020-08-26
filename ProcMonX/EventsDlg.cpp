#include "pch.h"
#include "EventConfiguration.h"
#include "EventsDlg.h"
#include "KernelEvents.h"
#include "View.h"

CEventsDlg::CEventsDlg(EventsConfig& config) : m_Config(config) {
}

constexpr WCHAR SaveLoadIniFilter[] = L"ini files (*.ini)\0*.ini\0All Files\0*.*\0";

void CEventsDlg::BuildEventsTree(const EventsConfig& config) {
	for (auto& category : KernelEventCategory::GetAllCategories()) {
		int image = CView::GetImageFromEventName(category.Name.c_str());
		auto item = m_Tree.InsertItem(category.Name.c_str(), image, image, TVI_ROOT, TVI_LAST);
		item.SetData((DWORD_PTR)&category);
		auto cat = config.GetCategory(category.Name.c_str());
		for (auto& evt : category.Events) {
			image = CView::GetImageFromEventName(evt.Name.c_str());
			auto child = item.InsertAfter(evt.Name.c_str(), TVI_LAST, image);
			child.SetData((DWORD_PTR)&evt);
			if(cat && cat->Contains(evt.Opcode))
				m_Tree.SetCheckState(child, TRUE);
		}
		if (cat) {
			if (cat->Opcodes.empty()) {
				// all events must be checked
				CheckTreeChildren(item, TRUE);
				m_Tree.SetCheckState(item, TRUE);
			}
			else {
				item.Expand(TVE_EXPAND);
			}
		}
		item.SortChildren(FALSE);
	}
	m_Tree.SortChildren(TVI_ROOT, FALSE);
}

LRESULT CEventsDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	DlgResize_Init(true);
	m_Tree.Attach(GetDlgItem(IDC_TREE));
	SetIcon(AtlLoadIconImage(IDI_TOOLS, 0, 16, 16), FALSE);
	SetIcon(AtlLoadIconImage(IDI_TOOLS, 0, 32, 32), TRUE);

	m_Tree.ModifyStyle(0, TVS_CHECKBOXES);
	ATLASSERT(m_Tree.GetStyle() & TVS_CHECKBOXES);
	m_Tree.SetExtendedStyle(TVS_EX_DIMMEDCHECKBOXES | TVS_EX_DOUBLEBUFFER, TVS_EX_DIMMEDCHECKBOXES);

	auto images = CView::GetEventImageList();
	m_Tree.SetImageList(images, TVSIL_NORMAL);
	m_Tree.SetItemHeight(20);
	m_Tree.SetIndent(4);

	BuildEventsTree(m_Config);
	m_Init = false;

	return 0;
}

LRESULT CEventsDlg::OnDestroy(UINT, WPARAM, LPARAM, BOOL& handled) {
	handled = FALSE;
	m_Tree.SetImageList(nullptr, TVSIL_NORMAL);

	return 0;
}

LRESULT CEventsDlg::OnGetMinMaxInfo(UINT, WPARAM, LPARAM lParam, BOOL& handled) {
	auto pMMI = (PMINMAXINFO)lParam;
	pMMI->ptMaxTrackSize.x = m_ptMinTrackSize.x;
	handled = FALSE;

	return 0;
}

void CEventsDlg::CheckTreeChildren(HTREEITEM hParent, bool check) {
	auto hChild = m_Tree.GetChildItem(hParent);
	while (hChild) {
		m_Tree.SetCheckState(hChild, check);
		hChild = hChild.GetNextSibling();
	}
}

bool CEventsDlg::BuildConfigFromTree(EventsConfig& config) {
	auto item = m_Tree.GetRootItem();
	CString text;
	while (item) {
		EventConfigCategory category;
		item.GetText(text);
		category.Name = text;
		if (m_Tree.GetCheckState(item)) {
			// entire category selected
			config.AddCategory(category);
		}
		else {
			auto child = item.GetChild();
			while (child) {
				if (m_Tree.GetCheckState(child)) {
					auto evt = (KernelEvent*)child.GetData();
					category.Opcodes.push_back(evt->Opcode);
				}
				child = child.GetNextSibling();
			}
			if(!category.Opcodes.empty())
				config.AddCategory(category);
		}
		item = item.GetNextSibling();
	}

	return false;
}

LRESULT CEventsDlg::OnTreeItemChanged(int, LPNMHDR hdr, BOOL&) {
	if (m_Init || m_Recurse > 0)
		return 0;

	auto tv = (NMTVITEMCHANGE*)hdr;
	if (((tv->uStateNew ^ tv->uStateOld) & TVIS_STATEIMAGEMASK) == 0)
		return 0;

	auto hItem = tv->hItem;
	auto checked = m_Tree.GetCheckState(hItem);
	auto child = m_Tree.GetChildItem(hItem);
	if (child) {
		m_Recurse++;
		CheckTreeChildren(hItem, checked);
		m_Recurse--;
	}
	else if (!checked) {
		m_Recurse++;
		m_Tree.SetCheckState(m_Tree.GetParentItem(hItem), FALSE);
		m_Recurse--;
	}

	m_Tree.SelectItem(hItem);
	return 0;
}

LRESULT CEventsDlg::OnSave(WORD, WORD id, HWND, BOOL&) {
	CSimpleFileDialog dlg(FALSE, L"ini", nullptr, OFN_EXPLORER | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT,
		SaveLoadIniFilter, *this);
	if (dlg.DoModal() == IDOK) {
		EventsConfig config;
		BuildConfigFromTree(config);
		config.Save(dlg.m_szFileName);
	}

	return 0;
}

LRESULT CEventsDlg::OnLoad(WORD, WORD id, HWND, BOOL&) {
	CSimpleFileDialog dlg(TRUE, L"ini", nullptr, OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST,
		SaveLoadIniFilter, *this);
	if (dlg.DoModal() == IDOK) {
		EventsConfig config;
		if (config.Load(dlg.m_szFileName)) {
			BuildEventsTree(config);
		}
	}
	return 0;
}

LRESULT CEventsDlg::OnExpandAll(WORD, WORD id, HWND, BOOL&) {
	auto item = m_Tree.GetRootItem();
	while (item) {
		m_Tree.Expand(item, TVE_EXPAND);
		item = item.GetNextSibling();
	}

	return 0;
}

LRESULT CEventsDlg::OnCollapseAll(WORD, WORD id, HWND, BOOL&) {
	auto item = m_Tree.GetRootItem();
	while (item) {
		m_Tree.Expand(item, TVE_COLLAPSE);
		item = item.GetNextSibling();
	}

	return 0;
}

LRESULT CEventsDlg::OnCloseCmd(WORD, WORD id, HWND, BOOL&) {
	if (id == IDOK) {
		m_Config.Clear();
		BuildConfigFromTree(m_Config);
	}
	EndDialog(id);
	return 0;
}
