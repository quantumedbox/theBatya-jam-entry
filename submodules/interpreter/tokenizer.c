// Text to token transformation

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

#include "tokenizer.h"

/*	@brief 		prints all tokens that are in given raw tokens struct
*	
*	@param[in]	raw 				pointer to RawTokens
*/

void printRawTokens(RawTokens* raw)
{
	for (int i = 0; i < raw->len; i++ )
		fprintf(stdout, "%d token: %s\n", i+1, raw->tokens[i]);
}

/*	@brief 		prints possible type of each raw token independently
*	
*	@param[in]	raw 				pointer to RawTokens
*/

void printRawTokensType(RawTokens* raw)
{
	#define case_print(type_desc) 	fprintf(stdout, "%d token: %s -- %s\n", i+1, raw->tokens[i], type_desc);	\
									break

	for (int i = 0; i < raw->len; i++ )
	{
		switch(getTokenType(raw->tokens[i])) {
		case TOKEN_VALUE:
			case_print("VALUE TOKEN");
		case TOKEN_ARITHM:
			case_print("ARITHMETIC OPERATOR TOKEN");
		case TOKEN_NAME:
			case_print("NAME TOKEN");
		case TOKEN_EOL:
			fprintf(stdout, "%d token: \\n -- EOL\n", i+1);
			break;
		case TOKEN_UNKNOWN:
			case_print("TYPE UNKNOWN");
		default:
			case_print("TYPE IS KNOWN BUT DESCRIPRION ISNT PRESENT");
		}
	}
}

/*	@brief 		get raw text tokens from a string depending of delim args
*
*	@param[in]	delimer 		pointer to a handler of type Delimer
*
*	@return 	new RawTokens*
*/

#define RAW_TOKENIZE_BATCH_SIZE 8	// array memory reallocates in batches if needed

RawTokens* rawTokenize(const char* string, const Delimer* delimer)
{
	// strtok modifies original string, so, we should create a copy
	char* str = (char*)calloc(1, strlen(string) + 1);
	strcpy(str, string);

	RawTokens* raw = (RawTokens*)malloc(sizeof(RawTokens));
	raw->tokens = (char**)malloc(RAW_TOKENIZE_BATCH_SIZE * sizeof(char*));

	size_t tokensAllocated = RAW_TOKENIZE_BATCH_SIZE;
	size_t tokensPopulated = 0;

	#define tokenArrReallocation()	if (tokensAllocated <= tokensPopulated) {														\
										tokensAllocated += RAW_TOKENIZE_BATCH_SIZE;													\
										if((raw->tokens = (char**)realloc(raw->tokens, tokensAllocated * sizeof(char*))) == NULL)	\
										{																							\
											fprintf(stderr, "Reallocation error on rawTokenize() %s\n", strerror(errno));			\
										}																							\
									}

	char* nextToken = strtok(str, delimer->delimChars);
	while(nextToken != NULL)
	{
		char* subToken;
		while((subToken = strpbrk(nextToken, delimer->delimCharTokens)) != NULL)
		{
			size_t lenToDelim = (size_t)subToken - (size_t)nextToken;

			// Token before delim char
			if (lenToDelim > 0)
			{
				tokenArrReallocation();
				raw->tokens[tokensPopulated] = (char*)calloc(1, lenToDelim + 1);
				strncpy(raw->tokens[tokensPopulated], nextToken, lenToDelim);
				++tokensPopulated;
			}

			// Delim char itself
			tokenArrReallocation();
			raw->tokens[tokensPopulated] = (char*)calloc(1, 2);
			*raw->tokens[tokensPopulated] = *subToken;
			++tokensPopulated;

			nextToken = &subToken[1];
		}

		// Token after last delimCharToken or full text if there's no delimCharToken
		if (strlen(nextToken) > 0)
		{
			tokenArrReallocation();
			raw->tokens[tokensPopulated] = (char*)malloc(strlen(nextToken) + 1);
			strcpy(raw->tokens[tokensPopulated], nextToken);
			++tokensPopulated;
		}
		nextToken = strtok(NULL, delimer->delimChars);
	}

	free(str);

	raw->len = tokensPopulated;

	return raw;
}

void delRawTokens(RawTokens* raw)
{
	for (int i = raw->len; i--;)
	{
		free(raw->tokens[i]);
	}

	free(raw->tokens);
	free(raw);
}

#define ARGV_TO_STR_BUFFER_SIZE 512

/*	@brief 		transform command line arguments to a original string
*
*	@param[in]	argc 	argument count
*	@param[in]	argv 	argument string array
*
*	@return 	0 terminated string of argv contents or NULL if no args are given
*/

char* argvToString(int argc, char** argv)
{
	if (argc < 2)
		return NULL;

	char buffer[ARGV_TO_STR_BUFFER_SIZE] = {'\0'};
	char* writePtr = buffer;
	size_t curBufferLen = 0;

	for (int i = 1; i < argc; i++)
	{
		size_t argLen = strlen(argv[i]);

		if (curBufferLen + argLen < ARGV_TO_STR_BUFFER_SIZE)
		{
			curBufferLen += argLen;

			strncpy(writePtr, argv[i], argLen);
			writePtr += argLen;

			if (i != argc)
				if (curBufferLen + 1 < ARGV_TO_STR_BUFFER_SIZE)
					*(writePtr++) = ' ';
				else goto err;
		}
		else goto err;
	}

	char* str = (char*)calloc(1, strlen(buffer) + 1);
	strcpy(str, buffer);
	return str;

	err:
		fprintf(stderr, "buffer overflow at argvToString()\n");
		exit(0x01);
}

char* fileToString(const char* filepath)
{
	FILE* file = fopen(filepath, "r");
	if (!file)
		perror("Cannot open the file");
	else
	{
		fseek(file, 0L, SEEK_END);
		char* str = (char*)calloc(1, ftell(file) + 1);
		size_t len = ftell(file);
		rewind(file);

		int curChar = fgetc(file);
		char* strPtr = str;

		while (curChar != EOF)
		{
			// printf("%c\n", curChar);
			*(strPtr++) = curChar;
			curChar = fgetc(file);
		}

		return str;
	}

	return NULL;
}

bool isTokenInt(const RawToken token)
{
	char* scan = token;
	while(*scan != '\0')
	{
		if (*scan > 57 || *scan < 48)
			return false;
		++scan;
	}

	long int overflow = strtol(token, NULL, 10);
	if (overflow == LONG_MAX || overflow == LONG_MIN)
		return false;

	return true;
}

bool isTokenArithmOperator(const RawToken token)
{
	const char** cmpToken = arithmTokens;
	while (**cmpToken != '\0')
	{
		if (!strcmp(token, *cmpToken))
			return true;
		++cmpToken;
	}

	return false;
}

bool isTokenName(const RawToken token)
{
	char* scan = token;

	#define NON_APLHABETICAL_CHAR 	!(*scan >= 65 && *scan <= 90) && !(*scan >= 97 && *scan <= 122)
	#define NON_NUMERIC_CHAR 		!(*scan >= 48 && *scan <= 57)

	// Name should always start from alphabetical symbol
	if (NON_APLHABETICAL_CHAR)
		return false;

	while(*scan != '\0')
	{
		if (NON_NUMERIC_CHAR && NON_APLHABETICAL_CHAR)
			return false;
		++scan;
	}

	return true;
}

__forceinline bool isTokenEOL(const RawToken token)
{
	return strlen(token) == 1 && *token == '\n';
}

TokenType getTokenType(const RawToken token)
{
	if (isTokenEOL(token))
		return TOKEN_EOL;

	if (isTokenInt(token))
		return TOKEN_VALUE;

	if (isTokenArithmOperator(token))
		return TOKEN_ARITHM;

	if (isTokenName(token))
		return TOKEN_NAME;

	return TOKEN_UNKNOWN;
}

void main(int argc, char** argv)
{
	char* str;

	if (argc >= 2 && !strcmp(argv[1], "open"))
		str = fileToString(argv[2]);
	else
		str = argvToString(argc, argv);

	if (!str)
		return;

	// fprintf(stdout, "--entered:\n%s\n", str);

	RawTokens* raw = rawTokenize(str, &defaultDelimer);

	// printRawTokens(raw);
	printRawTokensType(raw);

	delRawTokens(raw);
}
