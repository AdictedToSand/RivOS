#pragma once
#include <int.h>

#include <mem/alloc.hpp>
#include <mem/utils.hpp>

#include <gen/serial.hpp>

struct Visuals {
private:
    struct [[gnu::packed]] MultibootTag {
        u32 type;
        u32 size;
    };

    struct [[gnu::packed]] FramebufferTag {
        u32 type;
        u32 size;

        u64 addr;
        u32 pitch;
        u32 width;
        u32 height;
        u8 bpp;
        u8 typeAttr;
    };
    static inline u32* pixels;
    static inline u32 fbPitch, fbWidth, fbHeight;
    static inline u8 fbBpp;
    static inline u64 fbAddr;
public:
    static auto init(u32 mbiAddr) -> void {
        MultibootTag* tag = (MultibootTag*) (mbiAddr + 8);

        bool foundFb = false;
        while (tag->type != 0) {
            if (tag->type == 8) {
                const FramebufferTag* const fb = (FramebufferTag*) tag;
                foundFb = true;
                fbPitch = fb->pitch;
                fbWidth = fb->width;
                fbHeight = fb->height;
                fbBpp = fb->bpp;
                fbAddr = fb->addr;
                break;
            }

            tag = (MultibootTag*) ((u8*) tag + ((tag->size + 7) & ~7));
        }

        if (!foundFb)
            for (;;) asm volatile ("CLI; HLT");

        pixels = (u32*) (u32) fbAddr;
    }
    static inline auto putPixel(u32 argb, u32 x, u32 y) -> void {
        const u32 idx = y * (fbPitch / 4) + x;
        if (x >= fbWidth || y >= fbHeight) {
            Serial::logf("putPixel OOB: x=%u y=%u (screen %ux%u) idx=%u pixels=%x", 
                x, y, fbWidth, fbHeight, idx, (u32) pixels);
        }
        pixels[idx] = argb;
    }
    static inline auto getScreenHeight() -> u32 {
        return fbHeight;
    }
    static inline auto getScreenWidth() -> u32 {
        return fbWidth;
    }
    static inline auto getPitch() -> u32 {
        return fbPitch;
    }
    static auto fillScreen(u32 argb) -> void {
        for (u32 y = 0; y < getScreenHeight(); y++)
            for (u32 x = 0; x < getScreenWidth(); x++)
                putPixel(argb, x, y); 
    }

    static auto getFbPhysAddr() -> u32 { return (u32) fbAddr; }
    static auto getFbSizeBytes() -> u32 { return fbPitch * fbHeight; }
};

struct VisualsPidEnforced {
    static auto enforce() -> bool;

    static auto putPixel(u32 argb, u32 x, u32 y) -> bool;
    static auto getFbPhysAddr() -> u32;
    static auto getFbSizeBytes() -> u32;
    static auto getScreenHeight() -> u32;
    static auto getScreenWidth() -> u32;
    static auto getPitch() -> u32;
};
