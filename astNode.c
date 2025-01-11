#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <float.h>
#include "astNode.h"

struct astProgram *program = NULL;
static astProgram *head=NULL;
struct variableList *start=NULL;
struct FunctionNode* functionList=NULL;

#define MAX_SCOPE_DEPTH 100
#define MAX_VARIABLES 100


ScopeVariableList scopeStack[MAX_SCOPE_DEPTH];
int currentScope = -1;

void clearAllVariables() {
   
    struct variableList* current = start;
    while (current != NULL) {
        struct variableList* temp = current;
        current = current->next;
        
        
        if (temp->node->variable_name != NULL) {
            free(temp->node->variable_name);
        }
        
        
        free(temp->node);
        
        // Eliberează nodul din lista de variabile
        free(temp);
    }
    
    // Resetează pointer-ul de start
    start = NULL;
    
    // Curăță variabilele din toate scope-urile
    for (int i = 0; i <= currentScope; i++) {
        ScopeVariableList* scopeCurrent = &scopeStack[i];
        ScopeVariableList* temp = scopeCurrent->next;
        
        while (temp != NULL) {
            ScopeVariableList* nextTemp = temp->next;
            
            // Nu eliberăm node-ul aici deoarece a fost deja eliberat mai sus
            // în lista globală de variabile
            free(temp);
            
            temp = nextTemp;
        }
        
     
        scopeStack[i].node = NULL;
        scopeStack[i].next = NULL;
        scopeStack[i].count = 0;
    }
    
   // printf("Toate variabilele au fost sterse cu succes!\n");
}

void insertValuesToParameters(struct astNode* parameters, struct astNode* arguments) {
    struct astNode* currentParam = parameters;
    struct astNode* currentArg = arguments;
    
    while (currentParam != NULL && currentArg != NULL) {
        struct astNode* paramNode;
        if (currentParam->type == NODE_SEQUENCE) {
            paramNode = currentParam->left;
            currentParam = currentParam->right;
        } else {
            paramNode = currentParam;
            currentParam = NULL;
        }

        struct astNode* argNode;
        if (currentArg->type == NODE_SEQUENCE) {
            argNode = currentArg->left;
            currentArg = currentArg->right;
        } else {
            argNode = currentArg;
            currentArg = NULL;
        }

        struct astNode* newNode = (struct astNode*)malloc(sizeof(struct astNode));
        newNode->type = paramNode->type;
        newNode->variable_name = strdup(paramNode->variable_name);

        double argValue = evaluate(argNode);
        if (newNode->type == NODE_INT_DECLARATION) {
            newNode->value.ival = (int)argValue;
        } else {
            newNode->value.dval = argValue;
        }
        
    }
}

struct astNode* createFunctionNode(const char* name, struct astNode* parameters, struct astNode* body) {
    struct astNode* node = (struct astNode*)malloc(sizeof(struct astNode));
    node->type = NODE_FUNCTION_DEF;
    node->variable_name = strdup(name);
    node->left = parameters; 
    node->right = body;      
    if (functionList == NULL) {
      //  printf("Se adauga functie noua!\n ");
        functionList = (FunctionNode*)malloc(sizeof(FunctionNode));
        functionList->node=node;
    } else {
        FunctionNode * temp = functionList;
        while(temp->next!=NULL)
        {
            temp=temp->next;
        }
        temp->next =(FunctionNode*)malloc(sizeof(FunctionNode));
        temp->node=node;
    } 
    return node;
}

struct astNode* createParameterNode(const char* name, NodeType type) {
    struct astNode* node = (struct astNode*)malloc(sizeof(struct astNode));
    node->type = type;
    node->variable_name = strdup(name);
    node->left = NULL;
    node->right = NULL;
    addNodeToScopeVariableList(node);
    return node;
}

struct astNode* createParameterListNode(struct astNode* param, struct astNode* next) {
    if (next == NULL) {
        return param;
    }
    
    struct astNode* node = (struct astNode*)malloc(sizeof(struct astNode));
    node->type = NODE_SEQUENCE;
    node->left = param;
    node->right = next;
    return node;
}

struct astNode* createFunctionCallNode(const char* name, struct astNode* arguments) {
    struct astNode* node = (struct astNode*)malloc(sizeof(struct astNode));
    node->type = NODE_FUNCTION_CALL;
    node->variable_name = strdup(name);
    node->left = arguments; 
    node->right = NULL;
 //   printf("S-a creat un apel de functie!\n");
    return node;
}

struct astNode* createArgumentListNode(struct astNode* arg, struct astNode* next) {
    if (next == NULL) {
     //   printf("S-a adaugat nodul %d\n",arg->type);
        return arg;
    }
   // printf("S-a adaugat nodul %d\n",next->type);
    struct astNode* node = (struct astNode*)malloc(sizeof(struct astNode));
    node->type = NODE_SEQUENCE;
    node->left = arg;
    node->right = next;
    return node;
}

struct astNode* createReturnNode(struct astNode* value) {
    struct astNode* node = (struct astNode*)malloc(sizeof(struct astNode));
    node->type = NODE_RETURN;
    node->left = value;
    node->right = NULL;
    return node;
}

struct astNode* findFunction(const char* name) {
    FunctionNode* current = functionList;
    while (current != NULL) {
        if (strcmp(current->node->variable_name, name) == 0) {
          // printf("Functie gasita! %s\n",name);
            return current->node;
        }

        current = current->next;
    }
    return NULL;
}

void initializeGlobalScope() {
    currentScope = -1;  
    pushScope();      
}

void pushScope()
{
    if (currentScope + 1 >= MAX_SCOPE_DEPTH)
    {
        printf("Eroare: Stiva de scopuri prea adanca!\n");
        exit(1);
    }

    currentScope++;
 //   printf("PUSH SCOPE: %d \n", currentScope);
    scopeStack[currentScope].count = 0;
    scopeStack[currentScope].next = NULL;
    scopeStack[currentScope].node = NULL; 
}

void popScope() 
{
    if (currentScope < 0) 
    {
        printf("Eroare: Nu exista scopuri de eliminat!\n");
        exit(1);
    }
    currentScope--;
}


void testINT(double value)
{
    if ( value> INT_MAX || value < INT_MIN) {
       yyerror("int limit exceeded\n");
        exit(EXIT_FAILURE);
    }

}

void testDOUBLE(double value)
{
    if ( value > DBL_MAX || value < -DBL_MIN) {
       yyerror("double limit exceeded\n");
        exit(EXIT_FAILURE);
    }

}


int isInitialized(const char* name) {
    astNode* var = getVariableNode(name);
    if (var) {
        return var->initialized;
    }
    printf("Eroare: Variabila %s nu este definită.\n", name);
    exit(1);
}

double evaluate(astNode* node)
{
       if (node == NULL) {
        yyerror("Nodul este NULL.");
        return 0;
       }

        switch (node->type) {
            case NODE_ASSIGNMENT:
            {
                if(node->left->type==NODE_INT_DECLARATION)
                {
               //    printf("ASIGNARE %d \n",(int)evaluate(node->right));
                   testINT(evaluate(node->right));
                
                    node->left->value.ival=(int)evaluate(node->right);
                 //   printf("%d", node->left->value.ival);
                }
                else
                   { 
                        testDOUBLE(evaluate(node->right));
                        node->left->value.dval=evaluate(node->right);
                   }
                break;
            }

            case NODE_EQUAL:
            {
                if(evaluate(node->left)==evaluate(node->right))
                    return 1;
                return 0;
            }

            case NODE_GREATER:
            {
                double left=evaluate(node->left);
                double right=evaluate(node->right);
             //   printf("Comparare: %f > %f\n", left, right);
                if(left > right)
                    return 1;
                return 0;
            }

            case NODE_LESS:
            {
                if(evaluate(node->left)<evaluate(node->right))
                    return 1;
                return 0;
            }

            case NODE_NOT_EQUAL:
            {
                if(evaluate(node->left)!=evaluate(node->right))
                    return 1;
                return 0;
            }
            
            case NODE_ADD:
            {
               // printf("adunare\n");
                return evaluate(node->left) + evaluate(node->right);
            }

            case NODE_DIV:
            {
                if(evaluate(node->right)!=0)
                    return evaluate(node->left) / evaluate(node->right);
                yyerror("can't divide by 0.\n");
            }

            case NODE_MUL:
            {
                return evaluate(node->left) * evaluate(node->right);
            }

            case NODE_SUB:
            {
                return evaluate(node->left) - evaluate(node->right);
            }

            case NODE_CONSTANT_INT:
            {
                return node->value.ival;
            }

            case NODE_CONSTANT_DOUBLE:
            {
                return node->value.dval;
            }

            case NODE_INT_DECLARATION:
            {
                if(isInitialized(node->variable_name))
                    return node->value.ival;
                if(node->left!=NULL)
                {
                    node->value.ival=(int)evaluate(node->left);
                    node->initialized=1;
                    return node->value.ival;
                }
                if(node->right!=NULL)
                {
                    evaluate(node->right);
                    return node->value.ival;
                }
              return node->value.ival;
            }

            case NODE_DOUBLE_DECLARATION:
            {
                if(isInitialized(node->variable_name))
                    return node->value.dval;
                if(node->left!=NULL)
                {
                    node->value.dval=evaluate(node->left);
                     node->initialized=1;
                    return node->value.dval;
                }
                 if(node->right!=NULL)
                {
                    evaluate(node->right);
                    return node->value.dval;
                }
                  return node->value.dval;
            }

            case NODE_INPUT:
            {
                //printf("%d \n",NODE_INT_DECLARATION);
                FILE *original_stdin = stdin;
                FILE *tty = fopen("/dev/tty", "r");
                if (tty == NULL) {
                    fprintf(stderr, "Eroare: Nu se poate accesa terminalul pentru input interactiv.\n");
                    exit(1);
                }
               
                stdin = tty;
                printf("Introdu o valoare pt %s: ", node->left->variable_name);
                if(node->left->type==NODE_INT_DECLARATION)
                {
                   // printf("aici e citirea\n");
                    int value;
                    scanf("%d",&value);
                    testINT(value);
                    node->left->value.ival=value;
                }
                else
                   {
                        double value;
                        scanf("%lf",&value);
                        testDOUBLE(value);
                        node->left->value.dval=value;
                   }
                stdin = original_stdin;
                fclose(tty);
                break;
            }

            case NODE_OUTPUT:
            {
              if(node->left->type==NODE_INT_DECLARATION)
                {
                  printf("%s = %d\n",node->left->variable_name, node->left->value.ival);
                }
                else
                    printf("%s = %lf\n", node->left->variable_name, node->left->value.dval);

                break;
            }

            case NODE_INT_CAST:
            {
               // printf("NODE_INT_CAST %d\n",(int)evaluate(node->right));
                return (int)evaluate(node->right);
            }

            case NODE_DOUBLE_CAST:
            {
                return evaluate(node->right);
            }

            case NODE_IF:
            {
                if (evaluate(node->left)!=0)
                {
                   // printf("se executa if-statement\n");
                    evaluate(node->right);  
                } else if (node->elseStatement!=NULL) 
                    {
                      //   printf("se executa else-statement\n");
                        evaluate(node->elseStatement);  
                    }
                return 0;
            }

            case NODE_WHILE:
            {
                while (evaluate(node->left)!=0)
                {
                  //  printf("IN BUCLA");
                    evaluate(node->right);
                }
            break;
            }
            case NODE_SEQUENCE:
            {
                evaluate(node->left);  
                evaluate(node->right);  
                break;
            }
            case NODE_FUNCTION_CALL:
            {
               // printf("NODE_FUNCTION_CALL\n");
                astNode* func = findFunction(node->variable_name);
                if (func == NULL) {
                    printf("Eroare: Functia %s nu este definita\n", node->variable_name);
                    exit(1);
                }
  
                pushScope();
                
              // addParametersToScope(node->left);
                insertValuesToParameters(func->left, node->left);
                
                double result = evaluate(func->right);
                
                popScope();
                return result;
            }
        
            case NODE_RETURN:
                return node->left ? evaluate(node->left) : 0;

            default:
                 break;
        }
        return 0;
}

void addParametersToScope(struct astNode* parameters) {
    struct astNode* current = parameters;
    while (current != NULL) {
        if (current->type == NODE_SEQUENCE) {
            if (current->left != NULL) {
                addNodeToScopeVariableList(current->left);
            }
            current = current->right;
        } else {
            addNodeToScopeVariableList(current);
            break;
        }
    }
}

astNode* createSequenceNode(struct astNode* first, struct astNode* second) {
    struct astNode* node = malloc(sizeof(struct astNode));
    if (node == NULL) {
        printf("Eroare: Nu s-a putut aloca memorie pentru nod sequence\n");
        return NULL;
    }
    node->type = NODE_SEQUENCE;
    node->left = first;
    node->right = second;
    return node;
}
astNode* createWhileNode(astNode* condition, astNode* statement) {
    struct astNode* node = (astNode*)malloc(sizeof(astNode));
    if (!node) {
        yyerror("Eroare la alocarea memoriei");
        return NULL;
    }
    node->type = NODE_WHILE;
    node->left = condition;
    node->right = statement;
    return node;
}

astNode* createIfNode(astNode* condition, astNode* ifStatement, astNode* elseStatement) {
    struct astNode* node = (astNode*)malloc(sizeof(astNode));
    if (!node) {
        yyerror("Eroare la alocarea memoriei");
        return NULL;
    }
    node->type = NODE_IF;
    node->left = condition;
    node->right = ifStatement;
    node->elseStatement = elseStatement;
    addNodeToProgram(node);
    return node;
}


astNode *createCastNode(astNode *right, NodeType type)
{
    struct astNode* Node = (astNode*)malloc(sizeof(astNode));
    if (!Node) {
       yyerror("Eroare la alocarea memoriei");
       return NULL;
    }
    Node->type=type;
    Node->left=NULL;
    Node->right=right;
    addNodeToProgram(Node);
    return Node;
}

astNode* createOutputNode(astNode* node)
{
    struct astNode* Node = (astNode*)malloc(sizeof(astNode));
    if (!Node) {
       yyerror("Eroare la alocarea memoriei");
       return NULL;
    }
    Node->type=NODE_OUTPUT;
    Node->left=node;
    Node->right=NULL;
    addNodeToProgram(Node);
    return Node;
}

astNode* createInputNode(astNode* node)
{
    struct astNode* Node = (astNode*)malloc(sizeof(astNode));
    if (!Node) {
       yyerror("Eroare la alocarea memoriei");
       return NULL;
    }
    //printf("%s %d\n", node->variable_name,node->value.ival);
    Node->type=NODE_INPUT;
    Node->left=node;
    Node->right=NULL;
    addNodeToProgram(Node);
    return Node;
}

astNode* createAssignNode(astNode *left,astNode* right)
{
    struct astNode* node = (astNode*)malloc(sizeof(astNode));
    if (!node) {
       yyerror("Eroare la alocarea memoriei");
       return NULL;
    }
   // printf("ASSIGNMENT NODE\n");
    node->type=NODE_ASSIGNMENT;
    node->left=left;
    node->right=right;
    left->right=node;
    left->initialized=1;
    addNodeToProgram(node);
    return node;
}

void addNodeToProgram(astNode *node)
{
    if (program == NULL) {
                program = (astProgram*)malloc(sizeof(astProgram));
                program->node = node;
                program->next = NULL;
                head = program;
            } 
            else 
            {
                astProgram *temp = program;
                while (temp->next != NULL) {
                    temp = temp->next;
                }
                temp->next = (astProgram*)malloc(sizeof(astProgram));
                temp->next->node = node;
                temp->next->next = NULL;
            }
}

void addNodeToVariableList(astNode *node)
{
   // printf("addNodeToList\n");
    if (start == NULL) {
                start = (variableList*)malloc(sizeof(variableList));
                start->node = node;
                start->next = NULL;
            } 
            else 
            {
                variableList *temp = start;
                while (temp->next != NULL) {
                    if (strcmp(temp->node->variable_name, node->variable_name) == 0) {
                       printf("Eroare: Variabila %s a fost deja declarata!\n", node->variable_name);
                       exit(1);
                    }
                    temp = temp->next;
                }
                temp->next = (variableList*)malloc(sizeof(variableList));
                temp->next->node= node;
            }
}

void addNodeToScopeVariableList(astNode * node) {
    if (currentScope < 0) {
        printf("Eroare: Adaugare simbol in afara unui scop!\n");
        exit(1);
    }
    ScopeVariableList* current = &scopeStack[currentScope];
    ScopeVariableList *temp = current;
    if(temp->node==NULL)
    {
        temp->node=(astNode*)malloc(sizeof(astNode));
        temp->node=node;
       // printf("Varibiala %s a fost adaugata in scopul %d\n",   temp->node->variable_name,currentScope);
        return;
    }
    while (temp->next != NULL) 
    {
        if (strcmp(temp->node->variable_name, node->variable_name) == 0)
        {
            printf("Eroare: Variabila %s a fost deja declarata!\n", node->variable_name);
            exit(1);
        }
        temp = temp->next;
    }
    temp->next = (ScopeVariableList *)malloc(sizeof(ScopeVariableList));
    temp->next->node= node;
    temp->next->next=NULL;
   // printf("Varibiala %s a fost adaugata in scopul %d\n",  node->variable_name,currentScope);
}

astNode* createOperationNode(NodeType type, astNode* left, astNode * right)
{
    struct astNode* node = (astNode*)malloc(sizeof(astNode));
    if (!node) {
       yyerror("Eroare la alocarea memoriei");
       return NULL;
    }
    node->type=type;
    node->left=left;
    node->right=right;
    //addNodeToProgram(node);
    return node;
}

int isDeclared(const char* name) 
{
    if(currentScope<0)
    {
        return 1;
    }
    ScopeVariableList * current= &scopeStack[currentScope];
    ScopeVariableList *temp = current;
   // printf("intrare in isDeclared\n");
    while (temp != NULL) 
    {   
       // printf("Nume variabila: %s\n",temp->node->variable_name);
        if (strcmp(temp->node->variable_name,name) == 0)
            {  
              //  printf("intrare in isDeclared\n");
                return 1;
            }

        temp = temp->next;
    }
    current=&scopeStack[0];
    temp = current;
    while (temp != NULL) 
    {   
       // printf("Nume variabila: %s\n",temp->node->variable_name);
        if (strcmp(temp->node->variable_name,name) == 0)
            {  
                return 1;
            }

        temp = temp->next;
    }
    return 0;
}


astNode* createConstantNode(NodeType type, const double value )
{
    struct astNode* node = (astNode*)malloc(sizeof(astNode));
    if (!node) {
       yyerror("Eroare la alocarea memoriei");
       return NULL;
    }
    node->type = type;
    node->left = NULL;
    node->right = NULL;
    if(type == NODE_CONSTANT_INT)
    {
        node->value.ival=(int)value;
    }
    else
        node->value.dval=value;
  //  printf("S-a creat un NODE_CONSTANT\n");
    return node;
}

astNode* getVariableNode(const char *variable_name)
{
   if (start == NULL) {
    ScopeVariableList * current=&scopeStack[0];
        ScopeVariableList * temp2 = current;
        while (temp2 != NULL) 
        {  
            if (strcmp(temp2->node->variable_name,variable_name) == 0)
                {  
                    return temp2->node ;
                }

            temp2 = temp2->next;
        }
        char error_message[256]; 
        sprintf(error_message, "%s not declared, but used!\n", variable_name);
        yyerror(error_message); 
   } 
    else 
   {
        variableList *temp = start;
        while (temp!= NULL) {
            if(strcmp(temp->node->variable_name,variable_name)==0)
               { 
            //    printf("variabila gasita! %f \n", temp->node->value.dval);
                return temp->node;
               }
            temp=temp->next;
        }
        ScopeVariableList * current=&scopeStack[0];
        ScopeVariableList * temp2 = current;
        while (temp2 != NULL) 
        {  
            if (strcmp(temp2->node->variable_name,variable_name) == 0)
                {  
                    //printf("intrare in isDeclared\n");
                    return temp2->node ;
                }

            temp2 = temp2->next;
        }
        char error_message[256]; 
        sprintf(error_message, "%s not declared, but used!\n", variable_name);
        yyerror(error_message); 

   }
    return NULL;
}

astNode* createINTDeclarationNode(const char *Variable_Name, astNode* value) {

        struct astNode* node = (astNode*)malloc(sizeof(astNode));
        if (!node) {
            perror("Eroare la alocarea memoriei");
            return NULL;
        }
        
        node->initialized=0;
        node->left = value;
        node->right = NULL;
        node->type = NODE_INT_DECLARATION;
        node->variable_name = strdup(Variable_Name);

        addNodeToVariableList(node);
        if(currentScope>=0)
            addNodeToScopeVariableList(node);
      //  printf("Variabila %s a fost declarata cu succes!\n", Variable_Name);
        return node;
}

astNode* createDOUBLEDeclarationNode(const char *Variable_Name,astNode* value) {
    struct astNode* node = (astNode*)malloc(sizeof(astNode));
    if (!node) {
        perror("Eroare la alocarea memoriei");
        return NULL;
    }
    node->initialized=0;
    node->left = value;
    node->right = NULL;
    node->type = NODE_DOUBLE_DECLARATION;
    node->variable_name = strdup(Variable_Name);

    addNodeToVariableList(node);
    if(currentScope>=0)
        addNodeToScopeVariableList(node);
 //   printf("Variabila %s a fost declarata cu succes!\n", Variable_Name);
    return node;
}


void printVariables()
{
    if(start==NULL)
    {
        printf("Nu exista variabile de afisat!\n");
        return;
    }
    
    while(start!=NULL)
    {
        if(start->node->type==NODE_INT_DECLARATION)
            printf("Variabila %s are valoarea %d.\n",start->node->variable_name,start->node->value.ival);
        else
            printf("Variabila %s are valoarea %.2f.\n",start->node->variable_name,start->node->value.dval);
        start=start->next;
    }
}

// int main()
// {
//     printf("START:\n");
//     initializeGlobalScope();
//     yyparse();
//     printVariables();
//     return 0;
// }

// int yyerror(const char* msg) {
//      printf("Eroare: %s\n", msg);
//      exit(1);
// }
