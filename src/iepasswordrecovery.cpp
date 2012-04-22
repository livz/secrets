/*
 * Tool name   : IEPasswordRecovery
 * Description : A tool recover IE6 autocomplete and authentication 
 *               account data.
 * Version     : 0.1
 * OS          : Tested on Microsoft Windows XP
 * Todo        : IE 7 autocomplete + authentication account data.
 *
 * Changes     : - 
 * 
 *
 */

#include <windows.h>
#include <stdio.h>

#import "pstorec.dll" no_namespace

#pragma comment(lib, "Advapi32.lib")

#define MAX_BUF_SIZE 1024



/*
 * Constants, data types and function forward declarations.
 *
 */

void recoverIE6Accounts(void);



/*
 * Type definitions
 *
 */

struct PASSWORD_CACHE_ENTRY 
{
  WORD cbEntry; 
  WORD cbResource; 
  WORD cbPassword; 
  BYTE iEntry;
  BYTE nType; 
  char abResource[1];
};


typedef BOOL (FAR PASCAL *CACHECALLBACK)( struct PASSWORD_CACHE_ENTRY FAR *pce, DWORD dwRefData);
typedef DWORD (WINAPI *ENUMPASSWORD)(LPSTR pbPrefix, WORD  cbPrefix, BYTE  nType, CACHECALLBACK pfnCallback, DWORD dwRefData);
typedef HRESULT (WINAPI *tPStoreCreateInstance)(IPStore **, DWORD, DWORD, DWORD);




/*
 * Program entry point.
 *
 */

int main()
{
  int lRetVal = 0;

  recoverIE6Accounts();
  printf("\n\nHit enter to stop execution ...\n");
  getc(stdin);

  return(lRetVal);
}




/*
 * Enumerate IE 6
 *
 */

void recoverIE6Accounts(void)
{
  int lCounter = 0;
  int lCounter1 = 0;
  int lCounter2 = 0;
  int lIndex = 0;
  GUID lTypeGUID;
  char lItemName[512];       
  char lItemData[512];
  char lItemGUID[128];
  IPStorePtr lPStore;
  GUID lSubTypeGUID;
  char lCheckingData[256];
  unsigned long lDataLength = 0;
  unsigned char *lDataPointer = NULL;
  _PST_PROMPTINFO *lPSTInfo = NULL;
  IEnumPStoreTypesPtr lEnumPStoreTypes;
  IEnumPStoreItemsPtr lSPEnumItems;
  IEnumPStoreTypesPtr lEnumSubTypes;
  LPWSTR lTempItemName;
  BOOL lDeletedOEAccount = TRUE;
  tPStoreCreateInstance lPStoreCreateInstancePtr;
  HMODULE lDLLHandle = LoadLibrary("pstorec.dll");


  printf("%-50s %-20s %-15s %-15s\n\n", "Host/Name", "Type", "Username", "Password");

  if (lPStoreCreateInstancePtr = (tPStoreCreateInstance) GetProcAddress(lDLLHandle, "PStoreCreateInstance"))
  {    
    lPStoreCreateInstancePtr(&lPStore, 0, 0, 0); 

    if (!FAILED(lPStore->EnumTypes(0, 0, &lEnumPStoreTypes)))
    {
      while(lEnumPStoreTypes->raw_Next(1, &lTypeGUID, 0) == S_OK)
      {      
        wsprintf(lItemGUID, "%x", lTypeGUID);
        lPStore->EnumSubtypes(0, &lTypeGUID, 0, &lEnumSubTypes);
	  
        while(lEnumSubTypes->raw_Next(1, &lSubTypeGUID, 0) == S_OK)
        {
          lPStore->EnumItems(0, &lTypeGUID, &lSubTypeGUID, 0, &lSPEnumItems);


          while(lSPEnumItems->raw_Next(1, &lTempItemName, 0) == S_OK)
          {
            wsprintf(lItemName, "%ws", lTempItemName);			 
            lPStore->ReadItem(0, &lTypeGUID, &lSubTypeGUID, lTempItemName, &lDataLength, &lDataPointer, lPSTInfo, 0);

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
    } // if (!FAILED(lPStore->EnumTypes(0...
  } // if (lPStoreCreateInstancePtr = (tPSt
}
