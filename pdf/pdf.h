#ifndef PDF_H
#define PDF_H

#include <windows.h>
#include <imm.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <process.h>

#define nop()

typedef struct
{
    DWORD startxref;
    DWORD xrefObjCount;
    DWORD fileSize;
    DWORD Count;
} S_PDF_IF_PARAM;

extern BOOL PdfInit( void );
extern BOOL PdfConvert( PTSTR ptstrFileName, S_PDF_IF_PARAM *paramPtr );

#endif /* PDF_H */
