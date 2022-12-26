%{
	#include <stdio.h>
	#include "iml.h"
	char *malloc();
	double atof();
%}

alphachar [a-zA-Z]
numerchar [0-9]
specialchar [_]
blank [\ ]
character {alphachar}|{numerchar}|{specialchar}
identifier {alphachar}({character})*

%%

".img"		{
			return(k_filesuffix);
		}

"add"		{
			return(k_add);
		}

"name"		{
			return(k_name);
		}

"index"		{
			return(k_index);
		}

"remove"	{
			return(k_remove);
		}

"and"		{
			return(k_and);
		}

"or"		{
			return(k_or);
		}

"view"		{
			return(k_view);
		}

"read"		{
			return(k_read);
		}

"save"		{
			return(k_save);
		}

"end"		{
			return(k_end);
		}

({numerchar})*\.({numerchar})+	{
			yylval.rval = atof(yytext);
			return(v_realnumber);
		}

{numerchar}+	{
			yylval.ival = atoi(yytext);
			return(v_integer);
		}

{identifier} 	{	
			yylval.string = malloc(strlen(yytext)+1);
			strcpy(yylval.string,yytext);
			return(v_identifier);
		}

\"({character}|{blank})*\" {
			yylval.string = malloc(strlen(yytext)+1);
			strcpy(yylval.string,yytext);
			return v_string;
		}

\+		{
			return(c_plus);
		}

\-		{
			return(c_minus);
		}

\*		{
			return(c_star);
		}

\/		{
			return(c_slash);
		}

\,		{
			return(c_comma);
		}

\?		{
			return(c_question);
		}

\:		{
			return(c_colon);
		}

\;		{
			return(c_semicolon);
		}

\[		{
			return(c_leftbracket);
		}

\]		{
			return(c_rightbracket);
		}

\"		{
			return(c_quote);
		}

\.		{
			return(c_period);
		}

\(		{
			return(c_leftparen);
		}

\)		{
			return(c_rightparen);
		}

\=		{
			return(c_equal);
		}

[\n\t\ ]+	{;
		}

.		{	
			puts("this is an error");
			puts(yytext);
		}
