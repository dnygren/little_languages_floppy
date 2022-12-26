%{
	#include <stdio.h>

	char *malloc();
	char *s;
	char *name;
	char *var;
	char *var1;
	char *var2;
	char *irf;
	char *sp;
	char *val;

	char temp[129];

	int vcount;

	vcount = 0;
%}

%union
{
	char *string;
	double rval;
	int ival;
};

%left	c_plus c_minus

%type <string> imagename characteristicname characteristicvalue imagereferent
%type <string> imagereferentelement attributename attributevalue
%type <string> imagevariablename imageexpression imageexpressionelement
%type <string> imagefunction imagevariablename imagefunctionname
%type <string> argumentlist argument imageconnector operator imagefilename

%token	k_add
%token	k_name
%token	k_index
%token	k_remove
%token	k_and
%token	k_or
%token	k_view
%token	k_read
%token	k_save
%token	k_end
%token	k_filesuffix
%token 	c_eol
%token	c_plus
%token	c_minus
%token	c_star
%token	c_slash
%token	c_comma
%token	c_question
%token	c_colon
%token	c_semicolon
%token	c_leftbracket
%token	c_rightbracket
%token	c_quote
%token	c_period
%token	c_leftparen
%token	c_rightparen
%token	c_equal
%token	<string>	v_string
%token	<rval>		v_realnumber
%token	<ival>		v_integer
%token	<string>	v_identifier

%%


imlprogram:		imlstatements endstatement
			{
puts("got an imlprogram");
				exit(0);
			};

imlstatements:		imlstatement c_semicolon
		|	imlstatement c_semicolon imlstatements;

imlstatement:		addstatement
		|	removestatement
		|	viewstatement
		|	assignmentstatement
		|	readstatement
		|	savestatement;

addstatement:		k_add imagefilename k_name 
			imagename {name = allocateString($4);}
			k_index indexlist
			{
				printf("addimage(%s,%s)\n",$2,$4);
				name = allocateString($4);
			};

imagename:		v_identifier
			{
				$$ = $1;
			};

imagefilename:		v_identifier k_filesuffix
			{
				$$ = $1;
			};
			
indexlist:		indexelement
		|	indexelement c_comma indexlist;

indexelement:		c_leftbracket characteristicname c_comma
			characteristicvalue c_rightbracket
			{
				printf("setimageindex(%s,%s,%s)\n",name,$2,$4);
			};

characteristicname:	v_identifier
			{
				$$ = $1;
			};

characteristicvalue:	v_string
			{
				$$ = $1;
			}
		| 	v_integer
			{
				sprintf(temp,"%d",yylval.ival);
				$$ = temp;
			}
		| 	v_realnumber
			{
				sprintf(temp,"%lf",yylval.rval);
				$$ = temp;
			};

removestatement:	k_remove
			{
				printf("resetimagemarks()\n");
			}
			imagereferent
			{
				printf("remove(%s)\n",$3);
			};

imagereferent:		imagereferentelement
			{
				var = getnewvariablename("s");
				printf("%s=%s\n",var,$1);
				$$ = var;
			}
		|	imagereferentelement imageconnector imagereferent
			{
				var1 = getnewvariablename("s");
				printf("%s=%s\n",var1,$1);
				var2 = getnewvariablename("s");
				printf("%s=%s\n",var2,$3);
				var = getnewvariablename("s");
				printf("%s=%simagelist(%s,%s)\n",var,$2,var1,var2);
				$$ = var;
			}
		|	c_leftparen imagereferent c_rightparen
			{
				$$ = $2;
			};

imageconnector:		k_and
			{
				$$ = allocateString("and");
			}
		|	k_or
			{
				$$ = allocateString("or");
			};

imagereferentelement:	attributename c_leftparen attributevalue c_rightparen
			{
				sprintf(temp,"selectimage(%s,%s)",$1,$3);
				irf = allocateString(temp);
				$$ = irf;
			};

attributename:		v_identifier
			{
				$$ = $1;
			};

attributevalue:		v_string
			{
				val = allocateString(yylval.string);
				$$ = val;
			}
		|	v_integer
			{
				sprintf(temp,"%d",$1);
				val = allocateString(temp);
				$$ = val;
			}
		|	v_realnumber
			{
				sprintf(temp,"%lf",yylval.rval);
				val = allocateString(temp);
				$$ = val;
			}
		|	v_identifier
			{
				val = allocateString(yylval.string);
				$$ = val;
			};

viewstatement:		k_view
			{
				printf("resetimagemarks()\n");
			}
			imagereferent
			{
				printf("view(%s)\n",$3);
			};

assignmentstatement:	imagevariablename c_equal imageexpression
			{
				printf("assignimagevariable(%s,%s)\n",$1,$3);
			};

imagevariablename:	v_identifier
			{
				$$ = $1;
			};

imageexpression:	imageexpression operator imageexpression
			{
				var = getnewvariablename("i");
				printf("%s=computeimage(%s,%s,%s)\n",
					var,$2,$1,$3);
				$$ = var;
			}
		|	c_leftparen imageexpression c_rightparen
			{
				$$ = $2;
			}
		|	imageexpressionelement
			{
				$$ = $1;
			};
 
operator:		c_plus
			{
				$$ = allocateString("add");
			}
		|	c_minus
			{
				$$ = allocateString("sub");
			};

imageexpressionelement:	imagefunction
			{
				$$ = $1;
			}
		|	imagevariablename
			{
				$$ = $1;
			};

imagefunction:		imagefunctionname c_leftparen argumentlist
			c_rightparen
			{
				var = getnewvariablename("i");
				printf("%s=%s(%s)\n",var,$1,$3);
				$$ = var;
			};

imagefunctionname:	v_identifier
			{
				$$ = $1;
			};

argumentlist:		{
				sp = allocateString("");
				$$ = sp;
			}
		|	argument
			{
				$$ = $1;
			}
		|	argument c_comma argumentlist
			{
				sp = malloc(strlen($1) + strlen($3) + 2);
				strcpy(sp,$1);
				strcat(sp,",");
				strcat(sp,$3);
				$$ = sp;
			};

argument:		v_integer
			{
				sprintf(temp,"%d",$1);
				$$ = temp;
			}
		|	v_realnumber
			{
				sprintf(temp,"%lf",$1);
				$$ = temp;
			}
		|	v_string
			{
				$$ = $1;
			}
		|	imagevariablename
			{
				$$ = $1;
			};

readstatement:		k_read imagefilename imagevariablename
			{
				printf("readimage(%s,%s)\n",$2,$3);
			};

savestatement:		k_save imagevariablename imagefilename
			{
				printf("saveimage(%s,%s)\n",$2,$3);
			};

endstatement:		k_end
			{
puts("got an end statement");
			};

%%

char *getnewvariablename(prefix)

char *prefix;

{
	char *sp;
	char temp[11];
	char *allocateString();

	vcount++;

	sprintf(temp,"%s%d",prefix,vcount);

	sp = allocateString(temp);

	return(sp);
}

char *allocateString(s)

char *s;

{
	char *sp;

	sp = malloc(strlen(s)+1);
	strcpy(sp,s);
	return(sp);
}
