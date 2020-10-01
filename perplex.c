/*                       P E R P L E X . C
 * BRL-CAD
 *
 * Copyright (c) 2011-2019 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** @file perplex.c
 *
 * perplex scanner-generator
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "perplex.h"

/*
Desired Features:
- string and file inputs w/ autobuffering
- autogenerate scanner function
- indicate end-of-input by returning -1
- if rule actions don't end with a return,
  ignore the recognized token and continue parsing

- character classes (might implement using auto-included named patterns)
- include C code (re2c passes through by default) 
- error mechanism ? (reentrancy?)
- start conditions, and condition blocks in particular
- unquoted literal characters
- echo unmatched text?
- yylineno? (reentrancy?)

Possible Options (borrowed from flex usage message):
-?
-h,  --help		produce this help message
-i   --header=FILE	create a C header file in addition to the scanner
-L,  --noline		suppress #line directives in scanner
-o,  --outfile=FILE	specify output filename (stdout by default?)
-P,  --prefix=STRING	use STRING as prefix instead of "yy"
-t,  --template=FILE    specify path to scanner template file
-v,  --version		report perplex version
     --yylineno		track line count in yylineno
*/

static const struct option options[] =
{
	{"header", required_argument, NULL, 'i'},
	{"output", required_argument, NULL, 'o'},
	{"template", required_argument, NULL, 't'},
	{"conditions", no_argument, NULL, 'c'},
	{"safe-mode", no_argument, NULL, 's'},
	{"help", no_argument, NULL, 'h'},
	{"version", no_argument, NULL, 'v'},
	{ 0 }
};

static const char version[] = "1.0.1";

static const char usage[] =
"Usage: perplex [options] input\n"
"  -?\n"
"  -h, --help\t\tprints this message\n"
"  -c, --conditions\tenable support for start conditions\n"
"  -i, --header PATH\tspecify path of header file\n"
"  -o, --output\t\tspecify path of output file\n"
"  -s, --safe-mode\tprevent rule fall-through by skipping matched text by default\n"
"  -t, --template PATH\tspecify path to scanner template file\n"
"  -v, --version\t\tprint perplex version number and exit\n"
;

int main(int argc, char *argv[])
{
    int c;
    int tokenID;
    void *parser;
    perplex_t scanner;
    appData_t *appData;
    char defaultTemplate[] = "perplex_template.c";
    FILE *inFile;
    FILE *outFile = stdout;
    FILE *templateFile = NULL;
    FILE *headerFile = NULL;
    int usingConditions = 0;
    int safeMode = 0;

    if (argc < 2) {
	puts(usage);
	return 0;
    }

    while ((c = getopt_long(argc, argv, "i:o:t:cshv", options, 0)) != -1) {
	switch (c) {
	    case 'c':
		usingConditions = 1;
		break;
	    case 'i':
		if (optarg == NULL) {
		    fprintf(stderr, "Error: Header option requires file-path argument.\n");
		    exit(1);
		}
		if ((headerFile = fopen(optarg, "w")) == NULL) {
		    fprintf(stderr, "Error: Couldn't open \"%s\" for writing.\n", optarg);
		    exit(1);
		}
		break;
	    case 'o':
		if (optarg == NULL) {
		    fprintf(stderr, "Error: Output option requires file-path argument.\n");
		    exit(1);
		}
		if ((outFile = fopen(optarg, "w")) == NULL) {
		    fprintf(stderr, "Error: Couldn't open \"%s\" for writing.\n", optarg);
		    exit(1);
		}
		break;
	    case 's':
		safeMode = 1;
		break;
	    case 't':
		if (optarg == NULL) {
		    fprintf(stderr, "Error: Template option requires file-path argument.\n");
		    exit(1);
		}
		if ((templateFile = fopen(optarg, "r")) == NULL) {
		    fprintf(stderr, "Error: Couldn't open \"%s\" for reading.\n", optarg);
		    exit(1);
		}
		break;
            case 'v':
                puts(version);
                exit(0);
	    default:
		fprintf(stderr, "Error: Error in option string.\n");
            case 'h':
		puts(usage);
		return 1;
	}
    }

    /* look for default if scanner template not specified */
    if (templateFile == NULL) {
	if ((templateFile = fopen(defaultTemplate, "r")) == NULL) {
	    fprintf(stderr, "Error: Couldn't open default template \"%s\" "
		    "for reading.\n\tSpecify template file using \"-t PATH\" "
		    "option.\n", defaultTemplate);
	    exit(1);
	}
    }

    if ((inFile = fopen(argv[optind], "r")) == NULL) {
	fprintf(stderr, "Error: couldn't open input \"%s\" for reading\n", argv[optind]);
	exit(1);
    }

    /* create scanner and parser */
    scanner = perplexFileScanner(inFile);
    parser = ParseAlloc(malloc);

    scanner->appData = malloc(sizeof(appData_t));
    appData = scanner->appData;
    appData->in = inFile;
    appData->out = outFile;
    appData->header = headerFile;
    appData->scanner_template = templateFile;
    appData->safeMode = safeMode;
    appData->usingConditions = usingConditions;
    appData->conditions = (char*)NULL;

    /* parse */
    while ((tokenID = yylex(scanner)) != YYEOF) {
	Parse(parser, tokenID, appData->tokenData, appData);
    }
    Parse(parser, 0, appData->tokenData, appData);

    fclose(inFile);
    fclose(outFile);

    free(appData);
    ParseFree(parser, free);
    perplexFree(scanner);

    return 0;
}

/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
