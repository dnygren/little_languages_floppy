/* Utility Declarations */

#define		bool			int
#define		true			1
#define		false			0
#define		EOS				'\0'
#define		quotationMark	'"'
#define		blankString		" "
#define		BLANK				' '
#define		tokenFileName	"token.out"

#define		ischar(x)		(isalpha(x) || isdigit(x))

/* 
	All of the constants prefixed with a t_ are token types and
	will be assigned to the token type variable when a potential
	token is identified. 
*/
 
#define 	t_keyword			0
#define	t_identifier		1
#define	t_real				2
#define	t_integer			3
#define	t_string				4
#define	t_special			5
#define	t_imageFileName	6
#define	t_image				7
#define	t_imageList			8
#define	t_expression		9



/* 
	Constants prefixed with k_ are used to represent keywords
	of the IML language.
*/

#define	k_add				0
#define	k_name			1
#define	k_index			2
#define	k_remove			3
#define	k_and				4
#define	k_or				5
#define	k_view			6
#define	k_read			7
#define	k_save			8
#define	k_end				9
#define	k_assign			10


#define	c_leftBracket	'['
#define	c_rightBracket	']'
#define	c_comma			','
#define	c_none			EOS
#define	c_leftParen		'('
#define	c_rightParen	')'
#define	c_eol				'#'
#define	c_plus			'+'
#define	c_minus			'-'
#define	c_equals			'='

#define	o_and				0
#define	o_or				1
#define 	o_none			2
#define	o_plus			3
#define	o_minus			4

#define	s_connector		0
#define	s_element		1
#define	s_none			-1

#define	e_identifier	0
#define	e_function		1
#define	e_expression	2
#define	e_element		3
#define  e_eol				-1


/* 
	maxTokenLength is a specification of the maximum possible
	length of the token buffer. 
*/

#define	maxTokenLength	81


/* The error actions control what the error function does after it displays
	a message about the error. */

#define	errorAction_noTerminate		0
#define	errorAction_terminate		1


/* 
	bufferSize is the number of characters in buffers that will
	be used for input or output operations.
*/

#define	bufferSize		257



/* 
	numberOfKeywords specifies the number of keywords in the
	keyword table.
*/

#define	numberOfKeywords	10



/* 
	specialTokens are those single characters that are defined
	in the language and need special attention from the tokenizer 
*/

#define	specialTokens		"[](),+-="


/* 
	allocateMemory is a macro that calls on the memory allocation function
	to allocate memory of a certain type.
*/

#define	allocateMemory(x)	(struct x *)getmem(sizeof(struct x))
	

/* 
	tokenType is set to the type of the token (one of the t_
	constants) by the token classification functions.
*/


static	int	tokenType;


struct	tokenValueRecord
{
	int type;
	union
	{
		char sval[maxTokenLength];
		int ival;
		double dval;
		char cval;
	} u;
};



/* 
	tokenValue will be set to the value of the token aafter
	a token classification function identifies a token. 
*/

static	struct	tokenValueRecord	tokenValue;



/* 
	tokenBuffer will contain the current token extracted from
	the current line in the file input buffer. 
*/

static	char	tokenBuffer[maxTokenLength];



/* 
	newLine is a flag that is set to true whenever readLine
	fills the input buffer with a new line from the source program
	input file. 
*/

static	bool	newLine;




/* 
	Structure for representing the ADD statement . An add statement contains
	the name of an image file, the name of the image, and the indices 
	attached to the image. The indices are contained in their own structure.
*/

struct addStmtStruct
{
    char *imageFileName;
    char *imageName;
    struct indexListStruct *indexList;
};



/* 
	Structure for the indices of an ADD statement. The indices that can 
	be assigned to an image consist of a name for the index and a value 
	for it. An image can have any number of indices assigned to it. A 
	linked list structure is also used to contain index information. Since 
	an index value can be an integer, a real number, or a character string, 
	a union structure is used for the index  value.
*/

struct indexListStruct
{
	int type;
	char *characteristicName;
	union
	{
		int ival;	      /* for integers */
		double dval;  /* for real numbers */
		char *sval;    /* for strings */
	} u;
	struct indexListStruct *next;
};

/* 
	Structure for representing an image referent primitive element. Image 
	referents are used in view and remove statements. They are used to 
	designate or refer to images in the IML image data base. An image 
	referent consists of primitive elements that specify a class of indices. 
	A primitive element consists of an attribute (an index) and an associated 
	value.
*/

struct imageReferentElementStruct
{
	char *attributeName;
	int type;

	union
	{
		int ival;
		double dval;
		char *sval;
		char *identifier;
	} u;
};


/*
	The operands are pointed to by a structure whose elements can point 
	to either a primitive image referent elements or another connector 
	structure, meaning that the operand is another image referent expression. 
*/

struct operandPointerStruct
{
	int type;
	union
	{
		struct imageReferentConnectorStruct *connector;
		struct imageReferentElementStruct *element;
	} u;
};



/* 
	Structure for representing an image referent expression connector. An 
	image referent consists of either a primitive element or a combination 
	of elements connected by the operators AND and OR. The connector node 
	joins two primitive image referent elements or one or two other image 
	referent expressions.
*/

struct imageReferentConnectorStruct
{
	int typeOfConnector;
	struct operandPointerStruct leftOperand;
	struct operandPointerStruct rightOperand;
};


/* 
	Structure for representing the REMOVE statement. The REMOVE statement 
	consists of the keyword REMOVE and an image referent. The structure for 
	it will consists of a pointer to its image referent structure.
*/

struct removeStmtStruct
{
	struct imageReferentConnectorStruct *imageReferent;
};


/* 
	Structure for representing the VIEW statement. The VIEW statement also 
	consists of a keyword and a pointer to an image referent structure.
*/

struct viewStmtStruct
{
	struct imageReferentConnectorStruct *imageReferent;
};


/* 
	Structure for representing the assignment statement. An assignment 
	statement has a variable to which an image computation will be will be 
	assigned and an image expression which forms the computation.
*/

struct assignmentStmtStruct
{
	char *imageVariableName;
	struct imageExpressionStruct *imageExpression;
};


/* 
	Structure for representing an image expression element. On the simplest 
	level in an image expression are the operands. There are two kinds of 
	image expression operands. These are image variables and image functions. 
	These elements are combined by operators to form more complex image 
	expressions.
*/

struct imageExpressionElementStruct
{
	int type;
	union
	{
		char *imageVariableName;
		struct imageFunctionStruct *imageFunction;
	} u;
};


/* 
	Structure for representing image function. An image function in IML is 
	in the standard functional notation - a function name and a list of 
	arguments. Any number of arguments can be part of an image function. 
	The structure consists of the name of the function and a pointer to the 
	list of function arguments.
*/

struct imageFunctionStruct
{
	char *imageFunctionName;
	struct argumentStruct *arguments;
};


/* 
	Structure for representing arguments. A functional argument can be one 
	of an integer, real, string, or identifier value. The structure for this 
	is declared as a union. The functional structure also includes a type 
	specification and a pointer to the next argument in the argument list.
*/

struct argumentStruct
{
	int argumentType;
	union
	{
		int ival;
		double dval;
		char *sval;
		char *id;
	} u;
	struct argumentStruct *next;
};


/* 
	Structure for an Image Expression. Complex image expressions consist of 
	two operands connected with a (+ or -). The structure contains a field 
	for the operator and pointers for the left and right operands. The left 
	and/or right operand can be either a primitive expression or a pointer to 
	another expression structure.
*/

struct imageExpressionStruct
{
	int typeImageOperator;
	struct imageExpressionOperandStruct *leftOperand;
	struct imageExpressionOperandStruct *rightOperand;
};

struct imageExpressionOperandStruct
{
	int typeOperand;

	union
	{
		struct imageExpressionElementStruct *element;
		struct imageExpressionStruct			*expression;
	} u;
};


/* 
	Structure for an entry in the symbol table. The symbol table contains 
	the names of  any identifiers used in a program. In IML all variables 
	are the same type and all variables are global. A singly-linked list 
	whose elements contain the name of the symbol and a pointer to its data 
	satisfy the requirements for a symbol table for the IML language.
*/

struct symbolTableEntryStruct
{
	char *name;
	struct imageStruct *image;
	struct symbolTableEntryStruct *next;
};


/* 
	Structure for the  READ statement. The READ statement requires two 
	elements. The first is the name of the file from which the image is to 
	be read and the second is the name of the variable to which the image 
	will be assigned. Both of these elements are character strings, so the 
	structure will contain two pointers, each to a character string.
*/

struct readStmtStruct
{
	char *imageFileName;
	char *imageVariableName;
};

/* 
	Structure for the SAVE statement. The SAVE statement is similar to the 
	READ statement except that the specification of its elements are in the 
	reverse order. This is because a SAVE statement saves an image from an 
	image variable to a file.
*/

struct saveStmtStruct
{
	char *imageVariableName;
	char *imageFileName;
};

/* 
	Structure for the statement definition structure. This structure, used 
	in the programNode, is a structure that can be any one of 6 possible 
	structures to represent information about any of the IML statements. It 
	will be defined as a union and consist of pointers to each of the its 
	possible structures.
*/

union statementDefinitionUnion
{
		struct addStmtStruct *addStmt;
		struct removeStmtStruct *removeStmt;
		struct viewStmtStruct *viewStmt;
		struct assignmentStmtStruct *assignmentStmt;
		struct readStmtStruct *readStmt;
		struct saveStmtStruct *saveStmt;
};


/* 
	Fundamental program statement representation structure. A program 
	node will contain a value that defines the statement the node represents 
	and also a pointer to the structure that will represent information 
	contained in the statement. One node is connected to another with a 
	pointer.
*/

struct programNode
{
	int statementType;
	union statementDefinitionUnion sd;
	struct programNode *next;
};


struct imageListStruct
{
	char *image; 						
	struct imageListStruct *next;
};

struct valueStruct
{
	int type;
	union
	{
		int ival;
		double dval;
		char *sval;
	} u;
};

/* This is the dummy declaration of the structure to hold an image. In
	the actual implementation it will be significantly different. */

struct imageStruct
{
	char *name;
	char *image;
	struct characteristicStruct	*characteristics;
};

/* The characteristic structure is used to hold information about an image. */

struct characteristicStruct
{
	char *name;
	struct valueStruct *value;
	struct characteristicStruct *next;
};


struct	functionTableElementStruct
{
	char		*name;
	struct	imageStruct	*((*function)(struct argumentStruct *argumentList));
	struct	functionTableElementStruct	*next;
};


struct declarationStruct
{
	int type;
	char *name;
	struct declarationStruct *next;
};

static int  temporaryVariableCount = 0;


/* Although we will not be storing images, we will be manipulating image
	lists and their characteristics. The variable imageDB will serve as
	the global image database for the sample implementation. */

struct imageDBStruct
{
	struct imageStruct image;
	struct imageDBStruct *next;
};

union valueUnion
{
	int		ival;
	double	dval;
	char		*sval;
};
