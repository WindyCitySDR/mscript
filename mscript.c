/**
 *
 * Build: 
 *   system([matlabroot '/bin/mex -f ' matlabroot '/bin/engopts.sh mscript.c'])
 * 
 * or
 *
 *   make all  # change Makefile settings if required.
 *
 * @license: The MIT License (MIT)
 * Copyright (c) 2014 Yauhen Yakimovich <yauhen.yakimovich@uzh.ch>    
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include <dlfcn.h>
#include "engine.h"

#define  BUFSIZE 4194304
#define  STRSIZE 4096


// Handles from libeng.so
void *engine_handle;
Engine *(*engOpenFn)(const char *startcmd);
int (*engOutputBufferFn)(Engine *ep, char *buffer, int buflen);
int (*engCloseFn)(Engine *ep);
int (*engEvalStringFn)(Engine *ep, const char *string);


int load_enginge(const char *engine_lib_path) {
    engine_handle = dlopen (engine_lib_path, RTLD_LAZY);
    if (!engine_handle) {
        fputs (dlerror(), stderr);
        exit(1);
    } 
}

void *load_symbol(const char *name) { 
    void *symbol;
    char *error;

    symbol = dlsym(engine_handle, name);
    if ((error = dlerror()) != NULL)  {
        fputs(error, stderr);
        exit(1);
    }

    return symbol;
}

int unload_enginge() {    
    dlclose(engine_handle);
}


int main( int argc, char *argv[] )
{
    FILE *input;
    Engine *ep;
    char matlab_call[STRSIZE+1];
    char libeng_path[STRSIZE+1];
    char str[STRSIZE+1];
    char output_buffer[BUFSIZE+1];

    snprintf(matlab_call, sizeof matlab_call, 
             "%s/matlab -nodesktop -nosplash", 
             MATLABPATH);

    if (argc > 1) {
        if ((input = fopen(argv[1],"r")) == NULL) {        
            fprintf(stderr, "\nCannot open file: %s\n", argv[1]);
            return EXIT_FAILURE;
        }
    } else {
        /* assume STDIN */
        input = stdin;
    }
    
    /*
     * Load engine symbols dynamically.
     */
    if (strncmp(MATLABARCH, "mac", 3) == 0) {
        snprintf(libeng_path, sizeof libeng_path, 
             "%s/%s/%s", MATLABPATH, MATLABARCH, "libeng.dylib");    
    } else {
        snprintf(libeng_path, sizeof libeng_path, 
             "%s/%s/%s", MATLABPATH, MATLABARCH, "libeng.so");
    }
    puts(libeng_path);
    load_enginge(libeng_path);
    engOpenFn = load_symbol("engOpen");
    engOutputBufferFn = load_symbol("engOutputBuffer");
    engEvalStringFn = load_symbol("engEvalString");
    engCloseFn = load_symbol("engClose");

    /*
     * Call engOpen with a NULL string. This starts a MATLAB process 
     * on the current host using the command "matlab".
     */
    if (!(ep = engOpenFn(matlab_call))) {
        fprintf(stderr, "\nCan't start MATLAB engine.\n");
        return EXIT_FAILURE;
    }
    /* Set output buffer */
    output_buffer[BUFSIZE] = '\0';
    engOutputBufferFn(ep, output_buffer, BUFSIZE);
    
    /* Input loop */    
    str[STRSIZE] = '\0';
    while( fgets(str, STRSIZE, input) ) {    
        /*fprintf(stdout, "input: '%s'", (const char *)str);*/
        /* Ignore comments starting with hashtags*/
        if ((str[0] == '#')
            	/* Is the line blank? */
            	|| (str[strspn(str, " \t\v\r\n")] == '\0')) { 
            continue;
        }
        /* Eval matlab code string */
        engEvalStringFn(ep, str);
        /* Print output buffer to stdout */
        /* First three characters are always the double prompt and space (>> ).*/
        printf("%s", output_buffer + 3);
        /* TODO: error messages start with ??? */
        output_buffer[0] = '\0';
        str[0] = '\0';
    }   

    /* Close MATLAB engine and exit. */
    fclose(input);
    engCloseFn(ep);
    unload_enginge();
    
    return EXIT_SUCCESS;
}
