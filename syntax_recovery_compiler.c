/*
 * ======================================================================================
 * PROJECT TITLE : SYNTAX ERROR DETECTION AND RECOVERY GENERATOR IN COMPILER USING C
 * UNIVERSITY    : Rajasthan Technical University (RTU), Kota
 * SYLLABUS      : B.Tech V-Semester CSE(AI) - 5CAI4-02 (Compiler Design) & 5CAI4-22 (Lab)
 * INSTITUTION   : Jaipur Engineering College and Research Centre (JECRC), Jaipur
 * DEPARTMENT    : Computer Science & Engineering (Artificial Intelligence)
 *
 * SUBMITTED BY  :
 *   1. Rishabh Paliwal  [24EJCCA178]
 *   2. Shashwat Jain    [24EJCCA190]
 *   3. Shagun Mishra    [24EJCCA188]
 *
 * SUBMITTED TO  : Mrs. Renu Sharma (Assistant Professor)
 *
 * COMPILATION   : gcc syntax_recovery_compiler.c -o syntax_recovery_compiler -Wall
 * EXECUTION     : ./syntax_recovery_compiler
 * ======================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKENS   512
#define MAX_ERRORS   128
#define MAX_REPAIRED 2048
#define MAX_LEXEME   64

typedef enum {
    TOKEN_ID, TOKEN_NUM,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_MUL, TOKEN_DIV,
    TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_SEMI,
    TOKEN_INVALID, TOKEN_EOF
} TokenType;

typedef struct {
    int index;
    TokenType type;
    char lexeme[MAX_LEXEME];
    int line;
    int col;
} Token;

typedef struct {
    int line;
    int col;
    char strategy[64];
    char message[160];
    char suggestion[160];
} Diagnostic;

static Token tokenStream[MAX_TOKENS];
static int tokenCount = 0;
static int currentTokenIdx = 0;

static Diagnostic errorList[MAX_ERRORS];
static int errorCount = 0;
static char repairedBuffer[MAX_REPAIRED];

void parseStatement(void);
void parseExpression(void);
void parseExpressionPrime(void);
void parseTerm(void);
void parseTermPrime(void);
void parseFactor(void);

const char* getTokenTypeName(TokenType type) {
    switch (type) {
        case TOKEN_ID:      return "TOKEN_ID     ";
        case TOKEN_NUM:     return "TOKEN_NUM    ";
        case TOKEN_PLUS:    return "TOKEN_PLUS   ";
        case TOKEN_MINUS:   return "TOKEN_MINUS  ";
        case TOKEN_MUL:     return "TOKEN_MUL    ";
        case TOKEN_DIV:     return "TOKEN_DIV    ";
        case TOKEN_LPAREN:  return "TOKEN_LPAREN ";
        case TOKEN_RPAREN:  return "TOKEN_RPAREN ";
        case TOKEN_SEMI:    return "TOKEN_SEMI   ";
        case TOKEN_INVALID: return "TOKEN_INVALID";
        case TOKEN_EOF:     return "TOKEN_EOF    ";
        default:            return "UNKNOWN      ";
    }
}

void appendRepaired(const char* fragment) {
    if (strlen(repairedBuffer) + strlen(fragment) < MAX_REPAIRED - 1) {
        strcat(repairedBuffer, fragment);
    }
}

void reportSyntaxError(int line, int col, const char* strategy, const char* msg, const char* suggestion) {
    if (errorCount < MAX_ERRORS) {
        errorList[errorCount].line = line;
        errorList[errorCount].col = col;
        strncpy(errorList[errorCount].strategy, strategy, 63);
        strncpy(errorList[errorCount].message, msg, 159);
        strncpy(errorList[errorCount].suggestion, suggestion, 159);
        errorCount++;
    }
}

void lexicalAnalyze(const char* source) {
    tokenCount = 0;
    int i = 0, line = 1, col = 1;

    while (source[i] != '\0' && tokenCount < MAX_TOKENS - 1) {
        char ch = source[i];
        if (ch == '\n') { i++; line++; col = 1; continue; }
        if (ch == ' ' || ch == '\t' || ch == '\r') { i++; col++; continue; }

        int startLine = line, startCol = col;
        if (isalpha((unsigned char)ch) || ch == '_') {
            int k = 0;
            while ((isalnum((unsigned char)source[i]) || source[i] == '_') && k < MAX_LEXEME - 1) {
                tokenStream[tokenCount].lexeme[k++] = source[i++]; col++;
            }
            tokenStream[tokenCount].lexeme[k] = '\0';
            tokenStream[tokenCount].index = tokenCount;
            tokenStream[tokenCount].type = TOKEN_ID;
            tokenStream[tokenCount].line = startLine;
            tokenStream[tokenCount].col = startCol;
            tokenCount++;
            continue;
        }
        if (isdigit((unsigned char)ch)) {
            int k = 0;
            while (isdigit((unsigned char)source[i]) && k < MAX_LEXEME - 1) {
                tokenStream[tokenCount].lexeme[k++] = source[i++]; col++;
            }
            tokenStream[tokenCount].lexeme[k] = '\0';
            tokenStream[tokenCount].index = tokenCount;
            tokenStream[tokenCount].type = TOKEN_NUM;
            tokenStream[tokenCount].line = startLine;
            tokenStream[tokenCount].col = startCol;
            tokenCount++;
            continue;
        }

        tokenStream[tokenCount].index = tokenCount;
        tokenStream[tokenCount].lexeme[0] = ch;
        tokenStream[tokenCount].lexeme[1] = '\0';
        tokenStream[tokenCount].line = startLine;
        tokenStream[tokenCount].col = startCol;
        switch (ch) {
            case '+': tokenStream[tokenCount].type = TOKEN_PLUS; break;
            case '-': tokenStream[tokenCount].type = TOKEN_MINUS; break;
            case '*': tokenStream[tokenCount].type = TOKEN_MUL; break;
            case '/': tokenStream[tokenCount].type = TOKEN_DIV; break;
            case '(': tokenStream[tokenCount].type = TOKEN_LPAREN; break;
            case ')': tokenStream[tokenCount].type = TOKEN_RPAREN; break;
            case ';': tokenStream[tokenCount].type = TOKEN_SEMI; break;
            default:  tokenStream[tokenCount].type = TOKEN_INVALID; break;
        }
        i++; col++; tokenCount++;
    }
    tokenStream[tokenCount].index = tokenCount;
    tokenStream[tokenCount].type = TOKEN_EOF;
    strcpy(tokenStream[tokenCount].lexeme, "EOF");
    tokenStream[tokenCount].line = line;
    tokenStream[tokenCount].col = col;
    tokenCount++;
}

Token lookahead(void) { return tokenStream[currentTokenIdx]; }
Token consumeToken(void) {
    Token t = tokenStream[currentTokenIdx];
    if (t.type != TOKEN_EOF) currentTokenIdx++;
    return t;
}

void filterInvalidLexemes(void) {
    while (lookahead().type == TOKEN_INVALID) {
        Token bad = consumeToken();
        char msg[160], sug[160], delTag[64];
        snprintf(msg, sizeof(msg), "Unexpected invalid symbol '%s' in expression.", bad.lexeme);
        snprintf(sug, sizeof(sug), "Delete unrecognized symbol '%s' at Line %d, Col %d.", bad.lexeme, bad.line, bad.col);
        reportSyntaxError(bad.line, bad.col, "Phrase-Level Deletion", msg, sug);
        snprintf(delTag, sizeof(delTag), "/*del:%s*/", bad.lexeme);
        appendRepaired(delTag);
    }
}

void parseStatement(void) {
    filterInvalidLexemes();
    if (lookahead().type == TOKEN_EOF) return;
    parseExpression();
    filterInvalidLexemes();

    while (lookahead().type == TOKEN_RPAREN) {
        Token stray = consumeToken();
        char msg[160], sug[160];
        snprintf(msg, sizeof(msg), "Unmatched closing parenthesis ')' without matching '('.");
        snprintf(sug, sizeof(sug), "Delete extraneous ')' at Line %d, Col %d or insert '(' earlier.", stray.line, stray.col);
        reportSyntaxError(stray.line, stray.col, "Phrase-Level Deletion", msg, sug);
        appendRepaired("/*del:)*/");
        if (lookahead().type == TOKEN_MUL || lookahead().type == TOKEN_DIV) parseTermPrime();
        if (lookahead().type == TOKEN_PLUS || lookahead().type == TOKEN_MINUS) parseExpressionPrime();
        filterInvalidLexemes();
    }

    Token endTok = lookahead();
    if (endTok.type == TOKEN_SEMI) {
        consumeToken();
        appendRepaired(";\n");
    } else {
        Token prev = tokenStream[currentTokenIdx > 0 ? currentTokenIdx - 1 : 0];
        int errLine = prev.line, errCol = prev.col + (int)strlen(prev.lexeme);
        char msg[160], sug[160];
        snprintf(msg, sizeof(msg), "Missing semicolon ';' at end of statement.");
        snprintf(sug, sizeof(sug), "Insert ';' at Line %d, Column %d to terminate the statement.", errLine, errCol);
        reportSyntaxError(errLine, errCol, "Phrase-Level Insertion", msg, sug);
        appendRepaired(" /*ins*/;\n");
    }
}

void parseExpression(void) { parseTerm(); parseExpressionPrime(); }

void parseExpressionPrime(void) {
    filterInvalidLexemes();
    Token tok = lookahead();
    if (tok.type == TOKEN_PLUS || tok.type == TOKEN_MINUS) {
        consumeToken();
        appendRepaired(" "); appendRepaired(tok.lexeme); appendRepaired(" ");
        filterInvalidLexemes();
        Token next = lookahead();
        if (next.type == TOKEN_PLUS || next.type == TOKEN_MINUS ||
            next.type == TOKEN_MUL  || next.type == TOKEN_DIV   ||
            next.type == TOKEN_SEMI || next.type == TOKEN_RPAREN || next.type == TOKEN_EOF) {
            char msg[160], sug[160];
            snprintf(msg, sizeof(msg), "Missing right operand after '%s' (unexpected '%s').", tok.lexeme, next.lexeme);
            snprintf(sug, sizeof(sug), "Insert an identifier or number after '%s'.", tok.lexeme);
            reportSyntaxError(next.line, next.col, "Phrase-Level Insertion", msg, sug);
            appendRepaired("<id>");
            parseTermPrime();
            parseExpressionPrime();
        } else {
            parseTerm();
            parseExpressionPrime();
        }
    }
}

void parseTerm(void) { parseFactor(); parseTermPrime(); }

void parseTermPrime(void) {
    filterInvalidLexemes();
    Token tok = lookahead();
    if (tok.type == TOKEN_MUL || tok.type == TOKEN_DIV) {
        consumeToken();
        appendRepaired(" "); appendRepaired(tok.lexeme); appendRepaired(" ");
        filterInvalidLexemes();
        Token next = lookahead();
        if (next.type == TOKEN_PLUS || next.type == TOKEN_MINUS ||
            next.type == TOKEN_MUL  || next.type == TOKEN_DIV   ||
            next.type == TOKEN_SEMI || next.type == TOKEN_RPAREN || next.type == TOKEN_EOF) {
            char msg[160], sug[160];
            snprintf(msg, sizeof(msg), "Missing right operand after '%s' (unexpected '%s').", tok.lexeme, next.lexeme);
            snprintf(sug, sizeof(sug), "Insert an identifier, number, or '(E)' after '%s'.", tok.lexeme);
            reportSyntaxError(next.line, next.col, "Phrase-Level Insertion", msg, sug);
            appendRepaired("<id>");
            parseTermPrime();
        } else {
            parseFactor();
            parseTermPrime();
        }
    }
}

void parseFactor(void) {
    filterInvalidLexemes();
    Token tok = lookahead();
    if (tok.type == TOKEN_ID || tok.type == TOKEN_NUM) {
        consumeToken();
        appendRepaired(tok.lexeme);
    } else if (tok.type == TOKEN_LPAREN) {
        consumeToken();
        appendRepaired("(");
        parseExpression();
        filterInvalidLexemes();
        Token closeTok = lookahead();
        if (closeTok.type == TOKEN_RPAREN) {
            consumeToken();
            appendRepaired(")");
        } else {
            char msg[160], sug[160];
            snprintf(msg, sizeof(msg), "Missing closing parenthesis ')' for '(' opened at Line %d, Col %d.", tok.line, tok.col);
            snprintf(sug, sizeof(sug), "Insert ')' before '%s' at Line %d, Column %d.", closeTok.lexeme, closeTok.line, closeTok.col);
            reportSyntaxError(closeTok.line, closeTok.col, "Phrase-Level Insertion", msg, sug);
            appendRepaired("/*ins*/)");
        }
    } else {
        char msg[160], sug[160];
        snprintf(msg, sizeof(msg), "Expected operand (id, num, or '('), but found '%s'.", tok.lexeme);
        snprintf(sug, sizeof(sug), "Insert a valid variable identifier or number at Line %d, Column %d.", tok.line, tok.col);
        reportSyntaxError(tok.line, tok.col, "Phrase-Level Insertion", msg, sug);
        appendRepaired("<id>");
    }
}

int main(void) {
    const char* sample = "a + * b;";
    lexicalAnalyze(sample);
    while (lookahead().type != TOKEN_EOF) {
        int prev = currentTokenIdx;
        parseStatement();
        if (currentTokenIdx == prev && lookahead().type != TOKEN_EOF) consumeToken();
    }
    printf("Repaired Stream: %s\n", repairedBuffer);
    return 0;
}