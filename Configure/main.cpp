#include "stdio.h"
#include "Windows.h"

HWND Parent;

void Default_Config(LPSTR pattern,int type){
	if(type==-1) type = 4;
	if(type==1)  wsprintf(pattern,"{0}");
	if(type==2)  wsprintf(pattern,"{0} - {1}");
	if(type==3)  wsprintf(pattern,"{0} - {1} ({2})");
	if(type==4)  wsprintf(pattern,"{0} - {1} ({2} - {3})");
};
	
void Read_Config(int target,int type){
	char pattern[1024] = {0};
	char key[100] = {0};

	if(type>=0){
		wsprintf(key,"%d",type);
		GetPrivateProfileString("Send_Pattern",key,"(PLEASE DEFAULT VALUE)",pattern,1024,".\\wlmis.ini");
		if(strcmp(pattern,"(PLEASE DEFAULT VALUE)") == 0) Default_Config(pattern,type);
	} else {
		GetPrivateProfileString("Send_Pattern","DEFAULT","(PLEASE DEFAULT VALUE)",pattern,1024,".\\wlmis.ini");
		if(strcmp(pattern,"(PLEASE DEFAULT VALUE)") == 0) Default_Config(pattern,-1);
	}
	SendMessage(GetDlgItem(Parent,target),WM_SETTEXT,NULL,(LPARAM)pattern);
	return;
}
	
void Write_Config(){
	int i;
	char key[1024];
	char pattern[1024];

	for(i = 0;i<4;i++){
		wsprintf(key,"%d",i+1);
		SendMessage(GetDlgItem(Parent,(i*2)+1),WM_GETTEXT,1024,(LPARAM)pattern);
		WritePrivateProfileString("Send_Pattern",key,pattern,".\\wlmis.ini");
	}
	SendMessage(GetDlgItem(Parent,(4*2)+1),WM_GETTEXT,1024,(LPARAM)pattern);
	WritePrivateProfileString("Send_Pattern","DEFAULT",pattern,".\\wlmis.ini");
	
	wsprintf(pattern,TEXT("%d"),SendMessage(GetDlgItem(Parent,11),BM_GETCHECK,NULL,NULL));
	WritePrivateProfileString("Send_Pattern","WLMIS_Processing",pattern,".\\wlmis.ini");
}
	
int Create(LPCSTR CLASS,LPCSTR NAME,DWORD Style,int x,int y,int width,int height){
	static int objid = 0;
	HWND hwnd;
	if(strcmp(CLASS,"STATIC") == 0) {
		hwnd = CreateWindowEx(NULL,CLASS,NAME,WS_VISIBLE|WS_CHILD|Style,x,y,width,height,Parent,NULL,NULL,NULL);
	} else if(strcmp(CLASS,"EDIT") == 0){
		hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,CLASS,NAME,ES_AUTOHSCROLL|WS_VISIBLE|WS_CHILD|Style,x,y,width,height,Parent,(HMENU)objid,(HINSTANCE)GetWindowLong(Parent,GWL_HINSTANCE),NULL);
		objid++;
		SendMessage(hwnd,EM_SETLIMITTEXT,1024,0);
	} else {
		hwnd = CreateWindowEx(NULL,CLASS,NAME,WS_VISIBLE|WS_CHILD|Style,x,y,width,height,Parent,(HMENU)objid,NULL,NULL);
		objid++;
	}
	SendMessage(hwnd,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),true);
	return objid;
}
	
int Create(LPCSTR CLASS,LPCSTR NAME,int x,int y,int width,int height){
	return Create(CLASS,NAME,NULL,x,y,width,height);
}
	
LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
	HDC hdc;
	HPEN pen;
	switch(msg){
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_PAINT:
			hdc = GetDC(hwnd);
			pen = CreatePen(PS_SOLID,1,GetSysColor(COLOR_3DSHADOW));
			SelectObject(hdc,pen);
			MoveToEx(hdc,0,415,NULL);
			LineTo(hdc,300,415);
			DeleteObject(pen);
			ReleaseDC(hwnd,hdc);
			break;

		case WM_COMMAND:
			if(((LOWORD(wp)%2) == 0)&(LOWORD(wp) < 11)){
				if(MessageBox(hwnd,TEXT("デフォルト値に設定しますか？"),TEXT("Question"),MB_YESNO) == IDYES)
				{
					char pattern[1024] = {0};
					if(LOWORD(wp) <= 8) Default_Config(pattern,LOWORD(wp)/2);
					if(LOWORD(wp) == 10) Default_Config(pattern,-1);
					SendMessage(GetDlgItem(Parent,LOWORD(wp)-1),WM_SETTEXT,NULL,(LPARAM)pattern);
				}
			}
			if(LOWORD(wp) == 11){
				EnableWindow(GetDlgItem(Parent,7),SendMessage(GetDlgItem(Parent,11),BM_GETCHECK,0,0));
				EnableWindow(GetDlgItem(Parent,8),SendMessage(GetDlgItem(Parent,11),BM_GETCHECK,0,0));
			}
			if(LOWORD(wp) == 12){
				Write_Config();
				PostQuitMessage(0);
			}
			if(LOWORD(wp) == 13) PostQuitMessage(0);
			if(LOWORD(wp) == 14) Write_Config();
			return 0;

		case WM_CREATE:
			Parent = hwnd;
			Create("STATIC","Windows Live Messenger Music Info Sender",5,5,250,15);
			Create("STATIC","Advanced Settings",180,20,120,15);
			Create("BUTTON","送信形式設定　（次回送信より適用されます。）",BS_GROUPBOX,5,45,280,280);
			Create("STATIC","曲名のみ：",20,70,250,15);
			Create("EDIT","",30,85,200,20);
			Create("BUTTON","Default",230,85,50,20);
			Create("STATIC","曲名と歌手情報のみ：",20,120,250,15);
			Create("EDIT","",30,135,200,20);
			Create("BUTTON","Default",230,135,50,20);
			Create("STATIC","曲名と歌手情報とアルバム情報のみ：",20,170,250,15);
			Create("EDIT","",30,185,200,20);
			Create("BUTTON","Default",230,185,50,20);
			Create("STATIC","第4引数まで指定する場合：",20,220,250,15);
			Create("EDIT","",30,235,200,20);
			Create("BUTTON","Default",230,235,50,20);
			Create("STATIC","上記に該当しない場合の送信形式：",20,270,250,15);
			Create("EDIT","",30,285,200,20);
			Create("BUTTON","Default",230,285,50,20);
			Create("BUTTON","WLMIS 側で引数処理をしてから送信する(Advanced.)",BS_AUTOCHECKBOX,5,330,280,22);
			Create("STATIC","書式：",5,360,50,15);
			Create("STATIC","引数指定は {?} (?に引数番号) で指定する。",20,375,250,15);
			Create("STATIC","それ以外の文字については、そのまま表示されます",20,390,280,15);
			Create("BUTTON","OK",20,420,80,22);
			Create("BUTTON","キャンセル",110,420,80,22);
			Create("BUTTON","適用",200,420,80,22);
	}
	return DefWindowProc(hwnd,msg,wp,lp);
}

int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nShowCmd){
    HWND hwnd;
	MSG msg;
	WNDCLASS winc;
	int i;
	char dat[5];

	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(hPrevInstance);

	winc.style			= CS_HREDRAW|CS_VREDRAW;
	winc.lpfnWndProc	= &WndProc;
	winc.cbClsExtra		= winc.cbWndExtra = 0;
	winc.hInstance		= hInstance;
	winc.hIcon			= LoadIcon(hInstance,TEXT("IDI_ICON"));
	winc.hCursor		= LoadCursor(NULL,IDC_ARROW);
	winc.hbrBackground	= GetSysColorBrush(COLOR_3DFACE);
	winc.lpszMenuName	= NULL;
	winc.lpszClassName	= TEXT("ConfigWindow");
	if (!RegisterClass(&winc)) return 0;

	hwnd = CreateWindowEx(NULL,TEXT("ConfigWindow"),TEXT("WLMIS Advanced Settings."),
		WS_CAPTION|WS_SYSMENU,CW_USEDEFAULT,CW_USEDEFAULT,300,475,NULL,NULL,hInstance,NULL);
		
	if(hwnd==NULL) return 0;

	// 設定を全て読みこむ
	for(i=1;i<=7;i+=2) Read_Config(i,(i+1)/2);
	Read_Config(9,-1);
	GetPrivateProfileString("Send_Pattern","WLMIS_Processing","NULL",dat,5,".\\wlmis.ini");
	if(strcmp(dat,"NULL") == 0) wsprintf(dat,"0");
	SendMessage(GetDlgItem(Parent,11),BM_SETCHECK,atoi(dat),NULL);
	SendMessage(Parent,WM_COMMAND,11,NULL);
	
	ShowWindow(hwnd,nShowCmd);
	while(GetMessage(&msg,NULL,NULL,NULL)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
