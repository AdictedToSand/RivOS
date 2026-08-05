#pragma once
#include <stddef.h>

#include <gen/err.hpp>
#include <gen/vec.hpp>

template<typename T1, typename T2>
struct MapEntry {
private:
    T1 t1;
    T2 t2;
public:
    inline auto getv() -> T2& {
        return t2;
    }
    inline auto getk() -> T1& {
        return t1;
    }

    inline auto matchesk(T1 in) -> bool {
        return in == t1;
    }
    inline auto setk(T1 n) -> void {
        t1 = n;
    }
    MapEntry(T1 k, T2 v) {
        t1 = k; t2 = v;
    }
};

template<typename KT, typename VT> 
struct Map {
private:
    Vector<MapEntry<KT, VT>> entries;

public:
    ~Map() = default;
    Map() = default;

    inline auto size() -> size_t {
        return entries.size();
    }
    
    auto operator[](KT k) -> VT& {
        int foundIndex = -1;
        for (size_t i = 0; i < entries.size(); i++) {
            MapEntry<KT, VT>& pair = entries[i];
            if (pair.matchesk(k)) {
                foundIndex = i;
                break;
            }
        }

        if (foundIndex == -1) {
            size_t indOfNewItem = entries.size();

            entries.pushBack(MapEntry(k, VT{}));
            return entries[indOfNewItem].val().getv();
        }

        return entries[foundIndex].val().getv();
    }
    auto insert(KT k, VT v) -> void {
        entries.pushBack(MapEntry(k, v));
    }

    auto rmkey(KT k) -> void {
        int foundIndex = -1;

        for (size_t i = 0; i < entries.size(); i++) {
            MapEntry<KT, VT>& pair = entries[i];
            if (pair.matchesk(k)) {
                foundIndex = i;
                break;
            }
        }

        if (foundIndex >= 0) {
            entries.eraseAt(foundIndex);
        }
    }
    auto exists(KT k) -> bool {
        for (MapEntry<KT, VT>& p : entries) {
            if (p.getk() == k) return true;
        }
        return false;
    }
    auto hasVal(VT v) -> bool {
        for (MapEntry<KT, VT>& p : entries) {
            if (p.getv() == v) return true;
        }
        return false;
    }
    auto indraw(size_t ind) -> MapEntry<KT, VT>& {
        return entries[ind];
    }

    auto begin() -> MapEntry<KT, VT>* { return entries.begin(); }
    auto end() -> MapEntry<KT, VT>* { return entries.end(); }

    auto begin() const -> const MapEntry<KT, VT>* { return entries.begin(); }
    auto end() const -> const MapEntry<KT, VT>* { return entries.end(); }

};
