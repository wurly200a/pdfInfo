#ifndef PDF_LIB_H
#define PDF_LIB_H

DWORD readByteToCRorLF( FILE *fp, DWORD startPos, BYTE *dataPtr, DWORD maxLength );
DWORD readByteForwardToKeyword( FILE *fp, BYTE *dataPtr, DWORD maxLength, DWORD offsetPosFromStart, PTSTR ptstrKeyword, INT keywordLength );
DWORD readByteBackToKeyword( FILE *fp, BYTE *dataPtr, DWORD maxLength, DWORD offsetPosFromEnd, PTSTR ptstrKeyword, INT keywordLength );

#endif /* PDF_LIB_H */
