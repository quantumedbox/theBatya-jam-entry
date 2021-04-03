

// --------------------------------------------------------------- Type definitions -- //


typedef char** RawTokenArray;

typedef char* RawToken;


typedef enum token {
	TOKEN_UNKNOWN,
	TOKEN_EOL,

	TOKEN_VALUE,
	TOKEN_ARITHM,
	TOKEN_NAME,
}
TokenType;

typedef enum tokenArithm {
	TOKEN_ARITHM_PLUS,
	TOKEN_ARITHM_MINUS,
	TOKEN_ARITHM_MULT,
	TOKEN_ARITHM_DIV,
}
ArithmTokenType;

typedef enum scope {
	SCOPE_FILE,
	SCOPE_FUNC,
	SCOPE_TYPE,
	SCOPE_METHOD,
	SCOPE_ATTR,
	SCOPE_EXTERN_TYPE,
}
ScopeType;

typedef void* data_t;

typedef struct
{
	TokenType 	type;

	data_t 		value;
}
Token;

typedef struct
{
	ScopeType	type;

	// array of tokens that are visible in the scope
	Token* tokens;
}
Scope;

typedef struct
{
	size_t len;

	RawTokenArray tokens;	
}
RawTokens;

typedef struct
{
	const char* 	delimChars;

	const char* 	delimCharTokens;

	// const char* 	delimDoubleChars;
}
Delimer;

// ----------------------------------------------------------------- Token delimers -- //

// Default tokens

const char defaultDelimChars[] = {
	' ',	// Space

	'\0'
};

// Such symbols should delime the string but should not be removed
const char defaultDelimCharTokens[] = {
	'\n',	// Newline

	'(',
	')',

	'[',
	']',

	'{',
	'}',

	'+',
	'-',
	'*',
	'/',

	'=',

	':',	// body declaration

	'#',	// comment-line

	// '&',
	// '|',

	// '.',
	// ',',

	'\0'
};

// // One such character placed in front of another makes it a different operator
// const char defaultDelimDoubleChars[] = {
// 	'/',
// 	'&',
// 	'|',

// 	'\0'
// };

Delimer defaultDelimer = {
	.delimChars 		= defaultDelimChars,
	.delimCharTokens 	= defaultDelimCharTokens
	// .delimDoubleChars 	= defaultDelimDoubleChars
};

// --------------------------------------------------------------------- Token refs -- //


const char* arithmTokens[] = {
	"+",
	"-",
	"*",
	"/",
	"//",

	"\0"
};

// --------------------------------------------------------------------- Signatures -- //


/*	@brief 		prints all tokens that are in given raw tokens struct
*	
*	@param[in]	raw 				pointer to RawTokens
*/
void printRawTokens(RawTokens*);


/*	@brief 		prints possible type of each raw token independently
*	
*	@param[in]	raw 				pointer to RawTokens
*/
void printRawTokensType(RawTokens*);


/*	@brief 		get raw text tokens from a string depending of delim args
*
*	@param[in]	delimChars 			chars that should delim the string
*	@param[in]	delimCharTokens 	delim chars that should be returned as tokens
*
*	@return 	pointer to a new RawTokens struct
*/
RawTokens* rawTokenize(const char* string, const Delimer*);


/*	@brief 		transform command line arguments to a original string
*
*	@param[in]	argc 	argument count
*	@param[in]	argv 	argument string array
*
*	@return 	0 terminated string of argv contents or NULL if no args are given
*/
char* argvToString(int argc, char** argv);


TokenType getTokenType(const RawToken);


bool isTokenInt(const RawToken);


bool isTokenArithmOperator(const RawToken);


bool isTokenName(const RawToken);
