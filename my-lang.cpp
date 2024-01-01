#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <map>

enum Token{
    tok_eof = -1,
    tok_def = -2,
    tok_extern = -3,
    tok_identifier = -4,
    tok_nunmber = -5
};
static std :: string identifierString; 
static double numVal;

static int gettok() {
    static int lastChar = ' ';
    while(isspace(lastChar)){  //this check is important as the user can hit console with or without spaces in the input.
        lastChar = getchar();
    } 

    if(isalpha(lastChar)){
        identifierString = lastChar;
        while(isalnum(lastChar = getchar())){ //first assignment will be done to lastChar, then the isalnum condition evaluates.
            identifierString += lastChar;
        }
        if(identifierString == "def"){
            return tok_def;
        }
        if(identifierString == "extern"){
            return tok_extern;
        }
        return tok_identifier;
    }
    //we treat the strings 99nitya and nitya99 different, something similar to how we cannot name a variable starting with some number in c/c++!!!
    //soo 99nitya would be two tokens, -5 and -4 where as nitya99 would be only -4. 
    if(isdigit(lastChar) || lastChar == '.'){
        std :: string numStr;
        do{
            numStr += lastChar;
            lastChar = getchar();
        }while(isdigit(lastChar) || lastChar == '.');
        numVal = strtod(numStr.c_str(), 0);   //strtod converts charater string to double. Also it would separate 48 from 48nck. Strange!
        return tok_nunmber;
    }

    if(lastChar == '#'){ //if its a comment, then skip the line
        do{
            lastChar = getchar();
        }while(lastChar != EOF && lastChar != '\n' && lastChar != '\r');
        if(lastChar != EOF)
            return gettok();
    }
    if(lastChar == EOF)
        return tok_eof;
    
    int thisChar = lastChar;
    lastChar = getchar();
    return thisChar; //returns the ASCII value of the charater(special). Like 124 for '|'
}

//Absract Sysntax Tree (aka Parser Tree) || AST

namespace{
    //ExpAST - Base class for all expression nodes.
    class ExprAST {
        public:
        virtual ~ExprAST() = default;
    };

    // NumberExprAST - Expression class for numeric literals like "1.0".
    class NumberExprAST : public ExprAST {
        double Val;

        public:
        NumberExprAST (double value) : Val(value){};
    };

    // Expression class for referencing a variable like "a".
    class VariableExprAST : public ExprAST {
        std :: string Name;

        public:
        VariableExprAST(const std :: string &name) : Name(name) {} //const because we're not to modify the state of the object that called this method.
    };

    // BinaryExprAST - Expression class for binary operator like "x+y".
    class BinaryExprAST : public ExprAST { 
        char Op; //the operator
        std :: unique_ptr<ExprAST> LHS, RHS;

        public:
        BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
            :Op(op), LHS(std :: move(lhs)), RHS(std :: move(rhs)) {}
    };

    //CallExprAST - Expression class for function calls.
    class CallExprAST : ExprAST {
        std :: string Callee;
        std :: vector<std :: unique_ptr<ExprAST>> Args;
        
        public:
        CallExprAST(const std :: string &callee, std :: vector<std :: unique_ptr<ExprAST>> args)
            : Callee(callee), Args(std :: move(args)) {} 

    };

    // PrototypeAST - This class represnts the "prototype" for a function,
    // which captures its name, and its argument names (thus implicitly the number of arguments the function accepts)
    class PrototypeAST : public ExprAST {
        std :: string Name;
        std :: vector<std :: string> Args;

        public:
        PrototypeAST(const std::string &name, std::vector<std::string> args)
            : Name(name), Args(std::move(args)) {}

        const std::string &getName() const {  //& before getName implies the method returns a reference of the object.  
            return Name; //const after the getName indicates that the member function does not modify the state of the object on which it is called.
            // const member variables can only call const member functions, but not the other way round i.e const member function can be called by both const and non-const member variables
        }
    };

    // FunctionAST - This class represents a function definition itself.
    class FunctionAST : public ExprAST {
        std::unique_ptr<PrototypeAST> Proto;
        std::unique_ptr<ExprAST> Body;

        public:
        FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
            :Proto(std::move(proto)), Body(std::move(body)) {}
    };
} //end of anonymous namespace

// PARSER

// CurrTok/getNextToken - Provides a simple token buffer. CurrTok is the current token the Parser is looking at.
// getNextToken reads another token from the lexer and updates CurrTok with its results.

static int CurrTok;
static int getNextToken() {
    return CurrTok = gettok();
    }

// BinopPrecedence - This holds the precedence for each binary operator that is defined.  
static std::map<char, int> BinopPrecedence;

// GetTokPecedence - Get the precedence of the pending binary operator token.
static int GetTokPecedence() {
    if(!isascii(CurrTok))
        return -1;

    // Make sure it's a declared binop
    int TokPrec = BinopPrecedence[CurrTok];
    if(TokPrec <= 0)
        return -1;
    return TokPrec;
}

// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

std :: unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
}

static std::unique_ptr<ExprAST> ParseExpression();

// numberexpr ::= number
static std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = std::make_unique<NumberExprAST>(numVal);
    getNextToken();  //consume the number
    return std::move(Result);
}

// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken(); //eat (
    auto V = ParseExpression();
    if(!V)
        return nullptr;

    if(CurrTok != ')')
        return LogError("expected ')'");
    getNextToken(); //eat )
    return V;
}

// identifierexpr
//   ::=identifier
//   ::=identifier '(' expression* ')'
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = identifierString;

    getNextToken(); //eat identifier

    if(CurrTok != '(') //simple variable ref.
        return std::make_unique<VariableExprAST> (IdName);

    //Call
    getNextToken(); //eat (
    std::vector<std::unique_ptr<ExprAST>> Args;
    if(CurrTok != ')') {
        while(true) {
            if(auto Arg = ParseExpression())
                Args.push_back(std::move(Arg));
            else
                return nullptr;

            if(CurrTok == ')')
                break;

            if(CurrTok != ',')
                return LogError("Expected ')' or ',' in argument list");
            getNextToken();
        }
    }

    // Eat the ')'
    getNextToken();
    return std::make_unique<CallExprAST>(IdName, std::move(Args))
}

// primary 
// ::=identifierexpr
// ::=numberexpr
// ::parentexpr
static std::unique_ptr<ExprAST> ParsePrimary() {
    switch(CurrTok) {
        default:
            return LogError("unknown token when expecting an expression");
        case tok_identifier:
            return ParseIdentifierExpr();
        case tok_nunmber:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
    }
}

// binoprhs
// ::=('+' primary)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS) {
// If this is a binop, find its precedence.

    while(true) {
        int TokPrec = GetTokPecedence();

        // if this is binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        if(TokPrec < ExprPrec)
            return LHS;

        // okay, we know this is a binop
        int BinOp = CurrTok;
        getNextToken(); //eat binop

        // parse the primary expression after the binary operator.
        auto RHS = ParsePrimary();
        if(!RHS)
            return nullptr;

        // if BinOp binds less tightly with RHS than then operator after RHS, let
        // let the pending operator take RHS as its LHS.
        int NextPrec = GetTokPecedence();
        if(TokenPrec < NextPrec) {
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if(!RHS)
                return nullptr;

        // Merge LHS/RHS.
        LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
        }
    }
}

// expression
// ::= primary binoprhs

static std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if(!LHS)
        return nullptr;
    return ParseBinOpRHS(0, std::move(LHS));
}

// prototype
// ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype(){
    if(CurrTok != tok_identifier)
        return LogErrorP("Expected function name in prototype");

    std::string funName = identifierString;
    getNextToken();
    
    if(CurrTok != '(')
        return LogErrorP("Expected ')' in prototype");

    std::vector<std::string> ArgNames;
    while(getNextToken() == tok_identifier)
        ArgNames.push_back(identifierString);
    if(CurrTok != ')')
        return LogErrorP("Expected ')' in prototype");

    // success
    getNextToken();

    return std::make_unique<PrototypeAST>(funName, std::move(ArgNames));
}

// definition ::='def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition(){
    getNextToken(); //eat def
    auto Proto = ParsePrototype();
    if(!Proto)
        return nullptr;

    if(auto E = ParseExpression())
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
        return nullptr;
}

// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    if(auto E = ParseExpression()) {
        // Make an anonumous proto.
        auto Proto = std::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>());

        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}

// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern(){
    getNextToken(); //eat extern.
    return ParsePrototype();
}


// Top level parsing

static void HandleDefinition(){
    if(ParseDefinition())
        fprintf(stderr, "Parsed a function definition.\n");
    else
        // skip the token error recovery.
        getNextToken();
}

static void HandleExtern() {
    if(ParseExtern()){
        fprintf(stderr, "Parsed an extern.\n");
    else
        // skip the token for error recovery.
        getNextToken();
    }
}

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if(ParseTopLevelExpr()){
        fprintf(stderr, "Parsed a top-level expr.\n");
    else
        // skip token for error recovery.
        getNextToken();
    }
}

// top ::=definition |eternal |expression| ';'
static void MainLoop(){
    while(true) {
        frintf(stderr, "ready> ");
        switch(CurrTok) {
            case tok_eof:
                return;
            case ';': //ignore top level semicolons.
                getNextToken();
                break;
            case tok_def:
                HandleDefinition();
                break;
            case tok_extern:
                HandleExtern();
                break;
            default:
                HandleTopLevelExpression();
                break;
        }
    }
}

// Main driver code

int main() {
    // Install standard binary operators.
    // 1 is lowest precedence.
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 30;
    BinopPrecedence['*'] = 40; //highest.

    // prime the first token.
    fprintf(stderr, "ready> ");
    getNextToken();

    // Run the main "interpreter loop" now.
    MainLoop();

    return 0;
}