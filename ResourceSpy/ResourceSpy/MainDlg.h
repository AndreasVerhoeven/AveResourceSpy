// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "resource.h"

#include <vector>
#include <map>
#include "../../AveResourceReplacer/ResourceSpy.h"

#include "HexEditor.h"

#define IDC_HEXEDITOR 999

typedef BOOL (CALLBACK* PStartHook)(HMODULE, HWND);
typedef BOOL (CALLBACK* PStopHook)();
typedef BOOL (CALLBACK* PIsHookRunning)();
typedef BOOL (CALLBACK* PPatchAll)();
typedef BOOL (CALLBACK* PUnpatchAll)();
typedef BOOL (CALLBACK* PSetSpyCallbackWindow)(HWND, UINT);

class CMainDlg : public CDialogImpl<CMainDlg>, public CDialogResize<CMainDlg>
{
public:
	enum { IDD = IDD_MAINDLG };

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_COPYDATA, OnCopyData)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_HANDLER(IDC_START, BN_CLICKED, OnBnClickedStart)
		COMMAND_HANDLER(IDC_STOP, BN_CLICKED, OnBnClickedStop)
		NOTIFY_HANDLER(IDC_ENTRIES, LVN_ITEMCHANGED, OnLvnItemchangedEntries)
		COMMAND_HANDLER(IDC_CLEAR, BN_CLICKED, OnBnClickedClear)
		NOTIFY_HANDLER(IDC_ENTRIES, LVN_GETDISPINFO, OnLvnGetdispinfoEntries)
		NOTIFY_HANDLER(IDC_TREE1, TVN_SELCHANGED, OnTvnSelchangedTree1)

		MESSAGE_HANDLER(WM_CTLCOLORDLG,       OnColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC,    OnColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT,      OnColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORBTN,       OnColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORLISTBOX,   OnColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSCROLLBAR, OnColorDlg)

		CHAIN_MSG_MAP(CDialogResize<CMainDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CMainDlg)
		DLGRESIZE_CONTROL(IDC_TREE1, DLSZ_SIZE_Y)
		//DLGRESIZE_CONTROL(IDC_SPLITTER, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_ORIGINALRESIMAGE, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		//DLGRESIZE_CONTROL(IDC_ORIGINALRESICON, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_ENTRIES, DLSZ_SIZE_X)
		//DLGRESIZE_CONTROL(IDC_CLEAR, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_EDIT1, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_HEXEDITOR, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		//DLGRESIZE_CONTROL(ATL_IDW_STATUS_BAR, DLSZ_MOVE_Y | DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	PStartHook		StartHook;
	PStopHook		StopHook;
	PIsHookRunning	IsHookRunning;
	PPatchAll		PatchAll;
	PUnpatchAll		UnpatchAll;
	PSetSpyCallbackWindow	SetSpyCallbackWindow;

	CListViewCtrl listview;
	CTreeViewCtrl tree;

	CHexEditorCtrl hexEditor;

	struct AveResourceLoadNotificationWithTimeStamp : AveResourceLoadNotification
	{
		SYSTEMTIME when;
	};
	typedef std::vector<AveResourceLoadNotificationWithTimeStamp*> NotificationList;

	struct ResourceTypeMapItem
	{
		HTREEITEM treeItem;
		NotificationList notificationList;
	};
	typedef std::map<WTL::CString, ResourceTypeMapItem*> ResourceTypeMap;

	struct ModuleMapItem
	{
		HTREEITEM treeItem;
		ResourceTypeMap typeMapItem;
		NotificationList notificationList;
	};

	
	typedef std::map<WTL::CString, ModuleMapItem*> ModuleMap;
	struct ProcessMapItem
	{
		HTREEITEM treeItem;
		ModuleMap moduleMap;
		NotificationList notificationList;
	};

	typedef std::map<DWORD, ProcessMapItem*> ProcessMap;

	struct AllProcesssesItem
	{
		HTREEITEM treeItem;
		ProcessMap processes;
		NotificationList notificationList;

		AllProcesssesItem() : treeItem(){}
	};

	AllProcesssesItem allProcessItem;

	HMODULE hMod;

	NotificationList& GetSelectedNotificationList();

	void HideViewers();


// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		DWORD_PTR gdiplusToken;
		GdiplusStartupInput input;
		GdiplusStartup(&gdiplusToken, &input, NULL);

		// center the dialog on the screen
		CenterWindow();

		ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);

		// set icons
		HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		SetIcon(hIconSmall, FALSE);

		tree = GetDlgItem(IDC_TREE1);
		::SetWindowTheme(tree, L"Explorer", NULL);

		listview = GetDlgItem(IDC_ENTRIES);
		listview.AddColumn(L"time",0);
		//listview.AddColumn(L"pid",1);
		//listview.AddColumn(L"process",2);
		//listview.AddColumn(L"module",3);
		listview.AddColumn(L"type",1);
		listview.AddColumn(L"id",2);
		listview.AddColumn(L"replacement",3);

		listview.SetColumnWidth(0, 56);
		listview.SetColumnWidth(1, 85);
		listview.SetColumnWidth(2, 50);
		listview.SetColumnWidth(3, 200);
		//listview.SetColumnWidth(4, 60);
		//listview.SetColumnWidth(5, 50);
		//listview.SetColumnWidth(6, 200);

		listview.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
		::SetWindowTheme(listview, L"Explorer", NULL);

		//listview.ModifyStyleEx(0, WS_EX_COMPOSITED);

		StartHook = NULL;
		StopHook = NULL;
		IsHookRunning = NULL;
		PatchAll = NULL;
		UnpatchAll = NULL;
		SetSpyCallbackWindow = NULL;
	
		WCHAR hookPath[MAX_PATH] = {0};
		GetModuleFileName(NULL, hookPath, _countof(hookPath));
		PathRemoveFileSpec(hookPath);
		PathAppend(hookPath, L"AveResourceReplacer.dll");
		hMod = LoadLibrary(hookPath);
		if(hMod != NULL)
		{
			StartHook     = (PStartHook)GetProcAddress(hMod, "StartHook");
			StopHook      = (PStopHook)GetProcAddress(hMod, "StopHook");
			IsHookRunning = (PIsHookRunning)GetProcAddress(hMod, "IsHookRunning");

			PatchAll = (PPatchAll)GetProcAddress(hMod, "PatchAll");
			UnpatchAll = (PUnpatchAll)GetProcAddress(hMod, "UnpatchAll");
			SetSpyCallbackWindow = (PSetSpyCallbackWindow)GetProcAddress(hMod, "SetSpyCallbackWindow");
		}

		if(UnpatchAll != NULL)
		{
			UnpatchAll();
		}

		BOOL isRunning = IsHookRunning && IsHookRunning();
		::EnableWindow(GetDlgItem(IDC_START),!isRunning);
		::EnableWindow(GetDlgItem(IDC_STOP),isRunning);
		if(isRunning)
		{
			if(SetSpyCallbackWindow != NULL)
				SetSpyCallbackWindow(m_hWnd, 1);
		}


		RECT rc = {0};
		GetDlgItem(IDC_EDIT1).GetWindowRect(&rc);
		::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rc, 2);
		hexEditor.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE, IDC_HEXEDITOR);

		DlgResize_Init(false);

		return TRUE;
	}

	LRESULT OnCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnBnClickedStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLvnItemchangedEntries(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnBnClickedClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLvnGetdispinfoEntries(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnTvnSelchangedTree1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
};
