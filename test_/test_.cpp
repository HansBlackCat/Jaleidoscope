//#include "llvm/ADT/STLExtras.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <vector>

//
//  << Lexer >>
//

// ASCII Base lex

// Special Token outof range
enum Token {
    tok_eof = -1,
    // command
    tok_def = -2,
    tok_extern = -3,
    // primary
    tok_id = -4,
    tok_num = -5,
};

// tok_id
static std::string Idstr;
// tok_num
static double NumVal;

// Get token from Stdinput
static int tok_get() {
    static int LastChar = ' ';

    // pass whitespace
    while (isspace(LastChar)) LastChar = getchar();

    // check alphabetic char [a-zA-Z]*
    if (isalpha(LastChar)) { 
        Idstr = LastChar;
        while (isalnum(LastChar = getchar())) Idstr += LastChar;
        if (Idstr == "def") return tok_def;
        if (Idstr == "extern") return tok_extern;
        return tok_id;
    }

    // check digit char [0-9.]*
    if (isdigit(LastChar) || LastChar == '.') {
        std::string Numstr;
        do {
            Numstr += LastChar;
            LastChar = getchar();
        } while (isdigit(LastChar) || LastChar == '.'); 
        // extract double
        NumVal = strtod(Numstr.c_str(), 0);
        return tok_num;
    }

    // check # char 
    if (LastChar == '#') {
        do LastChar = getchar();
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
        if (LastChar != EOF) return tok_get();
    }

    // check EOF char
    if (LastChar == EOF) return tok_eof;

    int ThisChar = LastChar;
    LastChar = getchar();
    return ThisChar;
}

//
// Abstract Syntax Tree
//


namespace {

// Base class
class ExprAST {
    public:  
        virtual ~ExprAST() {}
};
// Derive for Numeric
class NumberExprAST : public ExprAST {
    double Val;
    public:
        NumberExprAST(double Val) : Val(Val) {}
};
// Derive for String
class VariableExprAST : public ExprAST {
    std::string Name;
    public:  
        VariableExprAST(const std::string name) : Name(name) {}
};
// Derive for Binary
class BinaryExprAST : public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;
    public:  
        BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
            : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};
// Derive for function call
class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
    public:  
        CallExprAST(const std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args)
            : Callee(Callee), Args(std::move(Args)) {}
};
class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;
    public:  
        PrototypeAST(const std::string &name, std::vector<std::string> Args)
            : Name(name), Args(std::move(Args)) {}
        const std::string &getName() const { return Name; }
};
class FunctionAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;
    public:  
        FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body)
            : Proto(std::move(Proto)), Body(std::move(Body)) {}
};

} // anonymous-namespace end

//
// Binary Expression Parsing
//

// Precedence
static std::map<char, int> BinopPrecedence;

static int TokGetPrecedence() {
    if (!isascii(CurTok)) return -1;
    int TokPrec = BinopPrecedence[CurTok];
    if (TokPrec <= 0) return -1;
    return TokPrec;
}


// int main() {}