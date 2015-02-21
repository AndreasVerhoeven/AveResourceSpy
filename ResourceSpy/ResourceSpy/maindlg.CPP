#include "StdAfx.h"
#include "MainDlg.h"



CString ResTypeToString(DWORD resTypeId, const WCHAR* resTypeName)
{
	switch(resTypeId)
	{
		case RT_CURSOR: return L"CURSOR";
		case RT_BITMAP: return L"BITMAP";
		case RT_ICON: return L"ICON";
		case RT_MENU: return L"MENU";
		case RT_DIALOG: return L"DIALOG";
		case RT_STRING: return L"STRING";
		case RT_FONTDIR: return L"FONTDIR";
		case RT_FONT: return L"FONT";
		case RT_ACCELERATOR: return L"ACCELERATOR";
		case RT_RCDATA: return L"RCDATA";
		case RT_MESSAGETABLE: return L"MESSAGETABLE";
		
		case RT_GROUP_CURSOR: return L"GROUP_CURSOR";
		case RT_GROUP_ICON: return L"GROUP_ICON";
		case RT_VERSION: return L"VERSION";
		case RT_DLGINCLUDE: return L"DLGINCLUDE";
		case RT_PLUGPLAY: return L"PLUGPLAY";
		case RT_VXD: return L"VXD";
		case RT_ANICURSOR: return L"ANICURSOR";
		case RT_ANIICON: return L"ANIICON";
		case RT_HTML: return L"HTML";
		case RT_MANIFEST: return L"MANIFEST";	
	}

	if(resTypeName != NULL && wcslen(resTypeName) > 0)
		return resTypeName;

	CString str;
	str.Append(resTypeId);
	return str;
}

CString ResNameToString(DWORD resNameId, const WCHAR* resName)
{
	if(resNameId != 0)
	{
		CString str;
		str.Append((WORD)resNameId);
		return str;
	}

	return resName;
}

Gdiplus::Bitmap* LoadBitmapResource(HINSTANCE instance, LPCTSTR resname, LPCTSTR section) 
{  

	HRSRC res = FindResource(instance, resname, section); 
	DWORD err = GetLastError(); 
	if (res == NULL) 
		return NULL;
	
	HGLOBAL hglob = LoadResource(instance, res); 
	DWORD rSize = SizeofResource(instance, res); 

	if (hglob == NULL) 
		return NULL;

	LPVOID lockMem = LockResource(hglob); 
	HGLOBAL copyGlob = GlobalAlloc(GMEM_MOVEABLE, rSize); 
	if (copyGlob == NULL) 
	{
		UnlockResource(hglob);
		return NULL; 
	}

	LPVOID copyMem = GlobalLock(copyGlob); 
	memcpy(copyMem, lockMem, rSize); 
	GlobalUnlock(copyGlob); 
	IStream *stream = NULL;
	CreateStreamOnHGlobal(copyGlob, true, &stream); 
	if (stream == NULL) 
	{
		GlobalUnlock(copyGlob); 
		GlobalFree(copyGlob); 
		UnlockResource(hglob);

		return NULL;
	}

	Bitmap *bmp = Bitmap::FromStream(stream); 
	
	GlobalUnlock(copyGlob); 
	GlobalFree(copyGlob); 
	UnlockResource(hglob); 
	stream->Release(); 

	return bmp; 
}

LRESULT CMainDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	EndDialog(0);
	return 0;
}

CString IntToString(int val)
{
	CString str;
	str.Append(val);
	return str;
}

LRESULT CMainDlg::OnColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	HDC hdc		= (HDC)	wParam;
	HWND hwnd	= (HWND) lParam;

	if(hwnd == GetDlgItem(IDC_EDIT1) || hwnd == GetDlgItem(IDC_ORIGINALRESIMAGE))
	{
		::SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
		return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
	}
	else
	{
		::SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
		return (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 
	}
}


LRESULT CMainDlg::OnCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	COPYDATASTRUCT* cds = (COPYDATASTRUCT*)lParam;
	AveResourceLoadNotification* notify = (AveResourceLoadNotification*)cds->lpData ;
	if(notify != NULL)
	{
		AveResourceLoadNotificationWithTimeStamp*  customNotification = new AveResourceLoadNotificationWithTimeStamp;
		AveResourceLoadNotification* tmp = customNotification;
		*tmp = *notify;
		GetLocalTime(&customNotification->when);
		

		if(NULL == allProcessItem.treeItem)
		{
			allProcessItem.treeItem = tree.InsertItem(L"all", NULL, NULL);
		}

		allProcessItem.notificationList.insert(allProcessItem.notificationList.begin(), customNotification);

		ProcessMapItem* processMapItem = allProcessItem.processes[notify->pid];
		if(NULL == processMapItem)
		{
			processMapItem = new ProcessMapItem;
			allProcessItem.processes[notify->pid] = processMapItem;

			CString name;
			name.Append(notify->pid);
			name += L" - ";
			name += PathFindFileName(notify->processPath);
			processMapItem->treeItem = tree.InsertItem(name, allProcessItem.treeItem, NULL);
			tree.SetItemData(processMapItem->treeItem, (DWORD_PTR)processMapItem);
		}

		processMapItem->notificationList.insert(processMapItem->notificationList.begin(), customNotification);

		CString moduleName = PathFindFileName(notify->modulePath);
		moduleName.MakeLower();

		ModuleMapItem* moduleMapItem = processMapItem->moduleMap[moduleName];
		if(NULL == moduleMapItem)
		{
			moduleMapItem = new ModuleMapItem;
			processMapItem->moduleMap[moduleName] = moduleMapItem;

			moduleMapItem->treeItem = tree.InsertItem(moduleName, processMapItem->treeItem, NULL);
			tree.SetItemData(moduleMapItem->treeItem, (DWORD_PTR)moduleMapItem);
		}

		moduleMapItem->notificationList.insert(moduleMapItem->notificationList.begin(), customNotification);

		CString typeName = ResTypeToString(notify->resourceTypeId, notify->resourceTypeName);
		ResourceTypeMapItem* typeMapItem = moduleMapItem->typeMapItem[typeName];
		if(NULL == typeMapItem)
		{
			typeMapItem = new ResourceTypeMapItem;
			moduleMapItem->typeMapItem[typeName] = typeMapItem;

			typeMapItem->treeItem = tree.InsertItem(typeName, moduleMapItem->treeItem, NULL);
			tree.SetItemData(typeMapItem->treeItem, (DWORD_PTR)typeMapItem);
		}

		typeMapItem->notificationList.insert(typeMapItem->notificationList.begin(), customNotification);

		if(tree.GetSelectedItem() == NULL)
		{
			tree.SelectItem(typeMapItem->treeItem);
		}

		HTREEITEM selectedItem = tree.GetSelectedItem();
		if(selectedItem == typeMapItem->treeItem || selectedItem == moduleMapItem->treeItem ||
			selectedItem == processMapItem->treeItem || selectedItem == allProcessItem.treeItem)
		{
			NotificationList& notificationList = GetSelectedNotificationList();
			listview.SetItemCount(notificationList.size());
		}
		
	}

	return 0;
}

LRESULT CMainDlg::OnBnClickedStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(NULL == hMod)
	{
		MessageBox(_T("could not load the hook module"), NULL, MB_ICONERROR);
		return 0;
	}

	if(SetSpyCallbackWindow != NULL)
		SetSpyCallbackWindow(m_hWnd, 1);

	StartHook(hMod, m_hWnd);
	BOOL isRunning = IsHookRunning();
	::EnableWindow(GetDlgItem(IDC_START),!isRunning);
	::EnableWindow(GetDlgItem(IDC_STOP),isRunning);


	return 0;
}

LRESULT CMainDlg::OnBnClickedStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(NULL == hMod)
	{
		MessageBox(_T("could not load the hook module"), NULL, MB_ICONERROR);
		return 0;
	}

	if(SetSpyCallbackWindow != NULL)
		SetSpyCallbackWindow(0, 0);

	StopHook();
	BOOL isRunning = IsHookRunning();
	::EnableWindow(GetDlgItem(IDC_START),!isRunning);
	::EnableWindow(GetDlgItem(IDC_STOP),isRunning);

	return 0;
}


void CMainDlg::HideViewers()
{
	CStatic img = GetDlgItem(IDC_ORIGINALRESIMAGE);
	CStatic icon = GetDlgItem(IDC_ORIGINALRESICON);

	img.SetBitmap(NULL);

	img.ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT1).ShowWindow(SW_HIDE);

	icon.ShowWindow(SW_HIDE);

	hexEditor.ShowWindow(SW_HIDE);
}

LRESULT CMainDlg::OnLvnItemchangedEntries(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	CStatic img = GetDlgItem(IDC_ORIGINALRESIMAGE);
	CStatic icon = GetDlgItem(IDC_ORIGINALRESICON);
	//img.ModifyStyle(SS_ICON, SS_BITMAP, 0);

	HideViewers();
	
	NotificationList& notificationList = GetSelectedNotificationList();


	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	int index = listview.GetSelectedIndex();//listview.GetItemCount() - listview.GetSelectedIndex() - 1;
	if(index >= 0 && index < (int)notificationList.size())
	{
		
		AveResourceLoadNotificationWithTimeStamp* notify = notificationList[index];

		const WCHAR* path = notify->modulePath;
		if(wcslen(path) == 0)
			path = notify->processPath;

		HMODULE library = LoadLibraryEx(path, NULL, LOAD_LIBRARY_AS_DATAFILE);
		if(library != NULL)
		{
			int type =	notify->resourceTypeId == (WORD)RT_BITMAP ? IMAGE_BITMAP :
						notify->resourceTypeId == (WORD)RT_ICON ? IMAGE_ICON :				
						notify->resourceTypeId == (WORD)IMAGE_CURSOR ? IMAGE_BITMAP : 0;
			

			if(MAKEINTRESOURCE(notify->resourceTypeId) == RT_STRING)
			{
				HRSRC res = FindResource(library, MAKEINTRESOURCE(notify->resourceId), RT_STRING);
				HGLOBAL hGlobal = LoadResource(library, res);
				CString strings;

				if(hGlobal != NULL)
				{
					LPCWSTR  data = (LPCWSTR )LockResource(hGlobal);
					DWORD size = SizeofResource(library, res);

					int startId = (notify->resourceId -1) * 16;
					
					//strings += L"{\r\n";
					for (int i = 0; i < 16; i++) 
					{
						UINT len = (UINT)*data;
						data += 1;
						if(len == 0)
							continue;
						const WCHAR* strNotZeroTerminated = data;
						data += len;

						strings.Append(startId + i);
						strings += L", \"";
						CString tmp(strNotZeroTerminated, len);
						strings += tmp;
						strings += L"\"\r\n";
					}

					//strings += L"\r\n";

					
					UnlockResource(res);
					FreeResource(hGlobal);
				}
				else
				{
					strings = L"Error loading resource";
				}

				GetDlgItem(IDC_EDIT1).SetWindowText(strings);
				GetDlgItem(IDC_EDIT1).ShowWindow(SW_SHOW);
				
			}
			else if(MAKEINTRESOURCE(notify->resourceTypeId) == RT_BITMAP)
			{
				HANDLE h = LoadImage(library, notify->resourceId ? (WCHAR*)notify->resourceId : notify->resourceName, type, 0, 0, 0);
				img.ShowWindow(SW_SHOW);
				HGDIOBJ oldh = img.SetBitmap((HBITMAP)h);
				DeleteObject(oldh);
				DeleteObject(h);
			}
			else if(MAKEINTRESOURCE(notify->resourceTypeId) == RT_ICON)
			{
				HANDLE h = LoadImage(library, notify->resourceId ? (WCHAR*)notify->resourceId : notify->resourceName, type, 0, 0, 0);
				icon.ShowWindow(SW_SHOW);
				HGDIOBJ oldh = icon.SetIcon((HICON)h);
				DeleteObject(oldh);
				DeleteObject(h);

			}
			else if(MAKEINTRESOURCE(notify->resourceTypeId) == RT_CURSOR)
			{
				HANDLE h = LoadImage(library, notify->resourceId ? (WCHAR*)notify->resourceId : notify->resourceName, type, 0, 0, 0);
				icon.ShowWindow(SW_SHOW);
				HGDIOBJ oldh = icon.SetCursor((HCURSOR)h);
				DeleteObject(oldh);
				DeleteObject(h);
			}
			else
			{
				Bitmap* bmp = LoadBitmapResource(library, notify->resourceId ? (WCHAR*)notify->resourceId : notify->resourceName, notify->resourceTypeId ? (WCHAR*)notify->resourceTypeId : notify->resourceTypeName);
				if(bmp != NULL && bmp->GetLastStatus() == Ok)
				{
					HBITMAP h = NULL;
					bmp->GetHBITMAP(Color::White, &h);
					img.ShowWindow(SW_SHOW);
					HGDIOBJ oldh = img.SetBitmap((HBITMAP)h);
					DeleteObject(oldh);
					DeleteObject(h);
					delete bmp;
				}
				else
				{
					delete bmp;

					HRSRC res = FindResource(library, notify->resourceId ? (WCHAR*)notify->resourceId : notify->resourceName, notify->resourceTypeId ? (WCHAR*)notify->resourceTypeId : notify->resourceTypeName);
					HGLOBAL hGlobal = LoadResource(library, res);
					LPCWSTR  data = (LPCWSTR )LockResource(hGlobal);
					DWORD size = SizeofResource(library, res);

		
					CString str;
					bool isPrintableText = true;
					if(hGlobal != NULL)
					{
						if(IsTextUnicode(data, size, NULL))
						{
							str = CString(data, size);

							WORD* charTypes = new WORD[size/2];
							memset(charTypes, 0, sizeof(WORD) * (size/2));
							::GetStringTypeW(CT_CTYPE1, data, size/2, charTypes);
							for(size_t i = 0; i < size/2; ++i)
							{
								if(charTypes[i] & C1_CNTRL)
								{
									if(i == 0 && data[i] == WCHAR(0xFFFE))
										continue;

									isPrintableText = false;
									break;
								}
							}
							delete[] charTypes;
						}
						else
						{
							WCHAR* buffer = new WCHAR[size+1];
							memset(buffer, 0, (size+1) * sizeof(WCHAR));
							MultiByteToWideChar(GetACP(), 0, (const char*)data, size, buffer, size+1);
							str = buffer;
							delete[] buffer;

							WORD* charTypes = new WORD[size];
							memset(charTypes, 0, sizeof(WORD) * (size));
							::GetStringTypeA(LOCALE_USER_DEFAULT, CT_CTYPE1, (LPCSTR)data, size, charTypes);
							for(size_t i = 0; i < size; ++i)
							{
								if(charTypes[i] & C1_CNTRL)
								{
									isPrintableText = false;
									break;
								}
							}
							delete[] charTypes;
						}

						
					}
					else
					{
						str = L"Error loading resource";
					}

					

					if(isPrintableText)
					{
						GetDlgItem(IDC_EDIT1).SetWindowText(str);
						GetDlgItem(IDC_EDIT1).ShowWindow(SW_SHOW);
					}
					else
					{
						static WCHAR tmpFile[MAX_PATH] = {0};
						if(wcslen(tmpFile) == 0)
						{
							GetTempPath(_countof(tmpFile), tmpFile);
							GetTempFileName(tmpFile, L"res", 0, tmpFile);

						}

						hexEditor.SetFilename(L"");
						CFile file;
						file.Create(tmpFile);
						file.Write(data, size);
						file.Close();

						
						hexEditor.SetFilename(tmpFile);
						hexEditor.ShowWindow(SW_SHOW);
					}

					UnlockResource(res);
					FreeResource(hGlobal);
				}
			}

			

			FreeLibrary(library);
		}
	}

	return 0;
}

LRESULT CMainDlg::OnBnClickedClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	NotificationList& notificationList = GetSelectedNotificationList();
	if(&notificationList == &allProcessItem.notificationList)
	{
		for(size_t i = 0; i < notificationList.size(); ++i)
		{
			delete notificationList[i];
		}
	}

	notificationList.clear();

	/*ResourceTypeMapItem* typeMapItem = GetSelectedResourceTypeMap();
	if(typeMapItem != NULL)
	{
		typeMapItem->notificationList.clear();
	}
	else
	{
		// XXX todo
	}*/

	listview.DeleteAllItems();

	CStatic img = GetDlgItem(IDC_ORIGINALRESIMAGE);
	img.SetBitmap(NULL);

	return 0;
}


CMainDlg::NotificationList& CMainDlg::GetSelectedNotificationList()
{
	CString debugName;
	HTREEITEM originalTreeItem = tree.GetSelectedItem();
	HTREEITEM treeItem = tree.GetSelectedItem();
	tree.GetItemText(treeItem, debugName);
	if(tree.GetParentItem(treeItem) == NULL)
		return allProcessItem.notificationList;

	treeItem  = tree.GetParentItem(treeItem);
	tree.GetItemText(treeItem, debugName);
	if(tree.GetParentItem(treeItem) == NULL)
		return ((ProcessMapItem*)tree.GetItemData(originalTreeItem))->notificationList;

	treeItem  = tree.GetParentItem(treeItem);
	tree.GetItemText(treeItem, debugName);
	if(tree.GetParentItem(treeItem) == NULL)
		return ((ModuleMapItem*)tree.GetItemData(originalTreeItem))->notificationList;

	treeItem  = tree.GetParentItem(treeItem);
	tree.GetItemText(treeItem, debugName);
	if(tree.GetParentItem(treeItem) == NULL)
		return ((ResourceTypeMapItem*)tree.GetItemData(originalTreeItem))->notificationList;

	return allProcessItem.notificationList;
}

void CopyStringToItem(NMLVDISPINFO *pDispInfo, const CString& str)
{
	wcsncpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, str, pDispInfo->item.cchTextMax);
}

LRESULT CMainDlg::OnLvnGetdispinfoEntries(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	if((pDispInfo->item.mask & LVIF_TEXT) == 0)
		return 0;

	NotificationList& notificationList = GetSelectedNotificationList();
	AveResourceLoadNotificationWithTimeStamp* customNotification = notificationList[pDispInfo->item.iItem];

	switch (pDispInfo->item.iSubItem)
	{
		case 0:
			{
				WCHAR nowAsString[1024] = {0};
				GetTimeFormat(LOCALE_USER_DEFAULT, 0, &customNotification->when, NULL, nowAsString, _countof(nowAsString));
				CopyStringToItem(pDispInfo, nowAsString);
			}
		break;

		case 1:
			CopyStringToItem(pDispInfo, ResTypeToString(customNotification->resourceTypeId, customNotification->resourceTypeName));
		break;

		case 2:
			CopyStringToItem(pDispInfo, ResNameToString(customNotification->resourceId, customNotification->resourceName));
		break;

		case 3:
			CopyStringToItem(pDispInfo, customNotification->overridenResourcePath);
		break;

	}

	return 1;
}

LRESULT CMainDlg::OnTvnSelchangedTree1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	
	HideViewers();

	NotificationList& notificationList = GetSelectedNotificationList();
	listview.SetItemState(listview.GetSelectedIndex(), 0, LVIS_SELECTED | LVIS_FOCUSED);
	listview.SetItemCount(notificationList.size());
	listview.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	return 0;
}
