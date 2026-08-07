#include <int.h>
#include <stdbool.h>
#include <stddef.h>

#include "lisp.h"

#include "../sys/sys.h"
#include "../sys/stdio.h"

#include "../gen/string.h"
#include "../gen/alpha.h"

#include "../mem/utils.h"

bool isSkippable(char c) {
    return c == ' ' || c == '\n' || c == '\t';
}

typedef struct Vector {
    u32 size;
    u32 capac;
    u32 typesize;
    void* arr;
} Vector;

#define VEC_MIN_SIZE 5

void vectorInit(Vector* vec, u32 typesize) {
    vec->typesize = typesize;
    vec->capac = VEC_MIN_SIZE;
    vec->arr = mmap(vec->capac * typesize);
    vec->size = 0;
}
void vectorPushBack(Vector* vec, void* elem) {
    logf("vec=%x arr=%x size=%u capac=%u typesize=%u\n",
        vec, vec->arr, vec->size, vec->capac, vec->typesize);

    if (vec->size >= vec->capac) {
        const u32 newCapac = vec->capac * 2;

        void* newArr = mmap(newCapac * vec->typesize);

        memcpy(newArr, vec->arr, vec->size * vec->typesize);

        munmap(vec->arr);

        vec->arr = newArr;
        vec->capac = newCapac;
    }
    memcpy((char*) vec->arr + (vec->size * vec->typesize),
        elem,
        vec->typesize);

     vec->size++;
}
void* vectorAt(Vector* vec, u32 ind) {
    if (ind >= vec->size) {
        printf("Vector OOB: index=%u size=%u\n", ind, vec->size);
        for (;;);
    }

    return (char*) vec->arr + (ind * vec->typesize);
}

void vectorFree(Vector* vec) {
    munmap(vec->arr);
}

#define VEC(T, name) \
    Vector name; \
    vectorInit(&name, sizeof(T));

typedef struct StringView {
    u32 len;
    const char* conts;
} StringView;

StringView svFromLit(const char* lit) {
    return (StringView) {
        .len = strlen(lit),
        .conts = lit,
    };
}
StringView svFromLitLen(const char* lit, u32 len) {
    return (StringView) {
        .len = len,
        .conts = lit,
    };
}

#define SV_ASLIT(sv, name, code) \
    char* name = mmap(sv.len + 1); \
    memset(name, 0, sv.len + 1); \
    for (u32 i = 0; i < sv.len; i++) { \
        name[i] = sv.conts[i]; \
    } \
    \
    code;\
    munmap(name); \
    \

typedef struct Token {
    StringView conts;
    enum {
        TT_OPENPAREN,
        TT_CLOSEPAREN,

        TT_INTLIT,
        TT_STRLIT,

        TT_IDENT,
    } type;
} Token;

Token makeToken(StringView conts, u8 type) {
    return (Token) {
        .conts = conts,
        .type = type,
    };
} 

bool isIdent(char c) {
    return isAlpha(c) || isdigit(c) || c == '_' || c == '-' ||
        c == '+' || c == '*' || c == '/' || c == '%';;
}

void tokenize(const char* src, Vector* tokenOutbuf) {
    // Idgaf abt a memory leak
    for (; *src; src++) {
        if (isSkippable(*src))
            continue;
        switch (*src) {
            case '(': {
                Token* tok = mmap(sizeof(Token));
                *tok = makeToken(svFromLitLen("(", 1), TT_OPENPAREN);
                vectorPushBack(tokenOutbuf, tok);
                break;
            }
            case ')': {
                Token* tok = mmap(sizeof(Token));
                *tok = makeToken(svFromLitLen(")", 1), TT_CLOSEPAREN);
                vectorPushBack(tokenOutbuf, tok);
                break;
            }
            case '"': {
                src++; // Skip the '"'
                u32 i = 0;
                while (src[i++] != '"') ;
                Token* tok = mmap(sizeof(Token));
                *tok = makeToken(svFromLitLen(src, i-1), TT_STRLIT);
                vectorPushBack(tokenOutbuf, tok);
                src += i-1;
                break;
            }
            default: {
                if (isdigit(*src)) {
                    u32 i = 0;
                    while (isdigit(src[i++])) ;
                    Token* tok = mmap(sizeof(Token));
                    *tok = makeToken(svFromLitLen(src, i-1), TT_INTLIT);
                    src += i-2;
                    vectorPushBack(tokenOutbuf, tok);
                    break;
                }
                if (isIdent(*src)) {
                    u32 i = 0;
                    while (isIdent(src[i++])) ;
                    Token* tok = mmap(sizeof(Token));
                    *tok = makeToken(svFromLitLen(src, i-1), TT_IDENT);
                    src += i-2;
                    vectorPushBack(tokenOutbuf, tok);
                    break;
                }
                break;
            };
        }
    }
}

const char* tokenNames[] = {
    "OPENPAREN",
    "CLOSEPAREN",
    "INTLIT",
    "STRLIT",
    "IDENT",
};

void printTok(Token* t) {
    const char* tAsS = tokenNames[t->type];

    SV_ASLIT(t->conts, conts, {
        printf("Token {Type=%s,Conts=%s}\n", tAsS, conts);
    });
}
void printTs(Vector* ts) {
    for (u32 i = 0; i < ts->size; i++) {
        Token* t = vectorAt(ts, i); 
        printTok(t);
    }
}


typedef enum ExprType {
    EXPR_INT,
    EXPR_STRING,
    EXPR_SYMBOL,
    EXPR_LIST
} ExprType;

typedef struct Expr {
    ExprType type;

    union {
        int intVal;
        StringView strVal;
        StringView symbol;

        struct {
            Vector items;
        } list;
    };
} Expr;

Expr* newExpr(ExprType t) {
    Expr* const ret = mmap(sizeof(Expr));
    ret->type = t;
    return ret;
} 

Expr* parseExpr(Vector* tokens, u32* pos);
Expr* parseList(Vector* tokens, u32* pos) {
    (*pos)++; // skip '('

    Expr* list = newExpr(EXPR_LIST);
    vectorInit(&list->list.items, sizeof(Expr*));

    while (1) {
        Token* tok = vectorAt(tokens, *pos);

        if (tok->type == TT_CLOSEPAREN) {
            (*pos)++; // skip ')'
            break;
        }

        Expr* child = parseExpr(tokens, pos);
        vectorPushBack(&list->list.items, &child);
    }

    return list;
}

Expr* parseExpr(Vector* tokens, u32* pos) {
    Token* tok = vectorAt(tokens, *pos);

    switch (tok->type) {
        case TT_INTLIT: {
            (*pos)++;

            Expr* expr = newExpr(EXPR_INT);
            SV_ASLIT(tok->conts, conts, {
                expr->intVal = stoi(conts);
            })

            return expr;
        }
        case TT_STRLIT: {
            (*pos)++;

            Expr* expr = newExpr(EXPR_STRING);
            expr->strVal = tok->conts;

            return expr;
        }
        case TT_IDENT: {
            (*pos)++;

            Expr* expr = newExpr(EXPR_SYMBOL);
            expr->symbol = tok->conts;

            return expr;
        }
        case TT_OPENPAREN: {
            return parseList(tokens, pos);
        }
        default: {
            puts("Unknown symbol\n");
            printTok(tok);
            for (;;) ;
        }
    }

    return NULL;
}

void printExpr(Expr* expr, u32 indent) {
    for (u32 i = 0; i < indent; i++)
        printf("  ");

    switch (expr->type) {
        case EXPR_INT:
            printf("INT: %u\n", expr->intVal);
            break;

        case EXPR_STRING: {
            SV_ASLIT(expr->strVal, str, {
                printf("STRING: \"%s\"\n", str);
            });
            break;
        }
        case EXPR_SYMBOL: {
            SV_ASLIT(expr->symbol, sym, {
                printf("SYMBOL: %s\n", sym);
            });
            break;
        }
        case EXPR_LIST: {
            printf("LIST\n");

            for (u32 i = 0; i < expr->list.items.size; i++) {
                Expr* child = *(Expr**)vectorAt(&expr->list.items, i);
                printExpr(child, indent + 1);
            }

            break;
        }
    }
}

void lispRun(const char* code) {
    VEC(Token, tokens);

    tokenize(code, &tokens); 
    printTs(&tokens);

    printf("Token count: %u\n", tokens.size);

    u32 pos = 0;
    while (pos < tokens.size) {
        Expr* expr = parseExpr(&tokens, &pos);

        printExpr(expr, 0);
    }

    vectorFree(&tokens);
}
