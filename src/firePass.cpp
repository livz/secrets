#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>

#include "sqlite3.h"
#include "utils.h"

/* Link with the Advapi32.lib file.
 * ( registry query functions, and others.. )
 */
#pragma comment (lib, "advapi32")

static void usage(char* exe );
static int dump_ff();
static int process_row(void *passed_db, int argc, char **argv, char **col_name);
static char *get_user_profile_path();
static char *GetFirefoxLibPath();

unsigned int log_level = LOG_LEVEL_VERY_VERBOSE;

#define MAX_KEY_LEN         128

int main(int argc, char **argv){
    char mk[MAX_KEY_LEN] = {0};

    if (argc == 2) {
        if (!strncmp(argv[1], "-h", 2)) {
            usage(argv[0]);
            exit(0);
        } else {
            strncpy(mk, argv[1], MAX_KEY_LEN-1);
            VERBOSE(printf("read key: %s\n", mk););
        }
    } else if (argc >= 3) {
        printf("Invalid number of parameters\n");
        usage(argv[0]);
        exit(1);
    }

    get_user_profile_path();
    char *ff_path = NULL;
    ff_path = GetFirefoxLibPath();

    dump_ff();

    free(ff_path);
    return 0;
}

static void usage(char* exe ) {
    printf( "Dump (and decrypt) saved FireFox passwords\n" );
    printf( "(tested with FF 10)\n" );
    printf( "Usage: %s [MK]\n" \
            "MK\tOptional master key\n" \
            "-h\tHelp", exe );
}

static int dump_ff()
{
    int rc = 0;
    sqlite3 *db = NULL;
    char *err_msg = NULL;

    /* Get FF passwords database */
    char user_profile[100];
    rc = GetEnvironmentVariable("UserProfile", user_profile, 100);
    if(0 != rc){
        VVERBOSE(printf("UserProfile folder: [%s]\n", user_profile););
    }

    char login_db[200] = {0};
    strcat(login_db, user_profile);
    /* Location may be different on other versions ... */
    strcat(login_db, "\\Application Data\\Mozilla\\Firefox\\Profiles\\qgeg3prd.default\\signons.sqlite");
    VVERBOSE(printf("key3.db and signons.sqlite path: [%s]\n", login_db););

    /* Use a copy of the db. (original may be already locked) */
    rc = CopyFile(login_db, "copy_ff_db",FALSE);
    if(!rc){
        fprintf(stderr, "CopyFile failed\n");
        exit(1);
    }

    rc = sqlite3_open("copy_ff_db", &db);
    if(rc){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }

   /* rc = sqlite3_exec(db, "SELECT * FROM moz_logins", process_row, db, &err_msg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s (%d)\n", err_msg, rc);
        sqlite3_free(err_msg);
    }

    sqlite3_free(err_msg);*/
    sqlite3_close(db);

    rc = DeleteFile("copy_ff_db");
    if( !rc ){
        fprintf(stderr, "DeleteFile failed\n");
    }

    return rc;

}

/* 4th argument of sqlite3_exec is the 1st argument to callback */
static int process_row(void *passed_db, int argc, char **argv, char **col_name){
    int i = 0;
    //    int rc = 0;
    //  int blob_size = 0;

    passed_db = passed_db;

    for(i=0; i<argc; i++){
        if ( !strcmp(col_name[i], "id")) {
            printf("Id: %s\n", argv[i]);
        } else if ( !strcmp(col_name[i], "hostname")) {
            printf("Url: %s\n", argv[i]);
        } else if ( !strcmp(col_name[i], "formSubmitURL")) {
            printf("Form URL: %s\n", argv[i]);
        } else if ( !strcmp(col_name[i], "encryptedUsername")) {
            printf("(Encryoted) username: %s\n", argv[i]);
        } else if ( !strcmp(col_name[i], "encryptedPassword")) {
            printf("(Encrypted) password: %s\n", argv[i]);
        }
    }

    printf("\n");
    return 0;
}

static char *get_user_profile_path(){
    char *user_prof = NULL;
    int rc = 0;
    char line[1024] = {0};

    /* Get FF passwords database */
    char user_profile[100];
    rc = GetEnvironmentVariable("UserProfile", user_profile, 100);
    if(0 != rc){
        VVERBOSE(printf("UserProfile folder: [%s]\n", user_profile););
    }

    char partial_prof_dir[200] = {0};
    strcat(partial_prof_dir, user_profile);
    strcat(partial_prof_dir, "\\Application Data\\Mozilla\\Firefox\\");

    char profile_file[250] = {0};
    strcat(profile_file, partial_prof_dir);
    strcat(profile_file, "\\profiles.ini");

    // Open the firefox profile setting file
    FILE *profile = fopen(profile_file, "r");

    if( profile == NULL )
    {
        printf("Unable to find firefox profile file: %s\n", profile_file);
        return NULL;
    }

    // Check each line of profile settings file for line "name=default" string
    // This indicates that we are looking under default profile...
    // So one among next few lines will have path information..just copy that...
    char prof_dir[500] = {0};
    strcat(prof_dir, partial_prof_dir);

    int isDefaultFound = 0;
    while(fgets(line, 1024, profile))
    {
        if( !isDefaultFound && ( strstr(line, "Name=default") != NULL) )
        {
            isDefaultFound = 1;
            continue;
        }

        // We have got default profile ..now check for path
        if( isDefaultFound )
        {
            if( strstr(line,"Path=") != NULL)
            {
                char *subdir = (strchr(line, '=') + 1);
                strcat(prof_dir, subdir);
                printf("User Profile dir: %s\n", prof_dir);
            }
        }

    }

    fclose(profile);
}

static char *GetFirefoxLibPath()
{
    char regSubKey[]    = "SOFTWARE\\Clients\\StartMenuInternet\\firefox.exe\\shell\\open\\command";
    char path[_MAX_PATH] ="";
    char *firefoxPath = NULL;
    DWORD pathSize = _MAX_PATH;
    DWORD valueType;
    HKEY rkey;

    // Open firefox registry key
    if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, regSubKey, 0, KEY_READ, &rkey) != ERROR_SUCCESS )
    {
        VERBOSE(printf("Failed to open the firefox registry key : HKLM\\%s", regSubKey ););
        return NULL;
    }

    // Read the firefox path value
    if( RegQueryValueEx(rkey, NULL, 0,  &valueType, (unsigned char*)&path, &pathSize) != ERROR_SUCCESS )
    {
        VERBOSE(printf("Failed to read the firefox path value from registry\n"););
        RegCloseKey(rkey);
        return NULL;
    }

    RegCloseKey(rkey);

    VERBOSE(printf("Path value read from registry is %s (len: %d)\n", path, pathSize););

    char * tmp = strrchr(path, '\\');   // Trim executable name
    *tmp = NULL;
    firefoxPath = (char*) malloc( strlen(path) + 1);
    strcpy(firefoxPath, path);

    VERBOSE(printf("Firefox path = [%s]\n", firefoxPath););

    return firefoxPath;
}
