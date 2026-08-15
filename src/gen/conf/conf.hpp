#pragma once
#include <int.h>
#include <sv.hpp>

#include <mem/alloc.hpp>

#include <gen/alpha.hpp>
#include <gen/serial.hpp>
#include <gen/map.hpp>

#include <drivers/fs/fs.hpp>
#include <drivers/fs/dev/dev.hpp>

struct Config {
    struct Value {
        union {
            i32 intVal;
            StringView strVal;
            bool boolVal;
        };
        bool isErr;
        enum class States {
            Int,
            String,
            Bool,
        } state;
    };

    bool isErr = false;
    char* src = nullptr;
    const char* o_err = nullptr;
    Map<StrOperatorEquals, Value> mapping;
    StringView currentSection;
    auto isIdent(const char c) -> bool {
        return isalpha(c) || isdigit(c) || c == '_';
    }
    auto isWhitespace(const char c) -> bool {
        return c == ' ' || c == '\t' || c == '\n';
    }
    auto parseVal(StringView val) -> Value {
        const char* cStr = val.toCStr();
        const char* const cStrBase = cStr;       
        Value ret = {};
        ret.isErr = true;
        if (strIsDigit(cStr)) {
            ret.isErr = false; 
            ret.intVal = stoi(cStr);
            ret.state = Value::States::Int;
        }
        else if (*cStr == '"') {
            cStr++;
            ret.isErr = false;
            u32 len = 0;
            while (cStr[len] != '"')
                len++;
            char* const strCopy = (char*) KernelAllocator::alloc(len + 1);
            memset(strCopy, 0, len + 1);
            for (u32 i = 0; i < len; i++)
                strCopy[i] = cStr[i];
            ret.strVal = StringView(strCopy, len);
            ret.state = Value::States::String;
        }
        else {
            u32 identLen = 0;
            while (isalpha(*cStr)) {
                identLen++;
                cStr++;
            }
            StringView ident = StringView(cStr - identLen, identLen);
            const char* identCStr = ident.toCStr();
            ret.state = Value::States::Bool;
            if (streq(identCStr, "true")) {
                ret.boolVal = true;
                ret.isErr = false;
            }
            else if (streq(identCStr, "false")) {
                ret.boolVal = false;
                ret.isErr = false;
            }
            else ret.isErr = true;

            KernelAllocator::free((void*) identCStr);
        }

        KernelAllocator::free((void*) cStrBase);
        return ret;
    }
    auto parseSrc() -> void {
        currentSection = StringView("");
        while (*src) {
            if (*src == '[') {
                u32 len = 0;
                while (src[len] != ']') len++;
                currentSection = StringView(src + 1, len - 1); // src + 1 to skip [ and len - 1 to skip ]
                const char* const sectionCStr = currentSection.toCStr();
                KernelAllocator::free((void*) sectionCStr);
                src += len + 1; // ']'
            }
            else if (*src == '#') {
                while (*src != '\n') src++;
            }
            else if (isIdent(*src)) {
                u32 identLen = 0;
                while (isIdent(*src)) {
                    identLen++;
                    src++;
                }
                StringView ident{src - identLen, identLen};
                const char* identCStr = ident.toCStr();
                
                while (isWhitespace(*src))
                    src++;
                if (*src != '=') {
                    Serial::logf("KernConfParser: at ident: '%s'", identCStr);
                    KernelAllocator::free((void*) identCStr);
                    isErr = true;
                    o_err = "Equals sign not found: expected format: name=val";
                    return;
                }
                src++;
                while (isWhitespace(*src))
                    src++;
                u32 lenUntilNewline = 0;
                while (src[lenUntilNewline] != '\n') lenUntilNewline++;
                Value init = parseVal(StringView(src, lenUntilNewline));
                if (init.isErr) {
                    isErr = true;
                    o_err = "Invalid initializer for variable";
                    KernelAllocator::free((void*) identCStr);
                    return;
                }
                src += lenUntilNewline;
                const char* const sectionAsCStr = currentSection.toCStr();
                // currentSection.len + strlen(".") + ident.len + \0
                const u32 keyLen = currentSection.len + 1 + ident.len + 1;
                char* const key = (char*) KernelAllocator::alloc(keyLen);
                memset(key, 0, keyLen);
                strcpy(key, sectionAsCStr);
                strcat(key, ".");
                strcat(key, identCStr);
                mapping.insert(key, init);

                KernelAllocator::free((void*) sectionAsCStr);
                KernelAllocator::free((void*) identCStr);
            }
            else if (isWhitespace(*src)) {
                src++;
            }
            else {
                Serial::logf("KernConfParser: Invalid character: '%c'", *src);
                isErr = true;
                o_err = "Invalid character";
                return;
            }
        }    
    }
    auto fromFile(const char* fp) -> void {
        fd_t fd = FileSystem::open(fp);
        const u32 filesize = FileSystem::fileSize(fd);
        src = (char*) KernelAllocator::alloc(filesize + 1);
        memset(src, 0, filesize + 1);
        FileSystem::read(fd, src, filesize);
        if (!fd) {isErr = true; o_err = "File not found"; }
        else isErr = false;
    }
    auto freeLeftover() {

    }
};
