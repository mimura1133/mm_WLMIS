#include "Windows.h"
#include "string.h"

bool IsMultiByte(unsigned char dat){
	return (((dat >= 0x81)&(dat <= 0x9F))|((dat >= 0xE0)&(dat <= 0xFC))) != 0;
}

int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nShowCmd)
{
	HWND messenger = NULL;
	COPYDATASTRUCT cds;
	WIN32_FIND_DATA ffd;
	int CmdCount = 0,id = 0,shift;
	wchar_t wbuf[256] = {0};
	char format[_MAX_PATH] = {0}, flags[10] = {0}, cmdline[256] = {0};
	char buf[_MAX_PATH] = {0},tokstring[256] = {0}
	char *nexttoken,*endpt;
	char **Analysis;
	

	strcpy_s(cmdline,sizeof(cmdline),lpCmdLine);
	nexttoken = cmdline;
	while(strstr(nexttoken,"\\0")){
		if(!IsMultiByte(*(char*)(nexttoken-1))){
			nexttoken = strstr(nexttoken,"\\0") +2;
			CmdCount++;
		} else nexttoken++;
	}
	if(strlen(nexttoken)>0) CmdCount++;

	// 書式設定
	GetModuleFileName(NULL,buf,_MAX_PATH);
	_splitpath_s(buf,flags,10,format,_MAX_PATH,NULL,0,NULL,0);
	wsprintf(buf,TEXT("%s%s"),flags,format);
	SetCurrentDirectory(buf);
	if(FindFirstFile(".\\wlmis.ini",&ffd) != INVALID_HANDLE_VALUE){
		wsprintf(buf,"%d",CmdCount);
		GetPrivateProfileString("Send_Pattern",buf,"(PLEASE DEFAULT VALUE)",format,1024,".\\wlmis.ini");
		if(strcmp(format,"(PLEASE DEFAULT VALUE)") == 0) GetPrivateProfileString("Send_Pattern","DEFAULT","(wlmis.ini error.)",format,1024,".\\wlmis.ini");
	} else {
		if(CmdCount == 1) wsprintf(format,"{0}");
		if(CmdCount == 2) wsprintf(format,"{0} - {1}");
		if(CmdCount >= 3) wsprintf(format,"{0} - {1} ({2})");
	}

	// 独自処理を行うかどうか
	GetPrivateProfileString("Send_Pattern","WLMIS_Processing","NULL",buf,1024,".\\wlmis.ini");
	if(strcmp(buf,"NULL") == 0) wsprintf(buf,"0");
	if((atoi(buf) == TRUE)&(lstrlen(cmdline) > 0)){
		// 独自処理（WLMIS Processing.)
		memset(tokstring,0,sizeof(tokstring));
		strcpy_s(tokstring,sizeof(cmdline),cmdline);
		Analysis = new char*[CmdCount+1];		// 分解データ

		//nexttoken
		Analysis[0] = tokstring; nexttoken = tokstring;
		id = 1;
		while(strstr(nexttoken,"\\0")){
			nexttoken = strstr(nexttoken,"\\0");
			memset(nexttoken,0,2);
			nexttoken +=2;
			Analysis[id] = nexttoken;
			id++;
		}

		wsprintf(buf,TEXT(""));	// Overflow.
		Analysis[CmdCount] = buf;
		nexttoken = format;

		// 置換作業
		while((nexttoken = strstr(nexttoken,"{")) != 0){
			if(!IsMultiByte(*(char*)(nexttoken-1))) {
				endpt = strstr(nexttoken,"}");
				if(!IsMultiByte(*(char*)(nexttoken-1))){
					id = atoi(nexttoken+1);
					if(id >= CmdCount) id = CmdCount;
					shift = strlen(Analysis[id]) - ((endpt+1) - nexttoken);
					if(shift>0) {
						memmove(nexttoken+shift,nexttoken,strlen(nexttoken)+1);
					} else if(shift<0){
						memmove(nexttoken,nexttoken-shift,strlen(nexttoken)+shift+1);
					}
					memmove(nexttoken,Analysis[id],strlen(Analysis[id]));
					nexttoken+=strlen(Analysis[id]);
				} else nexttoken++;
			} else nexttoken++;
		}
		
		delete Analysis;
		memset(cmdline,0,sizeof(cmdline));
		wsprintf(cmdline,TEXT("%s\\0"),format);
		wsprintf(format,"{0}");	// フォーマットを弄らないようにする。
	}

	if(lstrlen(cmdline) == 0){
		wsprintf(flags,"0");
		wsprintf(format,"");
	} else wsprintf(flags,"1");
	memset(buf,0,sizeof(buf));
	wsprintf(buf,"%s%s%s%s%s%s\\0",
		"Windows Live Messenger Music Sender\\0Music\\0",
		flags,"\\0",
		format,"\\0",
		cmdline);
	memset(wbuf,0,sizeof(wbuf));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buf, -1, wbuf,sizeof(wbuf));
	cds.dwData = 0x547;
	cds.cbData = sizeof(wbuf);
	cds.lpData = wbuf;

	messenger = 0;
	while(1){
		messenger = FindWindowEx(NULL,messenger,"MsnMsgrUIManager",NULL);
		if(messenger != 0)SendMessage(messenger,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds); else break;
	}
	return 0;
}