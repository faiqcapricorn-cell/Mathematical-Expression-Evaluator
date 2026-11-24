
#ifndef STACKS_TOKEN_HELPERS_H
#define STACKS_TOKEN_HELPERS_H

#include <iostream>

using namespace std;

const int MAX_SIZE = 256;
const double LIMIT = 1e300;

// Stack implementation
template <typename T>
class Stack {
private:
    T data[MAX_SIZE];
    int topIndex;

public:
    Stack() : topIndex(-1) {}

    bool isEmpty() const {
        return topIndex == -1;
    }

    bool isFull() const {
        return topIndex == MAX_SIZE - 1;
    }

    int size() const {
        return topIndex + 1;
    }

    void push(const T& value) {
        if (isFull())
            throw "Expression too long (stack overflow)";
        data[++topIndex] = value;
    }

    T pop() {
        if (isEmpty())
            throw "Malformed expression (missing operand)";
        return data[topIndex--];
    }

    T peek() const {
        if (isEmpty())
            throw "Malformed expression (empty stack)";
        return data[topIndex];
    }
};

// Token definitions
enum TokenType {
    NUMBER,
    OPERATOR,
    LEFT_PAREN,
    RIGHT_PAREN
};

struct Token {
    TokenType type;
    double value;
    char op;
};

// Helper function declarations
bool isDigit(char c);
bool isSpace(char c);

bool equals(const char* a, const char* b);

double power(double a, double b);

int precedence(char op);

bool isRightAssociative(char op);

int tokenize(const char* expr, Token tokens[]);

#endif
