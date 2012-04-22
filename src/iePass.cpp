#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>

#define LOG_LEVEL_VERY_VERBOSE		2
#define LOG_LEVEL_VERBOSE			1
#define LOG_LEVEL_NONE				0

#define VERBOSE(code) do { if (log_level >= LOG_LEVEL_VERBOSE) { code } } while( 0 )
#define VVERBOSE(code) do { if (log_level >= LOG_LEVEL_VERY_VERBOSE) {code } } while( 0 )

static void usage(char* exe );

int log_level = LOG_LEVEL_NONE;

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


	return 0;
}

static void usage(char* exe ) {
	printf( "Unprotect and dump saved IE passwords\n" );
	printf( "(tested with IE6)\n" );
	printf( "%s [-v | -vv | -h]\n-v\tVerbose\n-vv\tVery verbose\n-h\tHelp", exe );
}
