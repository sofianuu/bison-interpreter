%option noyywrap
%option noinput
%option nounput

%{
    #include "proiect_bison.tab.h"
    #include <stdlib.h>  
    #include <string.h>
    int lineNo = 1;
    int colNo = 1;
%}

%%

"int"           { colNo += 3; return TOK_INT; }
"float"         { colNo += 5; return TOK_DOUBLE; }
"double"        { colNo += 6; return TOK_DOUBLE; }
"printf"        { colNo += 6; return TOK_PRINTF; }
"scanf"         { colNo += 5; return TOK_SCANF; }
"if"            { colNo += 2; return TOK_IF; }
"while"         { colNo += 5; return TOK_WHILE; }
"else"          { colNo += 4; return TOK_ELSE; }
"return"        { colNo += 6; return TOK_RETURN; }
","             { colNo++; return ','; }
"("             { colNo++; return '('; }
")"             { colNo++; return ')'; }
"+"             { colNo++; return '+'; }
"-"             { colNo++; return '-'; }
"*"             { colNo++; return '*'; }
"/"             { colNo++; return '/'; }
";"             { colNo++; return ';'; }
"="             { colNo++; return '='; }
"<"             { colNo++; return '<'; }
">"             { colNo++; return '>'; }
"=="            { colNo += 2; return TOK_DOUBLE_EQUAL; }
"!="            { colNo += 2; return TOK_NOT_EQUAL; }
"{"             { colNo++; return '{'; }
"}"             { colNo++; return '}'; }

0|[1-9][0-9]*   { colNo += strlen(yytext);
                  yylval.dval = atof(yytext);
                  return INT_NUMBER; }

[a-zA-Z][a-zA-Z0-9]* { colNo += strlen(yytext);
                       yylval.sir = (char*)malloc(strlen(yytext) + 1);  
                       strcpy(yylval.sir, yytext);
                       return TOK_VARIABLE; }

[+-]?[0-9]+(\.[0-9]*)?[fF]? { colNo += strlen(yytext);
                              yylval.dval = atof(yytext);
                              return DOUBLE_NUMBER; }

"//"[^'\n']*    { colNo += strlen(yytext); }  

"/\*"[^\*]*"\*/" { colNo += strlen(yytext); }  

\r\n            { lineNo++; colNo = 1; }
\n              { lineNo++; colNo = 1; }
[ ]             { colNo++; }
\t              { colNo += 4; } // Assuming tab width of 4 spaces
.               { colNo++; return TOK_ERROR; }

%%

