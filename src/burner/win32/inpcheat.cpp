#include "burner.h"

HWND hInpCheatDlg = NULL;				// Handle to the Cheat Dialog

static HWND hInpCheatList = NULL;

static bool bOK;

static int nCurrentCheat;
static int* nPrevCheatSettings = NULL;

static int nDlgInitialWidth;
static int nDlgInitialHeight;
static int nDlgCheatLstInitialPos[4];
static int nDlgValueBoxInitialPos[4];
static int nDlgResetBtnInitialPos[4];
static int nDlgOkBtnInitialPos[4];
static int nDlgCancelBtnInitialPos[4];

#define GetInititalControlPos(a, b)								\
	GetWindowRect(GetDlgItem(hInpCheatDlg, a), &rect);			\
	memset(&point, 0, sizeof(POINT));							\
	point.x = rect.left;										\
	point.y = rect.top;											\
	ScreenToClient(hInpCheatDlg, &point);						\
	b[0] = point.x;												\
	b[1] = point.y;												\
	GetClientRect(GetDlgItem(hInpCheatDlg, a), &rect);			\
	b[2] = rect.right;											\
	b[3] = rect.bottom;

#define SetControlPosAlignTopLeft(a, b)							\
	SetWindowPos(GetDlgItem(hInpCheatDlg, a), hInpCheatDlg, b[0],          b[1], 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOSENDCHANGING);

#define SetControlPosAlignBottomRight(a, b)						\
	SetWindowPos(GetDlgItem(hInpCheatDlg, a), hInpCheatDlg, b[0] - xDelta, b[1] - yDelta, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOSENDCHANGING);

#define SetControlPosAlignBottomLeft(a, b)						\
	SetWindowPos(GetDlgItem(hInpCheatDlg, a), hInpCheatDlg, b[0],          b[1] - yDelta, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOSENDCHANGING);

#define SetControlPosAlignTopLeftResizeHorVert(a, b)			\
	SetWindowPos(GetDlgItem(hInpCheatDlg, a), hInpCheatDlg, b[0],          b[1],          b[2] - xDelta - xScrollBarDelta, b[3] - yDelta, SWP_NOZORDER | SWP_NOSENDCHANGING);

static void GetInitialPositions()
{
	RECT rect;
	POINT point;

	GetClientRect(hInpCheatDlg, &rect);
	nDlgInitialWidth = rect.right;
	nDlgInitialHeight = rect.bottom;

	GetInititalControlPos(IDC_INPCHEAT_LIST, nDlgCheatLstInitialPos);
	GetInititalControlPos(IDC_INPCX1_VALUE, nDlgValueBoxInitialPos);
	GetInititalControlPos(IDC_INPC_RESET, nDlgResetBtnInitialPos);
	GetInititalControlPos(IDOK, nDlgOkBtnInitialPos);
	GetInititalControlPos(IDCANCEL, nDlgCancelBtnInitialPos);
}


static int InpCheatListBegin()
{
	LVCOLUMN LvCol;
	if (hInpCheatList == NULL) {
		return 1;
	}

	// Full row select style:
	SendMessage(hInpCheatList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	// Make column headers
	memset(&LvCol, 0, sizeof(LvCol));
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	LvCol.cx = 0x105;
	LvCol.pszText = FBALoadStringEx(hAppInst, IDS_CHEAT_NAME, true);
	SendMessage(hInpCheatList, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol);
	LvCol.cx = 0x35;
	LvCol.pszText = FBALoadStringEx(hAppInst, IDS_CHEAT_STATUS, true);
	SendMessage(hInpCheatList, LVM_INSERTCOLUMN, 1, (LPARAM)&LvCol);

	return 0;
}

// Make a list view of the DIPswitches
static int InpCheatListMake(bool build)
{
	if (hInpCheatList == NULL) {
		return 1;
	}

	if (build) {
		SendMessage(hInpCheatList, LVM_DELETEALLITEMS, 0, 0);
	}

	int i = 0;
	CheatInfo* pCurrentCheat = pCheatInfo;

	while (pCurrentCheat) {

		LVITEM LvItem;
		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_TEXT;
		LvItem.iItem = i;
		if (build) {
			LvItem.iSubItem = 0;
			LvItem.pszText = pCurrentCheat->szCheatName;
			SendMessage(hInpCheatList, LVM_INSERTITEM, 0, (LPARAM)&LvItem);
			LvItem.mask = LVIF_TEXT;
		}
		
		LvItem.iSubItem = 1;
		LvItem.pszText = pCurrentCheat->pOption[pCurrentCheat->nCurrent]->szOptionName;
		SendMessage(hInpCheatList, LVM_SETITEM, 0, (LPARAM)&LvItem);

		pCurrentCheat = pCurrentCheat->pNext;
		i++;
	}

	return 0;
}

static int InpCheatInit()
{
	hInpCheatList = GetDlgItem(hInpCheatDlg, IDC_INPCHEAT_LIST);
	InpCheatListBegin();
	InpCheatListMake(true);

	// Save old cheat settings
	CheatInfo* pCurrentCheat = pCheatInfo;
	nCurrentCheat = 0;
	while (pCurrentCheat) {
		pCurrentCheat = pCurrentCheat->pNext;
		nCurrentCheat++;
	}

	nPrevCheatSettings = (int*)malloc(nCurrentCheat * sizeof(int));

	pCurrentCheat = pCheatInfo;
	nCurrentCheat = 0;
	while (pCurrentCheat) {
		nPrevCheatSettings[nCurrentCheat] = pCurrentCheat->nCurrent;
		pCurrentCheat = pCurrentCheat->pNext;
		nCurrentCheat++;
	}

	return 0;
}

static int InpCheatExit()
{
	if (nPrevCheatSettings) {
		free(nPrevCheatSettings);
		nPrevCheatSettings = NULL;
	}

	hInpCheatList = NULL;
	hInpCheatDlg = NULL;
	if(!bAltPause && bRunPause) {
		bRunPause = 0;
	}
	GameInpCheckMouse();
	return 0;
}

static void InpCheatCancel()
{
	if (!bOK) {
		CheatInfo* pCurrentCheat = pCheatInfo;
		nCurrentCheat = 0;
		while (pCurrentCheat) {
			CheatEnable(nCurrentCheat, nPrevCheatSettings[nCurrentCheat]);
			pCurrentCheat = pCurrentCheat->pNext;
			nCurrentCheat++;
		}
	}
}

static void InpCheatSelect()
{
	SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_RESETCONTENT, 0, 0);

	int nSel = SendMessage(hInpCheatList, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
	if (nSel >= 0) {
		LVITEM LvItem;
		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_PARAM;
		LvItem.iItem = nSel;
		SendMessage(hInpCheatList, LVM_GETITEM, 0, (LPARAM)&LvItem);

		CheatInfo* pCurrentCheat = pCheatInfo;
		nCurrentCheat = 0;
		while (pCurrentCheat && nCurrentCheat < nSel) {
			pCurrentCheat = pCurrentCheat->pNext;
			nCurrentCheat++;
		}

		for (int i = 0; pCurrentCheat->pOption[i]; i++) {
			TCHAR szText[256];
			_stprintf(szText, _T("%s: %s"), pCurrentCheat->szCheatName, pCurrentCheat->pOption[i]->szOptionName);
			SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_ADDSTRING, 0, (LPARAM)szText);
		}

		SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_SETCURSEL, (WPARAM)pCurrentCheat->nCurrent, 0);
	}
}

static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_INITDIALOG) {

//		EnableWindow(hScrnWnd, FALSE);

		hInpCheatDlg = hDlg;
		InpCheatInit();
		if (!kNetGame && bAutoPause) {
			bRunPause = 1;
		}

		GetInitialPositions();

		WndInMid(hDlg, hScrnWnd);
		SetFocus(hDlg);											// Enable Esc=close

		return TRUE;
	}

    if (Msg == WM_CLOSE) {
		EnableWindow(hScrnWnd, TRUE);
		DestroyWindow(hInpCheatDlg);
		return 0;
	}

	if (Msg == WM_GETMINMAXINFO) {
		MINMAXINFO *info = (MINMAXINFO*)lParam;

		info->ptMinTrackSize.x = nDlgInitialWidth;
		info->ptMinTrackSize.y = nDlgInitialHeight;

		return 0;
	}

	if (Msg == WM_WINDOWPOSCHANGED) {
		RECT rc;
		int xDelta;
		int yDelta;
		int xScrollBarDelta = -24;

		if (nDlgInitialWidth == 0 || nDlgInitialHeight == 0) return 0;

		GetClientRect(hDlg, &rc);

		xDelta = nDlgInitialWidth - rc.right;
		yDelta = nDlgInitialHeight - rc.bottom;

		if (xDelta == 0 && yDelta == 0) return 0;

		SetControlPosAlignTopLeftResizeHorVert(IDC_INPCHEAT_LIST, nDlgCheatLstInitialPos);
		SetControlPosAlignBottomLeft(IDC_INPCX1_VALUE, nDlgValueBoxInitialPos);
		SetControlPosAlignBottomRight(IDC_INPC_RESET, nDlgResetBtnInitialPos);
		SetControlPosAlignBottomRight(IDOK, nDlgOkBtnInitialPos);
		SetControlPosAlignBottomRight(IDCANCEL, nDlgCancelBtnInitialPos);
		InvalidateRect(hInpCheatDlg, NULL, true);
		UpdateWindow(hInpCheatDlg);

		return 0;
	}

	if (Msg == WM_DESTROY) {
		InpCheatCancel();
		InpCheatExit();
		return 0;
	}

	if (Msg == WM_COMMAND) {
		int Id = LOWORD(wParam);
		int Notify = HIWORD(wParam);

		if (Id == IDOK && Notify == BN_CLICKED) {			// OK button
			bOK = true;
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return 0;
		}
		if (Id == IDCANCEL && Notify == BN_CLICKED) {		// cancel = close
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return 0;
		}

		if (Id == IDC_INPCX1_VALUE && Notify == CBN_SELCHANGE) {

			int nSel = SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_GETCURSEL, 0, 0);

			CheatEnable(nCurrentCheat, nSel);

			InpCheatListMake(false);
			return 0;
		}

		if (Id == IDC_INPC_RESET && Notify == BN_CLICKED) {

			CheatInfo* pCurrentCheat = pCheatInfo;
			nCurrentCheat = 0;
			while (pCurrentCheat) {
				CheatEnable(nCurrentCheat, -1);
				pCurrentCheat = pCurrentCheat->pNext;
				nCurrentCheat++;
			}

			InpCheatListMake(true);								// refresh view
			SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_RESETCONTENT, 0, 0);
			return 0;
	   }

	}

	if (Msg == WM_NOTIFY && lParam) {
		int Id = LOWORD(wParam);
		NMHDR *pnm = (NMHDR*)lParam;

		if (Id == IDC_INPCHEAT_LIST && pnm->code == LVN_ITEMCHANGED) {
			if (((NM_LISTVIEW*)lParam)->uNewState & LVIS_SELECTED) {
				InpCheatSelect();
			}
			return 0;
		}

		if (Id == IDC_INPCHEAT_LIST && ((pnm->code == NM_DBLCLK) || (pnm->code == NM_RDBLCLK))) {
			// Select the next item of the currently selected one.
			int nSel_Dbl = SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_GETCURSEL, 0, 0);
			int nCount = SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_GETCOUNT, 0, 0);
			if ((nSel_Dbl != LB_ERR) && (nCount > 1)) {
				if (pnm->code == NM_DBLCLK) {
					if (++nSel_Dbl >= nCount) nSel_Dbl = 0;
				} else {
					if (--nSel_Dbl < 0) nSel_Dbl = nCount - 1;
				}
				SendMessage(GetDlgItem(hInpCheatDlg, IDC_INPCX1_VALUE), CB_SETCURSEL, nSel_Dbl, 0);
				CheatEnable(nCurrentCheat, nSel_Dbl);
				InpCheatListMake(false);
			}
			return 0;
		}
	}

	return 0;
}

int InpCheatCreate()
{
	if (bDrvOkay == 0) {									// No game is loaded
		return 1;
	}

	bOK = false;

//	DestroyWindow(hInpCheatDlg);							// Make sure exitted

	hInpCheatDlg = FBACreateDialog(hAppInst, MAKEINTRESOURCE(IDD_INPCHEAT), hScrnWnd, DialogProc);
	if (hInpCheatDlg == NULL) {
		return 1;
	}

	WndInMid(hInpCheatDlg, hScrnWnd);
	ShowWindow(hInpCheatDlg, SW_NORMAL);

//	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_INPCHEAT), hScrnWnd, (DLGPROC)DialogProc);

	return 0;
}

