#include <string>
#include <iostream>

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

    if(lastChar == '#'){
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

int main(){
    while(true){
        int tok = gettok();
        std :: cout << "got token = " << tok << std :: endl;
    }
}