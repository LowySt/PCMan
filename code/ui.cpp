#define RGBA(r,g,b,a)  (u32)((a<<24)|(r<<16)|(g<<8)|b)
#define RGB(r,g,b)     (u32)((0xFF<<24)|(r<<16)|(g<<8)|b)
#define RGBg(v)        (u32)((0xFF<<24)|(v<<16)|(v<<8)|v)
#define SetAlpha(v, a) (u32)((v & 0x00FFFFFF) | (((u32)a)<<24))
#define GetAlpha(v)    ( u8)(((v) & 0xFF000000) >> 24)

typedef u32 Color;

struct UIGlyph
{
    u8 *data;
    u32 size;
    
    u32 width;
    u32 height;
    
    s32 x0, x1;
    s32 y0, y1;
    
    s32 xAdv;
    s32 yAdv;
};

enum UIFontSize
{
    FS_SMALL = 0,
    FS_MEDIUM = 1,
    FS_LARGE = 2,
    FS_EXTRALARGE = 3
};

struct UIFont
{
    UIGlyph *glyph;
    s32 **kernAdvanceTable; //NOTE: Needs to keep this in the heap cause it stack-overflows.
    
    u32 pixelHeight;
    u32 maxCodepoint;
};

struct UIScissor
{
    struct UIRect { s32 x, y, w, h; };
    stack rects;
    
    UIRect *currRect;
};

struct UIContext;
typedef void(*ButtonProc)(UIContext *cxt);
struct UIButton
{
    unistring name;
    b32 isHot;
    b32 isHeld;
    
    ButtonProc onClick;
    ButtonProc onHold;
};

struct UITextBox
{
    unistring text;
    b32 isSelected;
    
    u32 dtCaret;
    b32 isCaretOn;
    s32 caretIndex;
};

struct UIListBox
{
    Array<unistring> list;
    s32 selectedIndex;
    
    u32 dtOpen;
    b32 isOpening;
    b32 isOpen;
};


enum SliderStyle { SL_LINE, SL_BOX };

struct UISlider
{
    b32 isHot;
    b32 isHeld;
    
    unistring text;
    
    s32 currValue;
    s32 maxValue;
    s32 minValue;
    
    f64 currPos;
    
    SliderStyle style;
    Color lColor;
    Color rColor;
};

struct UIContext
{
    u8 *drawBuffer;
    u32 width;
    u32 height;
    
    UIFont *fonts;
    u32 numFonts;
    
    UIFont *currFont;
    
    Color backgroundColor;
    Color highliteColor;
    Color pressedColor;
    Color widgetColor;
    Color borderColor;
    Color textColor;
    
    UIScissor scissor;
    
    void (*callbackRender)();
    u32 dt;
};

void ls_uiPushScissor(UIContext *cxt, s32 x, s32 y, s32 w, s32 h)
{
    UIScissor *scissor = &cxt->scissor;
    
    if(!scissor->rects.data) { scissor->rects = ls_stackInit(sizeof(UIScissor::UIRect), 8); }
    UIScissor::UIRect rect = { x, y, w, h };
    
    ls_stackPush(&scissor->rects, (void *)&rect);
    scissor->currRect = (UIScissor::UIRect *)ls_stackTop(&scissor->rects);
}

void ls_uiPopScissor(UIContext *cxt)
{
    UIScissor *scissor = &cxt->scissor;
    AssertMsg(scissor->rects.data, "Scissor stack is null\n");
    AssertMsg(scissor->rects.count > 0, "Scissor stack is already empty\n");
    
    ls_stackPop(&scissor->rects);
    if(scissor->rects.count > 0) //TODO: Top doesn't care if the count == 0
    {
        scissor->currRect = (UIScissor::UIRect *)ls_stackTop(&scissor->rects);
    }
    else
    {
        scissor->currRect = NULL;
    }
    
}

Color ls_uiDarkenRGB(Color c, u32 factor)
{
    u8 *c8 = (u8 *)&c;
    c8[0] -= factor;
    c8[1] -= factor;
    c8[2] -= factor;
    
    return c;
}

Color ls_uiLightenRGB(Color c, u32 factor)
{
    u8 *c8 = (u8 *)&c;
    c8[0] += factor;
    c8[1] += factor;
    c8[2] += factor;
    
    return c;
}

Color ls_uiAlphaBlend(Color source, Color dest, u8 alpha)
{
    u8 *c8 = (u8 *)&source;
    u8 bS = c8[0];
    u8 gS = c8[1];
    u8 rS = c8[2];
    u8 aS = alpha;
    
    c8 = (u8 *)&dest;
    u8 bD = c8[0];
    u8 gD = c8[1];
    u8 rD = c8[2];
    u8 aD = c8[3];
    
    f32 factor = (f32)(255 - aS) / 255.0f;
    f32 aMulti = (f32)aS / 255.0f;
    
    Color Result = 0;
    c8 = (u8 *)&Result;
    
    c8[0] = bS*aMulti + bD*factor;
    c8[1] = gS*aMulti + gD*factor;
    c8[2] = rS*aMulti + rD*factor;
    c8[3] = aS + aD*factor;
    
    return Result;
}

Color ls_uiAlphaBlend(Color source, Color dest)
{
    u8 Alpha = (u8)((source >> 24) & 0x000000FF);
    return ls_uiAlphaBlend(source, dest, Alpha);
}

Color ls_uiRGBAtoARGB(Color c)
{
    u8 *c8 = (u8 *)&c;
    
    u8 A = c8[0];
    
    c8[0] = c8[1];
    c8[1] = c8[2];
    c8[2] = c8[3];
    c8[3] = A;
    
    return c;
}

void ls_uiFillGSColorTable(Color c, Color baseColor, u8 darkenFactor, Color *table, u32 tableSize)
{
    table[0] = baseColor;
    
    u8 alphaCurr = (u32)ls_ceil((( 1.0f / (f32) tableSize ) * 255.0f));
    Color currColor = ls_uiAlphaBlend(c, baseColor, alphaCurr);
    table[1] = currColor;
    
    for(u32 i = 2; i < tableSize-1; i++)
    {
        table[i] = currColor;
        
        alphaCurr = (u32)ls_ceil((((f32)i / (f32) tableSize) * 255.0f));
        currColor = ls_uiAlphaBlend(c, baseColor, alphaCurr);
    }
    
    table[tableSize-1] = c;
    
    return;
}

void ls_uiFillRect(UIContext *cxt, s32 xPos, s32 yPos, s32 w, s32 h, Color c)
{
    UIScissor::UIRect *scRect = cxt->scissor.currRect;
    
    s32 diffWidth = (w % 4);
    s32 simdWidth = w - diffWidth;
    
    s32 diffHeight = (h % 4);
    s32 simdHeight = h - diffHeight;
    
    //TODO: Make AlphaBlending happen with SSE!!!
    
    //NOTE: Do the first Sub-Rectangle divisible by 4.
    //__m128i color = _mm_set1_epi32((int)c);
    
    for(s32 y = yPos; y < yPos+simdHeight; y++)
    {
        for(s32 x = xPos; x < xPos+simdWidth; x+=4)
        {
            if(x < 0 || x >= cxt->width)  continue;
            if(y < 0 || y >= cxt->height) continue;
            
            if(x < scRect->x || x >= scRect->x+scRect->w) continue;
            if(y < scRect->y || y >= scRect->y+scRect->h) continue;
            
            u32 idx = ((y*cxt->width) + x)*sizeof(s32);
            __m128i *At = (__m128i *)(cxt->drawBuffer + idx);
            
            Color c1 = ls_uiAlphaBlend(c, At->m128i_u32[0]);
            Color c2 = ls_uiAlphaBlend(c, At->m128i_u32[1]);
            Color c3 = ls_uiAlphaBlend(c, At->m128i_u32[2]);
            Color c4 = ls_uiAlphaBlend(c, At->m128i_u32[3]);
            
            __m128i color = _mm_setr_epi32(c1, c2, c3, c4);
            
            _mm_storeu_si128(At, color);
        }
    }
    
    //NOTE: Complete the 2 remaining Sub-Rectangles at the right and top. (if there are).
    u32 *At = (u32 *)cxt->drawBuffer;
    
    if(diffWidth) 
    {
        for(s32 y = yPos; y < yPos+h; y++)
        {
            for(s32 x = xPos+simdWidth; x < xPos+w; x++)
            {
                if(x < 0 || x >= cxt->width)  continue;
                if(y < 0 || y >= cxt->height) continue;
                
                if(x < scRect->x || x >= scRect->x+scRect->w) continue;
                if(y < scRect->y || y >= scRect->y+scRect->h) continue;
                
                Color base = At[y*cxt->width + x];
                Color blendedColor = ls_uiAlphaBlend(c, base);
                At[y*cxt->width + x] = blendedColor;
            }
        }
    }
    
    if(diffHeight)
    {
        for(s32 y = yPos+simdHeight; y < yPos+h; y++)
        {
            for(s32 x = xPos; x < xPos+w; x++)
            {
                if(x < 0 || x >= cxt->width)  continue;
                if(y < 0 || y >= cxt->height) continue;
                
                if(x < scRect->x || x >= scRect->x+scRect->w) continue;
                if(y < scRect->y || y >= scRect->y+scRect->h) continue;
                
                Color base = At[y*cxt->width + x];
                Color blendedColor = ls_uiAlphaBlend(c, base);
                At[y*cxt->width + x] = blendedColor;
            }
        }
    }
}

inline
void ls_uiBorder(UIContext *cxt, s32 xPos, s32 yPos, s32 w, s32 h)
{
    Color C = cxt->borderColor;
    
    ls_uiFillRect(cxt, xPos,     yPos,     w, 1, C);
    ls_uiFillRect(cxt, xPos,     yPos+h-1, w, 1, C);
    ls_uiFillRect(cxt, xPos,     yPos,     1, h, C);
    ls_uiFillRect(cxt, xPos+w-1, yPos,     1, h, C);
}

inline
void ls_uiBorder(UIContext *cxt, s32 xPos, s32 yPos, s32 w, s32 h, Color borderColor)
{
    Color C = borderColor;
    
    ls_uiFillRect(cxt, xPos,     yPos,     w, 1, C);
    ls_uiFillRect(cxt, xPos,     yPos+h-1, w, 1, C);
    ls_uiFillRect(cxt, xPos,     yPos,     1, h, C);
    ls_uiFillRect(cxt, xPos+w-1, yPos,     1, h, C);
}

inline
void ls_uiBorderedRect(UIContext *cxt, s32 xPos, s32 yPos, s32 w, s32 h)
{
    ls_uiBorder(cxt, xPos, yPos, w, h);
    ls_uiFillRect(cxt, xPos+1, yPos+1, w-2, h-2, cxt->widgetColor);
}

inline
void ls_uiBorderedRect(UIContext *cxt, s32 xPos, s32 yPos, s32 w, s32 h, Color widgetColor)
{
    ls_uiBorder(cxt, xPos, yPos, w, h);
    ls_uiFillRect(cxt, xPos+1, yPos+1, w-2, h-2, widgetColor);
}

inline
void ls_uiBorderedRect(UIContext *cxt, s32 xPos, s32 yPos, s32 w, s32 h, Color widgetColor, Color borderColor)
{
    ls_uiBorder(cxt, xPos, yPos, w, h, borderColor);
    ls_uiFillRect(cxt, xPos+1, yPos+1, w-2, h-2, widgetColor);
}

inline
void ls_uiRect(UIContext *cxt, s32 xPos, s32 yPos, s32 w, s32 h)
{
    ls_uiFillRect(cxt, xPos, yPos, w, h, cxt->widgetColor);
}

inline
void ls_uiRect(UIContext *cxt, s32 xPos, s32 yPos, s32 w, s32 h, Color widgetColor)
{
    ls_uiFillRect(cxt, xPos, yPos, w, h, widgetColor);
}

void ls_uiCircle(UIContext *cxt, s32 xPos, s32 yPos, s32 selRadius)
{
    s32 radSq = selRadius*selRadius;
    
    auto getBest = [radSq](s32 x, s32 y, s32 *bX, s32 *bY)
    {
        s32 bestX = 0, bestY = 0, bestSum = 0;
        
        s32 rx = x+1;
        s32 ry = y;
        s32 sum = (rx*rx) + (ry*ry);
        if(sum <= radSq) { bestX = rx; bestY = ry; bestSum = sum; }
        
        rx = x;
        ry = y-1;
        sum = (rx*rx) + (ry*ry);
        if((sum <= radSq) && sum > bestSum) { bestX = rx; bestY = ry; bestSum = sum; }
        
        rx = x+1;
        ry = y-1;
        sum = (rx*rx) + (ry*ry);
        if((sum <= radSq) && sum > bestSum) { bestX = rx; bestY = ry; bestSum = sum; }
        
        *bX = bestX;
        *bY = bestY;
    };
    
    f64 sin = 0, cos = 0;
    
    //NOTE: Everything is done relative to (0,0) until when drawing happens
    //NOTE: We start at the top middle pixel of the Circle
    s32 startX = 0;
    s32 startY = selRadius;
    
    //NOTE: And end at the first octant swept going clockwise.
    ls_sincos(PI/4.0, &sin, &cos);
    s32 endX = (cos*selRadius);
    s32 endY = (sin*selRadius);
    
    s32 currX = startX;
    s32 currY = startY;
    
    UIScissor::UIRect *scRect = cxt->scissor.currRect;
    u32 *At = (u32 *)cxt->drawBuffer;
    
    Color bCol   = cxt->borderColor;
    
    //TODO: Should be SIMDable
    b32 Running = TRUE;
    while(Running)
    {
        if((currX == endX) || (currY == endY)) { Running = FALSE; }
        
        s32 drawX1 = xPos + currX; s32 drawY1 = yPos + currY;
        s32 drawX2 = xPos - currX; s32 drawY2 = yPos + currY;
        s32 drawX3 = xPos + currY; s32 drawY3 = yPos + currX;
        s32 drawX4 = xPos - currY; s32 drawY4 = yPos + currX;
        s32 drawX5 = xPos + currX; s32 drawY5 = yPos - currY;
        s32 drawX6 = xPos - currX; s32 drawY6 = yPos - currY;
        s32 drawX7 = xPos + currY; s32 drawY7 = yPos - currX;
        s32 drawX8 = xPos - currY; s32 drawY8 = yPos - currX;
        
        
        s32 nextX = 0, nextY = 0;
        
        getBest(currX, currY, &nextX, &nextY);
        
        currX = nextX;
        currY = nextY;
        
#define CIRCLE_DRAW_BORD(xP, yP, r, c) \
if((xP) >= r->x && (xP) < r->x+r->w && (yP) >= r->y && (yP) < r->y+r->h) \
{ At[(yP)*cxt->width + (xP)] = c; }
        
        CIRCLE_DRAW_BORD(drawX1, drawY1,   scRect, bCol);
        //CIRCLE_DRAW_BORD(drawX1, drawY1-1, scRect, bCol);
        CIRCLE_DRAW_BORD(drawX2, drawY2,   scRect, bCol);
        //CIRCLE_DRAW_BORD(drawX2, drawY2-1, scRect, bCol);
        
        //ls_uiFillRect(cxt, drawX2+1, drawY2-1, (drawX1-drawX2)-1, 1, cxt->widgetColor);
        ls_uiFillRect(cxt, drawX2+1, drawY2, (drawX1-drawX2)-1, 1, cxt->widgetColor);
        
        
        CIRCLE_DRAW_BORD(drawX3,   drawY3, scRect, bCol);
        //CIRCLE_DRAW_BORD(drawX3-1, drawY3, scRect, bCol);
        CIRCLE_DRAW_BORD(drawX4,   drawY4, scRect, bCol);
        //CIRCLE_DRAW_BORD(drawX4+1, drawY4, scRect, bCol);
        
        //ls_uiFillRect(cxt, drawX4+2, drawY4, (drawX3-drawX4)-3, 1, cxt->widgetColor);
        ls_uiFillRect(cxt, drawX4+1, drawY4, (drawX3-drawX4)-1, 1, cxt->widgetColor);
        
        CIRCLE_DRAW_BORD(drawX5, drawY5,   scRect, bCol);
        //CIRCLE_DRAW_BORD(drawX5, drawY5+1, scRect, bCol);
        CIRCLE_DRAW_BORD(drawX6, drawY6,   scRect, bCol);
        //CIRCLE_DRAW_BORD(drawX6, drawY6+1, scRect, bCol);
        
        //ls_uiFillRect(cxt, drawX6+1, drawY6+1, (drawX5-drawX6)-1, 1, cxt->widgetColor);
        ls_uiFillRect(cxt, drawX6+1, drawY6, (drawX5-drawX6)-1, 1, cxt->widgetColor);
        
        CIRCLE_DRAW_BORD(drawX7,   drawY7, scRect, bCol);
        //CIRCLE_DRAW_BORD(drawX7-1, drawY7, scRect, bCol);
        CIRCLE_DRAW_BORD(drawX8,   drawY8, scRect, bCol);
        //CIRCLE_DRAW_BORD(drawX8+1, drawY8, scRect, bCol);
        
        //ls_uiFillRect(cxt, drawX8+2, drawY8, (drawX7-drawX8)-3, 1, cxt->widgetColor);
        ls_uiFillRect(cxt, drawX8+1, drawY8, (drawX7-drawX8)-1, 1, cxt->widgetColor);
    }
#undef CIRCLE_DRAW_BORD
}


void ls_uiCircleFloat(UIContext *cxt, s32 xPos, s32 yPos, s32 selRadius)
{
    f64 radSq = selRadius*selRadius;
    f64 sin = 0, cos = 0;
    
    //NOTE: Everything is done relative to (0,0) until when drawing happens
    //NOTE: We start at the top middle pixel of the Circle
    f64 startX = 0.0;
    f64 startY = selRadius;
    
    //NOTE: And end at the first octant swept going clockwise.
    
    f64 currX = startX;
    f64 currY = startY;
    
    UIScissor::UIRect *scRect = cxt->scissor.currRect;
    u32 *At = (u32 *)cxt->drawBuffer;
    
    for(f64 Angle = 0.0; Angle < TAU; Angle += TAU/360.0)
    {
        ls_sincos(Angle, &sin, &cos);
        
        startX = cos*selRadius;
        startY = sin*selRadius;
        
        s32 x1  = ls_floor(startX);
        f64 floatXA1 = 1.0 - ls_fabs(startX - x1);
        
        s32 x2 = ls_ceil(startX);
        f64 floatXA2 = 1.0 - ls_fabs(startX - x2);
        
        s32 y1 = ls_floor(startY);
        f64 floatYA1 = 1.0 - ls_fabs(startY - y1);
        
        s32 y2 = ls_ceil(startY);
        f64 floatYA2 = 1.0 - ls_fabs(startY - y2);
        
        s32 alpha1 = (s32)((floatXA1*floatYA1)*255);
        s32 alpha2 = (s32)((floatXA2*floatYA2)*255);
        
        s32 alpha3 = (s32)((floatXA1*floatYA2)*255);
        s32 alpha4 = (s32)((floatXA2*floatYA1)*255);
        
        Color c1 = ls_uiAlphaBlend(cxt->borderColor, cxt->backgroundColor, alpha1);
        Color c2 = ls_uiAlphaBlend(cxt->borderColor, cxt->backgroundColor, alpha2);
        
        Color c3 = ls_uiAlphaBlend(cxt->borderColor, cxt->backgroundColor, alpha3);
        Color c4 = ls_uiAlphaBlend(cxt->borderColor, cxt->backgroundColor, alpha4);
        
#define CIRCLE_DRAW_BORD(xP, yP, r, c) \
if((xP) >= r->x && (xP) < r->x+r->w && (yP) >= r->y && (yP) < r->y+r->h) \
{ At[(yP)*cxt->width + (xP)] = c; }
        
        CIRCLE_DRAW_BORD(xPos+x1, yPos+y1, scRect, c1);
        CIRCLE_DRAW_BORD(xPos+x2, yPos+y2, scRect, c2);
        
        CIRCLE_DRAW_BORD(xPos+x1, yPos+y2, scRect, c3);
        CIRCLE_DRAW_BORD(xPos+x2, yPos+y1, scRect, c4);
        
#undef CIRCLE_DRAW_BORD
    }
    
}


void ls_uiBackground(UIContext *cxt)
{
    AssertMsg((cxt->height % 4) == 0, "Window Height not divisible by 4 (SIMD)\n");
    AssertMsg((cxt->width % 4) == 0, "Window Width not divisible by 4 (SIMD)\n");
    
    __m128i color = _mm_set1_epi32 ((int)cxt->backgroundColor);
    
    u32 numIterations = (cxt->height*cxt->width) / 4;
    for(u32 i = 0; i < numIterations; i++)
    {
        u32 idx = i*sizeof(s32)*4;
        
        __m128i *At = (__m128i *)(cxt->drawBuffer + idx);
        _mm_storeu_si128(At, color);
    }
}

void ls_uiBitmap(UIContext *cxt, s32 xPos, s32 yPos, u32 *data, s32 w, s32 h)
{
    u32 *At = (u32 *)cxt->drawBuffer;
    
    for(s32 y = yPos, eY = 0; y < yPos+h; y++, eY++)
    {
        for(s32 x = xPos, eX = 0; x < xPos+w; x++, eX++)
        {
            if(x < 0 || x >= cxt->width)  continue;
            if(y < 0 || y >= cxt->height) continue;
            
            At[y*cxt->width + x] = data[eY*w + eX];
        }
    }
}

void ls_uiGlyph(UIContext *cxt, s32 xPos, s32 yPos, UIGlyph *glyph, Color textColor)
{
    UIScissor::UIRect *scRect = cxt->scissor.currRect;
    
    u32 *At = (u32 *)cxt->drawBuffer;
    
    for(s32 y = yPos-glyph->y1, eY = glyph->height-1; eY >= 0; y++, eY--)
    {
        for(s32 x = xPos+glyph->x0, eX = 0; eX < glyph->width; x++, eX++)
        {
            if(x < 0 || x >= cxt->width)  continue;
            if(y < 0 || y >= cxt->height) continue;
            
            if(x < scRect->x || x >= scRect->x+scRect->w) continue;
            if(y < scRect->y || y >= scRect->y+scRect->h) continue;
            
            Color base = At[y*cxt->width + x];
            
            u8 sourceA = GetAlpha(textColor);
            u8 dstA = glyph->data[eY*glyph->width + eX];
            
            f64 sA = (f64)sourceA / 255.0;
            f64 dA = (f64)dstA / 255.0;
            
            u8 Alpha = (sA * dA) * 255;
            
            Color blendedColor = ls_uiAlphaBlend(textColor, base, Alpha);
            At[y*cxt->width + x] = blendedColor;
        }
    }
}

s32 ls_uiGetKernAdvance(UIContext *cxt, s32 codepoint1, s32 codepoint2)
{
    UIFont *font = cxt->currFont;
    s32 kernAdvance = font->kernAdvanceTable[codepoint1][codepoint2];
    
    return kernAdvance;
}

void ls_uiGlyphString(UIContext *cxt, s32 xPos, s32 yPos, unistring text, Color textColor)
{
    s32 currXPos = xPos;
    s32 currYPos = yPos;
    for(u32 i = 0; i < text.len; i++)
    {
        u32 indexInGlyphArray = text.data[i];
        AssertMsg(indexInGlyphArray <= cxt->currFont->maxCodepoint, "GlyphIndex OutOfBounds\n");
        
        UIGlyph *currGlyph = &cxt->currFont->glyph[indexInGlyphArray];
        ls_uiGlyph(cxt, currXPos, yPos, currGlyph, textColor);
        
        s32 kernAdvance = 0;
        if(i < text.len-1) { kernAdvance = ls_uiGetKernAdvance(cxt, text.data[i], text.data[i+1]); }
        
        currXPos += (currGlyph->xAdv + kernAdvance);
    }
}

s32 ls_uiGlyphStringLen(UIContext *cxt, unistring text)
{
    s32 totalLen = 0;
    for(u32 i = 0; i < text.len; i++)
    {
        u32 indexInGlyphArray = text.data[i];
        AssertMsg(indexInGlyphArray <= cxt->currFont->maxCodepoint, "GlyphIndex OutOfBounds\n");
        
        UIGlyph *currGlyph = &cxt->currFont->glyph[indexInGlyphArray];
        
        s32 kernAdvance = 0;
        if(i < text.len-1) { kernAdvance = ls_uiGetKernAdvance(cxt, text.data[i], text.data[i+1]); }
        
        totalLen += (currGlyph->xAdv + kernAdvance);
    }
    
    return totalLen;
}

void ls_uiSelectFontByPixelHeight(UIContext *cxt, u32 pixelHeight)
{
    //TODO: Hardcoded
    b32 found = FALSE;
    for(u32 i = 0; i < 4; i++)
    { if(cxt->fonts[i].pixelHeight == pixelHeight) { found = TRUE; cxt->currFont = &cxt->fonts[i]; } }
    
    AssertMsg(found, "Asked pixelHeight not available\n");
}

inline
void ls_uiSelectFontByFontSize(UIContext *cxt, UIFontSize fontSize)
{ cxt->currFont = &cxt->fonts[fontSize]; }

void ls_uiButton(UIContext *cxt, UIButton button, s32 xPos, s32 yPos, s32 w, s32 h)
{
    Color bkgColor = cxt->widgetColor;
    
    if(MouseInRect(xPos, yPos, w, h))
    { 
        button.isHot = TRUE;
        bkgColor = cxt->highliteColor;
        
        if(LeftClick)
        {
            button.onClick(cxt);
        }
        if(LeftHold) 
        {
            button.isHeld = TRUE;
            bkgColor = cxt->pressedColor;
            button.onHold(cxt);
        }
    }
    
    ls_uiBorderedRect(cxt, xPos, yPos, w, h, bkgColor);
    
    
    ls_uiPushScissor(cxt, xPos+2, yPos+2, w-4, h-4);
    
    ls_uiSelectFontByFontSize(cxt, FS_SMALL);
    
    s32 strWidth  = ls_uiGlyphStringLen(cxt, button.name);
    s32 xOff      = (w - strWidth) / 2; //TODO: What happens when the string is too long?
    s32 strHeight = cxt->currFont->pixelHeight;
    s32 yOff      = strHeight*0.25;
    
    ls_uiGlyphString(cxt, xPos+xOff, yPos+yOff, button.name, cxt->textColor);
    
    ls_uiPopScissor(cxt);
}

void ls_uiTextBox(UIContext *cxt, UITextBox *box, s32 xPos, s32 yPos, s32 w, s32 h)
{
    ls_uiSelectFontByFontSize(cxt, FS_MEDIUM);
    
    ls_uiBorderedRect(cxt, xPos, yPos, w, h);
    
    ls_uiPushScissor(cxt, xPos+4, yPos, w-8, h);
    
    if(box->isSelected)
    {
        //NOTE: Draw characters.
        if(HasPrintableKey()) 
        {
            if(box->caretIndex == box->text.len) { ls_unistrAppendChar(&box->text, GetPrintableKey()); }
            else { ls_unistrInsertChar(&box->text, GetPrintableKey(), box->caretIndex); }
            
            box->caretIndex += 1; 
        }
        if(KeyPress(keyMap::Backspace) && box->text.len > 0 && box->caretIndex > 0) 
        {
            if(box->caretIndex == box->text.len) { ls_unistrTrimRight(&box->text, 1); }
            else { ls_unistrRmIdx(&box->text, box->caretIndex-1); }
            
            box->caretIndex -= 1;
        }
        if(KeyPress(keyMap::Delete) && box->text.len > 0 && box->caretIndex < box->text.len)
        {
            if(box->caretIndex == box->text.len-1) { ls_unistrTrimRight(&box->text, 1); }
            else { ls_unistrRmIdx(&box->text, box->caretIndex); }
        }
        
        if(KeyPress(keyMap::LArrow) && box->caretIndex > 0) 
        { box->isCaretOn = TRUE; box->dtCaret = 0; box->caretIndex -= 1; }
        if(KeyPress(keyMap::RArrow) && box->caretIndex < box->text.len) 
        { box->isCaretOn = TRUE; box->dtCaret = 0; box->caretIndex += 1; }
        if(KeyPress(keyMap::Home)) 
        { box->isCaretOn = TRUE; box->dtCaret = 0; box->caretIndex = 0; }
        if(KeyPress(keyMap::End)) 
        { box->isCaretOn = TRUE; box->dtCaret = 0; box->caretIndex = box->text.len; }
        
        if(KeyHeld(keyMap::Control) && KeyPress(keyMap::V))
        {
            u32 buff[128] = {};
            u32 copiedLen = GetClipboard(buff, 128);
            
            if(box->caretIndex == box->text.len) { ls_unistrAppendBuffer(&box->text, buff, copiedLen); }
            else { ls_unistrInsertBuffer(&box->text, buff, copiedLen, box->caretIndex); }
            
            box->caretIndex += copiedLen;
        }
        
#if 0
        if(KeyHeld(keyMap::Control) && KeyPress(keyMap::C))
        {
            SetClipboard(box->text.data, box->text.len);
        }
#endif
        
        //TODO: The positioning is hardcoded. Bad Pasta.
        if(box->text.len > 0)
        { 
            ls_uiGlyphString(cxt, xPos+10, yPos+8, box->text, cxt->textColor);
        }
        
        //NOTE: Draw the Caret
        box->dtCaret += cxt->dt;
        if(box->dtCaret >= 400) { box->dtCaret = 0; box->isCaretOn = !box->isCaretOn; }
        
        if(box->isCaretOn)
        {
            UIGlyph *caretGlyph = &cxt->currFont->glyph['|'];
            
            unistring tmp = {box->text.data, (u32)box->caretIndex, (u32)box->caretIndex};
            
            u32 stringLen = ls_uiGlyphStringLen(cxt, tmp);
            ls_uiGlyph(cxt, xPos+12+stringLen, yPos+10, caretGlyph, cxt->textColor);
        }
        
    }
    
    ls_uiPopScissor(cxt);
}

void ls_uiDrawArrow(UIContext *cxt, s32 x, s32 yPos, s32 w, s32 h)
{
    UIScissor::UIRect *scRect = cxt->scissor.currRect;
    u32 *At = (u32 *)cxt->drawBuffer;
    
    s32 xPos = x-1;
    
    Color bkgColor = cxt->widgetColor;
    if(MouseInRect(xPos, yPos, w, h)) { bkgColor = cxt->highliteColor; }
    
    ls_uiBorderedRect(cxt, xPos, yPos, w, h, bkgColor);
    
    s32 arrowWidth = 12;
    s32 hBearing = (w - arrowWidth)/2;
    
    s32 xBase = xPos+hBearing;
    s32 xEnd  = xBase + arrowWidth;
    
    s32 arrowHeight = 6;
    s32 vBearing = (h - arrowHeight)/2;
    s32 yStart = yPos + h - vBearing;
    
    for(s32 y = yStart-1; y >= (yStart-1)-arrowHeight; y--)
    {
        for(s32 x = xBase; x < xEnd; x++)
        {
            if(x < 0 || x >= cxt->width)  continue;
            if(y < 0 || y >= cxt->height) continue;
            
            if(x < scRect->x || x >= scRect->x+scRect->w) continue;
            if(y < scRect->y || y >= scRect->y+scRect->h) continue;
            
            At[y*cxt->width + x] = RGBg(0x00);
        }
        
        xBase += 1;
        xEnd  -= 1;
    }
}

inline void ls_uiListBoxAddEntry(UIContext *cxt, UIListBox *list, char *s)
{ 
    //TODO: Terrible.
    unistring text = ls_unistrFromAscii(s);
    list->list.push(text); 
    ls_unistrFree(&text);
}

inline void ls_uiListBoxAddEntry(UIContext *cxt, UIListBox *list, unistring s)
{ list->list.push(s); }

inline void ls_uiListBoxRemoveEntry(UIContext *cxt, UIListBox *list, u32 index)
{ list->list.remove(index); }

void ls_uiListBox(UIContext *cxt, UIListBox *list, s32 xPos, s32 yPos, s32 w, s32 h)
{
    ls_uiSelectFontByFontSize(cxt, FS_SMALL);
    
    ls_uiBorderedRect(cxt, xPos, yPos, w, h);
    
    const s32 arrowBoxWidth = 30;
    ls_uiPushScissor(cxt, xPos, yPos, w+arrowBoxWidth, h);
    
    ls_uiDrawArrow(cxt, xPos + w, yPos, arrowBoxWidth, h);
    
    if(LeftClick && MouseInRect(xPos+w, yPos, arrowBoxWidth, h))
    {
        if(list->isOpen) { list->isOpen = FALSE; }
        else { list->isOpening = TRUE; }
    }
    
    if(list->list.count)
    {
        unistring selected = list->list[list->selectedIndex];
        ls_uiGlyphString(cxt, xPos+10, yPos+12, selected, cxt->textColor);
    }
    
    ls_uiPopScissor(cxt);
    
    
    s32 maxHeight = (list->list.count)*h;
    //TODO: Should I try adding another Scissor? Has to be added inside the branches.
    if(list->isOpening)
    {
        list->dtOpen += cxt->dt;
        
        s32 height = 0;
        if(list->dtOpen > 17)  { height = maxHeight*0.10f; }
        if(list->dtOpen > 34)  { height = maxHeight*0.35f; }
        if(list->dtOpen > 52)  { height = maxHeight*0.70f; }
        if(list->dtOpen > 70) { list->isOpen = TRUE; list->isOpening = FALSE; list->dtOpen = 0; }
        
        if(!list->isOpen)
        {
            ls_uiFillRect(cxt, xPos+1, yPos-height, w-2, height, cxt->widgetColor);
        }
    }
    
    s32 toBeChanged = 9999;
    if(list->isOpen)
    {
        Color bkgColor = cxt->widgetColor;
        
        for(u32 i = 0; i < list->list.count; i++)
        {
            s32 currY = yPos - (h*(i+1));
            unistring currStr = list->list[i];
            
            if(MouseInRect(xPos+1, currY+1, w-2, h-1)) 
            { 
                bkgColor = cxt->highliteColor;
                
                if(LeftHold)
                {
                    bkgColor = cxt->pressedColor;
                }
                
                if(LeftUp)
                {
                    toBeChanged = i;
                }
            }
            
            ls_uiRect(cxt, xPos+1, currY, w-2, h, bkgColor);
            ls_uiGlyphString(cxt, xPos+10, yPos+12-(h*(i+1)), currStr, cxt->textColor);
            
            bkgColor = cxt->widgetColor;
        }
        
        ls_uiBorder(cxt, xPos, yPos-maxHeight, w, maxHeight+1);
    }
    
    //NOTE: We defer the selectedIndex change to the end of the frame 
    //      to avoid the for loop blinking wrongly for a frame
    if(toBeChanged != 9999) { list->selectedIndex = toBeChanged; list->isOpen = FALSE; }
}

//TODO: The things are rendered in a logical order, but that makes the function's flow very annoying
//      going in and out of if blocks to check hot/held and style.
void ls_uiSlider(UIContext *cxt, UISlider *slider, s32 xPos, s32 yPos, s32 w, s32 h)
{
    if(LeftUp) { slider->isHeld = FALSE; }
    
    //NOTE: Box Slider Branchless Opacity Check
    u8 opacity = 0xEE - (0xB0*slider->isHeld);
    
    if(slider->style == SL_LINE) 
    {
        ls_uiBorderedRect(cxt, xPos, yPos, w, h);
        
        s32 selRadius = 50;
        
        ls_uiPushScissor(cxt, xPos-selRadius, yPos-selRadius, w+selRadius, h+(selRadius*2));
        
        ls_uiCircleFloat(cxt, xPos+400, yPos+(h/2), selRadius);
        
        ls_uiPopScissor(cxt);
    }
    else if(slider->style == SL_BOX)
    {
        s32 slideWidth = 3;
        
        ls_uiBorder(cxt, xPos, yPos, w, h);
        
        ls_uiPushScissor(cxt, xPos-4, yPos-3, w+8, h+6);
        
        slider->currValue = ((slider->maxValue - slider->minValue) * slider->currPos) + slider->minValue;
        s32 slidePos = w*slider->currPos;
        
        ls_uiFillRect(cxt, xPos+1, yPos+1, slidePos, h-2, slider->lColor);
        ls_uiFillRect(cxt, xPos+slidePos, yPos+1, w-slidePos-1, h-2, slider->rColor);
        
        unistring val = ls_unistrIntToStr(slider->currValue);
        
        ls_uiPushScissor(cxt, xPos+1, yPos+1, w-2, h-2);
        
        ls_uiSelectFontByFontSize(cxt, FS_SMALL);
        u32 textLen = ls_uiGlyphStringLen(cxt, val);
        
        
        s32 strXPos = xPos + slidePos - textLen - 2;
        s32 strHeight = cxt->currFont->pixelHeight;
        Color textBkgC = slider->lColor;
        if(strXPos < xPos+1) { strXPos = xPos + slidePos + slideWidth + 2; textBkgC = slider->rColor; }
        
        Color valueColor = RGBA(0x22, 0x22, 0x22, 0x00 + (slider->isHeld*0xFF));
        ls_uiGlyphString(cxt, strXPos, yPos + h - strHeight, val, valueColor);
        
        ls_uiPopScissor(cxt);
        
        if(MouseInRect(xPos + slidePos-5, yPos, 10, h)) 
        {
            slider->isHot = TRUE; 
            if(LeftHold) { slider->isHeld = TRUE; }
            
            
            s32 actualX = (xPos + slidePos) - 1;
            s32 actualY = yPos - 2;
            
            s32 actualWidth  = slideWidth+2;
            s32 actualHeight = 4 + h;
            
            ls_uiFillRect(cxt, actualX, actualY, actualWidth, actualHeight, cxt->borderColor);
        }
        else
        {
            ls_uiFillRect(cxt, xPos+slidePos, yPos, slideWidth, h, cxt->borderColor);
        }
        
        ls_uiPopScissor(cxt);
    }
    
    //NOTE: Draw the displayed text, and hide through Alpha the slider info.
    ls_uiPushScissor(cxt, xPos, yPos, w, h);
    
    Color rectColor = cxt->widgetColor;
    rectColor = SetAlpha(rectColor, opacity);
    ls_uiBorderedRect(cxt, xPos, yPos, w, h, rectColor);
    
    ls_uiSelectFontByFontSize(cxt, FS_MEDIUM);
    
    s32 strWidth  = ls_uiGlyphStringLen(cxt, slider->text);
    s32 xOff      = (w - strWidth) / 2; //TODO: What happens when the string is too long?
    s32 strHeight = cxt->currFont->pixelHeight;
    s32 yOff      = strHeight*0.25;
    
    Color textColor = cxt->textColor;
    textColor = SetAlpha(textColor, opacity);
    
    ls_uiGlyphString(cxt, xPos+xOff, yPos+yOff+(h/3), slider->text, textColor);
    
    ls_uiPopScissor(cxt);
    
    
    if(KeyPress(keyMap::C))
    {
        DebugBreak();
    }
    
    
    if(slider->isHeld) { 
        s32 deltaX = (UserInput.Mouse.prevPos.x - UserInput.Mouse.currPos.x);//*cxt->dt;
        
        f64 fractionMove = (f64)deltaX / (f64)w;
        
        slider->currPos -= fractionMove;
        
        slider->currPos = ls_mathClamp(slider->currPos, 1.0, 0.0);
    }
    
    slider->isHot = FALSE;
}

void ls_uiRender(UIContext *c)
{
    c->callbackRender();
    return;
}
