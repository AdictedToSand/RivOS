#pragma once
#include <int.h>

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
    static inline FramebufferTag* fb;
public:
    static auto init(u32 mbiAddr) -> void {
        MultibootTag* tag = (MultibootTag*) (mbiAddr + 8);

        fb = nullptr;

        while (tag->type != 0) {
            if (tag->type == 8) {
                fb = (FramebufferTag*) tag;
                break;
            }

            tag = (MultibootTag*) ((u8*) tag + ((tag->size + 7) & ~7));
        }

        if (!fb)
            for (;;);

        pixels = (u32*) (u32) fb->addr;
    }
    static auto putPixel(u32 argb, u32 x, u32 y) -> void {
        pixels[y * (fb->pitch / 4) + x] = argb; 
    }
    static auto getScreenHeight() -> u32 {
        return fb->height;
    }
    static auto getScreenWidth() -> u32 {
        return fb->width;
    }
};
