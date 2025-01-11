%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "astNode.h"
    extern int lineNo;
    int yylex();
%}

%union {
    int ival;           //valori intregi
    double dval;        // valori double si float
    char* sir;         // nume variabile
    struct astNode* node; // nod ast 
}

%token TOK_INT TOK_DOUBLE TOK_ERROR TOK_PRINTF TOK_SCANF TOK_IF TOK_ELSE TOK_WHILE TOK_DOUBLE_EQUAL TOK_RETURN TOK_NOT_EQUAL

%token <sir> TOK_VARIABLE
%token <dval> DOUBLE_NUMBER INT_NUMBER

%type <node> declaration expression assignment input_statement output_statement cast statement function while_statement if_statement function_definition statement_list block function_call return_statement argument_list parameter_declaration parameter_list

%start program

%left '+' '-'            
%left '*' '/'
%left TOK_DOUBLE_EQUAL TOK_NOT_EQUAL '<' '>'

%%
program:
        | program statement  { 
                                if($2!=NULL)
                                {
                                    // printf("Se evalueaza nodul\n");
                                    evaluate($2); 
                                }
                                }
        | TOK_ERROR { printf("Caracter necunoscut la linia %d.\n",lineNo);
                    YYERROR; }
        ; 
statement_list: statement { $$ = $1; }
             | statement_list statement { $$ = createSequenceNode($1, $2); }
             ;
statement: block
        | function_definition
        | function_call ';'
        | declaration ';' 
        | assignment  ';' 
        | if_statement 
        | while_statement
        | function 
        | return_statement ';'
        ;
block: '{' { pushScope(); } statement_list '}' { popScope(); $$=$3;}
     ;

function_definition:  TOK_INT TOK_VARIABLE '(' parameter_list ')' block {  // printf("Definire functie: %s\n", $2); 
                                                                            $$ = createFunctionNode($2, $4, $6); }
                   ;
parameter_list:   { $$ = NULL; }
                |  parameter_declaration { $$ = createParameterListNode($1, NULL); }
                |  parameter_list ',' parameter_declaration { $$ = createParameterListNode($3, $1); }
                ;
parameter_declaration: TOK_INT TOK_VARIABLE { $$ = createParameterNode($2, NODE_CONSTANT_INT); }
                     | TOK_DOUBLE TOK_VARIABLE { $$ = createParameterNode($2, NODE_CONSTANT_DOUBLE); }
                     ;
function_call: TOK_VARIABLE '(' argument_list ')' {  //printf("Apel funcție: %s\n", $1); 
                                                     $$ = createFunctionCallNode($1, $3); }
               ;
argument_list:   { $$ = NULL; }
             | expression { $$ = createArgumentListNode($1, NULL); }
             | argument_list ',' expression {  $$ = createArgumentListNode($3, $1); }
             ;

return_statement: TOK_RETURN expression  { // printf("Returnare valoare\n"); 
                                               $$ = createReturnNode($2); }
                 ;

if_statement: TOK_IF '(' expression ')' '{' statement_list '}' { $$ = createIfNode($3, $6, NULL);  }
            | TOK_IF '(' expression ')' '{' statement_list '}' TOK_ELSE '{' statement_list '}' { $$ = createIfNode($3, $6, $10); }
            ;
while_statement: TOK_WHILE '(' expression ')' '{' statement_list '}'  { $$ = createWhileNode($3, $6);  }
               ;
declaration: TOK_INT TOK_VARIABLE '=' expression { $$ = createINTDeclarationNode($2,$4); }
            |   TOK_DOUBLE TOK_VARIABLE '=' expression {$$ = createDOUBLEDeclarationNode($2,$4); }
            |   TOK_INT TOK_VARIABLE   { $$ = createINTDeclarationNode($2,NULL); }
            |   TOK_DOUBLE TOK_VARIABLE  { $$ = createDOUBLEDeclarationNode($2,NULL); }
            |   TOK_INT TOK_VARIABLE '=' cast { createINTDeclarationNode($2,0);
                                                      $$ = createAssignNode(getVariableNode($2), $4); 
                                                      evaluate($$);}
            |   TOK_DOUBLE TOK_VARIABLE '=' cast { createDOUBLEDeclarationNode($2,0);
                                                      $$ = createAssignNode(getVariableNode($2), $4); 
                                                      evaluate($$);}
            ;  

assignment:     TOK_VARIABLE '=' expression {  if (isDeclared($1)==0) 
                                                { 
                                                  printf("Eroare: Variabila %s nedeclarata in acest scope!\n", $1);
                                                  exit(1); 
                                                }
                                                $$ = createAssignNode(getVariableNode($1), $3);  }
               | TOK_VARIABLE '=' function_call {  if (isDeclared($1)==0) 
                                                { 
                                                  printf("Eroare: Variabila %s nedeclarata in acest scope!\n", $1);
                                                  exit(1); 
                                                }
                                                $$ = createAssignNode(getVariableNode($1), $3);  }
            ;
function:    input_statement ';' 
            | output_statement ';' {  }
            ;
cast:   '(' TOK_INT ')' expression { $$ = createCastNode($4,NODE_INT_CAST); }
        | '(' TOK_DOUBLE ')' expression { $$ = createCastNode($4,NODE_DOUBLE_CAST); }

input_statement:     TOK_SCANF '(' TOK_VARIABLE ')'  {  $$ = createInputNode(getVariableNode($3)); }                                                
                ;
output_statement:    TOK_PRINTF '(' TOK_VARIABLE ')' { if (isDeclared($3)==0) 
                                                        { 
                                                                printf("Eroare: Variabila %s nedeclarata in acest scope!\n", $3);
                                                                exit(1); 
                                                        } $$ = createOutputNode(getVariableNode($3)); }
                ;

expression:     expression '+' expression { $$ = createOperationNode(NODE_ADD,$1, $3); }
            | expression '-' expression { $$ = createOperationNode(NODE_SUB,$1, $3); }
            | expression '*' expression { $$ = createOperationNode( NODE_MUL,$1, $3); }
            | expression '/' expression { $$ = createOperationNode (NODE_DIV,$1, $3); }
            | expression TOK_DOUBLE_EQUAL expression { $$ = createOperationNode (NODE_EQUAL,$1, $3); }
            | expression TOK_NOT_EQUAL expression { $$ = createOperationNode (NODE_NOT_EQUAL,$1, $3); }
            | expression '<' expression { $$ = createOperationNode (NODE_LESS,$1, $3); }
            | expression '>' expression { $$ = createOperationNode (NODE_GREATER,$1, $3); }
            | INT_NUMBER {  testINT($1); $$ = createConstantNode(NODE_CONSTANT_INT,$1); }
            | DOUBLE_NUMBER { testDOUBLE($1); $$ = createConstantNode(NODE_CONSTANT_DOUBLE,$1); }
            | TOK_VARIABLE { if (isInitialized($1)==0) 
                             {
                                printf("Eroare: Variabila %s utilizată fără a fi inițializată.\n", $1);
                                exit(1);
                             }
                           $$ = getVariableNode($1);}
            ;

%%



