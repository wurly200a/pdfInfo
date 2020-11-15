#include "Pdf.h"
#include "PdfLib.h"

/**
 * @brief: read to CR or LF
 * @param: fp
 * @param: startPos
 * @param: dataPtr
 * @param: maxLength
 * @return: count
 **/
DWORD
readByteToCRorLF( FILE *fp, DWORD startPos, BYTE *dataPtr, DWORD maxLength )
{
    DWORD i,count=(DWORD)0;
    BOOL bFlag = (BOOL)FALSE;
    BYTE data;

    memset( dataPtr, 0, maxLength );

    fseek(fp, startPos, SEEK_SET);

    if( fseek(fp, 0, SEEK_CUR) == 0 )
    { /* if file is not empty  */
        i=0;
        do
        {
            fread(&data,sizeof(char),1,fp);

            if( (data == '\r') ||
                (data == '\n') )
            {
                bFlag = (BOOL)TRUE;
            }
            else
            {
                if( bFlag )
                {
                    break;
                }
                else
                {
                    *(dataPtr+i) = data;
                    i++;
                }
            }
            count++;
        }
        while( (fseek(fp, 0, SEEK_CUR) == 0) && (i<maxLength) );
    }
    else
    {
    }

    return count;
}

DWORD
readByteForwardToKeyword( FILE *fp, BYTE *dataPtr, DWORD maxLength, DWORD offsetPosFromStart, PTSTR ptstrKeyword, INT keywordLength )
{
    DWORD i,j;
    DWORD count=(DWORD)0;
    BOOL bFlag = (BOOL)FALSE;

    for( i=0;i<maxLength;i++)
    {
        *(dataPtr+i) = '\0';
    }

    fseek(fp, offsetPosFromStart, SEEK_SET);

    if( fseek(fp, 0, SEEK_CUR) == 0 )
    { /* if file is not empty  */
        count = 0;
        do
        {
            if( count < maxLength )
            {
                count++;
                *(dataPtr+count) = getc(fp);
                if( keywordLength <= count )
                {
                    DWORD matchCount;

                    for(j=0,matchCount=0;j<(keywordLength);j++)
                    {
                        if( (*(dataPtr - keywordLength + (count+j)) == *(ptstrKeyword+j)) )
                        {
                            matchCount++;
                        }
                        else
                        {
                            nop();
                        }
                    }

                    if( matchCount == (keywordLength) )
                    {
                        bFlag = (BOOL)TRUE;
                        count -= keywordLength;
                        break;
                    }
                    else
                    {
                        nop();
                    }
                }
                else
                {
                    nop();
                }
            }
            else
            {
                break;
            }
        }
        while( fseek(fp, 0, SEEK_CUR) == 0 );
    }
    else
    {
        nop();
    }

    if( bFlag )
    {
        nop();
    }
    else
    {
        count = (DWORD)0;
    }

#if 0
    DebugWndPrintf("readByteForwardToKeyword\r\n");
    DebugWndPrintf("%d\r\n",count);

    for( i=0; i<maxLength; i++ )
    {
        DebugWndPrintf("%c ",*(dataPtr+i));
        if( i%16 == (16-1) )
        {
            DebugWndPrintf("\r\n");
        }
    }
#endif

    return count;
}

DWORD
readByteBackToKeyword( FILE *fp, BYTE *dataPtr, DWORD maxLength, DWORD offsetPosFromEnd, PTSTR ptstrKeyword, INT keywordLength )
{
    DWORD i,j;
    DWORD count=(DWORD)0;
    BOOL bFlag = (BOOL)FALSE;

    for( i=0;i<maxLength;i++)
    {
        *(dataPtr+i) = '\0';
    }

    fseek(fp, -offsetPosFromEnd, SEEK_END);

    if( fseek(fp, -1L, SEEK_CUR) == 0 )
    { /* if file is not empty  */
        count = 0;
        do
        {
            if( count < maxLength )
            {
                count++;
                *(dataPtr+(maxLength-count)) = getc(fp);
                if( keywordLength <= count )
                {
                    DWORD matchCount;

                    for(j=0,matchCount=0;j<(keywordLength);j++)
                    {
                        if( (*(dataPtr+(maxLength-count+j)) == *(ptstrKeyword+j)) )
                        {
                            matchCount++;
                        }
                        else
                        {
                            nop();
                        }
                    }

                    if( matchCount == (keywordLength) )
                    {
                        bFlag = (BOOL)TRUE;
                        break;
                    }
                    else
                    {
                        nop();
                    }
                }
                else
                {
                    nop();
                }
            }
            else
            {
                break;
            }
        }
        while( fseek(fp, -2L, SEEK_CUR) == 0 ); /* 2 pos backwards, because 1 pos foward */
    }
    else
    {
        nop();
    }

    if( bFlag )
    {
        nop();
    }
    else
    {
        count = (DWORD)0;
    }

#if 0
    DebugWndPrintf("readByteBackToKeyword\r\n");

    for( i=0; i<maxLength; i++ )
    {
        DebugWndPrintf("%c ",*(dataPtr+i));
        if( i%16 == (16-1) )
        {
            DebugWndPrintf("\r\n");
        }
    }
#endif

    return count;
}
