#include "vcl-shim.h"

BOOL FileExists(LPCTSTR szPath)
{
  DWORD dwAttrib = GetFileAttributes(szPath);

  return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
         !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

VclString GetCurrentDir() {
  TCHAR NPath[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, NPath);
  return VclString(NPath);
}

void ShowMessage(const VclString& Message, const VclString& Title) {
  MessageBox(NULL, Message.c_str(), Title.c_str(), MB_OK);
}
