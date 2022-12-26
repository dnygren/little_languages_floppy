#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "c_iml.h"


#define		bool 	int

void	error(char * message, int action);
char	*allocateString(char *string);
char	*getmem(int size);
bool 	checkToken(int type, char c);
bool	readToken();


bool lexicalAnalyzer(char *sourceFileName);
bool imlParser(struct programNode **program);
bool interpretIMLProgram(struct programNode *program);
bool compileIMLProgram(struct programNode *program, char *outputFileName, bool listStatementsFlag);	/* May 10, 1994 */
bool isReferentConnector(struct operandPointerStruct *p);
bool isReferentElement(struct operandPointerStruct *p);
bool isReferentOperator(int operator);

struct imageDBStruct *imageDB;
struct 	declarationStruct        *declarations = NULL;

/* imageFunctionTable is the global pointer to the table of image manipulation
	functions that can be used in image expressions. */

struct functionTableElementStruct *imageFunctionTable;

/* symbolTable is the global variable pointer that points to the table
	of image symbols defined in an IML program. */

struct symbolTableEntryStruct *symbolTable;

/* The log function takes the place of printf in the abstract operations
	so that we can implement a screen display pause and also output the
	results of a run to a file. These options are controlled by flags
	that are input in the command line. */

/************************* LOG declarations *********************************/

void log(char *format, char *s);

/* line keeps track of the current line written to the screen */

int lines = 0;

/* pause_between_screens is a flag set to true when the user specifies 
	that they want to pause between pages in screen output. */

bool pause_between_screens = false;

/* log_to_file is a flag set to true when the user specified that they
	want to output the operation log to a file. */

bool log_to_file = false;

/* log_file is the file to which the operation log will be sent. It is
	initially set to stdout so that output, by default, will be sent to the
	display. */

FILE *log_file = stdout;

/****************************************************************************/
/* current statetment holds the current program line of text being          */
/* processed																					 */
/*                                                                          */
char		currentStatement[129];
/****************************************************************************/

static 	int	parenCount;

void main (argc,argv)

int argc;
char **argv;

{
	struct 	programNode				*program;
	bool		compileProgramFlag;
	bool		listStatementsFlag;							/* May 10, 1994 */
	char		outputFileName[33];
	bool		result;
	struct	functionTableElementStruct		*newFunction;

	void		startupHelp();

	struct	imageStruct			*reduce(struct argumentStruct *arguments);
	struct	imageStruct			*enlarge(struct argumentStruct *arguments);
	struct	imageStruct			*extract(struct argumentStruct *arguments);
	struct	imageStruct			*rotate(struct argumentStruct *arguments);
	struct	imageStruct			*position(struct argumentStruct *arguments);

	

/* For the purposes of showing how image functions are constructed, the
	following code constructs a small image function table. */

	newFunction = allocateMemory(functionTableElementStruct);
	newFunction -> name = allocateString("reduce");
	newFunction -> function = reduce;

	newFunction -> next = NULL;
	imageFunctionTable = newFunction;

	newFunction = allocateMemory(functionTableElementStruct);
	newFunction -> name = allocateString("enlarge");
	newFunction -> function = enlarge;
	newFunction -> next = NULL;

	newFunction -> next = imageFunctionTable;
	imageFunctionTable = newFunction;

	newFunction = allocateMemory(functionTableElementStruct);
	newFunction -> name = allocateString("extract");
	newFunction -> function = extract;
	newFunction -> next = NULL;

	newFunction -> next = imageFunctionTable;
	imageFunctionTable = newFunction;

	newFunction = allocateMemory(functionTableElementStruct);
	newFunction -> name = allocateString("rotate");
	newFunction -> function = rotate;
	newFunction -> next = NULL;

	newFunction -> next = imageFunctionTable;
	imageFunctionTable = newFunction;

	newFunction = allocateMemory(functionTableElementStruct);
	newFunction -> name = allocateString("position");
	newFunction -> function = position;
	newFunction -> next = NULL;

	newFunction -> next = imageFunctionTable;
	imageFunctionTable = newFunction;



	puts("*** Image Manipulation Language *** Version 1.0");
	puts("*** May 1994              A Little Language ***");
	puts("");
	puts("");

	if (strcmp(argv[1],"/h") == 0) startupHelp();
	if (strcmp(argv[1],"?") == 0) startupHelp();

	if (argc < 2) startupHelp();

	compileProgramFlag = false;
	listStatementsFlag = false;
								 
	if (argc > 2)
	{
		if (strcmp(argv[2],"/c") == 0)
		{
			compileProgramFlag = true;
			if (argc < 4) startupHelp();
			strcpy(outputFileName,argv[3]);

			if (argc > 4) startupHelp();
		} else if (strcmp(argv[2],"/p") == 0)
		{
			pause_between_screens = true;

			if (argc > 3) startupHelp();
		} else if (strcmp(argv[2],"/f") == 0)
		{
			if (argc < 4) startupHelp();

			log_file = fopen(argv[3],"w");
			log_to_file = true;

			if (argc > 4) startupHelp();
		} else if (strcmp(argv[2],"/cl") == 0)
		{
		  compileProgramFlag = true;
		  listStatementsFlag = true;
		  if (argc < 4) startupHelp();
		  strcpy(outputFileName,argv[3]);

		  if (argc > 4) startupHelp();
		} else startupHelp();
	}

	if (!lexicalAnalyzer(argv[1])) puts("error in lexical analysis");
	else 
	{
		puts("... lexical analysis completed ...");
		puts("... parsing phase beginning ...");

		if (!imlParser(&program)) puts("error in parsing IML program");
		else
		{
			puts("... parsing phase complete ...");
			printf("... IML interpreter/compiler phase starting ...\n\n");

			if (!compileProgramFlag) result = interpretIMLProgram(program);
				else result = compileIMLProgram(program,outputFileName,listStatementsFlag);
			if (!result)
 				puts("error in interpreting/compiling the IML program");
  			else 
			{
	 			puts("... IML program successfully run/compiled ...");
				if (log_to_file) fclose(log_file);
	  			exit(0);
			}
		}
	}
}

void startupHelp()
{
		puts("run with the command: ");
		puts("1. c_iml sourceFile");
		puts("where sourceFile is the name of the IML program to translate");
		puts("and execute.");
		puts("2. c_iml sourceFile /c compiledFileName");
		puts("The /c option specifies that you want to compile the program");
		puts("as opposed to interpret. A file name is needed in this case");
		puts("for the output of the compilation process.");
		puts("3. c_iml sourceFile /p");
		puts("The /p option causes c_iml to pause between screens as it");
		puts("displays a log of information.");
		puts("4. c_iml sourceFile /f logFileName");
		puts("The /f options causes c_iml to write the operation log to");
		puts("a specified file.");
		puts("5. c_iml [/h|?]");
		puts("The /h option or ? causes this information to be displayed. ");
		puts("");

		abort();
}

FILE	*sourceFile;			/* pointer to the source file
										control structure */
									
FILE	*tokenFile;				/* pointer to the output
										file control structure */



bool lexicalAnalyzer(char *sourceFileName)

{
	/* Forward declarations of functions used in lexicalAnalyzer. */

	bool 	readLine(FILE *file, char *buffer);
	bool	getNextToken(char *buffer);
	bool	identifyToken();
	void	writeToken(FILE *file);

	char	buffer[bufferSize];		/* Buffer used for input file
										reads */


	/* open the source file. If the open fails, return to the
	caller of the lexicalAnalyzer with failure. */

	sourceFile = fopen(sourceFileName,"r");
	if (sourceFile == NULL)
	{
		error("cannot open source file",errorAction_terminate);
		return(false);
	}

	/* open the token output file. If the open fails, return
	to the caller of the lexicalAnalyzer with failure. */

	tokenFile = fopen(tokenFileName,"w");
	if (tokenFile == NULL)
	{
		error("cannot open token output file",errorAction_terminate);
		return(false);
	}

	/* With the source and token file open, the tokenization
	process can proceed. */

	/* The outer loop of the lexical analyzer reads lines from
	the input file. When there are no more lines in the file,
	processing of the source file in complete. */

	while (readLine(sourceFile,buffer))
	{
		fprintf(tokenFile,"%s\n",buffer);		/* write the current statement to the token file
																so that it can be processed by the parser. */
															/* May 10, 1994 */

		while (getNextToken(buffer))
		{

	/* Tokens are processed one at a time. An attempt is made
	to identify a token. If the token can be identified, then the
	result of that identification is written to the token output
	file. Otherwise, an error message is displayed and the
	next token is processed. */

			if (identifyToken())
				writeToken(tokenFile);
			else
				error("unidentifiable token",errorAction_terminate);
		}

		tokenType = t_special;
		tokenValue.cval = c_eol;
		writeToken(tokenFile);
	}

	fclose(sourceFile);
	fclose(tokenFile);

	puts("... IML program successfully tokenized ...");


	return(true);
}


bool readLine(FILE *filep, char *buffer)

{
	/* Read a line of the file into the input buffer. If successful, return
	a true, otherwise a false. */

	if (fgets(buffer,bufferSize,filep) == NULL) return(false);

	/* Edit the end of line character out of the buffer as this may
	sometimes cause problems. */

	buffer[strlen(buffer) - 1] = EOS;

	newLine = true;

	return(true);
}



/* This global is used to point into the current buffer. It is maintained
   as a global so that its value can survive from call to call. */
   
int _specIndex;			
				

/* specStrTok returns true if another token is found in buffer. The token
will be copied into the tokenBuffer. */

bool specStrTok(char *buffer, char *delimiters, char *specTokens, 
		bool newBuffer)

{
	int	i;
	
	bool isOneOf(char c, char *list);

	/* If the buffer has been filled (as per the newBuffer argument,
	then the index into the buffer is initialized to zero. */

	if (newBuffer) _specIndex = 0;

	/* First check to see if the current character is one of the
	special characters. */

	if (isOneOf(buffer[_specIndex],specTokens))
	{
		tokenBuffer[0] = buffer[_specIndex];
		_specIndex++;

		return(true);
	}

	/* Skip over any leading delimiter characters. */

	while (isOneOf(buffer[_specIndex],delimiters) &&
		!(buffer[_specIndex] == EOS))
		_specIndex++;

	/* If at the end of the string, then return false. */

	if (buffer[_specIndex] == EOS)
		return(false);

	/* Check to see if the current character is one of the
	special characters. If so, increment the index and 
	return it. */

	if (isOneOf(buffer[_specIndex],specTokens))
	{
		tokenBuffer[0] = buffer[_specIndex];
		_specIndex++;

		return(true);
	}

	/* Check to see if the current character is a quotation mark.
	This starts a string. Capture the string. */

	if (buffer[_specIndex] == quotationMark)
	{
		i = 0;

		tokenBuffer[i++] = buffer[_specIndex++];

		while ((buffer[_specIndex] != quotationMark) &&
			(buffer[_specIndex] != EOS))
				tokenBuffer[i++] = buffer[_specIndex++];

	/* if the last character encountered was and end of string then
	no closing quote was found and this is a token in error. */

		if (buffer[_specIndex] == EOS)
			return(false);

		if (buffer[_specIndex] == quotationMark) _specIndex++;

		tokenBuffer[i++] = quotationMark;
		tokenBuffer[i] = EOS;

		return(true);

	}

	/* If the character was not a quotation mark and not a special
	character then it must be the first chartacter of a token. Process
	characters until a delimiter character is found, or until a special
	character is found. Save all of the characters in the token
	buffer. */

	i = 0;

	tokenBuffer[i++] = buffer[_specIndex++];

	while (!isOneOf(buffer[_specIndex],delimiters) &&
		!isOneOf(buffer[_specIndex],specTokens) &&
		(buffer[_specIndex] != EOS))
			tokenBuffer[i++] = buffer[_specIndex++];

	tokenBuffer[i] = EOS;

	return(true);
}

/* This function determines if character c is one of the list of characters in the
string list. */

bool isOneOf(char c, char *list)

{
	unsigned int	p;

	for (p = 0; p<strlen(list); p++)
		if (c == *(list + p)) return(true);

	return(false);
}

bool getNextToken(char * buffer)

{
	/* if newLine is true, then the buffer has been newly filled and
	it is necessary to call strtok in new-line mode. */

	if (newLine)
	{
		if (!specStrTok(buffer,blankString,specialTokens,true)) 
			return (false);

		newLine = false;	/* The mode is now rest-of-line
				since the first token was read. */
	}
	else
		if (!specStrTok(buffer,blankString,specialTokens,false)) 
			return (false);

	return(true);
}

bool identifyToken()

{
	/* forward declarations of functions used in this function */

	bool isTokenKeyword();
	bool isTokenFileName();
	bool isTokenIdentifier();
	bool isTokenReal();
	bool isTokenInteger();
	bool isTokenString();
	bool isSpecialToken();

	if (isalpha(tokenBuffer[0]))
	{
		if (!(isTokenKeyword() || isTokenFileName() || isTokenIdentifier()))
			return(false);
			else return(true);
	}
	
	if (isdigit(tokenBuffer[0]))
	{
		if (!(isTokenReal() || isTokenInteger()))
			return(false);
			else return(true);
	}
	
	
	if (tokenBuffer[0] == quotationMark)
	{
		if (!isTokenString()) return(false);
			else return(true);
	}
	
	if (!isSpecialToken()) return(false);
		else return(true);
}

/* The keyword table contains the keywords in the IML language. Each of the
keyword is in uppercase characters. Before the tokenBuffer is compared to
any elements in the table, it will be converted to upper case. */

char keywordTable[10][33] = {"ADD", "NAME", "INDEX", "REMOVE", "AND", "OR",
"VIEW", "READ", "SAVE", "END"};

bool isTokenKeyword()

{
	bool match();

	void convertToUppercase();

	int i;

	char cToken[maxTokenLength];

	strcpy(cToken,tokenBuffer);
	convertToUppercase(cToken);

	for (i=0; i<numberOfKeywords; i++)
	{
		if (strcmp(cToken,keywordTable[i]) == 0)
		{

/* Since the keyword values are numbered sequentially from zero, returning
i is the same as returning one of the keyword values defined in the header 
file. */

			tokenValue.ival = i;
			tokenType = t_keyword;

			return(true);
		}
	}

	return(false);
}

struct trieNodeStruct
{
	char	edge;			/* current edge label */

	struct trieNodeStruct		*sibling; 	/* pointer to next node at
					the same level of the tree */

	struct trieNodeStruct		*child;	/* pointer to child of node 
					if any at next level */

	int	keyword;			/* element contains keyword
					constant value if node is a
					leaf node */
};

bool keywordInTrie(struct trieNodeStruct *trie, char *keyword, int *value)

{
	struct 	trieNodeStruct	*p;
	char	c;

	c = *keyword;
	
	p = trie;

	while (c != p -> edge && p != NULL) p = p -> sibling;

	
	/* When p is NULL, this means that there is no matching character in
	   the trie for this character of the token. */
	   
	if (p == NULL) return(false);		
	

	/* if there are no more children and if there is no more token, 
	   then return the keyword value in the leaf node, and return
	   success. */
	   
	if (p -> child == NULL)		
	{				
		if (*(keyword+1) == EOS)	
		{			
			*value = p -> keyword;
			return(true);	
		}
		
	/* if characters remain in the token, then this cannot be a token, 
	   so return failure. */
	   
		else return(false);		
	}
	/* else if there are no more characters in the keyword, but there are 
       characters left in the trie, this also cannot be a keyword. */
       
	else if (*(keyword+1) == EOS) return(false);		

	/* This recursive call corresponds to step 7 of the algorithm, go to 
	   step 3. It is a recursive call to the procedure using the child
	   node as the root of a new tree to search. */	
					
	return(keywordInTrie(p -> child, keyword+1,value));
					
}


bool isTokenCharacterSequence(char *token, int n, int *counter)

{
	int	index;

	*counter = 0;
	index = 0;

	while (ischar(*(token+index)))		
	{
		if (*(token+index) == EOS) return(true);

		index++;
		(*counter)++;

	/* if n is zero, there is no limit on the number of characters in the
	   subsequence, and this loop can continue until a non-character
	   is encountered. */

		if (((*counter) > n) && (n != 0)) return(false);
	}

	return(true);
}

bool isTokenFileName()

{
	bool 	isTokenCharacterSequence();		/* function to
															process a
															seqeunce of
															characters in the
															token. */

	int	counter;									/* number of times
															in a transition
															loop */

	int	index;									/* character pointer
															into tokenBuffer */



	if (!ischar(tokenBuffer[0])) return(false);		/* If the first
																		character is not 
																		an alphabetic
																		character, then
																		no transition can
																		be made from the
																		start state S to
																		state 1 */

	index = 1;

	/* State 1 automata */

	if (!isTokenCharacterSequence(&tokenBuffer[index],7,&counter))
		return(false);

	index += counter;

	/* transition from state 1 to state 2. Each file name must have a
		period */


	if (tokenBuffer[index] != '.') return(false);		
	
	index++;

	/* at the end of and in a final state, so this is a file name */

	if (tokenBuffer[index] == EOS)			
	{					
		tokenType = t_imageFileName;
		strcpy(tokenValue.sval,tokenBuffer);

		return(true);
	}
	
	/* State 3 automata */

	if (!(isTokenCharacterSequence(&tokenBuffer[index],3,&counter)))
		return(false);

	tokenType = t_imageFileName;
	strcpy(tokenValue.sval,tokenBuffer);

	return(true);
}


/* function to process a seqeunce of characters in the token. */


bool isTokenIdentifier()

{
	bool 	isTokenCharacterSequence();		

	int	counter;

	/* if the first character is not an alphabetic characater, then this 
		token is not an identifier. */


	if (!isalpha(tokenBuffer[0])) return(false);		

	if (!isTokenCharacterSequence(&tokenBuffer[1],0,&counter))
		return(false);

	tokenType = t_identifier;
	strcpy(tokenValue.sval,tokenBuffer);

	return(true);
}	

bool isTokenDigitSequence(char *token, int n, int *counter)

{
	int	index;

	*counter = 0;
	index = 0;

	while (isdigit(*(token+index)))		
	{
		index++;
		(*counter)++;

	/* if n is zero, there is no limit on the number of characters in the
	subsequence, and this loop can continue until a non-character
	is encountered. */

		if (((*counter) > n) && (n != 0)) return(false);

		if (*(token+index)== EOS) return(true);
	}

	return(true);
}

bool isTokenReal()

{
	int	counter;
	int	index;

	counter = 0;
	index = 0;


	/* transition from start state S to state 1. */

	if (isdigit(tokenBuffer[0]))			
	{
		if (!isTokenDigitSequence(&tokenBuffer[0],0,&counter))
			return(false);
	}

	index += counter;


	/* transition from start state S to state 2 or state 1 to state 2. */

	if (tokenBuffer[index] == '.')			
	{
		index++;
		if (!isTokenDigitSequence(&tokenBuffer[index],0,&counter))
			return(false);
	}
	else return(false);

	index += counter;

	/* Make sure that all of the token has been processed. If not,
	and there are other characters, this token is not a real number. */

	if (tokenBuffer[index] != EOS) 
		return(false);

	tokenType = t_real;
	tokenValue.dval = atof(tokenBuffer);

	return(true);
}

bool isTokenInteger()

{
	int	counter;
	int	index;

	counter = 0;
	index = 0;


	/* transition from start state S to state 1. */
	
	if (isdigit(tokenBuffer[0]))			
	{
		if (!isTokenDigitSequence(&tokenBuffer[0],0,&counter))
			return(false);
	}

	index += counter;

	/* Make sure that all of the token has been processed. If not,
	and there are other characters, this token is not a real number. */

	if (tokenBuffer[index] != EOS) 
		return(false);

	tokenType = t_integer;
	tokenValue.ival = atoi(tokenBuffer);

	return(true);
}


bool isTokenString()

{
	if (tokenBuffer[0] == quotationMark)
	{
		strcpy(tokenValue.sval,tokenBuffer);
		tokenType = t_string;

		return(true);
	}
	else return(false);
}


bool isSpecialToken()

{
	if (isOneOf(tokenBuffer[0],specialTokens))
	{
		tokenValue.cval = tokenBuffer[0];
		tokenType = t_special;

		return(true);
	}
	else return(false);
}


void writeToken(FILE *file)

{
	/* The tokenType determines exactly what gets written to the
	file.

	In the case of a keyword, only the keyword value gets written.
	In the case of a real number, the value of the real number
	gets written. */

	char temp[81];

	switch (tokenType)
	{
		case	t_keyword:
		case	t_integer:
					fprintf(file,"%d %d\n",tokenType,tokenValue.ival);
					break;

		case	t_identifier:
		case	t_imageFileName:
					fprintf(file,"%d %s\n",tokenType,tokenValue.sval);
					break;

		case	t_string:
					strcpy(temp,tokenValue.sval + 1);
					temp[strlen(temp) - 1] = EOS;
					fprintf(file,"%d %s\n",tokenType,temp);
					break;

		case	t_real:
					fprintf(file,"%d %lf\n",tokenType,tokenValue.dval);
					break;

		case	t_special:
					fprintf(file,"%d %c\n",tokenType,tokenValue.cval);
					break;

	}
}

bool imlParser(struct programNode **program)

{
	char					fileName[65];
	char					imageName[33];
	char					variableName[33];

	struct	programNode			*p;
	struct	programNode			*current;
	struct	indexListStruct			*inLstPtr;
	struct	imageReferentConnectorStruct		*imRefPtr;


	bool 	parsAddStmt(char *fileName, char *imageName,
					struct indexListStruct
					**indexList);

	struct	programNode	*bldAddStmtStruct(
					char *fileName,
					char *imageName,
					struct indexListStruct
						*indexList);

	bool	parsRemoveStmt(struct imageReferentConnectorStruct **imRefPtr);

	struct	programNode	*bldRemoveStmtStruct(
					struct imageReferentConnectorStruct *imRefPtr);

	
	bool	parsViewStmt(struct imageReferentConnectorStruct **imRefPtr);

	struct	programNode	*bldViewStmtStruct(
					struct imageReferentConnectorStruct *imRefPtr);

	struct	programNode	*bldAsgStmtStruct();

	bool	parsSaveStmt(char *variableName, char *fileName);
	struct	programNode	*bldSaveStmtStruct(char *variableName,
							 char *fileName);


	bool	parsReadStmt(char *fileName, char *variableName);
	struct	programNode	*bldReadStmtStruct(char *fileName,
							 char *variableName);

	bool	parsEndStmt();
	struct	programNode	*bldEndStmtStruct();


	tokenFile = fopen(tokenFileName,"r");
	if (tokenFile == NULL) 
	{
		error("cannot open lexical file",errorAction_terminate);
		return(false);
	}

	*program = NULL;

	fgets(currentStatement,128,tokenFile);		/* read the statement from the
																token file */
															/* May 10, 1994 */
	currentStatement[strlen(currentStatement) - 1] = EOS;
															/* May 10, 1994 */

	if (!readToken()) 
	{
		error("no token in file",errorAction_terminate);	
		return(false);
	}

	while (true)
	{
		if (tokenType == t_keyword) 
		switch (tokenValue.ival)
		{
			case k_add:
				if (!parsAddStmt(fileName,imageName,&inLstPtr))
				{
					error("bad add statement",errorAction_terminate);
					return(false);
				}

				if ((p = bldAddStmtStruct(fileName,imageName,
					inLstPtr)) == NULL)
				{
					error("error in building add structure",errorAction_terminate);
					return(false);
				}

				p -> statementType = k_add;

				break;


			case k_remove:
				if (!parsRemoveStmt(&imRefPtr))
				{
					error("bad remove statement",errorAction_terminate);
					return(false);
				}

				if ((p = bldRemoveStmtStruct(imRefPtr)) == NULL)
				{
					error("error in building remove statement",errorAction_terminate);
					return(false);
				}
				
				p -> statementType = k_remove;

				break;

			case k_view:
				if (!parsViewStmt(&imRefPtr))
				{
					error("bad view statement",errorAction_terminate);
					return(false);
				}

				if ((p = bldViewStmtStruct(imRefPtr)) == NULL)
				{
					error("error in building view statement",errorAction_terminate);
					return(false);
				}

				p -> statementType = k_view;

				break;

			case k_read:
				if (!parsReadStmt(fileName,variableName))
				{
					error("bad read statement",errorAction_terminate);
					return(false);
				}

				if ((p = bldReadStmtStruct(fileName,
					variableName)) == NULL)
				{
					error("error in building read statement",errorAction_terminate);
					return(false);
				}

				p -> statementType = k_read;

				break;

			case k_save:
				if (!parsSaveStmt(variableName,fileName))
				{
					error("bad save statement",errorAction_terminate);
					return(false);
				}

				if ((p = bldSaveStmtStruct(variableName,
					fileName)) == NULL)
				{
					error("error in building save statement",errorAction_terminate);
					return(false);
				}

				p -> statementType = k_save;

				break;

			case k_end:
				if (!parsEndStmt())
				{
					error("bad end statement",errorAction_terminate);
					return(false);
				}

				if ((p = bldEndStmtStruct()) == NULL)
				{
					error("error in building end statement",errorAction_terminate);
					return(false);
				}

				p -> statementType = k_end;

				puts("... IML program parse complete ...");
				return(true);

				break;


			default:	error("token of unknown type",errorAction_terminate);
				return(false);
		}
		else if (tokenType == t_identifier)
		{
			parenCount = 0;

			if ((p = bldAsgStmtStruct()) == NULL)
			{
				error("error in building assignment statement",errorAction_terminate);
				return(false);
			}

			p -> statementType = k_assign;
		}
		else
		{
			error("token of unknown type",errorAction_terminate);
			return(false);
		}

		if (*program == NULL) *program = p;
		else current -> next = p;

		p -> statement = allocateString(currentStatement);

		current = p;

		if (fgets(currentStatement,128,tokenFile) == NULL) break;		/* save the current statement */
		currentStatement[strlen(currentStatement) - 1] = EOS;				/* May 10, 1994 */

		if (!readToken()) break;
	}
}


bool readToken()

{
	char 	buffer[257];

	int 	i;


	if (fgets(buffer,256,tokenFile) == NULL) return(false);
	buffer[strlen(buffer) - 1] = EOS;

	sscanf(buffer,"%d",&tokenType);

	switch (tokenType)
	{
		case	t_keyword:
		case	t_integer:
					sscanf(buffer,"%d %d",
						&tokenType,
						&tokenValue.ival);
					break;

		case	t_identifier:
		case	t_imageFileName:
					sscanf(buffer,"%d %s",
						&tokenType,
						&tokenValue.sval);
					break;

		case	t_real:
					sscanf(buffer,"%d %lf",
						&tokenType,
						&tokenValue.dval);
					break;

		case	t_string:
					i = 0;
					while (buffer[i++] != BLANK) ;

					strcpy(tokenValue.sval,
						&buffer[i]);
					break;

		case	t_special:
					sscanf(buffer,"%d %c",
						&tokenType,
						&tokenValue.cval);
					break;

	}

	return(true);
}


FILE	*tokenFile;

bool parsAddStmt(char *fileName, char *imageName, 	struct indexListStruct **indexList)

{
	bool parsIndexList(struct indexListStruct **p);

	if (!readToken()) return(false);
	if (tokenType != t_imageFileName) return(false);
	strcpy(fileName, tokenValue.sval);

	if (!readToken()) return(false);
	if (tokenValue.ival != k_name) return(false);

	if (!readToken()) return(false);
	if (tokenType != t_identifier) return(false);
	strcpy(imageName, tokenValue.sval);
						
	if (!readToken()) return(false);
	if (tokenValue.ival != k_index) return(false);

	if (!parsIndexList(indexList)) return(false);

	return(true);
}

struct	programNode	*bldAddStmtStruct(char *fileName, char *imageName,
					struct indexListStruct	*indexList)

{
	struct	programNode		*p;

	p = allocateMemory(programNode);

	p -> sd.addStmt = allocateMemory(addStmtStruct);

	(p -> sd.addStmt) -> imageFileName = allocateString(fileName);
	(p -> sd.addStmt) -> imageName = allocateString(imageName);
	(p -> sd.addStmt) -> indexList = indexList;

	p -> next = NULL;

	return(p);
}


bool parsIndexElement(char *name, struct tokenValueRecord *value)

{
	if (!readToken()) return(false);
	if (!checkToken(t_special, c_leftBracket)) return(false);

	if (!readToken()) return(false);
	if (!checkToken(t_identifier, c_none)) return(false);
	strcpy(name,tokenValue.sval);

	if (!readToken()) return(false);
	if (!checkToken(t_special, c_comma)) return(false);

	if (!readToken()) return(false);
	if (checkToken(t_integer, c_none)) 
	{
		value -> ival = tokenValue.ival;
		value -> type  = t_integer;
	}
	else if (checkToken(t_real, c_none)) 
	{
		value -> dval = tokenValue.dval;
		value -> type = t_real;
	}
	else if (checkToken(t_string, c_none)) 
	{
		strcpy(value -> sval,tokenValue.sval);
		value -> type = t_string;
	}
	else if (checkToken(t_identifier, c_none))
	{
		strcpy(value -> sval, tokenValue.sval);
		value -> type = t_string;
	}
	else return(false);

	if (!readToken()) return(false);
	if (!checkToken(t_special, c_rightBracket)) return(false);

	return(true);
}

bool parsIndexList(struct indexListStruct **inLstPtr)

{
	struct	indexListStruct	*head;
	struct	indexListStruct	*current;
	struct	indexListStruct	*new;

	bool	first;

	struct	tokenValueRecord	value;
	struct	indexListStruct	*bldIndexListStruct(char *name, 
					struct tokenValueRecord *value);

	char	name[maxTokenLength];
	

	head = NULL;
	current = NULL;
	new = NULL;

	first = true;

	while (true)
	{
		if (!first)
		{
			if (!readToken()) return(false);

			if (checkToken(t_special,c_eol)) 
			{
				*inLstPtr = head;
				return(true);
			}

			if (!checkToken(t_special,c_comma)) return(false);
		}

		first = false;

		if (!parsIndexElement(name,&value)) return(false);

		new = bldIndexListStruct(name, &value);

		if (head == NULL) head = new;
		new -> next = NULL;

		if (current != NULL) current -> next = new;
		current = new;

	}
}

struct indexListStruct 
	*bldIndexListStruct(char *name, struct tokenValueRecord *value)

{
	struct	indexListStruct	*new;

	new = allocateMemory(indexListStruct);

	new -> characteristicName = allocateString(name);
	new -> type = value -> type;

	switch (value -> type)
	{
		case t_integer:
				new -> ival = value -> ival;
				break;

		case t_real:
				new -> dval = value -> dval;
				break;

		case t_string:
				new -> sval = allocateString(value -> sval);
				break;
	}

	return(new);
}

bool parsSaveStmt(char *name, char *filename)

{
	if (!readToken()) return(false);
	if (tokenType != t_identifier) return(false);

	strcpy(name, tokenValue.sval);

	if (!readToken()) return(false);
	if (tokenType != t_imageFileName) return(false);

	strcpy(filename, tokenValue.sval);

	if (!readToken()) return(false);
	if (!checkToken(t_special, c_eol)) return(false);

	return(true);
}

struct programNode *bldSaveStmtStruct(char *name, char *filename)

{
	struct	programNode				*new;
	struct	saveStmtStruct	*s;

	new = allocateMemory(programNode);

	new -> statementType = k_save;
	new -> next = NULL;

	s = allocateMemory(saveStmtStruct);

	s -> imageVariableName = allocateString(name);
	s -> imageFileName = allocateString(filename);

	new -> sd.saveStmt = s;

	return(new);
}

bool parsReadStmt(char *filename, char *name)

{
	if (!readToken()) return(false);
	if (tokenType != t_imageFileName) return(false);

	strcpy(filename, tokenValue.sval);

	if (!readToken()) return(false);
	if (tokenType != t_identifier) return(false);

	strcpy(name, tokenValue.sval);

	if (!readToken()) return(false);
	if (!checkToken(t_special, c_eol)) return(false);

	return(true);
}

struct programNode *bldReadStmtStruct(char *filename, char *name)

{
	struct programNode *new;
	struct readStmtStruct *p;

	new = allocateMemory(programNode);

	new -> statementType = k_read;
	new -> next = NULL;

	p = allocateMemory(readStmtStruct);

	p -> imageFileName = allocateString(filename);
	p -> imageVariableName = allocateString(name);

	new -> sd.readStmt = p;

	new -> next = NULL;

	return(new);
}

bool parsImageReferentElement(char *name, struct tokenValueRecord *value)

{

	if (!checkToken(t_identifier, c_none)) return(false);
	strcpy(name, tokenValue.sval);

	if (!readToken()) return(false);
	if (!checkToken(t_special, c_leftParen)) return(false);

	if (!readToken()) return(false);

	switch (tokenType)
	{
		case	t_integer:
								value -> ival = tokenValue.ival;
								value -> type = t_integer;
								break;

		case	t_real:
								value -> dval = tokenValue.dval;
								value -> type = t_real;
								break;

		case	t_string:
								strcpy(value -> sval, tokenValue.sval);
								value -> type = t_string;
								break;

		case	t_identifier:
								strcpy(value -> sval, tokenValue.sval);
								value -> type = t_identifier;
								break;
	}

	if (!readToken()) return(false);

	if (checkToken(t_special, c_rightParen)) return(true);

	return(false);
}


struct imageReferentElementStruct *bldImageReferentElementStruct(char *name,
	struct tokenValueRecord *value)

{
	struct imageReferentElementStruct *p;

	p = allocateMemory(imageReferentElementStruct);

	p -> attributeName = allocateString(name);

	switch (value -> type)
	{
		case t_integer:
							p -> ival = value -> ival;
							p -> type = t_integer;
							break;

		case t_real:
							p -> dval = value -> dval;
							p -> type = t_real;
							break;

		case t_string:
							p -> sval = allocateString(value -> sval);
							p -> type = t_string;
							break;

		case t_identifier:
							p -> sval = allocateString(value -> sval);
							p -> type = t_identifier;
							break;
	}

	return(p);
}


/* The purpose of checkToken is to test to see whether a token is of a
   specificed type. */
   
   
bool checkToken(int type, char c)

{
	if (type == t_special)
	{
		if ((tokenType == t_special) && (c == tokenValue.cval)) return(true);
			else return(false);
	}
	else
	{
		if (tokenType == type) return(true);
			else return(false);
	}
}

char *getmem(int size)

{
	char *p;
	
	p = malloc(size);
	if (p == NULL)
	{
		error("could not allocate memory",errorAction_terminate);
		abort();
	}
	
	return(p);
}


struct imageReferentConnectorStruct *bldImageReferent()
{
	int	ltype;
	int	rtype;
	int	op;

	void	*left;
	void	*right;

	char	name[33];

	struct	tokenValueRecord					value;
	struct	imageReferentConnectorStruct	*p;

	bool parsImageReferentElement(char *name, struct tokenValueRecord *value);
	struct imageReferentElementStruct *bldImageReferentElementStruct(
				char *name, struct tokenValueRecord *value);


	if (!readToken()) return(NULL);

	if (checkToken(t_special, c_leftParen))
	{
		left = (void *)bldImageReferent();
		ltype = s_connector;
	}
	else
	{
		if (!parsImageReferentElement(name, &value)) return(NULL);

		left = (void *)bldImageReferentElementStruct(name, &value);
		ltype = s_element;
	}

	if (left == NULL) return(NULL);

	if (!readToken()) return(NULL);

	if (tokenValue.ival != k_and &&
		 tokenValue.ival != k_or &&
		 tokenValue.cval != c_eol) return(NULL);

	/* allocate space for the image referent structure node to be built here */
	p = allocateMemory(imageReferentConnectorStruct);			/* May 12, 1994 */

	if (tokenValue.cval == c_eol)
	{
		p -> typeOfConnector = o_none;

		if (ltype == s_connector)
			p -> leftOperand.connector =
				(struct imageReferentConnectorStruct *)left;

		if (ltype == s_element)
			p -> leftOperand.element =
				(struct imageReferentElementStruct *)left;

		p -> leftOperand.type = ltype;

		p -> rightOperand.connector = NULL;
		p -> rightOperand.type = s_none;

		return(p);
	}

	p -> rightOperand.connector = NULL;
	p -> rightOperand.type = s_none;

	op = tokenValue.ival;
	if (op == k_and) op = o_and;
	if (op == k_or) op = o_or;

	while (true)
	{
		if (!readToken()) return(NULL);

		if (checkToken(t_special, c_rightParen)) break;

		if (checkToken(t_special, c_leftParen))
		{
			rtype = s_connector;
			right = (void *)bldImageReferent();
		}
		else
		{
			if (!parsImageReferentElement(name, &value)) return(NULL);

			right = (void *)bldImageReferentElementStruct(name, &value);
			rtype = s_element;
		}

		if (right == NULL) return(NULL);


		p = allocateMemory(imageReferentConnectorStruct);

		if (ltype == s_connector)
			p -> leftOperand.connector =
				(struct imageReferentConnectorStruct *)left;

		if (ltype == s_element)
			p -> leftOperand.element =
				(struct imageReferentElementStruct *)left;

		p -> leftOperand.type = ltype;

		p -> typeOfConnector = op;

		if (rtype == s_connector)
			p -> rightOperand.connector =
				(struct imageReferentConnectorStruct *)right;

		if (rtype == s_element)
			p -> rightOperand.element =
				(struct imageReferentElementStruct *)right;

		p -> rightOperand.type = rtype;

		left = (void *)p;					 						/* correction view8 */
		ltype = s_connector;

		if (!readToken()) return(NULL);

		if (checkToken(t_special, c_eol)) break;

		if (checkToken(t_special, c_rightParen)) break;

		if (!checkToken(t_keyword, c_none)) return(NULL);

		if (tokenValue.ival != k_and && tokenValue.ival != k_or)
			return(NULL);

		op = tokenValue.ival;
		if (op == k_and) op = o_and;
		if (op == k_or) op = o_or;
	}

	return(p);
}


bool parsViewStmt(struct imageReferentConnectorStruct **p)

{
	struct	imageReferentConnectorStruct		*bldImageReferent();

	*p = bldImageReferent();

	if (*p == NULL) return(false);

	return(true);
}

struct programNode *bldViewStmtStruct(
		struct imageReferentConnectorStruct *r)

{
	struct	programNode		*p;

	p = allocateMemory(programNode);

	p -> statementType = k_view;
	p -> next = NULL;

	p -> sd.viewStmt = allocateMemory(viewStmtStruct);
	p -> sd.viewStmt -> imageReferent = r;

	p -> next = NULL;

	return(p);
}


bool parsRemoveStmt(struct imageReferentConnectorStruct **p)

{
	struct	imageReferentConnectorStruct		*bldImageReferent();

	*p = bldImageReferent();;

	if (*p == NULL) return(false);

	return(true);
}

struct programNode *bldRemoveStmtStruct(
							struct imageReferentConnectorStruct *r)

{
	struct	programNode		*p;

	p = allocateMemory(programNode);

	p -> statementType = k_remove;
	p -> next = NULL;

	p -> sd.removeStmt = allocateMemory(removeStmtStruct);
	p -> sd.removeStmt -> imageReferent = r;

	return(p);
}


bool parsImageExpressionElement(int *expressionType, char *name, 
	struct imageFunctionStruct **pf, int *nextTokenType, 
	struct tokenValueRecord *nextToken)

{
 	bool isOneOf(char c, char *list);

	struct imageFunctionStruct *bldImageFunctionStruct();

	void copyToken(struct tokenValueRecord *to, struct tokenValueRecord *from);



	if (!readToken()) return(false);

	if (checkToken(t_special, c_eol))
	{
		*expressionType = e_eol;
		return(true);
	}

	if (!checkToken(t_identifier, c_none)) return(false);

	strcpy(name, tokenValue.sval);

	if (!readToken()) return(false);
	
	if (checkToken(t_special, c_eol))
	{
		*expressionType = e_identifier;

		*nextTokenType = tokenType;
		nextToken -> cval = tokenValue.cval;

		return(true);
	}

	if (!isOneOf(tokenValue.cval, "+-()")) return(false);

	if (isOneOf(tokenValue.cval, "+-)"))
	{
		*expressionType = e_identifier;

		*nextTokenType = tokenType;
		nextToken -> cval = tokenValue.cval;

		return(true);
	}

	*expressionType = e_function;

	*pf = bldImageFunctionStruct(name);

	if (*pf == NULL) return(false);

	if (!readToken()) return(false);

	*nextTokenType = tokenType;
	copyToken(nextToken,&tokenValue);

	return(true);
}



void copyToken(struct tokenValueRecord *to, struct tokenValueRecord *from)

{
	if (to == NULL) return;
	if (from == NULL) return;

	memcpy(to, from, sizeof(struct tokenValueRecord));
}


struct imageFunctionStruct *bldImageFunctionStruct(char *name)

{
	struct	argumentStruct		*current;
	struct	argumentStruct		*new;
	struct	argumentStruct		*head;
	struct	imageFunctionStruct	*p;

	bool		lastWasComma;	 										/* exp33 error fix */

	head = NULL;
	new = NULL;
	current = NULL;

	while (true)
	{
		lastWasComma = false;										/* exp33 error fix */

		if (checkToken(t_special, c_comma)) 
			lastWasComma = true;										/* exp33 error fix */

		if (!readToken()) return(NULL);
																			/* exp33 error fix */
		if (checkToken(t_special, c_rightParen) && lastWasComma) return(NULL);
		if (checkToken(t_special, c_rightParen)) break;

		new = allocateMemory(argumentStruct);

		switch (tokenType)
		{
			case	t_real:
									new -> argumentType = t_real;
									new -> dval = tokenValue.dval;

									break;

			case	t_integer:
									new -> argumentType = t_integer;
									new -> ival = tokenValue.ival;

									break;

			case	t_string:
									new -> argumentType = t_string;
									new -> sval = allocateString(tokenValue.sval);

									break;

			case	t_identifier:
									new -> argumentType = t_identifier;
									new -> id = allocateString(tokenValue.sval);

									break;
		}


		new -> next = NULL;

		if (head == NULL) head = new;

		if (current != NULL) current -> next = new;

		current = new;

		if (!readToken()) return(NULL);

		if (checkToken(t_special, c_rightParen)) break;

		if (!checkToken(t_special, c_comma)) return(NULL);
	}

	p = allocateMemory(imageFunctionStruct);

	p -> imageFunctionName = allocateString(name);

	p -> arguments = head;

	return(p);
}

struct programNode *bldAsgStmtStruct ()

{
	struct	programNode 	*p;
	struct	imageExpressionStruct	*q;
	struct	imageExpressionStruct	*bldImageExpressionStruct();

	p = allocateMemory(programNode);

	p -> sd.assignmentStmt = allocateMemory(assignmentStmtStruct);

	p -> sd.assignmentStmt -> imageVariableName = 
		allocateString(tokenValue.sval);

	if (!readToken()) return(false);

	if (!checkToken(t_special, c_equals)) return(false);

	if ((q = bldImageExpressionStruct()) == NULL) return(false);

	p -> sd.assignmentStmt -> imageExpression = q;

	p -> next = NULL;

	return(p);
}


struct imageExpressionStruct *bldImageExpressionStruct()

{
	void	*left;
	void	*right;

	bool parsImageExpressionElement(int *expressionType, char *name, 
		struct imageFunctionStruct **pf, int *nextTokenType, 
		struct tokenValueRecord *nextToken);

	int	type;
	int	ntt;
	int	ltype;
	int	rtype;
	int	op;

	char	name[33];

	struct	imageFunctionStruct 				*f;
	struct	tokenValueRecord					v;
	struct	imageExpressionElementStruct	*pElement;
	struct	imageExpressionStruct			*p;
	struct	imageExpressionOperandStruct	*pOperand;

	bool	first;

	first = true;


	if (!parsImageExpressionElement(&type, name, &f, &ntt, &v))
	{
		if (first && checkToken(t_special, c_eol)) return(NULL);

		first = false;

		if (!checkToken(t_special, c_leftParen)) return(NULL);

		parenCount++;

		left = (void *)bldImageExpressionStruct();
		ltype = e_expression;

		if (left == NULL) return(NULL);

		if (!readToken()) return(NULL);

		copyToken(&v, &tokenValue);								/* exp6/7 correction */
		ntt = tokenType;												/* Correctly sets value of the
																				token */
	}
	else
	{
		pElement = allocateMemory(imageExpressionElementStruct);

		if (type == e_identifier)
		{
			pElement -> type = e_identifier;
			pElement -> imageVariableName = allocateString(name);
		}

		if (type == e_function)
		{
			pElement -> type = e_function;
			pElement -> imageFunction = f;
		}

		left = (void *)pElement;
		ltype = e_element;
	}


	copyToken(&tokenValue, &v);
	tokenType = ntt;

	if (checkToken(t_special, c_eol))
	{
		p = allocateMemory(imageExpressionStruct);

		p -> typeImageOperator = o_none;

		pOperand = allocateMemory(imageExpressionOperandStruct);

		pOperand -> typeOperand = ltype;

		if (ltype == e_element)
			pOperand -> element = (struct imageExpressionElementStruct *)left;

		if (ltype == e_expression)
			pOperand -> expression = (struct imageExpressionStruct *)left;

		p -> leftOperand = pOperand;
		p -> rightOperand = NULL;

		return(p);
	}

	if (!isOneOf(tokenValue.cval,"+-)")) return(NULL);

	if (checkToken(t_special, c_rightParen)) 
	{
		parenCount--;

		if (parenCount < 0)
			error("unbalanced parenthesis",errorAction_terminate);

/* fix for exp24 */

		p = allocateMemory(imageExpressionStruct);

		p -> typeImageOperator = o_none;

		pOperand = allocateMemory(imageExpressionOperandStruct);

		pOperand -> typeOperand = ltype;

		if (ltype == e_element)
			pOperand -> element = (struct imageExpressionElementStruct *)left;

		if (ltype == e_expression)
			pOperand -> expression = (struct imageExpressionStruct *)left;

		p -> leftOperand = pOperand;
		p -> rightOperand = NULL;

/* end of fix for exp24 */

		return(p);
	}

	if (checkToken(t_special, c_plus)) op = o_plus;
		else op = o_minus;

	while (true)
	{
		if (!parsImageExpressionElement(&type, name, &f, &ntt, &v))
		{
			if (!checkToken(t_special, c_leftParen)) return(NULL);

			parenCount++;

			right = (void *)bldImageExpressionStruct();
			rtype = e_expression;

			 if (!readToken()) return(NULL);

			ntt = tokenType;
			copyToken(&v, &tokenValue);

			if (right == NULL) return(NULL);
		}
		else
		{
			if (type == e_eol) return(NULL);

			pElement = allocateMemory(imageExpressionElementStruct);

			if (type == e_identifier)
			{
				pElement -> type = e_identifier;
				pElement -> imageVariableName = allocateString(name);

			}

			if (type == e_function)
			{
				pElement -> type = e_function;
				pElement -> imageFunction = f;
			}

			right = (void *)pElement;
			rtype = e_element;
		}

		p = allocateMemory(imageExpressionStruct);

		p -> typeImageOperator = op;

		pOperand = allocateMemory(imageExpressionOperandStruct);

		pOperand -> typeOperand = ltype;

		if (ltype == e_element)
			pOperand -> element = (struct imageExpressionElementStruct *)left;

		if (ltype == e_expression)
			pOperand -> expression = (struct imageExpressionStruct *)left;

		p -> leftOperand = pOperand;

	
		pOperand = allocateMemory(imageExpressionOperandStruct);

		pOperand -> typeOperand = rtype;

		if (rtype == e_element)
			pOperand -> element = (struct imageExpressionElementStruct *)right;

		if (rtype == e_expression)
			pOperand -> expression = (struct imageExpressionStruct *)right;

		p -> rightOperand = pOperand;

		copyToken(&tokenValue, &v);
		tokenType = ntt;

		if (checkToken(t_special, c_eol))
		{
			if (parenCount > 0)
				error("unbalanced parentheses",errorAction_terminate);

			return(p);
		}

		if (checkToken(t_special, c_rightParen)) 
		{
			parenCount--;

			if (parenCount < 0)
				error("unbalance parentheses",errorAction_terminate);

			return(p);
		}


		if (!isOneOf(tokenValue.cval,"+-")) return(NULL);

		if (checkToken(t_special, c_plus)) op = o_plus;
			else op = o_minus;

		left = (void *)p;
		ltype = e_expression;
	}
}

char *allocateString(char *s)

{
	char *p;

	if (s == NULL) return(NULL);

	p = getmem(strlen(s) + 1);

	strcpy(p,s);

	return(p);
}


void error(char *message, int action)

{
	puts("error while translating/compiling/interpreting IML program");
	puts(message);
	puts("");
	puts("");

	if (action == errorAction_terminate) abort();
}


void convertToUppercase(char *s)

{
	unsigned int i;


	if (s == NULL) return;

	for (i=0; i<strlen(s); i++)
		*(s+i) = (char)toupper(*(s+i));

}

bool parsEndStmt()

{
	if (!readToken()) return(false);

	if (!checkToken(t_special,c_eol)) return(false);

	return(true);
}

struct programNode *bldEndStmtStruct()

{
	struct programNode *p;

	p = allocateMemory(programNode);

	p -> statementType = k_end;
	p -> sd.addStmt = NULL;
	p -> next = NULL;

	return(p);
}

/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/
/*=================================I M L====================================*/
/*=============================C O M P I L E R==============================*/
/*=========================I N T E R P R E T E R============================*/
/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/

/*                        Forward Declarations                              */

bool addImage(char *imageFileName, char *imageName);

bool setImageIndex(char *imageName, char *characteristic,
							struct valueStruct *characteristicValue);

bool iml_remove(struct imageListStruct *imageList);

bool selectImage(char *name,
						struct valueStruct *value, 
						struct imageListStruct **result);

bool andImageList(struct imageListStruct *imageList1, 
						struct imageListStruct *imageList2,
						struct imageListStruct **imageList);

bool orImageList(struct imageListStruct *imageList1, 
						struct imageListStruct *imageList2,
						struct imageListStruct **imageList);

bool view(struct imageListStruct *imageList);

struct imageStruct *computeImage(int operation, 
											struct imageStruct *p,
											struct imageStruct *q);

bool readImage(char *imageFileName, char *imageName);

bool saveImage(char *imageName, char *imageFileName);

bool interpretAddStatement(struct programNode *p);

bool interpretRemoveStatement(struct programNode *p);

bool interpretViewStatement(struct programNode *p);

bool interpretReadStatement(struct programNode *p);

bool interpretSaveStatement(struct programNode *p);

bool interpretAssignmentStatement(struct programNode *p);

void storeValue(int type, union valueUnion *value, struct valueStruct *target);

struct symbolTableEntryStruct *findSymbolTableEntry(char *name, struct symbolTableEntryStruct *symbolTable);

void addToImageDB(struct imageStruct *image);



/* The top level IML interpreter program */


bool interpretIMLProgram(struct programNode *program)
{
	struct		programNode		*p;
	bool			result;
	bool			done;

	p = program;
	done = false;
	symbolTable = NULL;
	imageDB = NULL;

	while (p != NULL)
	{
		switch (p -> statementType)
		{
			case	k_add:		result = interpretAddStatement(p);
									break;

			case	k_remove:	result = interpretRemoveStatement(p);
									break;	

			case	k_view:		result = interpretViewStatement(p);
									break;

			case	k_save:		result = interpretSaveStatement(p);
									break;

			case	k_read:		result = interpretReadStatement(p);
									break;

			case k_assign:		result = interpretAssignmentStatement(p);
									break;

			case k_end:			done = true;
									result = true;
									break;
		}

		if (!result) return(false);

		if (done) break;

		p = p -> next;
	}

	return(true);
}

/*==========================================================================*/
/*==========================================================================*/

/* This function interprets the structure created by the parser for the 
   ADD statement by calling the appropriate abstract machine functions. */

bool interpretAddStatement(struct programNode *p)
{

	struct addStmtStruct		*ap;
	struct indexListStruct	*ip;
	struct valueStruct		value;

/* the program node p points to a program node that in turn points 
	to an add statement structure. ap is set to point to the add statement 
	structure. */

	ap = p -> sd.addStmt;

/* call the function to add the image to the image database. */

	if (!addImage(ap ->  imageFileName, ap -> imageName)) return(false); 


/* ip will be set to point to the list of indices for the image. 
	ip will be used to traverse this list. */

	ip = ap -> indexList;


	while (ip != NULL)
	{

/* store the value into a value structure. */

		storeValue(ip -> type, (union valueUnion *)&(ip -> ival), &value);
  
		if (!setImageIndex(ap -> imageName, ip -> characteristicName, &value))
			return(false);

		ip = ip -> next; 
	}

/* All indicies have been entered into the database, so we can return 
	with success. */

	return(true);
}


void storeValue(int type, union valueUnion *value, struct valueStruct *target)
{
	target -> type = type;

	switch (type)
	{
		case t_integer:	target -> ival = value -> ival;
								break;

		case t_real:		target -> dval = value -> dval; 
								break;

		case t_string:		
		case t_identifier:
								target -> sval = value -> sval;
								target -> type = t_string;
								break;   
	} 
}


struct imageListStruct *processImageReferentExpression(struct imageReferentConnectorStruct *root)
{
	struct	imageListStruct	*p;
	struct	imageListStruct	*q;
	struct	imageListStruct 	*combine(int operator, struct imageListStruct *p, struct imageListStruct *q);
	struct	valueStruct	value;
	
	void		storeValue(int type, union valueUnion *value, struct valueStruct *target);



/* Process the left subtree */

	if (isReferentConnector(&(root -> leftOperand)))
		p = processImageReferentExpression(root -> leftOperand.connector);
	else if (isReferentElement(&(root -> leftOperand)))
	{
		storeValue((root -> leftOperand.element) -> type, 
						(union valueUnion *)&((root -> leftOperand.element) ->  ival),
						&value);
		selectImage((root -> leftOperand.element) -> attributeName, &value, &p);
	} 

/* Process the right subtree */

	if (isReferentConnector(&(root -> rightOperand)))
		q = processImageReferentExpression(root -> rightOperand.connector);
	else if (isReferentElement(&(root -> rightOperand)))
	{
		storeValue((root -> rightOperand.element) -> type, 
						(union valueUnion *)&((root -> rightOperand.element) -> ival),
						&value);
		selectImage((root -> rightOperand.element) -> attributeName, &value, &q);
	} 

	return(combine(root -> typeOfConnector, p, q)); 
}

/*============================================================================*/
/* Image list manipulation functions                                                                                                 */
/*============================================================================*/

struct	imageListStruct 	*combine(int operator, struct imageListStruct *p,
						struct imageListStruct *q)

{
	struct	imageListStruct	*target;
	struct	imageListStruct	*andImageReferentLists(struct imageListStruct *p,
								struct imageListStruct *q);
	struct	imageListStruct	*orImageReferentLists(struct imageListStruct *p,
								struct imageListStruct *q);

	if (operator == o_none)
		target = p;

	if (operator == o_and)
		target = andImageReferentLists(p, q);

	if (operator == o_or)
		target = orImageReferentLists(p,q);

	return(target);
}


struct	imageListStruct	*andImageReferentLists(struct imageListStruct *p,
							struct imageListStruct *q)

{
	bool		inList(struct imageListStruct *e, struct imageListStruct *p);

	struct	imageListStruct	*new;
	struct	imageListStruct	*last;
	struct	imageListStruct	*r;

	puts("in AND image referent lists");
	puts("");

	r = p;

	last = NULL;

	while (r != NULL)
	{
		if (inList(r,q))
		{
			new = allocateMemory(imageListStruct);
			new -> image = allocateString(r -> image);

			new -> next = last;
			last = new;

		}

		r = r -> next;    

	}

	return(last);
}



struct	imageListStruct	*orImageReferentLists(struct imageListStruct *p,
							struct imageListStruct *q)

{
	struct	imageListStruct	*new;
	struct	imageListStruct	*last;
	struct	imageListStruct	*r;

	puts("in OR image referent lists");
	puts("");

	r = p;

	last = NULL;

	while (r != NULL)
	{
			new = allocateMemory(imageListStruct);
			new -> image = allocateString(r -> image);

			new -> next = last;
			last = new;

			r = r -> next;    
	}

	r = q;

	while (r != NULL)
	{
		if (!inList(r, last))
		{
			new = allocateMemory(imageListStruct);
			new -> image = allocateString(r -> image);

			new -> next = last;
			last = new;
		}

		r = r -> next;
	}

	return(last);
}


bool inList(struct imageListStruct *e, struct imageListStruct *l)

{
	struct 	imageListStruct	*p;

	p = l;

	while (p != NULL)
	{
		if (strcmp(p ->  image, e -> image) == 0) return(true);

		p = p -> next;  
	}

	return(false);
}


bool	isReferentConnector(struct operandPointerStruct *p)

{
	if (p -> type == s_connector) return(true);
		else return(false); 
}


bool	isReferentElement(struct operandPointerStruct *p)

{
	if (p -> type == s_element) return(true);
		else return(false); 
}



bool		isReferentOperator(int operator)

{
	if (operator == o_and || operator == o_or) return(true);
		else return(false);
}


bool interpretRemoveStatement(struct programNode *p)

{
	struct imageListStruct 
		*processImageReferentExpression(struct imageReferentConnectorStruct  *root);

	struct imageListStruct	*target;

	target = processImageReferentExpression((p -> sd.removeStmt) -> imageReferent);

	return(iml_remove(target));
}

bool interpretViewStatement(struct programNode *p)

{
	struct imageListStruct 
		*processImageReferentExpression(struct imageReferentConnectorStruct  *root);

	struct imageListStruct	*target;

	bool			view(struct imageListStruct *p);


	target = processImageReferentExpression((p ->sd.viewStmt) -> imageReferent);

	return(view(target));
}

bool interpretSaveStatement(struct programNode *p)

{
	return(saveImage((p -> sd.saveStmt) -> imageVariableName, (p -> sd.saveStmt) -> imageFileName)); 
}

bool interpretReadStatement(struct programNode *p)

{
	return(readImage((p -> sd.readStmt) -> imageFileName, (p -> sd.readStmt) -> imageVariableName)); 
}

struct	functionTableElementStruct *findImageFunction(char *name)

{
	struct	functionTableElementStruct	*p;

	p = imageFunctionTable;

	while (p != NULL)
	{
		if (strcmp(name, p -> name) == 0) return(p);  
		p = p -> next; 
	}

	return(NULL);
}


struct	imageStruct *processImageExpressionElement(
				struct imageExpressionElementStruct *p)

{
	struct	imageStruct	*processImageFunction(struct imageFunctionStruct *f);
	struct	imageStruct	*findImage(char *name);


	if (p -> type == e_identifier) return(findImage(p -> imageVariableName));

	if (p -> type == e_function) return(processImageFunction(p -> imageFunction));
}


struct	imageStruct *findImage(char *name)

{
	struct	symbolTableEntryStruct *p;
	struct	imageDBStruct			  *d;

	p = symbolTable;

	while (p != NULL)
	{
		if (strcmp(p -> name, name) == 0) return(p -> image);

		p = p -> next; 
	}

	d = imageDB;

	while (d != NULL)
	{
		if (strcmp(d -> image.name, name) == 0) return(&(d -> image));

		d = d -> next;
	}

	error("image variable not found", errorAction_terminate);
}


struct imageStruct	*processImageFunction(struct imageFunctionStruct *function)

{
	struct functionTableElementStruct *p;
	struct imageStruct *result;

	p = findImageFunction(function -> imageFunctionName);

	if (p != NULL)
	{
		result = (p -> function)(function -> arguments);
		addToImageDB(result);
		return(result); 
	}

	error("image function not found",errorAction_terminate);
}


struct imageStruct	*processImageExpression(struct imageExpressionStruct *root)

{
	struct imageStruct	*leftResult;
	struct imageStruct	*rightResult;

	/* process the left operand */

	if ((root -> leftOperand) -> typeOperand == e_expression)
		leftResult = processImageExpression((root -> leftOperand) -> expression);
	else if ((root -> leftOperand) -> typeOperand == e_element)
		leftResult = processImageExpressionElement((root -> leftOperand) -> element); 


	/* process the right operand (if any) */

	rightResult = NULL;

	if (root -> rightOperand != NULL)
	{
		if ((root -> rightOperand) -> typeOperand == e_expression)
			rightResult = processImageExpression((root -> rightOperand) -> expression);
		else if ((root -> rightOperand) -> typeOperand == e_element)
			rightResult = processImageExpressionElement((root -> rightOperand) -> element);
	}

	/* combine the left and right results using the operator. Return the result of this combination
	    as the result of the expression. */

	return(computeImage(root -> typeImageOperator, leftResult, rightResult));
	
}

struct assignmentStatementStruct
{
	char *imageVariableName;
	struct imageExpressionStruct *imageExpression;	
};

bool interpretAssignmentStatement(struct programNode *p)
{
	struct imageStruct *processImageExpression(struct imageExpressionStruct *p);
	struct imageStruct *result;
	struct symbolTableEntryStruct *t;

	bool createSymbolTableEntry(char *name, struct imageStruct *image);

	void freeImage(struct imageStruct *p);
	void storeImage(struct symbolTableEntryStruct *p, struct imageStruct *i);


	/* First process the image expression */

	result = processImageExpression(p -> sd.assignmentStmt -> imageExpression);

	/* Find the target image variable in the symbol table */

	t = findSymbolTableEntry((p -> sd.assignmentStmt) -> imageVariableName, symbolTable);

	/* Create an entry in the symbol table */

	if (t == NULL) createSymbolTableEntry((p -> sd.assignmentStmt) -> imageVariableName, result);
	else 
	{
		freeImage(t -> image);
		storeImage(t,result);
	}

	return(true);
}

struct symbolTableEntryStruct *findSymbolTableEntry(char *name, struct symbolTableEntryStruct *table)
{
	struct symbolTableEntryStruct *p;

	p = table;

	while (p != NULL)
	{
		if (strcmp(name, p -> name) == 0) return(p);

		p = p -> next;
	}

	return(NULL);
}

bool compileAddStatement(struct programNode *p, FILE *f);
bool compileRemoveStatement(struct programNode *p,FILE *f);
bool compileViewStatement(struct programNode *p,FILE *f);
bool compileReadStatement(struct programNode *p, FILE *f);
bool compileSaveStatement(struct programNode *p, FILE *f);
bool compileAssignmentStatement(struct programNode *p, FILE *f);
bool writeProgramPrologue(FILE *f);
bool writeProgramEpilogue(FILE *f);
    
bool compileIMLProgram(struct programNode *program, char *outputFileName, 
								bool listStatementsFlag)		/* May 10, 1994 */
{
	struct    programNode    *p;
	bool      result;
	bool      done;
	FILE      *outputFile;
	void		 declareVariables(struct declarationStruct *declarations);
     
	p = program;
	done = false;
     
	outputFile = fopen(outputFileName,"w");
     
	if (outputFile == NULL) 
		error("compiler output file could not be opened",errorAction_terminate);
     
	/* write the program prologue for the compiled IML program */
	writeProgramPrologue(outputFile);

	fprintf(outputFile, "\n\n");
     
	/* dispatch the current program node to the appropriate processing function */
	while (p != NULL)
	{
		if (listStatementsFlag) fprintf(outputFile,"\n\n\057\052 %s \052\057\n",p -> statement);	/* May 10, 1994 */

		switch (p -> statementType)
		{
			case k_add:		result = compileAddStatement(p,outputFile);
								break;

			case k_remove:	result = compileRemoveStatement(p,outputFile);
								break;

			case k_view:	result = compileViewStatement(p,outputFile);
								break;

			case k_read:	result = compileReadStatement(p,outputFile);
								break;

			case k_save:	result = compileSaveStatement(p,outputFile);
								break;

			case k_assign: result = compileAssignmentStatement(p,outputFile);
								break;

			case k_end:		done = true;
								result = true;
								break;
		}

		if (!result) return(false);

		if (done) break;

		p = p -> next;
	}
     
	fprintf(outputFile, "\n\n");

	/* write the program epilogue */
	writeProgramEpilogue(outputFile);
	fclose(outputFile);

	declareVariables(declarations);

	return(true);
}

bool writeProgramPrologue(FILE *f)
{
	/* output the preprocessor directives */
	fprintf(f, "#include <stdio.h>\n");
	fprintf(f, "#include <stdtype.h>\n");
	fprintf(f, "#include \"c_iml.h\"\n");

     
	/* this header file is created by declareVariables and defines any 
	   temporary variables */
	fprintf(f, "#include \"declarations.h\"\n");

	fprintf(f, "\nstruct declarationStruct *declarations = NULL;\n");
	fprintf(f, "struct imageDBStruct *imageDB;\n");
	fprintf(f, "struct functionTableElementStruct *imageFunctionTable;\n");
	fprintf(f, "struct symbolTableEntryStruct *symbolTable;\n\n");

	/* output the declarations necessary for the log file. */

	fprintf(f,"\nvoid log(char *format, char *s);\n");
	fprintf(f,"int lines = 0;\n");
	fprintf(f,"bool pause_between_screens = false;\n");
	fprintf(f,"bool log_to_file = false;\n");
	fprintf(f,"FILE *log_file = stdout;\n\n");

     
	/* output the program header */
	fprintf(f, "void main(argc,argv)\n");
	fprintf(f, "int argc;\n");
	fprintf(f, "char **argv;\n");
	fprintf(f, "\n{\n");
     
	/* output function declarations */
	fprintf(f,"bool addImage(char *filename, char *imagename);\n");
	fprintf(f,"bool setImageIndex(char *name, char *characteristic, struct valueStruct *value);\n");
	fprintf(f,"bool iml_remove(struct imageListStruct *list);\n");
	fprintf(f,"struct imageListStruct *selectImage(char *name, struct valueStruct *value, struct imageListStruct **result);\n");
	fprintf(f,"struct imageListStruct *andImageReferentList(struct imageListStruct *imageList1, struct imageListStruct *imageList2);\n");
	fprintf(f,"struct imageListStruct *orImageReferentList(struct imageListStruct *imageList1, struct imageListStruct *imageList2);\n");
	fprintf(f,"struct imageListStruct *combine(int operator, struct imageListStruct *p, struct imageListStruct *q);\n");
	fprintf(f,"struct imageStruct *computeImage(int op, struct imageStruct *image1, struct imageStruct *image2);\n");
	fprintf(f,"bool readImage(char *name, char *variable);\n");
	fprintf(f,"bool saveImage(char *variable, char *name);\n");
	fprintf(f,"void freeSymbolTable(struct symbolTableEntryStruct);\n");
	fprintf(f,"struct valueStruct value;\n");
	fprintf(f,"struct argumentStruct *arguments;\n");
	fprintf(f,"struct argumentStruct *arg;\n");
	fprintf(f,"char *allocateString(char *s);\n");
	fprintf(f,"struct	imageStruct	*reduce(struct argumentStruct *arguments);\n");
	fprintf(f,"struct	imageStruct	*enlarge(struct argumentStruct *arguments);\n");
	fprintf(f,"struct	imageStruct	*extract(struct argumentStruct *arguments);\n");
	fprintf(f,"struct	imageStruct	*rotate(struct argumentStruct *arguments);\n");
	fprintf(f,"struct	imageStruct	*position(struct argumentStruct *arguments);\n");
	fprintf(f,"struct	imageDBStruct *findImageInDB(char *name);\n");
	fprintf(f,"struct imageDBStruct *dbp;\n");
	fprintf(f,"char *getmem(int size);\n");

	/* output the code to process the command line flags for controlling
		log output */

	fprintf(f,"if (argc > 1)\n");
	fprintf(f,"{\n");
	fprintf(f,"if (strcmp(argv[1],\"/p\") == 0)\n");
	fprintf(f,"{\n");
	fprintf(f,"pause_between_screens = true;\n");
	fprintf(f,"}\n\n");
	fprintf(f,"if (strcmp(argv[1],\"/f\") == 0)\n");
	fprintf(f,"{\n");
	fprintf(f,"log_to_file = true;\n");
	fprintf(f,"log_file = fopen(argv[2],\"w\");\n");
	fprintf(f,"}\n");
	fprintf(f,"}\n\n");

	
	return(true);
}

bool writeProgramEpilogue(FILE *f)
{
	fprintf(f,"if (log_to_file) fclose(log_file);\n");
	/* Write terminating brace for the main program function */
	fprintf(f,"}\n");
	
	return(true);
}

void declareVariables (struct declarationStruct *declarations)
{
	FILE      *f;
	struct declarationStruct      *d;
     
	f = fopen("declarations.h","w");
     
	if (f == NULL) error("could not open the declarations file", errorAction_terminate);
     
	d = declarations;
     
	while (d != NULL)
	{
		if (d -> type == t_imageList) 
			fprintf(f, "struct imageListStruct *%s;\n", d -> name);
     
		if (d -> type == t_image)
			fprintf(f, "struct imageStruct *%s;\n", d -> name);
     
		d = d -> next;
	}
     
	fclose(f);
}

bool compileAddStatement(struct programNode *p, FILE *f)
{
	struct    addStmtStruct  *ap;
	struct    indexListStruct     *ip;
     
	bool writeValueAssignment(FILE *f, int type, union valueUnion *value);
     
	ap = p -> sd.addStmt;
     
	fprintf(f, "if(!addImage(\"%s\",\"%s\")) error(\"addimage error\",errorAction_terminate);\n",
		ap -> imageFileName, ap -> imageName);
     
	ip = ap -> indexList;
     
	while (ip != NULL)
	{
		writeValueAssignment(f, ip -> type, (union valueUnion *)&(ip -> ival));

		fprintf(f, "if(!setImageIndex(\"%s\", \"%s\", &value",
					ap -> imageName, ip -> characteristicName);
     
		fprintf(f, ")) error(\"setImageIndex error\", errorAction_terminate);\n");
     
		ip = ip -> next;
	}
     
	return(true);
}

char *createTemporaryVariable(int type)
{
     
	void	addVariableDeclaration(char *name, int type);
	char	*createNewVariableName();
	char 	*name;
     

   name = createNewVariableName();  
	addVariableDeclaration(name, type);
     
	return(name);
}

char *createNewVariableName()
{
	char      name[11];
     
	temporaryVariableCount++;

	sprintf(name,"t%d",temporaryVariableCount);
     
	return(allocateString(name));
}
      
void addVariableDeclaration(char *name, int type)
{
	struct    declarationStruct        *p;

	p = declarations;

	while (p != NULL)
	{
		if (strcmp(p -> name, name) == 0) return;

		p = p -> next;
	}
     
	p = allocateMemory(declarationStruct);
     
	p -> name = allocateString(name);
	p -> type = type;
	p -> next = declarations;
     
	declarations = p;
}

char *referentTransducer(FILE *f, struct imageReferentConnectorStruct *root)
{
	struct    imageReferentConnectorStruct *r;
     
	char      *p;
	char      *q;
	char      *s;
	char      *createTemporaryVariable(int type);
     
	bool      writeValueAssignment(FILE *f, int type, union valueUnion *value);
	bool      writeTypedVariable(FILE *f, int type);
     
	r = root;
     
	/* Handle the left operand of the image referent expression tree */
     
	if (r -> leftOperand.type == s_connector)
		p = referentTransducer(f, r -> leftOperand.connector);
	else
	{
		p = createTemporaryVariable(t_imageList);

		writeValueAssignment(f,(r -> leftOperand.element) -> type,
									(union valueUnion *)&((r -> leftOperand.element) -> ival));

		fprintf(f, "selectImage(\"%s\", &value, &%s);\n",
					(r -> leftOperand.element) -> attributeName,
					p);
	}
     
	/* Handle the right operand of the image referent expression tree */

	if (r -> rightOperand.connector != NULL)
	{
		if (r -> rightOperand.type == s_connector)
			q = referentTransducer(f, r -> rightOperand.connector);
		else
		{
			q = createTemporaryVariable(t_imageList);


			writeValueAssignment(f,(r -> rightOperand.element) -> type,
										(union valueUnion *)&((r -> rightOperand.element) -> ival));

			fprintf(f, "selectImage(\"%s\", &value, &%s);\n",
						(r -> rightOperand.element) -> attributeName,
						q);
		}
	}
     
	/* Generate the operation that combines the operands computed from the 
		left and right operands */
     
	s = createTemporaryVariable(t_imageList);
     
	fprintf(f, "%s = combine(%d, %s, ",s, r -> typeOfConnector, p);
	if (r -> rightOperand.connector != NULL) fprintf(f, "%s);\n",q);
		else fprintf(f, "NULL);\n");

	return(s);
}


bool writeValueAssignment(FILE *f, int type, union valueUnion *value)
{
	switch (type)
	{
		case t_integer:		
									fprintf(f, "value.type = %d;\n", t_integer);
									fprintf(f, "value.ival = %d;\n", value -> ival);
									break;
     
		case t_real:        	
									fprintf(f, "value.type = %d;\n", t_real);
									fprintf(f, "value.dval = %lf;\n", value -> dval);
									break;
     
		case t_string:
		case t_identifier:
									fprintf(f, "value.type = %d;\n", t_string);
									fprintf(f, "value.sval = allocateString(\"%s\");\n", value -> sval);
									break;
	}
	
	return(true);
}

bool writeTypedVariable(FILE *f, int type)
{
	switch (type)
	{
		case t_integer:			fprintf(f,"ival");
										break;
     
		case t_real:				fprintf(f, "dval");
										break;
     
		case t_string:				fprintf(f, "sval");
										break;
	}
	
	return(true);
}

bool compileRemoveStatement(struct programNode *p, FILE *f)
{
	char *referentTransducer(FILE *f, struct imageReferentConnectorStruct *p);
     
	char *result;
     
	result = referentTransducer(f, (p -> sd.removeStmt) -> imageReferent);
     
	fprintf(f, "iml_remove(%s);\n", result);
     
	free(result);
     	  	
	return(true);
}


bool compileViewStatement(struct programNode *p, FILE *f)
{
	char *referentTransducer(FILE *f, struct imageReferentConnectorStruct *p);
     
	char *result;
     
	result = referentTransducer(f, (p -> sd.viewStmt) -> imageReferent);
     
	fprintf(f, "view(%s);\n", result);
     
	free(result);
     
	return(true);
}

bool compileSaveStatement(struct programNode *p, FILE *f)
{
	fprintf(f, "saveImage(\"%s\", \"%s\");\n",
				p -> sd.saveStmt -> imageVariableName,
				p -> sd.saveStmt -> imageFileName);
     
	return(true);
}

bool compileReadStatement(struct programNode *p, FILE *f)
{
	fprintf(f, "readImage(\"%s\", \"%s\");\n", 
				p -> sd.readStmt -> imageFileName,
				p -> sd.readStmt -> imageVariableName);
     
	return(true);
}

char *expressionTransducer(FILE *f, struct imageExpressionStruct *root)
{
	char		*p;
	char  	*q;
	char		*s;
	struct	imageExpressionStruct		*r;
	char		*createTemporaryVariable(int type);
	bool		writeTypedVariable(FILE *f, int type);
	bool 		writeImageFunction(FILE *f, char *target, struct imageFunctionStruct *function);
	char		*null = "NULL";

	r = root;
	p = NULL;
	q = NULL;
     
	/* Process the left operand of the expression tree */
	if ((r -> leftOperand) -> typeOperand == e_expression)
		p = expressionTransducer(f, (r -> leftOperand) -> expression);
	else 	if ((r -> leftOperand) -> typeOperand == e_element)
	{
		if (((r -> leftOperand) -> element) -> type == e_identifier)
		{
			p = createTemporaryVariable(t_image);
			fprintf(f,"dbp = findImageInDB(\"%s\");\n",
						((r -> leftOperand)  -> element) -> imageVariableName);
			fprintf(f,"%s = &(dbp -> image);\n",p);
		}
		else
		{
			p = createTemporaryVariable(t_image);
			writeImageFunction(f, p, ((r -> leftOperand) -> element) -> imageFunction);
		}
	}
     
	/* Process the right operand of the expression tree */
	if (r -> rightOperand != NULL)
	{
		if ((r -> rightOperand) -> typeOperand == e_expression)
			q = expressionTransducer(f, (r -> rightOperand) -> expression);
		else if ((r -> rightOperand) -> typeOperand == e_element)
		{
			if (((r -> rightOperand) -> element) -> type == e_identifier)
			{
				q = createTemporaryVariable(t_image);
				fprintf(f,"dbp = findImageInDB(\"%s\");\n",
							((r -> rightOperand)  -> element) -> imageVariableName);
				fprintf(f,"%s = &(dbp -> image);\n",q);
			}
			else
			{
				q = createTemporaryVariable(t_image);
				writeImageFunction(f, q, ((r -> rightOperand) -> element) -> imageFunction);
			}
		}
	}
     
	/* Combine the results of the left and write operand processing */
	s = createTemporaryVariable(t_image);
	if (p == NULL) p = null;
	if (q == NULL) q = null;
	fprintf(f, "%s = computeImage(%d, %s, %s);\n",s, r -> typeImageOperator, p, q);

	return(s);
}

bool writeImageFunction(FILE *f, char *target, struct imageFunctionStruct *function)
{
	struct		argumentStruct		*p;
	bool			writeArgumentValue(FILE *f, struct  argumentStruct *p);

	p = function -> arguments;

/* This code builds an argument list for the call to the image function. */

	fprintf(f,"arguments = allocateMemory(argumentStruct);\n");
	fprintf(f,"arg = arguments;\n");
	fprintf(f,"arg -> next = NULL;\n");
     
	while (p != NULL)
	{
		fprintf(f,"if (arg -> next != NULL) arg = (arg -> next);\n");

		writeArgumentValue(f, p);
     
		p = p -> next;

		fprintf(f, "arg -> next = allocateMemory(argumentStruct);\n");
	}

	fprintf(f,"free(arg -> next);\n");
	fprintf(f,"arg -> next = NULL;\n");
     
	fprintf(f, "%s = %s(arguments);\n",target, function -> imageFunctionName);

	return(true);
}

bool writeArgumentValue(FILE *f, struct argumentStruct *p)
{
	switch (p -> argumentType)
	{
		case t_integer:		fprintf(f, "arg -> ival = %d;\n", p -> ival);
									fprintf(f, "arg -> argumentType = t_integer;\n");
									break;
     
		case t_real:			fprintf(f, "arg -> dval = %lf;\n", p -> dval);
									fprintf(f, "arg -> argumentType = t_real;\n");
									break;

     	case t_identifier:
		case t_string:			
									fprintf(f, "arg -> sval = allocateString(\"%s\");\n", p -> sval);
									fprintf(f, "arg -> argumentType = t_string;\n");
									break;
	}
	
	return(true);
}

bool compileAssignmentStatement(struct programNode *p, FILE *f)
{
	char		*result;
	void		addVariableDeclaration(char *name, int type);
     
	result = expressionTransducer(f, (p -> sd.assignmentStmt) -> imageExpression);

	addVariableDeclaration((p -> sd.assignmentStmt) -> imageVariableName, t_image);
     
	fprintf(f, "%s = computeImage(%d, %s, NULL);\n", 
				(p -> sd.assignmentStmt) -> imageVariableName, o_none, result);
     
	free(result);
     
	return(true);
}


/* The writeValue function takes a pointer to a value of a type, and
	writes the value, using the correct format to the file f. */

void writeValue(FILE *f, int type, union valueUnion *value)
{
	switch (type)
	{
		case	t_integer:		fprintf(f,"%d",value -> ival);
									break;

		case	t_real:			fprintf(f,"%lf",value -> dval);
									break;

		case t_string:
		case t_identifier:	fprintf(f,"%s",value -> sval);
									break;
	}

}

/* createSymbolTableEntry adds a symbol to the symbol table. Since there
	is no implementation for an image, this field of a symbol structure
	will be set to NULL. */

bool createSymbolTableEntry(char *name, struct imageStruct *image)
{
	struct symbolTableEntryStruct		*new;

	new = allocateMemory(symbolTableEntryStruct);

	new -> name = allocateString(name);
	new -> image = image;							/* dummy set */
	new -> next = symbolTable;

	symbolTable = new;

	return(true);
}


/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/
/*==============================IML ABSTRACT MACHINE========================*/
/*===============================DUMMY OPERATIONS===========================*/
/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/

/* iml_remove is the IML abstact machine operation remove. It is named
	iml_remove to avoid a conflict with the standard remove function. */


bool iml_remove(struct imageListStruct *l)

{
	void displayImageList(struct imageListStruct *l);

	
	log("remove(abstract machine)\n",NULL);
	displayImageList(l);
	log("\n",NULL);

	return(true);
}


/* freeImage - the IML language function to free storage for an image
	in the IML database */

void freeImage(struct imageStruct *p)
{
	log("freeImage(abstract machine)\n",NULL);
}


/* setImageIndex is the IML abstract function for setting a characteristic
	of an image in the IML database. */


bool setImageIndex(char *imageName, char *characteristic, struct valueStruct *characteristicValue)
{
	struct	imageDBStruct			*p;
	struct	imageDBStruct			*findImageInDB(char *name);
	struct	characteristicStruct *c;

/* find the image in the imageDB */

	p = findImageInDB(imageName);
	if (p == NULL)
		error("image not in image database", errorAction_terminate);

	c = p -> image.characteristics;

	c = allocateMemory(characteristicStruct);
	c -> name = allocateString(characteristic);
	c -> value = allocateMemory(valueStruct);

	storeValue(characteristicValue -> type, (union valueUnion *)&(characteristicValue -> ival), c -> value);

	c -> next = p -> image.characteristics;
	p -> image.characteristics = c;

	log("setImageindex(abstract machine)\n",NULL);

	log("image name: %s\n",imageName);
	log("characteristic: %s\n", characteristic);
	log("value: ",NULL);
	writeValue(log_file, characteristicValue -> type, (union valueUnion *)&(characteristicValue -> ival));
	log("\n\n",NULL);

	return(true);
}

struct imageDBStruct *findImageInDB(char *name)
{
	struct	imageDBStruct		*p;

	p = imageDB;

	while (p != NULL)
	{
		if (strcmp(name, p -> image.name) == 0) return(p);

		p = p -> next;
	}

	return(NULL);
}


/* IML abstract machine instruction view */

bool view(struct imageListStruct *l)
{
	void displayImageList(struct imageListStruct *l);

	log("view(abstract machine)\n",NULL);
	displayImageList(l);
	log("\n",NULL);

	return(true);
}


/* IML abstract machine instruction addImage. This function adds the image
	to the imageDB list. */

bool addImage(char *imageFileName, char *imageName)
{
	struct imageDBStruct *new;
	struct imageDBStruct *findImageInDB(char *name);

/* see if the image already exists. If so, then don't add it again. */

	new = findImageInDB(imageName);
	if (new != NULL) return(true);

	new = allocateMemory(imageDBStruct);

	new -> image.name = allocateString(imageName);
	new -> image.image = NULL;
	new -> image.characteristics = NULL;

	new -> next = imageDB;

	imageDB = new;

	log("addImage(abstract machine)\n",NULL);
	log("file name: %s\n",imageFileName);
	log("image name: %s\n\n",imageName);

	return(true);
}

void displayImageList(struct imageListStruct *l)
{
	struct imageListStruct *p;

	if (l == NULL)
	{
		log("image list is empty\n",NULL);
		return;
	}

	p = l;

	while (p != NULL)
	{
		log("%s\n", p -> image);

		p = p -> next;
	}
}

bool readImage(char *imageFileName, char *imageName)
{
	addImage(imageFileName, imageName);

	log("readImage(abstract machine)\n",NULL);
	log("file name: %s\n", imageFileName);
	log("image name: %s\n\n", imageName);

	return(true);
}


bool saveImage(char *imageName, char *imageFileName)
{
	log("saveImage(abstract machine)\n",NULL);
	log("image name: %s\n", imageName);
	log("file name: %s\n\n", imageFileName);

	return(true);
}

void storeImage(struct symbolTableEntryStruct *p, struct imageStruct *i)
{
	log("storeImage(abstract machine)\n",NULL);

	log("symbol name: %s\n", p -> name);
	log("image name: %s\n", i -> name);
}

/* The next function scans the imageDB for images with the specified attribute
	and value. A list of these is returned in an image list structure. */


bool selectImage(char *name, struct valueStruct *value, struct imageListStruct **result)
{
	struct 	imageListStruct *r;
	struct 	imageDBStruct *p;
	struct	characteristicStruct *c;
	struct	imageListStruct	*new;

	bool		compareValue(struct valueStruct *value1, struct valueStruct *value2);


	r = NULL;
	p = imageDB;

	while (p != NULL)
	{
		c = p -> image.characteristics;
		while (c != NULL)
		{
			if (strcmp(c -> name,name) == 0)
			{
				if (compareValue(c -> value, value))
				{
					new = allocateMemory(imageListStruct);
					new -> image = allocateString(p -> image.name);
					new -> next = r;
					r = new;
				}
			}
			c = c -> next;
		}

		p = p -> next;
	}

	*result = r;

	log("selectImage(abstract machine)\n",NULL);
	log("characteristic name: %s\n", name);
	log("characteristic value: ",NULL);
	writeValue(log_file, value -> type, (union valueUnion *)&(value -> ival));
	log("\n",NULL);
	displayImageList(r);
	log("\n",NULL);

	return(true);
}

struct imageStruct *computeImage(int operation, struct imageStruct *p, struct imageStruct *q)
{
	char	*createTemporaryVariable(int type);
	char 	*new;

	struct imageStruct *i;

	log("computeImage(abstract machine)\n",NULL);

	log("operation: ",NULL);
	if (operation == o_none) log("o_none\n",NULL);
	if (operation == o_plus) log("o_plus\n",NULL);
	if (operation == o_minus) log("o_minus\n",NULL);

	log("operanda: %s\n", p -> name);
	if (q != NULL) log("operandb: %s\n", q -> name);
		else log("operandb: NULL\n",NULL);

	new = createTemporaryVariable(t_image);

	i = allocateMemory(imageStruct);

	i -> name = new;
	i -> image = NULL;
	i -> characteristics = NULL;

	addToImageDB(i);

	log("result: %s\n\n", new);

	return(i);
}


bool compareValue(struct valueStruct *v1, struct valueStruct *v2)
{
	if (v1 -> type != v2 -> type) return(false);

	switch (v1 -> type)
	{
		case		t_integer:			if (v1 -> ival == v2 -> ival)
												return(true);
											break;

		case		t_real:				if (v1 -> dval == v2 -> dval)
												return(true);
											break;

		case		t_string:			if (strcmp(v1 -> sval, v2 -> sval) == 0)
												return(true);
											break;

	}
	return(false);
}

struct imageStruct *reduce(struct argumentStruct *arguments)
{
	char *name;
	char *createNewVariableName();
	struct imageStruct *result;

	log("reduce(image function)\n",NULL);
	log("image name: %s\n", arguments -> sval);
	log("reduction factor: ",NULL);
	writeValue(log_file,
					(arguments -> next) -> argumentType,
					(union valueUnion *)&((arguments -> next) -> dval));
	log("\n",NULL);

	name = createNewVariableName();
	result = allocateMemory(imageStruct);

	result -> name = name;
	result -> image = NULL;
	result -> characteristics = NULL;

	log("resultant image: %s\n\n", name);

	return(result);
}

struct imageStruct *enlarge(struct argumentStruct *arguments)
{
	char *name;
	char *createNewVariableName();
	struct imageStruct *result;

	log("enlarge(image function)\n",NULL);
	log("image name: %s\n", arguments -> sval);
	log("enlarge factor: ",NULL);
	writeValue(log_file,
					(arguments -> next) -> argumentType,
					(union valueUnion *)&((arguments -> next) -> dval));
	log("\n",NULL);

	name = createNewVariableName();
	result = allocateMemory(imageStruct);

	result -> name = name;
	result -> image = NULL;
	result -> characteristics = NULL;

	log("resultant image: %s\n\n", name);

	return(result);
}

struct imageStruct *extract(struct argumentStruct *arguments)
{
	char *name;
	char *createNewVariableName();
	struct imageStruct *result;

	log("extract(image function)\n",NULL);
	log("image name: %s\n", arguments -> sval);

	log("x: ",NULL);
	writeValue(log_file,
					(arguments -> next) -> argumentType,
					(union valueUnion *)&((arguments -> next) -> ival));
	log("\n",NULL);
	log("y: ",NULL);
	writeValue(log_file,
					((arguments -> next) -> next) -> argumentType,
					(union valueUnion *)&(((arguments -> next) -> next) -> ival));
	log("\n",NULL);
	log("w: ",NULL);
	writeValue(log_file,
					(((arguments -> next) -> next) -> next ) -> argumentType,
					(union valueUnion *)&((((arguments -> next) -> next) -> next) -> ival));
	log("\n",NULL);
	log("h: ",NULL);
	writeValue(log_file,
					((((arguments -> next) -> next) -> next) -> next) -> argumentType,
					(union valueUnion *)&(((((arguments -> next) -> next) -> next) -> next) -> ival));
	log("\n",NULL);

	name = createNewVariableName();
	result = allocateMemory(imageStruct);

	result -> name = name;
	result -> image = NULL;
	result -> characteristics = NULL;

	log("resultant image: %s\n\n", name);

	return(result);
}

struct imageStruct *rotate(struct argumentStruct *arguments)
{
	char *name;
	char *createNewVariableName();
	struct imageStruct *result;

	log("rotate(image function)\n",NULL);
	log("image name: %s\n", arguments -> sval);
	log("rotate factor: ",NULL);
	writeValue(log_file,
					(arguments -> next) -> argumentType,
					(union valueUnion *)&((arguments -> next) -> dval));
	log("\n",NULL);

	name = createNewVariableName();
	result = allocateMemory(imageStruct);

	result -> name = name;
	result -> image = NULL;
	result -> characteristics = NULL;

	log("resultant image: %s\n\n", name);

	return(result);
}

struct imageStruct *position(struct argumentStruct *arguments)
{
	char *name;
	char *createNewVariableName();
	struct imageStruct *result;

	log("position(image function)\n",NULL);
	log("image name: %s\n", arguments -> sval);
	log("\n",NULL);

	log("x: ",NULL);
	writeValue(log_file,
					(arguments -> next) -> argumentType,
					(union valueUnion *)&((arguments -> next) -> ival));
	log("\n",NULL);
	log("y: ",NULL);
	writeValue(log_file,
					((arguments -> next) -> next) -> argumentType,
					(union valueUnion *)&(((arguments -> next) -> next) -> ival));
	log("\n",NULL);

	name = createNewVariableName();
	result = allocateMemory(imageStruct);

	result -> name = name;
	result -> image = NULL;
	result -> characteristics = NULL;

	log("resultant image: %s\n\n", name);

	return(result);
}

void addToImageDB(struct imageStruct *p)
{
	struct imageDBStruct *new;

	new = allocateMemory(imageDBStruct);

	new -> image.name =  p -> name;
	new -> image.image = NULL;
	new -> image.characteristics = NULL;

	new -> next = imageDB;
	imageDB = new;
}

/* The log function outputs the operation log to the display or
	to a file depending on the setting of the log_to_file flag.
	If output to the display, then it checks pause_between_screens
	to see if it should wait after displaying a screen. */

void log(char *format, char *s)
{
	char buffer[81];

	if (s == NULL) fprintf(log_file,format);
		else fprintf(log_file,format,s);

	if (!log_to_file)
	{
		if (pause_between_screens)
		{
			lines++;
			if (lines > 18)
			{
				printf("\n... press ENTER to continue ...");
				gets(buffer);
				lines = 0;
			}
		}
	}
}


