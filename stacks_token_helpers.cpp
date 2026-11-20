/*
 * Mathematical Expression Evaluator
 * ---------------------------------
 * Evaluates expressions such as:  3 + 4 * (2 - 1) ^ 2
 *
 * Steps:
 *   1. Tokenization          : text    -> tokens
 *   2. Infix to Postfix      : tokens  -> postfix tokens (Shunting-yard)
 *   3. Postfix Evaluation    : postfix -> final result
 *
 * Supports: + - * / ^  (with precedence and associativity), parentheses,
 *           decimals, unary minus, and error handling.
 * Data structure: Stack implemented with an array.
 * Only the <iostream> library is used.
 * Note: the exponent in '^' must be a whole number (e.g. 2 ^ 10, 2 ^ -1).
 */

#include <iostream>

using namespace std;

const int MAX_SIZE = 256;
const double LIMIT = 1e300;   // used to detect overflow

// ---------------------------------------------------------------
// Stack implemented using an array
// ---------------------------------------------------------------
template <typename T>
class Stack {
private:
    T data[MAX_SIZE];
    int topIndex;

public:
    Stack() : topIndex(-1) {}

    bool isEmpty() const { return topIndex == -1; }
    bool isFull() const { return topIndex == MAX_SIZE - 1; }
    int size() const { return topIndex + 1; }

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

// ---------------------------------------------------------------
// Token definition
// ---------------------------------------------------------------
enum TokenType { NUMBER, OPERATOR, LEFT_PAREN, RIGHT_PAREN };

struct Token {
    TokenType type;
    double value;   // used when type == NUMBER
    char op;        // used when type == OPERATOR ('~' means unary minus)
};

// ---------------------------------------------------------------
// Small helper functions (replace <cctype> and <cmath>)
// ---------------------------------------------------------------
bool isDigit(char c) { return c >= '0' && c <= '9'; }

bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }

bool equals(const char* a, const char* b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) return false;
        i++;
    }
    return a[i] == b[i];
}

double power(double a, double b) {
    if (b > 1000 || b < -1000)
        throw "Exponent too large";

    long long n = (long long)b;
    if ((double)n != b)
        throw "Exponent must be a whole number";

    bool negative = false;
    if (n < 0) {
        negative = true;
        n = -n;
    }

    double result = 1;
    for (long long i = 0; i < n; i++) {
        result *= a;
        if (result > LIMIT || result < -LIMIT)
            throw "Result too large";
    }

    if (negative) {
        if (result == 0) throw "Division by zero";
        result = 1 / result;
    }
    return result;
}

// ---------------------------------------------------------------
// Operator helpers
// ---------------------------------------------------------------
int precedence(char op) {
    switch (op) {
        case '+':
        case '-': return 1;
        case '*':
        case '/': return 2;
        case '~': return 3;   // unary minus
        case '^': return 4;
        default:  return 0;
    }
}

bool isRightAssociative(char op) {
    return op == '^' || op == '~';
}