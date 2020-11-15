#include "Pdf.h"
#include "PdfLib.h"

#define DebugWndPrintf printf

typedef struct
{
    BOOL  bValid;
    DWORD objectNum;
    DWORD generationNum;
} S_PDF_REFERENCE_INFO;

/* external function define */
/* external variable define */

/* internal function define */
static BOOL divideStringBySpace( PTSTR str, INT length, PTSTR *strPtr, INT cnt );
static DWORD getValue( PTSTR str, INT length );
static DWORD readPdfObject( FILE *fp, char **dataPtrPtr, DWORD offsetPosFromStart );
static DWORD readStartXref( FILE *fp, DWORD *startXrefPtr );
static DWORD readTrailer( FILE *fp, char **dataPtrPtr, DWORD offsetPosFromEnd );
static DWORD readXrefHeader( FILE *fp, INT startPos, DWORD *headerCountPtr, DWORD *objCountPtr );
static DWORD readByteFromInsideBlock( FILE *fp, DWORD startPos, BYTE *dataPtr, DWORD maxLength );
static PTSTR getDataFromDictionary( BYTE *dataPtr, DWORD maxLength, PTSTR ptstrKeyword, INT keywordLength );
static void getReferenceInfo( S_PDF_REFERENCE_INFO *infoPtr, BYTE *dataPtr );
static DWORD getObjectByteOffset( FILE *fp, DWORD startPos, DWORD objectNum );

/* internal variable define */

typedef struct
{
    BOOL bInit;
    TCHAR fileName[MAX_PATH];
    TCHAR outFileName[MAX_PATH];

    DWORD startxref;
    DWORD xrefObjCount;
    DWORD xrefHeaderCount;

//    BYTE trailer[XREF_MAX];
//    INT  trailerLength;

    DWORD fileSize;

    char *PagesCopyDataPtr;
    DWORD PagesCopyDataSize;

    char *DocumentInfoDictionaryCopyDataPtr;
    DWORD DocumentInfoDictionaryCopyDataSize;

    char *DocumentCatalogCopyDataPtr;
    DWORD DocumentCatalogCopyDataSize;

    char *trailerCopyDataPtr;
    DWORD trailerCopyDataSize;

    DWORD xrefObjStartPos;

    S_PDF_REFERENCE_INFO Root;
    S_PDF_REFERENCE_INFO Info;
    S_PDF_REFERENCE_INFO Pages;
} S_PDF_DATA;

static S_PDF_DATA pdfData;

#define TMP_DATA_MAX 256
static BYTE tmpData[TMP_DATA_MAX];

#define TMP2_DATA_MAX 256
static BYTE tmp2Data[TMP2_DATA_MAX];

/**
 * @brief: initialize
 * @param: none
 * @return: TRUE or FALSE
 **/
BOOL
PdfInit( void )
{
    memset( &pdfData, 0, sizeof(pdfData) );

    pdfData.bInit = (BOOL)TRUE;
    GetTempPath( sizeof(pdfData.outFileName)/sizeof(pdfData.outFileName[0]), pdfData.outFileName );
//    strcat( pdfData.outFileName, "\outputImage.tif" );

    return pdfData.bInit;
}

/**
 * @brief: 
 * @param: ptstrFileName
 * @param: paramPtr
 * @return: TRUE or FALSE
 **/
BOOL
PdfConvert( PTSTR ptstrFileName, S_PDF_IF_PARAM *paramPtr )
{
    BOOL bRtn = (BOOL)FALSE;
    int code, code1;
    const char * pdfargv[14];
    int pdfargc;
    static TCHAR szTemp[MAX_PATH];
    int i;
    char data;

    strncpy( pdfData.fileName, ptstrFileName, MAX_PATH );
    pdfData.fileSize = paramPtr->fileSize;

    if( pdfData.bInit )
    {
        FILE *fp;
        DWORD filePosCount;
        DWORD objectByteOffset;
        PTSTR tmpStr;

        DebugWndPrintf("--------------------------------------------------------------\r\n");
        DebugWndPrintf("fileName: %s\r\n",ptstrFileName);
        DebugWndPrintf("--------------------------------------------------------------\r\n");

        fp = fopen(ptstrFileName, "rb");

        if( fp != NULL )
        {
//            pdfData.bInit = (BOOL)FALSE;

            /* read start xref */
            filePosCount = readStartXref(fp,&(pdfData.startxref));
            DebugWndPrintf("startxref:%d\r\n",pdfData.startxref);
            DebugWndPrintf("--------------------------------------------------------------\r\n");

            /* read trailer */
            pdfData.trailerCopyDataSize = readTrailer(fp,&(pdfData.trailerCopyDataPtr),filePosCount);
            if( pdfData.trailerCopyDataSize )
            {
                char *ptr = pdfData.trailerCopyDataPtr;

                DebugWndPrintf("trailer is found\r\n");

                tmpStr = getDataFromDictionary(pdfData.trailerCopyDataPtr,pdfData.trailerCopyDataSize,"Root",strlen("Root"));
                getReferenceInfo(&pdfData.Root,tmpStr);
                DebugWndPrintf("Root: %s\r\n",tmpStr);

                tmpStr = getDataFromDictionary(pdfData.trailerCopyDataPtr,pdfData.trailerCopyDataSize,"Size",strlen("Size"));
                DebugWndPrintf("Size: %s\r\n",tmpStr);

                tmpStr = getDataFromDictionary(pdfData.trailerCopyDataPtr,pdfData.trailerCopyDataSize,"Info",strlen("Info"));
                getReferenceInfo(&pdfData.Info,tmpStr);
                DebugWndPrintf("Info: %s\r\n",tmpStr);

                tmpStr = getDataFromDictionary(pdfData.trailerCopyDataPtr,pdfData.trailerCopyDataSize,"ID",strlen("ID"));
                DebugWndPrintf("ID: %s\r\n",tmpStr);
            }
            else
            {
                DebugWndPrintf("trailer is not found\r\n");
            }

            if( pdfData.trailerCopyDataPtr != NULL )
            {
                free(pdfData.trailerCopyDataPtr);
                pdfData.trailerCopyDataPtr = (char *)NULL;
                pdfData.trailerCopyDataSize = (DWORD)0;
            }
            else
            {
                nop();
            }
            DebugWndPrintf("--------------------------------------------------------------\r\n");

            /* read xref */
            if( pdfData.startxref )
            {
                pdfData.xrefObjStartPos = readXrefHeader(fp,pdfData.startxref,&(pdfData.xrefHeaderCount),&(pdfData.xrefObjCount));

                if( pdfData.xrefHeaderCount )
                {
                    DebugWndPrintf("xref is found\r\n");
#if 0
                    filePosCount = pdfData.xrefObjStartPos;
                    for( i=0; i<pdfData.xrefObjCount; i++ )
                    {
                        filePosCount += readByteToCRorLF(fp,filePosCount,tmpData,TMP_DATA_MAX);
                        DebugWndPrintf("%d: %s\r\n",i,tmpData);
                    }
#endif
                }
                else
                {
                    DebugWndPrintf("xref is not found\r\n");
                }

                paramPtr->startxref = pdfData.startxref;
                paramPtr->xrefObjCount = pdfData.xrefObjCount;

                DebugWndPrintf("xrefObjCount:%d\r\n",pdfData.xrefObjCount);
            }
            else
            {
                pdfData.xrefObjStartPos = 0;
                paramPtr->startxref = 0;
                paramPtr->xrefObjCount = 0;
            }

            DebugWndPrintf("--------------------------------------------------------------\r\n");
            if( pdfData.xrefObjStartPos )
            {
                DebugWndPrintf("objectNum=%d\r\n",pdfData.Root.objectNum);

                objectByteOffset = getObjectByteOffset(fp,pdfData.xrefObjStartPos,pdfData.Root.objectNum);
//                DebugWndPrintf("objectByteOffset=%d\r\n",objectByteOffset);

                pdfData.DocumentCatalogCopyDataSize = readPdfObject(fp,&(pdfData.DocumentCatalogCopyDataPtr),objectByteOffset);

                if( pdfData.DocumentCatalogCopyDataSize )
                {
                    DebugWndPrintf("DocumentCalalog is found\r\n");

                    tmpStr = getDataFromDictionary(pdfData.DocumentCatalogCopyDataPtr,pdfData.DocumentCatalogCopyDataSize,"Pages",strlen("Pages"));
                    getReferenceInfo(&pdfData.Pages,tmpStr);
                    DebugWndPrintf("Pages: %s\r\n",tmpStr);

//                    DebugWndPrintf("pdfData.DocumentCatalogCopyDataSize=%d\r\n",pdfData.DocumentCatalogCopyDataSize);
                    for( i=0; i<pdfData.DocumentCatalogCopyDataSize; i++ )
                    {
                        DebugWndPrintf("%c",*(pdfData.DocumentCatalogCopyDataPtr+i));
                    }
                    DebugWndPrintf("\r\n");
                }
                else
                {
                    DebugWndPrintf("DocumentCalalog is not found\r\n");
                }
            }
            else
            {
                nop();
            }
            DebugWndPrintf("--------------------------------------------------------------\r\n");

            if( pdfData.xrefObjStartPos )
            {
                DebugWndPrintf("objectNum=%d\r\n",pdfData.Info.objectNum);

                objectByteOffset = getObjectByteOffset(fp,pdfData.xrefObjStartPos,pdfData.Info.objectNum);
//                DebugWndPrintf("objectByteOffset=%d\r\n",objectByteOffset);

                pdfData.DocumentInfoDictionaryCopyDataSize = readPdfObject(fp,&(pdfData.DocumentInfoDictionaryCopyDataPtr),objectByteOffset);

                if( pdfData.DocumentInfoDictionaryCopyDataSize )
                {
                    DebugWndPrintf("DocumentInfoDictionary is found\r\n");

//                    DebugWndPrintf("pdfData.DocumentInfoDictionaryCopyDataSize=%d\r\n",pdfData.DocumentInfoDictionaryCopyDataSize);
                    for( i=0; i<pdfData.DocumentInfoDictionaryCopyDataSize; i++ )
                    {
                        DebugWndPrintf("%c",*(pdfData.DocumentInfoDictionaryCopyDataPtr+i));
                    }
                    DebugWndPrintf("\r\n");
                }
                else
                {
                    DebugWndPrintf("DocumentInfoDictionary is not found\r\n");
                }
            }
            else
            {
                nop();
            }
            DebugWndPrintf("--------------------------------------------------------------\r\n");
            if( pdfData.xrefObjStartPos && pdfData.Pages.bValid )
            {
                DebugWndPrintf("objectNum=%d\r\n",pdfData.Pages.objectNum);

                objectByteOffset = getObjectByteOffset(fp,pdfData.xrefObjStartPos,pdfData.Pages.objectNum);
                pdfData.PagesCopyDataSize = readPdfObject(fp,&(pdfData.PagesCopyDataPtr),objectByteOffset);

                if( pdfData.PagesCopyDataSize )
                {
                    DebugWndPrintf("Pages is found\r\n");
                    for( i=0; i<pdfData.PagesCopyDataSize; i++ )
                    {
                        DebugWndPrintf("%c",*(pdfData.PagesCopyDataPtr+i));
                    }
                    DebugWndPrintf("\r\n");

                    tmpStr = getDataFromDictionary(pdfData.PagesCopyDataPtr,pdfData.PagesCopyDataSize,"Count",strlen("Count"));
                    getReferenceInfo(&pdfData.Root,tmpStr);
                    DebugWndPrintf("Count: %s\r\n",tmpStr);

                    paramPtr->Count = atoi(tmpStr);
                }
                else
                {
                    DebugWndPrintf("Pages is not found\r\n");
                }
            }
            else
            {
                nop();
            }

            DebugWndPrintf("--------------------------------------------------------------\r\n");

            fclose(fp);
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

    return bRtn;
}


/**
 * @brief: 
 **/
static BOOL
divideStringBySpace( PTSTR str, INT length, PTSTR *strPtr, INT cnt )
{
    INT i,c;
    BOOL bFlag;

    for( c=0,i=0,bFlag=(BOOL)FALSE; i<length&&c<cnt; i++ )
    {
        if( (str[i] == ' ') )
        {
            bFlag = (BOOL)FALSE;
            c++;
        }
        else
        {
            if( bFlag )
            {
                nop();
            }
            else
            {
                *(strPtr+c) = &str[i];
                bFlag = (BOOL)TRUE;
            }
        }
    }

    return TRUE;
}

#define TEMP_VALUE_MAX 128
/**
 * @brief: 
 **/
static DWORD
getValue( PTSTR str, INT length )
{
    DWORD value = (DWORD)0;
    INT i,j;
    TCHAR temp[TEMP_VALUE_MAX];
//    TCHAR *ptr;

    memset( &temp, 0, sizeof(temp) );

    for( i=0,j=0; (i<length)&&(i<TEMP_VALUE_MAX); i++ )
    {
        if( (str[i] == '\r') ||
            (str[i] == '\n') )
        {
            nop();
        }
        else
        {
            temp[j] = str[i];
            j++;
        }
    }

//    DebugWndPrintf("temp:%s\r\n",temp);

#if 0
    ptr = temp;
    for(i=0;i<cnt&i<TEMP_VALUE_MAX;i++)
    {
        BOOL flag=(BOOL)FALSE;

        for(j=0;;ptr++)
        {
            if( *ptr != '\0' )
            {
                if( flag )
                {
                    break;
                }
                else
                {
                    nop();
                }
            }
            else
            {
                flag = (BOOL)TRUE;
            }
        }
    }
    value = atol(ptr);
#endif
    value = atol(temp);

    return value;
}

#define STRING_OBJ_START TEXT("obj")
#define OBJ_START_LEN    3

#define STRING_OBJ_END   TEXT("endobj")
#define OBJ_END_LEN      6
/**
 * @brief: 
 **/
static DWORD
readPdfObject( FILE *fp, char **dataPtrPtr, DWORD offsetPosFromStart )
{
    DWORD i;
    DWORD count;
    DWORD count2 = (DWORD)0;
    char *ptr;

    if( fp != NULL )
    {
        DWORD objectByteOffset = offsetPosFromStart;
        DWORD objectStartCount,objectEndCount;

        memset( &tmpData, 0, sizeof(tmpData) );

        objectStartCount = readByteForwardToKeyword(fp, tmpData, TMP_DATA_MAX, objectByteOffset, STRING_OBJ_START, OBJ_START_LEN );
        objectByteOffset += objectStartCount + OBJ_START_LEN;
        objectEndCount = readByteForwardToKeyword(fp, tmpData, TMP_DATA_MAX, objectByteOffset, STRING_OBJ_END, OBJ_END_LEN );
        objectByteOffset += objectEndCount + OBJ_END_LEN;

//        DebugWndPrintf("objectStartCount=%d,objectEndCount=%d\r\n",objectStartCount,objectEndCount);
//        for( i=0; i<objectEndCount; i++ )
//        {
//            DebugWndPrintf("%c",tmpData[i]);
//        }
//        DebugWndPrintf("\r\n");

        if( objectStartCount && objectEndCount )
        {
            count2 = readByteFromInsideBlock(fp,offsetPosFromStart+objectStartCount,&tmpData,objectEndCount);

            *(dataPtrPtr) = malloc(count2);
            memcpy(*(dataPtrPtr),tmpData,count2);

            for( i=0,ptr=*(dataPtrPtr); i<count2; i++ )
            {
                if( *(ptr+i) == '\r' ||
                    *(ptr+i) == '\n' ||
                    *(ptr+i) == '/' )
                {
                    *(ptr+i) = '\0';
                }
                else
                {
                    nop();
                }
            }
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

    return count2;
}

#define STRING_STARTXREF TEXT("startxref")
#define STARTXREF_LEN    9
/**
 * @brief: 
 **/
static DWORD
readStartXref( FILE *fp, DWORD *startXrefPtr )
{
    INT  xrefStartLength;
    INT i;

    if( fp != NULL )
    {
        xrefStartLength = readByteBackToKeyword(fp, tmpData, TMP_DATA_MAX, 0/* end of file */, STRING_STARTXREF,STARTXREF_LEN);

        if( xrefStartLength )
        {
            INT offset = (TMP_DATA_MAX-xrefStartLength+STARTXREF_LEN);

            *(startXrefPtr) = getValue( &(tmpData[offset]),TMP_DATA_MAX-offset );
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

    return xrefStartLength;
}


#define STRING_TRAILER TEXT("trailer")
#define TRAILER_LEN    7
/**
 * @brief: 
 **/
static DWORD
readTrailer( FILE *fp, char **dataPtrPtr, DWORD offsetPosFromEnd )
{
    DWORD i;
    DWORD count;
    DWORD count2 = (DWORD)0;
    char *ptr;

    if( fp != NULL )
    {
        memset( &tmpData, 0, sizeof(tmpData) );

        count = readByteBackToKeyword(fp, tmpData, TMP_DATA_MAX, offsetPosFromEnd, STRING_TRAILER,TRAILER_LEN);

        if( count )
        {
            count2 = readByteFromInsideBlock(fp,pdfData.fileSize-(offsetPosFromEnd+count),&tmpData,TMP_DATA_MAX);

            *(dataPtrPtr) = malloc(count2);
            memcpy(*(dataPtrPtr),tmpData,count2);

            for( i=0,ptr=*(dataPtrPtr); i<count2; i++ )
            {
                if( *(ptr+i) == '\r' ||
                    *(ptr+i) == '\n' ||
                    *(ptr+i) == '/' )
                {
                    *(ptr+i) = '\0';
                }
                else
                {
                    nop();
                }
            }
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

    return count2;
}

/**
 * @brief: 
 **/
static DWORD
readXrefHeader( FILE *fp, INT startPos, DWORD *headerCountPtr, DWORD *objCountPtr )
{
    INT i;
    DWORD objCount = (DWORD)0;
    DWORD filePosCount;
    PTSTR divideStrArray[2];
    DWORD xrefHeaderCount = (DWORD)0;
    DWORD objStartPos = (DWORD)0;

    if( fp != NULL )
    {
        filePosCount = readByteToCRorLF(fp,startPos,tmpData,TMP_DATA_MAX);

        if( !strcmp(tmpData,"xref") )
        {
            filePosCount += readByteToCRorLF(fp,startPos+filePosCount,tmpData,TMP_DATA_MAX);

            divideStringBySpace(tmpData,TMP_DATA_MAX,&divideStrArray,2);
            objCount = getValue(divideStrArray[1],TMP_DATA_MAX);

            *(headerCountPtr) = filePosCount;
            *(objCountPtr) = objCount;

            objStartPos = startPos + filePosCount;
        }
        else
        {
            *(headerCountPtr) = 0;
        }
    }
    else
    {
        nop();
    }

    return objStartPos;
}

/**
 * @brief: 
 **/
static DWORD
readByteFromInsideBlock( FILE *fp, DWORD startPos, BYTE *dataPtr, DWORD maxLength )
{
    DWORD i,count=(DWORD)0;
    INT blockCount = (INT)0;
    BYTE data;
    BYTE prevData = '\0';

    memset( dataPtr, 0, maxLength );

    fseek(fp, startPos, SEEK_SET);

    if( fseek(fp, 0, SEEK_CUR) == 0 )
    { /* if file is not empty  */
        i=0;
        do
        {
            fread(&data,sizeof(char),1,fp);

            if( (prevData == '<') &&
                (data == '<') )
            {
                blockCount++;
            }
            else if( (prevData == '>') &&
                     (data == '>') )
            {
                blockCount--;
                if( blockCount )
                {
                    nop();
                }
                else
                {
                    break;
                }
            }
            else
            {
                if( (data == '<') ||
                    (data == '>') )
                {
                    nop();
                }
                else
                {
                    if( blockCount )
                    {
                        *(dataPtr+i) = data;
                        i++;
                    }
                    else
                    {
                    }
                }
            }
            count++;
            prevData = data;
        }
        while( (fseek(fp, 0, SEEK_CUR) == 0) && (i<maxLength) );
    }
    else
    {
    }

    return count;
}

/**
 * @brief: 
 **/
static PTSTR
getDataFromDictionary( BYTE *dataPtr, DWORD maxLength, PTSTR ptstrKeyword, INT keywordLength )
{
    BYTE *ptr = dataPtr;
    DWORD i,j;
    BOOL bStartFlag = (BOOL)FALSE;
    BOOL bMatchFlag = (BOOL)FALSE;
    DWORD matchCount = (DWORD)0;
    static BYTE dummyData = '\0';
    PTSTR rtnStr = (PTSTR)&dummyData;

    for( i=0,j=0; i<maxLength; i++ )
    {
        if( *(ptr+i) == '\0' )
        {
            bStartFlag = (BOOL)TRUE;
            matchCount = (DWORD)0;
            j = 0;
        }
        else
        {
            if( bStartFlag )
            {
                if( *(ptr+i) == *(ptstrKeyword+j) )
                {
                    matchCount++;
                }
                else
                {
                    matchCount = (DWORD)0;
                }

                if( matchCount == keywordLength )
                {
                    rtnStr = (ptr+i+2);
                    break;
                }
                else
                {
                    nop();
                }

                j++;
            }
            else
            {
                nop();
            }
        }
    }

    return rtnStr;
}

#define REFER_STRING_MAX 128
/**
 * @brief: 
 **/
static void
getReferenceInfo( S_PDF_REFERENCE_INFO *infoPtr, BYTE *dataPtr )
{
    TCHAR referString[REFER_STRING_MAX];
    INT i;
    BOOL flag1 = (BOOL)FALSE;
    BOOL flag2 = (BOOL)FALSE;

    memset( referString, 0, REFER_STRING_MAX );

    for( i=0; i<REFER_STRING_MAX; i++ )
    {
        if( *(dataPtr+i) != '\0')
        {
            flag1 = (BOOL)TRUE;
            referString[i] = *(dataPtr+i);

            if( (referString[i] == ' ') )
            {
                flag2 = (BOOL)TRUE;
                referString[i] = '\0';
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

    if( flag1 && flag2 )
    {
        infoPtr->bValid = (BOOL)TRUE;
        infoPtr->objectNum = atol(referString);
        infoPtr->generationNum = 0;
    }
    else
    {
        infoPtr->bValid = (BOOL)FALSE;
    }
}

#define XREF_UNIT 20
/**
 * @brief: 
 **/
static DWORD
getObjectByteOffset( FILE *fp, DWORD startPos, DWORD objectNum )
{
    PTSTR divideStrArray[2];

    readByteToCRorLF(fp,startPos+(XREF_UNIT*objectNum),tmpData,TMP_DATA_MAX);
//    DebugWndPrintf("%d: %s\r\n",objectNum,tmpData);
    divideStringBySpace(tmpData,TMP_DATA_MAX,&divideStrArray,2);
    return getValue(divideStrArray[0],TMP_DATA_MAX);
}
