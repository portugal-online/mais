#ifndef COLORALLOCATOR_H__
#define COLORALLOCATOR_H__

#include <inttypes.h>

typedef struct {
    uint8_t r,g,b;
} colortriplet_t;

static const colortriplet_t colors[] = {

    { 0,0,128 },
    { 0,128,0 },
    { 128,0,0 },
    { 128,128,0 },
    { 128,0,128 },
    { 0,128,128 },
};

class ColorAllocator
{
public:
    ColorAllocator(): m_index(0) {}
    QColor get() {
        QColor r = QColor(colors[m_index].r,
                          colors[m_index].g,
                          colors[m_index].b,
                          128);
        if (m_index<sizeof(colors)/sizeof(colors[1]))
            m_index++;
        return r;
    }
private:
    uint8_t m_index;
};

#endif
