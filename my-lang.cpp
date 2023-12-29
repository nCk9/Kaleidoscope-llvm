#include <string>
#include <iostream>
#include <memory>
#include <vector>


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

    //CallExprAST - Expression class for funtion calls.
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


// int main(){
//     while(true){
//         int tok = gettok();
//         std :: cout << "got token = " << tok << std :: endl;
//     }
// }