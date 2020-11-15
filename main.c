#include <windows.h>
#include <imm.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <process.h>

#include "pdf\pdf.h"

S_PDF_IF_PARAM pdfParam;

DWORD FileGetLength( PTSTR pstrFileName );

int
main(int argc, char *argv[])
{
    static TCHAR szTemp[MAX_PATH];

    if( 2 <= argc )
    {
        wsprintf( szTemp, "%s", argv[1] );
        printf("%s\n",szTemp);

        PdfInit();
        pdfParam.fileSize = FileGetLength(szTemp);
        PdfConvert(szTemp,&pdfParam);

    }
    else
    {
        printf("Usage:\n");
    }
}

/**
 * @brief: get file length (Max: 4Gbyte)
 * @param: ptstrFileName (Full Path of File)
 * @return: length
 **/
DWORD
FileGetLength( PTSTR pstrFileName )
{
    HANDLE hFile;
    DWORD  dwFileLength;

    hFile = CreateFile(pstrFileName, GENERIC_READ, FILE_SHARE_READ,NULL, OPEN_EXISTING, 0, NULL);
    if( hFile == INVALID_HANDLE_VALUE )
    {
        dwFileLength = 0;
    }
    else
    {
        dwFileLength = GetFileSize(hFile, NULL);
    }

    return dwFileLength;
}
