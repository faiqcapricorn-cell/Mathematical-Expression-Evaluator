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

// ---------------------------------------------------------------
// Step 1: Tokenization
// Returns the number of tokens stored in the tokens[] array.
// ---------------------------------------------------------------
int tokenize(const char* expr, Token tokens[]) {
    int count = 0;
    int i = 0;

    while (expr[i] != '\0') {
        char c = expr[i];

        // Skip whitespace
        if (isSpace(c)) {
            i++;
            continue;
        }

        if (count >= MAX_SIZE - 1)
            throw "Expression too long";

        // Numbers (integers and decimals)
        if (isDigit(c) || c == '.') {
            double value = 0;
            double scale = 1;
            int digits = 0, dots = 0;

            while (isDigit(expr[i]) || expr[i] == '.') {
                if (expr[i] == '.') {
                    dots++;
                } else {
                    digits++;
                    if (dots == 0) {
                        value = value * 10 + (expr[i] - '0');
                    } else {
                        scale /= 10;
                        value += (expr[i] - '0') * scale;
                    }
                    if (value > LIMIT)
                        throw "Number too large";
                }
                i++;
            }
            if (dots > 1) throw "Invalid number (too many decimal points)";
            if (digits == 0) throw "Invalid number";

            tokens[count].type = NUMBER;
            tokens[count].value = value;
            tokens[count].op = ' ';
            count++;
            continue;
        }

        // Is the previous token something that ends an operand?
        bool prevIsOperand = count > 0 &&
            (tokens[count - 1].type == NUMBER || tokens[count - 1].type == RIGHT_PAREN);

        if (c == '+' || c == '-') {
            if (prevIsOperand) {
                tokens[count].type = OPERATOR;      // binary + or -
                tokens[count].op = c;
                count++;
            } else if (c == '-') {
                tokens[count].type = OPERATOR;      // unary minus
                tokens[count].op = '~';
                count++;
            }
            // unary plus is simply ignored
        }
        else if (c == '*' || c == '/' || c == '^') {
            tokens[count].type = OPERATOR;
            tokens[count].op = c;
            count++;
        }
        else if (c == '(') {
            tokens[count].type = LEFT_PAREN;
            tokens[count].op = '(';
            count++;
        }
        else if (c == ')') {
            tokens[count].type = RIGHT_PAREN;
            tokens[count].op = ')';
            count++;
        }
        else {
            throw "Invalid character in expression";
        }
        i++;
    }
    return count;
}

// ---------------------------------------------------------------
// Step 2: Infix to Postfix (Shunting-yard algorithm)
// Returns the number of tokens stored in the postfix[] array.
// ---------------------------------------------------------------
int infixToPostfix(const Token tokens[], int count, Token postfix[]) {
    int outCount = 0;
    Stack<Token> ops;

    for (int i = 0; i < count; i++) {
        Token t = tokens[i];

        switch (t.type) {
            case NUMBER:
                postfix[outCount++] = t;
                break;

            case OPERATOR:
                // Unary minus is a prefix operator: just push it
                if (t.op == '~') {
                    ops.push(t);
                    break;
                }
                while (!ops.isEmpty() && ops.peek().type == OPERATOR) {
                    char top = ops.peek().op;
                    bool popIt = precedence(top) > precedence(t.op) ||
                                 (precedence(top) == precedence(t.op) &&
                                  !isRightAssociative(t.op));
                    if (!popIt) break;
                    postfix[outCount++] = ops.pop();
                }
                ops.push(t);
                break;

            case LEFT_PAREN:
                ops.push(t);
                break;

            case RIGHT_PAREN:
                while (!ops.isEmpty() && ops.peek().type != LEFT_PAREN)
                    postfix[outCount++] = ops.pop();
                if (ops.isEmpty())
                    throw "Mismatched parentheses";
                ops.pop();   // discard '('
                break;
        }
    }

    while (!ops.isEmpty()) {
        Token t = ops.pop();
        if (t.type == LEFT_PAREN)
            throw "Mismatched parentheses";
        postfix[outCount++] = t;
    }
    return outCount;
}

// ---------------------------------------------------------------
// Step 3: Postfix evaluation
// ---------------------------------------------------------------
double evaluatePostfix(const Token postfix[], int count) {
    Stack<double> st;

    for (int i = 0; i < count; i++) {
        Token t = postfix[i];

        if (t.type == NUMBER) {
            st.push(t.value);
        }
        else if (t.op == '~') {
            st.push(-st.pop());
        }
        else {
            double b = st.pop();
            double a = st.pop();
            double result = 0;

            switch (t.op) {
                case '+': result = a + b; break;
                case '-': result = a - b; break;
                case '*': result = a * b; break;
                case '/':
                    if (b == 0) throw "Division by zero";
                    result = a / b;
                    break;
                case '^':
                    result = power(a, b);
                    break;
            }
            if (result > LIMIT || result < -LIMIT)
                throw "Result too large";
            st.push(result);
        }
    }

    if (st.size() != 1)
        throw "Malformed expression";
    return st.pop();
}

// ---------------------------------------------------------------
// Helper: display the postfix form
// ---------------------------------------------------------------
void printPostfix(const Token postfix[], int count) {
    for (int i = 0; i < count; i++) {
        if (postfix[i].type == NUMBER) cout << postfix[i].value << ' ';
        else if (postfix[i].op == '~') cout << "neg ";
        else cout << postfix[i].op << ' ';
    }
    cout << endl;
}

// ---------------------------------------------------------------
// Main program
// ---------------------------------------------------------------
int main() {
    cout << "=== Mathematical Expression Evaluator ===" << endl;
    cout << "Operators: + - * / ^   Parentheses: ( )" << endl;
    cout << "Type 'exit' to quit." << endl << endl;

    cout.precision(10);

    char line[MAX_SIZE];
    Token tokens[MAX_SIZE];
    Token postfix[MAX_SIZE];

    while (true) {
        cout << "Enter expression: ";
        if (!cin.getline(line, MAX_SIZE)) {
            if (cin.eof()) break;
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Error   : Expression too long" << endl << endl;
            continue;
        }

        if (equals(line, "exit") || equals(line, "quit") || equals(line, "q")) {
            cout << "Goodbye!" << endl;
            break;
        }

        try {
            int tokenCount = tokenize(line, tokens);
            if (tokenCount == 0)
                throw "Empty expression";

            int postfixCount = infixToPostfix(tokens, tokenCount, postfix);
            double result = evaluatePostfix(postfix, postfixCount);

            cout << "Postfix : ";
            printPostfix(postfix, postfixCount);
            cout << "Result  : " << result << endl << endl;
        }
        catch (const char* message) {
            cout << "Error   : " << message << endl << endl;
        }
    }
    return 0;
}