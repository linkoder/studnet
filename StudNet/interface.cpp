#include "interface.h"


HWND			_hWnd;

std::wostream& wendl(std::wostream& out)
{
	out.put(L'\r');
	out.put(L'\n');
	out.flush();
	return out;
}//wendl*/

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	 return (INT_PTR)true;

	case WM_CLOSE:	
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)true;
		break;
	} 
	  return (INT_PTR)false;
}

bool App::SaveSettings()
{
	
	//ShowWindow(_hwndEdit,SW_HIDE);
	//::RedrawWindow(_hWnd, NULL, NULL, RDW_INVALIDATE);
		std::wfstream file;
		
			null_wcodecvt wcodec(1);
			std::locale wloc(std::locale::classic(), &wcodec);
			file.imbue(wloc);

			if (week==numweek::firstweek)
			{
				file.open("week", std::ios::out | std::ios::binary);
			}
			else if (week==numweek::secondweek)
			{
				file.open("week2", std::ios::out | std::ios::binary);
			}
			else
			{
				return false;
			}

			if (!file)
			{
				MessageBox(0,L"Failed to open week for writting",L"Error",MB_ICONERROR);
				return false;
			}

				file << UTF_BOM;
				file << L"Lin" << " ";
					for (int x = _rows-1;x--;)
					{
						for (int y =_columns-1; y--;)
						{						
							file << _data[x][y].c_str() << L" ";					
						}
						
					}
		
				file <<  CHECK_SYM << wendl;
				file.close();
	//	MessageBox(0,L"Сохранение прошло успешно",L"Done",MB_ICONINFORMATION);
	return true;
}

bool App::LoadSettings()
{
	std::wfstream file;

	null_wcodecvt wcodec(1);
	std::locale wloc(std::locale::classic(), &wcodec);
	file.imbue(wloc);
	if (week==numweek::firstweek)
	{
		file.open("week", std::ios::in | std::ios::binary);
	}
	else if (week==numweek::secondweek)
	{
		file.open("week2", std::ios::in | std::ios::binary);
	}
	else
	{
		return false;
	}
	
	if (!file)
	{
		MessageBox(0,L"Failed to open data for writting",L"Error",MB_ICONERROR);
		return false;
	}

//	file << UTF_BOM;
	wchar_t str[10] ;
	
	file >> str;
	file >> str;
	for (int x = _rows-1;x--;)
	{
		for (int y =_columns-1; y--;)
		{						
    		_data[x][y]=str;
 			file >> str;
		}

	}

//	file <<  CHECK_SYM << wendl;
	file.close();



	return true;
}
LRESULT CALLBACK App::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	int wmId,wmEvent;
	PAINTSTRUCT ps;
 	HDC hdc;

	switch(message)
	{
#pragma region draw
	case WM_CREATE:
			{				
				
				SCROLLINFO info;
				info.cbSize = sizeof(info);
				info.fMask  = SIF_RANGE;
				info.nMin	= 0;

				info.nMax	= _columns;
				::SetScrollInfo(hWnd,SB_HORZ,&info,TRUE);

				info.nMax	= _rows;
				::SetScrollInfo(hWnd,SB_VERT,&info,TRUE);
				
				break;
			};//WM_CREATE
	case WM_DESTROY:
		{
			App *This = (App*) GetWindowLong(hWnd, 0);
			if(This->SaveSettings())
			{
				;
			}
			else
			{
				MessageBox(0,L"приложение не сохранено",L"Warning",MB_ICONINFORMATION);
			}
			PostQuitMessage(0);
			break;
		};//WM_DESTROY
	case WM_ERASEBKGND:
		{
			if(::WindowFromDC((HDC)wParam) == hWnd){
				return TRUE;
			}
			break;
		};
	case WM_PAINT:
		{
			HGDIOBJ hmembmp;
			HDC  hmemdc;
			LONG ox,oy,ucx,ucy,ux,uy;
			LONG cx,cy;

			hdc = BeginPaint(hWnd,&ps);

			cx		= ps.rcPaint.right - ps.rcPaint.left;
			cy		= ps.rcPaint.bottom - ps.rcPaint.top;


			ox	= ::GetScrollPos(hWnd,SB_HORZ);
			oy	= ::GetScrollPos(hWnd,SB_VERT);				

			if(_hWnd==GetActiveWindow() && _hWnd !=0)
			{
				App *This = (App*) GetWindowLong(hWnd, 0);

				ux	= ps.rcPaint.left /  This->_column_width	;
				uy	= ps.rcPaint.top /	 This->_row_height	;
			}
			else
			{
				ux	= ps.rcPaint.left / 11	;
				uy	= ps.rcPaint.top /	10	;
			}
			hmemdc	= ::CreateCompatibleDC(hdc);
			hmembmp	= ::CreateCompatibleBitmap(hdc,cx,cy);
			hmembmp	= ::SelectObject(hmemdc,hmembmp);					

			::SetWindowOrgEx(hmemdc,ps.rcPaint.left,ps.rcPaint.top,NULL);
			::SendMessage(hWnd,WM_ERASEBKGND,(WPARAM)hmemdc,0);

			::SendMessage(hWnd,WM_ERASEBKGND,(WPARAM)hmemdc,0);
			if( (ox + ux < _columns) && (oy + uy < _rows) )
			{
				//

				if(_hWnd==GetActiveWindow() && _hWnd !=0)
				{
					App *This = (App*) GetWindowLong(_hWnd, 0);
					ucx	= (ps.rcPaint.right / This->_column_width) + ( (ps.rcPaint.right % This->_column_width)?1:0 ) - ux;
					ucy	= (ps.rcPaint.bottom / This->_row_height) + ( (ps.rcPaint.bottom % This->_row_height)?1:0 ) - uy;
					//
				}
				else
				{

					ucx	=	10;
					ucy	=	10;
				}
				if(ox + ux + ucx > _columns){
					ucx	= _columns - ox - ux;
				}
				if(oy + uy + ucy > _rows){
					ucy	= _rows - oy - uy;
				}
				
					App *This = (App*) GetWindowLong(hWnd, 0);
					This->DrawGrid(hWnd,hmemdc,ox,oy,ux,uy,ucx,ucy);
					This->DrawCells(hWnd,hmemdc,ox,oy,ux,uy,ucx,ucy);
				
				::BitBlt(hdc,ps.rcPaint.left,ps.rcPaint.top,cx,cy,hmemdc,ps.rcPaint.left,ps.rcPaint.top,SRCCOPY);
				hmembmp	= ::SelectObject(hmemdc,hmembmp);
				::DeleteObject(hmembmp);
				EndPaint(hWnd,&ps);
				break;
			}
		}

	case WM_VSCROLL:{
		SCROLLINFO info;
		RECT rc;
		LONG oldy,newy;
		//
		info.cbSize	= sizeof(info);
		info.fMask	= SIF_PAGE | SIF_POS;
		::GetScrollInfo(hWnd,SB_VERT,&info);
		oldy		= info.nPos;
		switch(LOWORD(wParam)){
	case SB_LINELEFT:
		--info.nPos;
		break;
	case SB_LINERIGHT:
		++info.nPos;
		break;
	case SB_PAGELEFT:
		info.nPos	-= info.nPage;
		break;
	case SB_PAGERIGHT:
		info.nPos	+= info.nPage;
		break;
	case SB_THUMBTRACK:
		info.nPos	 = HIWORD(wParam);
		break;
		}
		::SetScrollInfo(hWnd,SB_VERT,&info,TRUE);
		newy	= GetScrollPos(hWnd,SB_VERT);
		::GetClientRect(hWnd,&rc);
		if(_hWnd==GetActiveWindow() && _hWnd !=0)
		{
			App *This = (App*) GetWindowLong(_hWnd, 0);
			rc.left	+= This->_column_width;
			rc.top	+= This->_row_height;
			::ScrollWindow(hWnd,0,(oldy - newy) * This->_row_height,&rc,&rc);
			rc.right	 = rc.left;
			rc.left		-= This->_column_width;
			::ScrollWindow(hWnd,0,(oldy - newy) * This->_row_height,&rc,&rc);
			This->ShiftSelection(hWnd);
		}
		break;
					}
	case WM_HSCROLL:{
		SCROLLINFO info;
		RECT rc;
		LONG oldx,newx;
		//
		info.cbSize	= sizeof(info);
		info.fMask	= SIF_PAGE | SIF_POS;
		::GetScrollInfo(hWnd,SB_HORZ,&info);
		oldx		= info.nPos;
		switch(LOWORD(wParam)){
	case SB_LINELEFT:
		--info.nPos;
		break;
	case SB_LINERIGHT:
		++info.nPos;
		break;
	case SB_PAGELEFT:
		info.nPos	-= info.nPage;
		break;
	case SB_PAGERIGHT:
		info.nPos	+= info.nPage;
		break;
	case SB_THUMBTRACK:
		info.nPos	 = HIWORD(wParam);
		break;
		}
		::SetScrollInfo(hWnd,SB_HORZ,&info,TRUE);
		newx	= GetScrollPos(hWnd,SB_HORZ);
		::GetClientRect(hWnd,&rc);
		if(_hWnd==GetActiveWindow() && _hWnd !=0)
		{
			App *This = (App*) GetWindowLong(_hWnd, 0);
			rc.left	+= This->_column_width;
			rc.top	+= This->_row_height;
			::ScrollWindow(hWnd,(oldx - newx) * This->_column_width,0,&rc,&rc);
			rc.bottom	 = rc.top;
			rc.top		-= This->_row_height;
			::ScrollWindow(hWnd,(oldx - newx) * This->_column_width,0,&rc,&rc);
			This->ShiftSelection(hWnd);
		}

		break;
					}
	case WM_SIZE:
		{

			SCROLLINFO info;
			POINT oldp,newp;
			//
			oldp.x	= ::GetScrollPos(hWnd,SB_HORZ);
			oldp.y	= ::GetScrollPos(hWnd,SB_VERT);
			//
			info.cbSize	= sizeof(info);
			info.fMask	= SIF_PAGE;
			//
			if(_hWnd==GetActiveWindow() && _hWnd !=0)
			{
				App *This = (App*) GetWindowLong(_hWnd, 0);
				info.nPage	= (short)LOWORD(lParam) / This->_column_width;
				if((short)LOWORD(lParam) % This->_column_width){
					++info.nPage;
				}
				::SetScrollInfo(hWnd,SB_HORZ,&info,TRUE);
				//
				info.nPage	= (short)HIWORD(lParam) / This->_row_height;
				if((short)HIWORD(lParam) % This->_row_height)
				{
					++info.nPage;
				}

				::SetScrollInfo(hWnd,SB_VERT,&info,TRUE);
				newp.x	= ::GetScrollPos(hWnd,SB_HORZ);
				newp.y	= ::GetScrollPos(hWnd,SB_VERT);
				if( (oldp.x != newp.x) || (oldp.y != newp.y) )
				{
					::InvalidateRect(hWnd,NULL,FALSE);
					This->ShiftSelection(hWnd);
				}
			}
			break;


		}
#pragma endregion draw
	
#pragma region change
	case WM_LBUTTONDOWN:{
			RECT rc;
			POINT pt;
			LONG x,y,nc,nr;
			//
			pt.x	= (short)LOWORD(lParam);
			pt.y	= (short)HIWORD(lParam);
			::GetClientRect(hWnd,&rc);
			x	= ::GetScrollPos(hWnd,SB_HORZ);
			y	= ::GetScrollPos(hWnd,SB_VERT);
				if(_hWnd==GetActiveWindow() && _hWnd !=0)
			{
				App *This = (App*) GetWindowLong(hWnd, 0);
			nc	= x + (pt.x - rc.left) / This->_column_width;
			nr	= y + (pt.y - rc.top) / This->_row_height;
			if( (nc > 0) && (nr > 0) && (nc < _columns) && (nr < _rows) ){
				if(This->modifable){
				This->SelectCell(hWnd,nc,nr);
				::SetCapture(hWnd);
				This->_captured	= TRUE;
				}
			}
				break;
			}
			break;
			}
	case WM_CAPTURECHANGED:{
			if(_hWnd==GetActiveWindow() && _hWnd !=0)
			{
				App *This = (App*) GetWindowLong(_hWnd, 0);
			if( !This->_captured )break;
			//
			This->_captured	= FALSE;
			}
			break;
							   }

	case WM_KEYDOWN:
		{// Which key was pressed?
		switch (wParam) 
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case VK_TAB:
			App *This = (App*) GetWindowLong(_hWnd, 0);
			if(This->week == numweek::firstweek)
			{
				This->week = numweek::secondweek;
				This->LoadSettings();
				::RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			}
			else
			{
				This->week = numweek::firstweek;
				This->LoadSettings();
				::RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			}
			break;
		} // End Switch
		}
		break;

		case WM_LBUTTONUP:{
			if(_hWnd==GetActiveWindow() && _hWnd !=0)
			{
				App *This = (App*) GetWindowLong(_hWnd, 0);
			if( !This->_captured )break;
			::ReleaseCapture();
			}
			break;
						  }
		case WM_MOUSEMOVE:{
			RECT rc;
			POINT pt;
			LONG x,y,nc,nr;
			//
			pt.x	= (short)LOWORD(lParam);
			pt.y	= (short)HIWORD(lParam);
			::GetClientRect(hWnd,&rc);
			x	= ::GetScrollPos(hWnd,SB_HORZ);
			y	= ::GetScrollPos(hWnd,SB_VERT);
			if(_hWnd==GetActiveWindow() && _hWnd !=0)
			{
				App *This = (App*) GetWindowLong(_hWnd, 0);
			if( !This->_captured )break;
			nc	= x + (pt.x - rc.left) / This->_column_width;
			nr	= y + (pt.y - rc.top) / This->_row_height;
			if( (nc != This->_sel_column) || (nr != This->_sel_row) ){
				if( (nc > x) && (nr > y) && (nc < _columns) && (nr < _rows) ){
					if(This->modifable){
					This->SelectCell(hWnd,nc,nr);
}
			}
					break;
				}
			}
			break;
						  }
		case WM_SETFOCUS:
			{
				if(_hWnd==GetActiveWindow() && _hWnd !=0)
				{
					App *This = (App*) GetWindowLong(_hWnd, 0);
					if( This->_hwndEdit ){
						::SetFocus(This->_hwndEdit);
					}
				}
				break;
			}
		case WM_COMMAND:{
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
					if(_hWnd==GetActiveWindow() && _hWnd !=0)
					{
						App *This = (App*) GetWindowLong(_hWnd, 0);
						DialogBox(This->_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX),_hWnd,About);

					}
					break;
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;

				case ID_FILE_LOAD:
					{App *This = (App*) GetWindowLong(_hWnd, 0);
						This->LoadSettings();
						break;
					}

				case ID_SAVE:
					{App *This = (App*) GetWindowLong(_hWnd, 0);
						This->SaveSettings();
						break;
					}

				case ID_CLEAR_ALL:
				{
					App *This = (App*) GetWindowLong(_hWnd, 0);
					for(int len,x,y = _rows - 1;y--;)
					{
						for(x = _columns - 1;x--;)
						{
					This->_data[y][x].erase();
						}
					}
					::RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);

					break;
				}
				
				case ID_CLEAR_DATA:
					{

						App *This = (App*) GetWindowLong(_hWnd, 0);

						LPCWSTR lpszFormat	= L"пара#%i:%i";
						for(int len,x,y = _rows - 1;y--;)
						{
							for(x = _columns - 1;x--;)
							{
								len	= _scwprintf(lpszFormat,y,x);
								This->_data[y][x].resize(len);
								swprintf_s(&*This->_data[y][x].begin(),This->_data[y][x].size() + 1,lpszFormat,x+1,y+1);
							}
						}
						::RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
						break;
					}
				case ID_EDIT_MODIFABLE:
					{
						App *This = (App*) GetWindowLong(_hWnd, 0);
						This->modifable=!This->modifable;
						::CheckMenuItem( ::GetMenu(_hWnd ), ID_EDIT_MODIFABLE, 
							MF_BYCOMMAND |((This->modifable) ?   MF_CHECKED : MF_UNCHECKED ));
						This->_captured = FALSE;
						break;
					}
				case ID_WEEK_FIRSTWEEK:
					{
						App *This = (App*) GetWindowLong(_hWnd, 0);

							if (This->week==numweek::firstweek)

							break;

							else 
							{
								This->SaveSettings();
								This->week = numweek::firstweek;
								This->LoadSettings();
								
								::CheckMenuItem( ::GetMenu(_hWnd ), ID_WEEK_SECONDWEEK, 
									MF_BYCOMMAND | MF_UNCHECKED );
								::CheckMenuItem( ::GetMenu(_hWnd ), ID_WEEK_FIRSTWEEK, 
									MF_BYCOMMAND | MF_CHECKED );
								//SendMessage(hWnd,WM_PAINT,wParam,lParam);
								::RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
								break;
							}
					}
				case ID_WEEK_SECONDWEEK:
					{

								App *This = (App*) GetWindowLong(_hWnd, 0);
								if (This->week==numweek::secondweek)
									break;
								else 
								{
										This->SaveSettings();
										This->week = numweek::secondweek;
										This->LoadSettings();
										
										::CheckMenuItem( ::GetMenu(_hWnd ), ID_WEEK_FIRSTWEEK, 
											MF_BYCOMMAND | MF_UNCHECKED );
										::CheckMenuItem( ::GetMenu(_hWnd ), ID_WEEK_SECONDWEEK, 
											MF_BYCOMMAND | MF_CHECKED );
										//Invalidate();
										//SendMessage(hWnd,WM_PAINT,wParam,lParam);
										::RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
										break;
									}
								}
					}
		case EN_CHANGE:{
			if((_hWnd==GetActiveWindow()) && (_hWnd !=0))
			{
				App *This = (App*) GetWindowLong(_hWnd, 0);

				std::wstring&value	= This->_data[This->_sel_row - 1][This->_sel_column - 1];

				if(This->_hwndEdit!=NULL)
				{


					value.resize(::GetWindowTextLength(This->_hwndEdit));

					if(value.size()){::GetWindowText(This->_hwndEdit,
						&*This->_data[This->_sel_row - 1][This->_sel_column - 1].begin(),
						value.size() + 1);
					}
				}
			}
			break;
					   }
					   break;

						}
					}


#pragma endregion change
		
	return::DefWindowProc(hWnd, message, wParam, lParam);
}


void App::ShiftSelection(HWND hWnd)
{	
	if(_hWnd==GetActiveWindow() && _hWnd !=0)
{
	App *This = (App*) GetWindowLong(_hWnd, 0);
	RECT rc;
	POINT cell;
	LONG x,y;
	if( !This->_hwndEdit )return;
	::GetClientRect(hWnd,&rc);
	x		= ::GetScrollPos(hWnd,SB_HORZ);
	y		= ::GetScrollPos(hWnd,SB_VERT);
	if( (This->_sel_column <= x) || (This->_sel_row <= y) ){
		::ShowWindow(This->_hwndEdit,SW_HIDE);
	}else{
		cell.x	= rc.left + (This->_sel_column - x) * This->_column_width;
		cell.y	= rc.top + (This->_sel_row - y) * This->_row_height;
		::SetWindowPos(
			This->_hwndEdit,0,
			cell.x + 2,
			cell.y + 2,0,0,
			SWP_NOSIZE | SWP_NOZORDER | (::IsWindowVisible(This->_hwndEdit)?SWP_NOREDRAW|SWP_NOACTIVATE:SWP_SHOWWINDOW)
			);
		::SetFocus(This->_hwndEdit);
	}
	}
}
	void App::DrawGrid(HWND hWnd,HDC hdc,LONG ox,LONG oy,LONG ux,LONG uy,LONG ucx,LONG ucy)
	{
		RECT rc;
		::GetClientRect(hWnd,&rc);
		for(int x,i = ucx + 1;i--;){
			x	= rc.left + (ux + i) * _column_width;
			::MoveToEx(hdc,x,rc.top + uy * _row_height,NULL);
			::LineTo(hdc,x,rc.top + (uy + ucy) * _row_height);
		}

		for(int y,i = ucy + 1;i--;){
			y	= rc.top + (uy + i) * _row_height;
			::MoveToEx(hdc,rc.left + ux * _column_width,y,NULL);
			::LineTo(hdc,rc.left + (ux + ucx) * _column_width,y);
		}


	};

	void App::DrawCells(HWND hWnd, HDC hdc, LONG ox, LONG oy, LONG ux, LONG uy, LONG ucx, LONG ucy)
	{
		_TCHAR text[20];
		LOGFONT lf;
		RECT cell,rc;
		HGDIOBJ hFont,hThin,hBold;
		//
		::GetClientRect(hWnd,&rc);
		//
		hFont		= ::GetCurrentObject(hdc,OBJ_FONT);
		hThin		= ::GetStockObject(ANSI_VAR_FONT);
		::GetObject(hThin,sizeof(lf),&lf);
		lf.lfWeight	= FW_BOLD;
		hBold		= ::CreateFontIndirect(&lf);
		::SetBkMode(hdc,TRANSPARENT);
		//
		if (ucy<=0)
		{
			ucy =9;uy=0;
		}
		if(ucx<=0)
		{
			ucx = 7;ux = 0;
		}
		for(int x,y = ucy;y--;){
			for(x = ucx;x--;){
				cell.right	= (cell.left = rc.left + (ux + x) * _column_width) + _column_width;
				cell.bottom	= (cell.top = rc.top + (uy + y) * _row_height) + _row_height;
				if( !(ux + x) || !(uy + y) ){
					::DrawEdge(hdc,&cell,BDR_RAISEDOUTER | BDR_RAISEDINNER,BF_RECT | BF_MIDDLE);
					::SelectObject(hdc,hBold);
					if( !(ux + x) && !(uy + y) ){
						_tcscpy_s(text,_T("Пара#День"));
					}else if( !(ux + x) ){
						_stprintf_s(text,_T("Пара#%i"),oy + uy + y);
					}else if( !(uy + y) ){
						//_stprintf_s(text,_T("День недели#%i"),ox + ux + x);

						int den = ox + ux + x;
					switch(den)
					{
						case 1:
							_stprintf_s(text,_T("Понедельник"));
							break;
						case 2:
							_stprintf_s(text,_T("Вторник"));
							break;
						case 3:
							_stprintf_s(text,_T("Среда"));
							break;
						case 4:
							_stprintf_s(text,_T("Четверг"));
							break;
						case 5:
							_stprintf_s(text,_T("Пятница"));
							break;
						case 6:
							_stprintf_s(text,_T("Суббота"));
							break;

					}
					}
					::DrawText(hdc,text,-1,&cell,DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);
				}else{
					::SelectObject(hdc,hThin);
				
					if( ((ox + ux + x) != _sel_column) || ((oy + uy + y) != _sel_row) ){
					
						::DrawTextW(
							hdc,
							_data[oy + uy + y - 1][ox + ux + x - 1].c_str(),
							_data[oy + uy + y - 1][ox + ux + x - 1].size(),
							&cell,
							DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS
							);

					}
				}
			}
		}
		::SelectObject(hdc,hFont);
		::DeleteObject(hBold);
	}

	void App::SelectCell(HWND hWnd,LONG nc,LONG nr)
	{
		RECT rc;
		POINT cell;
		LONG x,y;
		_sel_column	= nc;
		_sel_row		= nr;
		x			= ::GetScrollPos(hWnd,SB_HORZ);
		y			= ::GetScrollPos(hWnd,SB_VERT);
		::GetClientRect(hWnd,&rc);
		cell.x = rc.left + (nc - x) * _column_width;
		cell.y = rc.top + (nr - y) * _row_height;
		if( !_hwndEdit ){
			_hwndEdit	= ::CreateWindow(
				WC_EDIT,
				_data[nr - 1][nc - 1].c_str(),
				WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
				cell.x + 2,
				cell.y + 2,
				_column_width - 3,
				_row_height - 3,
				hWnd,NULL,
				_hInst,NULL
				);
			::SendMessage(_hwndEdit,WM_SETFONT,(WPARAM)::GetStockObject(ANSI_VAR_FONT),0);
		}else{
			::SetWindowPos(
				_hwndEdit,0,
				cell.x + 2,
				cell.y + 2,0,0,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE
				);

			::SetWindowTextW(_hwndEdit,_data[nr - 1][nc - 1].c_str());
		}
		::ShowWindow(_hwndEdit,SW_SHOW);
		::SendMessage(_hwndEdit,EM_SETSEL,0,-1);
		::SetFocus(_hwndEdit);

	}
App::App(HINSTANCE hinst,int CmdShow)
{

	modifable			= false;
	week				= numweek::firstweek;

	_hWnd		= 0;
	_captured	= FALSE;
	_hInst = hinst;


 	LPCWSTR lpszFormat	= L"пара#%i:%i";
 	for(int len,x,y = _rows - 1;y--;)
	{
 		for(x = _columns - 1;x--;)
		{
 			len	= _scwprintf(lpszFormat,y,x);
 			_data[y][x].resize(len);
 			swprintf_s(&*_data[y][x].begin(),_data[y][x].size() + 1,lpszFormat,x+1,y+1);
 		}
	}

	LoadSettings();
	
			_hwndEdit		= NULL;
			_column_width	= 110;
			_row_height		= 23;
			_sel_column		= -1;
			_sel_row		= -1;
			_captured		= FALSE;

	LoadString(hinst,IDS_APP_TITLE ,_Title, MAX_LOADSTRING);
	LoadString(hinst,IDS_TABLE, _WindowClass, MAX_LOADSTRING);
	
	MyRegClass(hinst);


	if (!InitInstance(hinst,CmdShow))
	{
		MessageBox(0,L"Невозможно построить приложение",L"Ошибка",MB_ICONERROR);
		ExitProcess(-1);
		//требуеться доработка
	}

	_AccelTable = LoadAccelerators(hinst, MAKEINTRESOURCE(IDA_STUDNET));

	while (GetMessage(&_msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(_msg.hwnd, _AccelTable, &_msg))
		{
			TranslateMessage(&_msg);
			DispatchMessage(&_msg);
		}
	}
};

ATOM App::MyRegClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	//ZeroMemory(&wcex,sizeof(wcex));

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= 0;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 4;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_STUDNET));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(BLACK_BRUSH);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDM_STUDNET);
	wcex.lpszClassName	= _WindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_STUDNET));
		
		return RegisterClassEx(&wcex);
};

bool App::InitInstance(HINSTANCE hinst, int CmdShow)
{
	RECT wrk;
	LONG cx,cy;

	cx		= 805;
	cy		= 285;

	::SystemParametersInfo(SPI_GETWORKAREA,0,&wrk,NULL);
	_hWnd	= ::CreateWindow(
		_WindowClass,
		_Title,
		WS_OVERLAPPEDWINDOW  ,
		(wrk.left + wrk.right - cx) / 2,
		(wrk.top + wrk.bottom - cy) / 2,
		cx,
		cy,
		NULL,
		NULL,
		hinst,
		NULL
		);



 	if (!_hWnd)
	{
		return FALSE;
	}

	SetWindowLong(_hWnd, 0, (long) this);
	
	ShowWindow(_hWnd,CmdShow);
	UpdateWindow(_hWnd);

	return TRUE;
}