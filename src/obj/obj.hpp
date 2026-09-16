#pragma once
#include <obj/handle/handle.hpp>

#include <gen/time.hpp>

static u32 lastObjId = 1;
template<typename T>
struct Object : public Handle<T> {
    // Things object should contain:
    // Proper data (EG name, timestamp, etc.)
    // Lots of debug info!
private:
#ifdef DEBUG
    const char* name;
    const char* type;
    bool isFromDrv;
    Time creationTime;
    u32 id;
#endif
public:
    Object(const char* objname, const char* itype, bool fromDrv) 
#ifdef DEBUG
    : name(heapCopyStr(objname)), type(itype), isFromDrv(fromDrv), id(lastObjId++)
#endif 
    {
        (void) objname; (void) type; (void) fromDrv;
        creationTime.fromCurrTime();
    }
    auto getPtr() -> T* {
#ifdef DEBUG
        Time currTime;
        currTime.fromCurrTime();
        Serial::writef(
            "[%u.%u.%u] %s#%u accessed (cls=%s, fromDrv=%s, creationtime=[%u.%u.%u])\n", 
            (u32) currTime.hours, (u32) currTime.mins, (u32) currTime.secs, 
            name, id, type, (isFromDrv ? "true" : "false"), (u32) creationTime.hours, (u32) creationTime.mins, (u32) creationTime.secs
        );
#endif
        return Handle<T>::getPtr();
    }
};
constexpr bool OBJ_IS_FROM_DRV = true;
constexpr bool OBJ_ISNT_FROM_DRV = false;

static auto test() -> void {
    Object<int> obj("GenFsObj", "Fs", OBJ_IS_FROM_DRV);
    int* iptr = obj.getPtr();
    *iptr = 5;
    Terminal::printf("ObjAccess=%i", *obj.getPtr());
}
