//int a=10;
extern int yyparse(void);
extern int yyerror(const char* msg);

typedef enum {
    NODE_INT_DECLARATION,
    NODE_DOUBLE_DECLARATION,
    NODE_ASSIGNMENT,
    NODE_CONSTANT_INT,
    NODE_CONSTANT_DOUBLE,
    NODE_ADD,
    NODE_DIV,
    NODE_MUL,
    NODE_SUB,
    NODE_GREATER,
    NODE_LESS,
    NODE_EQUAL,
    NODE_NOT_EQUAL,
    NODE_INPUT,
    NODE_OUTPUT,
    NODE_INT_CAST,
    NODE_DOUBLE_CAST,
    NODE_WHILE,
    NODE_IF,
    NODE_SEQUENCE,
    NODE_BLOCK,
    NODE_SCOPE_BEGIN,
    NODE_SCOPE_END,
    NODE_FUNCTION_CALL,
    NODE_RETURN,
    NODE_FUNCTION_DEF

} NodeType;


typedef struct astProgram
{
    struct astProgram* next;
    struct astNode *node;
}astProgram;

typedef struct variableList
{
    struct astNode * node;
    struct variableList *next;
}variableList;

typedef struct ScopeVariableList{
    struct astNode * node;
    struct ScopeVariableList *next;
    int count;
} ScopeVariableList;

typedef struct FunctionNode
{
    struct astNode *node;
    struct FunctionNode *next;
}FunctionNode;

typedef struct astNode
{
    
    NodeType type; //tipul nodului
    char *variable_name; //numele variabilei
    union 
    {
        int ival; //valoarea intreaga
        double dval; //valoarea double
    }value;
    int initialized;
    struct astNode * left;
    struct astNode *right;
    struct astNode *elseStatement;
    struct astNode *parentScope;
}astNode;

void clearAllVariables();
int isInitialized(const char* name);
void insertValuesToParameters(struct astNode* parameters, struct astNode* arguments) ;
void addParametersToScope(struct astNode* parameters) ;
astNode* findFunction(const char* name);
astNode* createArgumentListNode(struct astNode* arg, struct astNode* next);
astNode* createParameterListNode(struct astNode* param, struct astNode* next);
astNode* createParameterNode(const char* name, NodeType type);
astNode* createFunctionNode(const char* name, struct astNode* parameters, struct astNode* body);
astNode* createReturnNode(struct astNode* value);
astNode* createFunctionCallNode(const char* name, struct astNode* arguments);
void initializeGlobalScope();
void pushScope();
void popScope();
int isDeclared(const char* name);
astNode* createSequenceNode(struct astNode* first, struct astNode* second);
astNode* createWhileNode(astNode* condition, astNode* statement);
astNode* createIfNode(astNode* condition, astNode* ifStatement, astNode* elseStatement);
astNode *createCastNode(astNode *left, NodeType type);
void testINT(double value);
void testDOUBLE(double value);
astNode* createInputNode(astNode* node);
astNode* createOutputNode(astNode* node);
astNode* createOperationNode(NodeType type, astNode* left, astNode * right);
astNode* createConstantNode(NodeType type,const double value );
astNode* getVariableNode(const char *variable_name);
astNode* createAssignNode(astNode *left,astNode* right);
astNode* createINTDeclarationNode(const char * Variable_Name,astNode * value);
astNode* createDOUBLEDeclarationNode(const char* Variable_Name,astNode * value);
void addNodeToProgram(astNode* node);
void addNodeToVariableList(astNode *node);
void addNodeToScopeVariableList(astNode *node);
double evaluate(astNode* node);

void printVariables();

