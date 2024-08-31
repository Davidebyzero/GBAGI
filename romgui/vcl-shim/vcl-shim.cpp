#include "vcl-shim.h"

BOOL FileExists(LPCTSTR szPath)
{
  DWORD dwAttrib = GetFileAttributes(szPath);

  return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
         !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

AnsiString GetCurrentDir() {
  TCHAR NPath[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, NPath);
  return AnsiString(NPath);
}

void ShowMessage(const AnsiString& Message, const AnsiString& Title) {
  MessageBox(NULL, Message.c_str(), Title.c_str(), MB_OK);
}
