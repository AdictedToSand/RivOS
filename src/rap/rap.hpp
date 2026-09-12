#pragma once
#include <arrv.hpp>
#include <sv.hpp>

#include <gen/map.hpp>

#include <drivers/fs/dev/dev.hpp>

#include <mem/alloc.hpp>

struct RapFile {
private:
    struct RapEntryParam {
        StringView type;
        StringView name;

        RapEntryParam(StringView itype, StringView iname) : type(itype), name(iname) {}
    };
    struct RapEntry {
        ArrayView<RapEntryParam> args; 
        void* addr;

        RapEntry(ArrayView<RapEntryParam> iargs, void* iaddr) : args(iargs), addr(iaddr) {}
    };
    Map<StrOperatorEquals, RapEntry> mainMapping;
    StringView src;
public:
    struct Parameter {
        const char* name; 
        const char* type;

        Parameter(const char* iname, const char* itype) : name(iname), type(itype) {}
    };
    enum class SucessCodes : u8 {
        Success,
        Error,
    };

    RapFile(StringView isrc) : src(isrc) {};
    RapFile(const char* isrc) : src(isrc) {};

    //TODO: Invalid file handling
    auto parseFile() -> SucessCodes {
        char* cStr = src.toCStr(); // For string operations
        const u32 rapEntriesCount = countOccurence(cStr, '\n');

        u32 rapEntriesParsed = 0;
        while (true) {
            const u32 namelen = strlenSpecChar(cStr, '(');
            StringView nameSv(cStr, namelen); 
            cStr += namelen; 

            cStr += 1; // '('
            // TODO: MemLeak
            Vector<RapEntryParam> paraml = {};

            while (*cStr != ')') {
                const char* typeStart = cStr;
                u32 paramTypeLen = 0;
                while (*cStr != ' ') {
                    cStr++;
                    paramTypeLen++;
                }
                cStr++; // skip space between type and name

                const char* nameStart = cStr;
                u32 paramNameLen = 0;
                while (*cStr != ' ') {
                    cStr++;
                    paramNameLen++;
                }
                cStr++; // skip space after name

                const StringView svType(typeStart, paramTypeLen);
                const StringView svName(nameStart, paramNameLen);
                paraml.pushBack(RapEntryParam(svType, svName));
            } 
            cStr += strlenSpecChar(cStr, ')') + 2; // Uh...
            
            const u32 distanceUntilNewline = strlenSpecChar(cStr, '\n');

            const char originalC = cStr[distanceUntilNewline];
            cStr[distanceUntilNewline] = 0;
            
            //TODO: stou
            const u32 addr = stoi(cStr);
            cStr[distanceUntilNewline] = originalC;
            cStr += distanceUntilNewline;

            mainMapping.insert(nameSv.toCStr(), RapEntry(ArrayView<RapEntryParam>(paraml.copyPermanent(), paraml.size()), (void*) addr));

            cStr++; // '\n'
            rapEntriesParsed++;
            if (rapEntriesParsed >= rapEntriesCount || !(*cStr)) {
                break;
            }
        }
        // Can't free bc of StringViews relying on it, TODO Fix
        return SucessCodes::Success;
    }
    auto dump() -> void {
        for (auto entry : mainMapping) {
            Terminal::printf("Function: %s (at %x)", entry.getk(), entry.getv().addr);
            for (auto param : entry.getv().args) {
                Terminal::printf(" Param=%s (%s)", param.name.toCStr(), param.type.toCStr());        
            }
            Terminal::putChar('\n');
        }
    }
    auto getRapEntry(const char* name, ArrayView<Parameter> params) -> void* {
        for (auto entry : mainMapping) {
            if (entry.getk() != name) continue;

            const u32 expectedParamSize = params.size();
            const u32 actualParamSize = entry.getv().args.size();

            if (expectedParamSize != actualParamSize) return nullptr;
    
            for (u32 i = 0; i < actualParamSize; i++) {
                RapEntryParam* param = entry.getv().args.at(i).val();
                Parameter* expectedParam = params.at(i).val();

                if (!param->name.eq(expectedParam->name)) {
                    return nullptr; 
                }
                if (!param->type.eq(expectedParam->type)) {
                    return nullptr;
                }
            }
            return entry.getv().addr;
        }

        return nullptr;
    }

    ~RapFile() {

    }
};

using RapParam = RapFile::Parameter;
using RapSucessCodes = RapFile::SucessCodes;
