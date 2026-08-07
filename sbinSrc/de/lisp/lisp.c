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
bool svEq(StringView sv1, StringView sv2) {
    if (sv1.len != sv2.len) return false;

    for (u32 i = 0; i < sv1.len; i++) {
        if (sv1.conts[i] != sv2.conts[i]) return false;
    }

    return true;
}
bool svEqLit(StringView sv, const char* cstr) {
    for (u32 i = 0; i < sv.len; i++) {
        if (sv.conts[i] != cstr[i]) return false;
    }

    return true;
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
        c == '+' || c == '*' || c == '/' || c == '%' || c == '^' || c == '&' || c == '|' || c == '~';
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
                Expr* child = *(Expr**) vectorAt(&expr->list.items, i);
                printExpr(child, indent + 1);
            }

            break;
        }
    }
}

typedef enum ValueType {
    VAL_NONE,
    VAL_INT,
    VAL_STRING
} ValueType;

typedef struct Value {
    ValueType type;

    union {
        int intVal;
        StringView strVal;
    };
} Value;

typedef struct VariableMapEntry {
    StringView name;
    Value val;
} VariableMapEntry;

typedef struct Enviroment {
    Vector variableMap;
} Enviroment;

void enviromentInit(Enviroment* env) {
    vectorInit(&env->variableMap, sizeof(VariableMapEntry));
}
void createVariable(Enviroment* env, StringView name, Value init) {
    // Check if variable already exists
    for (u32 i = 0; i < env->variableMap.size; i++) {
        VariableMapEntry* entry = vectorAt(&env->variableMap, i);
    
        if (svEq(entry->name, name)) {
            return; 
        }
    }
    VariableMapEntry* mentry = mmap(sizeof(VariableMapEntry));
    mentry->val = init;
    mentry->name = name;
    vectorPushBack(&env->variableMap, mentry);
}
void setVariable(Enviroment* env, StringView name, Value init) {
    u32 i;
    for (i = 0; i < env->variableMap.size; i++) {
        VariableMapEntry* entry = vectorAt(&env->variableMap, i);

        if (svEq(name, entry->name)) {
            break;
        }

        if (i == env->variableMap.size - 1) {
            i = -1;
            break;
        }
    }

    if (i == -1) {
        printf("Undefined variable: %s\n", name);
        for (;;) ;
    }

    VariableMapEntry* entry = vectorAt(&env->variableMap, i);
    entry->val = init;
}
Value getVariable(Enviroment* env, StringView name) {
    for (u32 i = 0; i < env->variableMap.size; i++) {
        VariableMapEntry* entry = vectorAt(&env->variableMap, i); 

        if (svEq(entry->name, name))
            return entry->val;
    }

    SV_ASLIT(name, conts, {
        printf("Undefined variable: %s\n", conts);
    });
    for (;;) ;
}

Enviroment globalScope;

Value makeInt(int x) {
    return (Value) {
        .type = VAL_INT,
        .intVal = x
    };
}

Value makeString(StringView str) {
    return (Value) {
        .type = VAL_STRING,
        .strVal = str
    };
}

Value execExpr(Expr* expr);

extern u32 screenWidth;
extern u32 screenHeight;

extern void (*putPixel)(u32 argb, u32 x, u32 y);

float fabs(float x) {
    return x < 0 ? -x : x;
}

int sinInt(int x) {
    const int PI = 3141;
    const int TWO_PI = 6283;

    // wrap angle to [-pi, pi]
    while (x > PI)
        x -= TWO_PI;

    while (x < -PI)
        x += TWO_PI;

    // parabolic approximation
    int y = x * (1000 - (x < 0 ? -x : x) * 1000 / PI) / 1000;

    // correction
    y = (225 * (y * (y < 0 ? -y : y) / 1000 - y) / 1000) + y;

    return y;
}

Value execList(Expr* expr) {
    Expr* first = *(Expr**) vectorAt(&expr->list.items, 0);

    if (first->type != EXPR_SYMBOL) {
        puts("First item is not callable\n");
        for (;;);
    }

    const StringView name = first->symbol;

    if (svEqLit(name, "+") || svEqLit(name, "-") || svEqLit(name, "*") || svEqLit(name, "/") || svEqLit(name, "%")
        || svEqLit(name, "^") || svEqLit(name, "&") || svEqLit(name, "|") || svEqLit(name, "~")) {
        Expr* a = *(Expr**) vectorAt(&expr->list.items, 1);
        Expr* b = *(Expr**) vectorAt(&expr->list.items, 2);

        Value va = execExpr(a);
        Value vb = execExpr(b);
        if (svEqLit(name, "+"))
            return makeInt(va.intVal + vb.intVal);
        else if (svEqLit(name, "-")) 
            return makeInt(va.intVal - vb.intVal);
        else if (svEqLit(name, "*")) 
            return makeInt(va.intVal * vb.intVal);
        else if (svEqLit(name, "/")) {
            if (vb.intVal == 0) return makeInt(0);
            return makeInt(va.intVal / vb.intVal);
        }
        else if (svEqLit(name, "%")) {
            if (vb.intVal == 0) return makeInt(0);
            return makeInt(va.intVal % vb.intVal);
        }
        else if (svEqLit(name, "^")) {
            return makeInt(va.intVal ^ vb.intVal);
        }
        else if (svEqLit(name, "&")) {
            return makeInt(va.intVal & vb.intVal);
        }
        else if (svEqLit(name, "|")) {
            return makeInt(va.intVal | vb.intVal);
        }
        else if (svEqLit(name, "~")) {
            return makeInt(~va.intVal);
        }
    }
    else if (svEqLit(name, "var")) {
        Expr* symb = *(Expr**) vectorAt(&expr->list.items, 1);
        Expr* initExpr = *(Expr**) vectorAt(&expr->list.items, 2);

        if (symb->type != EXPR_SYMBOL) {
            puts("Usage: (var <name> <init>)");
        }

        Value init = execExpr(initExpr);

        createVariable(&globalScope, symb->symbol, init);

        return (Value) {
            .type = VAL_NONE
        };
    }
    else if (svEqLit(name, "set")) {
        Expr* symb = *(Expr**) vectorAt(&expr->list.items, 1);
        Expr* initExpr = *(Expr**) vectorAt(&expr->list.items, 2);

        if (symb->type != EXPR_SYMBOL) {
            puts("Usage: (var <name> <init>)");
        }

        Value init = execExpr(initExpr);

        setVariable(&globalScope, symb->symbol, init);

        return (Value) {
            .type = VAL_NONE
        };
    }
    else if (svEqLit(name, "fragment")) {
        Expr* returnValue = *(Expr**) vectorAt(&expr->list.items, 1);      

        for (u32 x = 0; x < screenWidth; x++) {
            for (u32 y = 0; y < screenHeight; y++) {
                setVariable(&globalScope, svFromLit("x"), makeInt(x));
                setVariable(&globalScope, svFromLit("y"), makeInt(y));

                Value argb = execExpr(returnValue);
                if (argb.type != VAL_INT) {
                    printf("Invalid usage of a fragment shader"); 
                    for (;;) ;
                }
                putPixel(argb.intVal, x, y);
            }
        }

        return (Value) {
            .type = VAL_NONE
        };
    }

    else if (svEqLit(name, "print")) {
        Expr* arg = *(Expr**) vectorAt(&expr->list.items, 1);

        Value val = execExpr(arg);

        if (val.type == VAL_STRING) {
            SV_ASLIT(val.strVal, str, {
                printf("%s\n", str);
            });
        }

        if (val.type == VAL_INT) {
            printf("%u\n", val.intVal);
        }

        return (Value) {
            .type = VAL_NONE
        };
    }
    else if (svEqLit(name, "sin")) {
        Expr* arg = *(Expr**) vectorAt(&expr->list.items, 1);

        Value val = execExpr(arg);
        if (val.type != VAL_INT) return makeInt(0);
        return makeInt(sinInt(val.intVal));
    }

    puts("Unknown function");
    for (;;);

    return (Value) {0};
}
Value execIdent(Expr* expr) {
    return getVariable(&globalScope, expr->symbol);
}

Value execExpr(Expr* expr) {
    switch (expr->type) {

        case EXPR_INT:
            return makeInt(expr->intVal);

        case EXPR_STRING:
            return makeString(expr->strVal);

        case EXPR_SYMBOL:
            return execIdent(expr);

        case EXPR_LIST:
            return execList(expr);
    }

    return (Value) {0};
}

void lispRun(const char* code) {
    VEC(Token, tokens);

    tokenize(code, &tokens); 

    enviromentInit(&globalScope);
    createVariable(&globalScope, svFromLit("x"), makeInt(0));
    createVariable(&globalScope, svFromLit("y"), makeInt(0));

    u32 pos = 0;
    while (pos < tokens.size) {
        Expr* expr = parseExpr(&tokens, &pos);
        Value result = execExpr(expr);
    }

    vectorFree(&tokens);
}
