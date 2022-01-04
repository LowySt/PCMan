#define RGBA(r,g,b,a)  (u32)((a<<24)|(r<<16)|(g<<8)|b)
#define RGB(r,g,b)     (u32)((0xFF<<24)|(r<<16)|(g<<8)|b)
#define RGBg(v)        (u32)((0xFF<<24)|(v<<16)|(v<<8)|v)
#define SetAlpha(v, a) (u32)((v & 0x00FFFFFF) | (((u32)a)<<24))
#define GetAlpha(v)    ( u8)(((v) & 0xFF000000) >> 24)

const     u32 THREAD_COUNT       = 2;
constexpr u32 RENDER_GROUP_COUNT = THREAD_COUNT == 0 ? 1 : THREAD_COUNT;

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

//TODO: Add max descent of font to adjust text vertical position in text boxes.
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

struct UILPane
{
    u32 dtOpen;
    b32 isOpening;
    b32 isOpen;
};

struct UIContext;

enum UIButtonStyle { UIBUTTON_TEXT, UIBUTTON_TEXT_NOBORDER, UIBUTTON_BMP };

typedef b32(*ButtonProc)(UIContext *cxt, void *data);
struct UIButton
{
    UIButtonStyle style;
    
    unistring name;
    u8 *bmpData;
    s32 bmpW, bmpH;
    
    b32 isHot;
    b32 isHeld;
    
    ButtonProc onClick;
    ButtonProc onHold;
    
    void *data; //TODO: Separate onClick / onHold user data
};


typedef b32(*TextBoxProc)(UIContext *cxt, void *data);
struct UITextBox
{
    unistring text;
    u32 maxLen;
    
    b32 isReadonly;
    
    u32 dtCaret;
    b32 isCaretOn;
    s32 caretIndex;
    
    s32 viewBeginIdx;
    s32 viewEndIdx;
    
    s32 selectBeginIdx;
    s32 selectEndIdx;
    b32 isSelecting;
    
    TextBoxProc preInput;
    TextBoxProc postInput;
    
    void *data; //TODO: Separate preInput / postInput user data
};

struct UIListBoxItem
{
    unistring name;
    Color bkgColor;
    Color textColor;
};

typedef void(*ListBoxProc)(UIContext *cxt, void *data);
struct UIListBox
{
    Array<UIListBoxItem> list;
    s32 selectedIndex;
    
    u32 dtOpen;
    b32 isOpening;
    b32 isOpen;
    
    Color arrowBkg;
    
    ListBoxProc onSelect;
    void *data;
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


//TODO: Very Shitty implementation of Menus
struct UIMenu
{
    UIButton  closeWindow;
    UIButton  minimize;
    
    b32 isOpen;
    s32 openIdx;
    Array<UIButton> items;
    
    UIMenu *sub;            //NOTE: There is 1 sub for every item
    u32     maxSub;
};

enum RenderCommandExtra
{
    UI_RCE_NULL,
    
    UI_RCE_LEFT,
    UI_RCE_RIGHT,
};

enum RenderCommandType
{
    UI_RC_TEXTBOX = 1,
    UI_RC_BUTTON,
    UI_RC_LISTBOX,
    UI_RC_LISTBOX_ARR,
    UI_RC_SLIDER,
    UI_RC_RECT,
    UI_RC_MENU,
    
    UI_RC_FRAG_OFF,
    
    UI_RC_FRAG_TEXTBOX,
    UI_RC_FRAG_BUTTON,
    UI_RC_FRAG_LISTBOX,
    UI_RC_FRAG_LISTBOX_ARR,
    UI_RC_FRAG_SLIDER,
    UI_RC_FRAG_RECT,
    UI_RC_FRAG_MENU,
};

struct RenderCommand
{
    RenderCommandType  type;
    
    s32 x, y, w, h;
    
    union
    {
        UITextBox *textBox;
        UIButton  *button;
        UIListBox *listBox;
        UISlider  *slider;
        UIMenu    *menu;
    };
    
    Color bkgColor;
    Color textColor;
    
    RenderCommandExtra extra;
    s32 oX, oY, oW, oH;
};


const u32 UI_Z_LAYERS = 3;

struct RenderGroup
{
    stack RenderCommands[UI_Z_LAYERS];
    volatile b32 isDone;
};

typedef void (*RenderCallback)();
struct UIContext
{
    u8 *drawBuffer;
    u32 width;
    u32 height;
    
    UIFont *fonts;
    u32 numFonts;
    
    UIFont *currFont;
    
    Color backgroundColor;
    Color borderColor;
    
    Color highliteColor;
    Color pressedColor;
    
    Color widgetColor;
    Color textColor;
    Color invWidgetColor;
    Color invTextColor;
    
    UIScissor scissor;
    
    u64 *currentFocus;
    u64 *lastFocus;
    b32 focusWasSetThisFrame;
    
    b32 nextFrameFocusChange;
    u64 *nextFrameFocus;
    
    u64 *mouseCapture;
    
    RenderGroup renderGroups[RENDER_GROUP_COUNT];
    
    CONDITION_VARIABLE startRender;
    CRITICAL_SECTION crit;
    
    RenderCallback renderFunc;
    u32 dt;
};

struct ___threadCtx
{
    UIContext *c;
    u64 ThreadID;
};

void ls_uiRender__(UIContext *c, u32 threadID);
DWORD ls_uiRenderThreadProc(void *param)
{
    ___threadCtx *t = (___threadCtx *)param;
    
    UIContext *c = t->c;
    u64 threadID = t->ThreadID;
    
    while(TRUE)
    {
        EnterCriticalSection(&c->crit);
        SleepConditionVariableCS(&c->startRender, &c->crit, INFINITE);
        LeaveCriticalSection(&c->crit);
        
        ls_uiRender__(c, threadID);
    }
    
    AssertMsg(FALSE, "Render Thread should never exit\n");
    
    return 0;
}

UIContext *ls_uiInitDefaultContext(u8 *drawBuffer, u32 width, u32 height, RenderCallback cb)
{
    UIContext *uiContext       = (UIContext *)ls_alloc(sizeof(UIContext));
    uiContext->drawBuffer      = drawBuffer;
    uiContext->width           = width;
    uiContext->height          = height;
    uiContext->renderFunc      = cb;
    uiContext->backgroundColor = RGBg(0x38);
    uiContext->highliteColor   = RGBg(0x65);
    uiContext->pressedColor    = RGBg(0x75);
    uiContext->widgetColor     = RGBg(0x45);
    uiContext->borderColor     = RGBg(0x22);
    uiContext->textColor       = RGBg(0xCC);
    uiContext->invWidgetColor  = RGBg(0xBA);
    uiContext->invTextColor    = RGBg(0x33);
    
    if(THREAD_COUNT != 0)
    {
        for(u32 i = 0; i < THREAD_COUNT; i++)
        {
            uiContext->renderGroups[i].RenderCommands[0] = ls_stackInit(sizeof(RenderCommand), 384);
            uiContext->renderGroups[i].RenderCommands[1] = ls_stackInit(sizeof(RenderCommand), 64);
            uiContext->renderGroups[i].RenderCommands[2] = ls_stackInit(sizeof(RenderCommand), 64);
        }
        
        
        InitializeConditionVariable(&uiContext->startRender);
        InitializeCriticalSection(&uiContext->crit);
        
        for(u32 i = 0; i < THREAD_COUNT; i++)
        {
            ___threadCtx *t = (___threadCtx *)ls_alloc(sizeof(___threadCtx));
            t->c            = uiContext;
            t->ThreadID     = i;
            
            CreateThread(NULL, KBytes(512), ls_uiRenderThreadProc, t, NULL, NULL);
        }
    }
    else
    {
        uiContext->renderGroups[0].RenderCommands[0] = ls_stackInit(sizeof(RenderCommand), 512);
        uiContext->renderGroups[0].RenderCommands[1] = ls_stackInit(sizeof(RenderCommand), 64);
        uiContext->renderGroups[0].RenderCommands[2] = ls_stackInit(sizeof(RenderCommand), 64);
    }
    
    return uiContext;
}

void ls_uiFocusChange(UIContext *cxt, u64 *focus)
{
    cxt->nextFrameFocusChange = TRUE;
    cxt->nextFrameFocus = focus;
}

b32 ls_uiInFocus(UIContext *cxt, void *p)
{
    if(cxt->currentFocus == (u64 *)p) { return TRUE; }
    return FALSE;
}

b32 ls_uiHasCapture(UIContext *cxt, void *p)
{
    if(cxt->mouseCapture == (u64 *)p) { return TRUE; }
    return FALSE;
}

struct RenderRect { s32 x,y,w,h; };
b32 ls_uiRectIsInside(RenderRect r1, RenderRect check)
{
    if((r1.x >= check.x) && ((r1.x + r1.w) <= (check.x + check.w)))
    {
        if((r1.y >= check.y) && ((r1.y + r1.h) <= (check.y + check.h)))
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

void ls_uiPushRenderCommand(UIContext *c, RenderCommand command, s32 zLayer)
{
    switch(THREAD_COUNT)
    {
        case 0:
        case 1:
        {
            ls_stackPush(&c->renderGroups[0].RenderCommands[zLayer], (void *)&command);
            return;
        } break;
        
        case 2:
        {
            RenderRect commandRect = { command.x, command.y, command.w, command.h };
            
            if(ls_uiRectIsInside(commandRect, {0, 0, (s32)(c->width / 2), (s32)c->height}))
            {
                ls_stackPush(&c->renderGroups[0].RenderCommands[zLayer], (void *)&command);
                return;
            }
            else if(ls_uiRectIsInside(commandRect, {(s32)(c->width / 2), 0, (s32)(c->width / 2), (s32)c->height}))
            {
                ls_stackPush(&c->renderGroups[1].RenderCommands[zLayer], (void *)&command);
                return;
            }
            else
            {
                //Half in one, half in another.
                
                RenderCommand section = command;
                
                section.type  = (RenderCommandType)(command.type + UI_RC_FRAG_OFF);
                section.extra = UI_RCE_LEFT;
                section.oX = command.x;
                section.oY = command.y;
                section.oW = command.w;
                section.oH = command.h;
                
                section.x     = command.x;
                section.w     = (c->width/2) - command.x;
                
                ls_stackPush(&c->renderGroups[0].RenderCommands[zLayer], (void *)&section);
                
                section.extra = UI_RCE_RIGHT;
                section.x     = c->width/2;
                section.w     = command.w - section.w;
                
                ls_stackPush(&c->renderGroups[1].RenderCommands[zLayer], (void *)&section);
                return;
            }
            
        } break;
        
        default: { AssertMsg(FALSE, "Thread count not supported\n"); } return;
    }
    
    AssertMsg(FALSE, "Should never reach this case!\n");
}

//TODO:NOTE: Scissors are busted. A smaller scissor doesn't check if it is inside it's own parent!
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
    AssertMsg(scissor->rects.data,      "Scissor stack is null\n");
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
            
            __m128i val = _mm_loadu_si128(At);
            
            u32 a1 = _mm_cvtsi128_si32(_mm_shuffle_epi32(val, 0b00000000));
            u32 a2 = _mm_cvtsi128_si32(_mm_shuffle_epi32(val, 0b01010101));
            u32 a3 = _mm_cvtsi128_si32(_mm_shuffle_epi32(val, 0b10101010));
            u32 a4 = _mm_cvtsi128_si32(_mm_shuffle_epi32(val, 0b11111111));
            
            Color c1 = ls_uiAlphaBlend(c, a1);
            Color c2 = ls_uiAlphaBlend(c, a2);
            Color c3 = ls_uiAlphaBlend(c, a3);
            Color c4 = ls_uiAlphaBlend(c, a4);
            
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
void ls_uiBorderFrag(UIContext *c, s32 xPos, s32 yPos, s32 w, s32 h, RenderCommandExtra rce)
{
    Color C = c->borderColor;
    
    if(rce == UI_RCE_LEFT)
    {
        ls_uiFillRect(c, xPos, yPos,     w, 1, C);
        ls_uiFillRect(c, xPos, yPos+h-1, w, 1, C);
        ls_uiFillRect(c, xPos, yPos,     1, h, C);
    }
    else if(rce == UI_RCE_RIGHT)
    {
        ls_uiFillRect(c, xPos,     yPos,     w, 1, C);
        ls_uiFillRect(c, xPos,     yPos+h-1, w, 1, C);
        ls_uiFillRect(c, xPos+w-1, yPos,     1, h, C);
    }
}


inline
void ls_uiBorder(UIContext *c, s32 xPos, s32 yPos, s32 w, s32 h)
{
    Color C = c->borderColor;
    
    ls_uiFillRect(c, xPos,     yPos,     w, 1, C);
    ls_uiFillRect(c, xPos,     yPos+h-1, w, 1, C);
    ls_uiFillRect(c, xPos,     yPos,     1, h, C);
    ls_uiFillRect(c, xPos+w-1, yPos,     1, h, C);
}

inline
void ls_uiBorder(UIContext *c, s32 xPos, s32 yPos, s32 w, s32 h, Color borderColor)
{
    Color C = borderColor;
    
    ls_uiFillRect(c, xPos,     yPos,     w, 1, C);
    ls_uiFillRect(c, xPos,     yPos+h-1, w, 1, C);
    ls_uiFillRect(c, xPos,     yPos,     1, h, C);
    ls_uiFillRect(c, xPos+w-1, yPos,     1, h, C);
}

inline
void ls_uiBorderedRectFrag(UIContext *c, s32 xPos, s32 yPos, s32 w, s32 h, RenderCommandExtra rce)
{
    Color C = c->borderColor;
    Color W = c->widgetColor;
    
    if(rce == UI_RCE_LEFT)
    {
        ls_uiFillRect(c, xPos, yPos,     w, 1, C);
        ls_uiFillRect(c, xPos, yPos+h-1, w, 1, C);
        ls_uiFillRect(c, xPos, yPos,     1, h, C);
        ls_uiFillRect(c, xPos+1, yPos+1, w-1, h-2, W);
    }
    else if(rce == UI_RCE_RIGHT)
    {
        ls_uiFillRect(c, xPos,     yPos,     w, 1, C);
        ls_uiFillRect(c, xPos,     yPos+h-1, w, 1, C);
        ls_uiFillRect(c, xPos+w-1, yPos,     1, h, C);
        ls_uiFillRect(c, xPos, yPos+1, w-1, h-2, W);
    }
}

inline
void ls_uiBorderedRectFrag(UIContext *c, s32 xPos, s32 yPos, s32 w, s32 h, Color widgetColor, RenderCommandExtra rce)
{
    Color C = c->borderColor;
    
    
    if(rce == UI_RCE_LEFT)
    {
        ls_uiFillRect(c, xPos, yPos,     w, 1, C);
        ls_uiFillRect(c, xPos, yPos+h-1, w, 1, C);
        ls_uiFillRect(c, xPos, yPos,     1, h, C);
        ls_uiFillRect(c, xPos+1, yPos+1, w-1, h-2, widgetColor);
    }
    else if(rce == UI_RCE_RIGHT)
    {
        ls_uiFillRect(c, xPos,     yPos,     w, 1, C);
        ls_uiFillRect(c, xPos,     yPos+h-1, w, 1, C);
        ls_uiFillRect(c, xPos+w-1, yPos,     1, h, C);
        ls_uiFillRect(c, xPos, yPos+1, w-1, h-2, widgetColor);
    }
}


inline
void ls_uiBorderedRectFrag(UIContext *c, s32 xPos, s32 yPos, s32 w, s32 h, 
                           Color widgetColor, Color borderColor, RenderCommandExtra rce)
{
    Color C = borderColor;
    
    if(rce == UI_RCE_LEFT)
    {
        ls_uiFillRect(c, xPos, yPos,     w, 1, C);
        ls_uiFillRect(c, xPos, yPos+h-1, w, 1, C);
        ls_uiFillRect(c, xPos, yPos,     1, h, C);
        ls_uiFillRect(c, xPos+1, yPos+1, w-1, h-2, widgetColor);
    }
    else if(rce == UI_RCE_RIGHT)
    {
        ls_uiFillRect(c, xPos,     yPos,     w, 1, C);
        ls_uiFillRect(c, xPos,     yPos+h-1, w, 1, C);
        ls_uiFillRect(c, xPos+w-1, yPos,     1, h, C);
        ls_uiFillRect(c, xPos, yPos+1, w-1, h-2, widgetColor);
    }
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
    UIScissor::UIRect *scRect = cxt->scissor.currRect;
    
    u32 *At = (u32 *)cxt->drawBuffer;
    
    for(s32 y = yPos, eY = 0; y < yPos+h; y++, eY++)
    {
        for(s32 x = xPos, eX = 0; x < xPos+w; x++, eX++)
        {
            if(x < 0 || x >= cxt->width)  continue;
            if(y < 0 || y >= cxt->height) continue;
            
            if(x < scRect->x || x >= scRect->x+scRect->w) continue;
            if(y < scRect->y || y >= scRect->y+scRect->h) continue;
            
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

//TODO: Use font max descent to determine yOffsets globally
void ls_uiGlyphString(UIContext *c, s32 xPos, s32 yPos, unistring text, Color textColor)
{
    s32 currXPos = xPos;
    for(u32 i = 0; i < text.len; i++)
    {
        u32 indexInGlyphArray = text.data[i];
        AssertMsg(indexInGlyphArray <= c->currFont->maxCodepoint, "GlyphIndex OutOfBounds\n");
        
        UIGlyph *currGlyph = &c->currFont->glyph[indexInGlyphArray];
        ls_uiGlyph(c, currXPos, yPos, currGlyph, textColor);
        
        s32 kernAdvance = 0;
        if(i < text.len-1) { kernAdvance = ls_uiGetKernAdvance(c, text.data[i], text.data[i+1]); }
        
        currXPos += (currGlyph->xAdv + kernAdvance);
    }
}

void ls_uiGlyphFrag(UIContext *c, s32 xPos, s32 yPos, s32 oX, s32 oY, 
                    s32 minX, s32 minY, s32 maxX, s32 maxY, 
                    UIGlyph *glyph, Color textColor)
{
    UIScissor::UIRect *scRect = c->scissor.currRect;
    
    u32 *At = (u32 *)c->drawBuffer;
    
    for(s32 y = yPos-glyph->y1, eY = glyph->height-1; eY >= 0; y++, eY--)
    {
        for(s32 x = xPos+glyph->x0, eX = 0; eX < glyph->width; x++, eX++)
        {
            if((x < minX)  || (y < minY)) continue;
            if((x >= maxX) || (y >= maxY)) continue;
            
            if((x < oX) || (y < oY)) continue;
            
            if(x < 0 || x >= c->width)  continue;
            if(y < 0 || y >= c->height) continue;
            
            if(x < scRect->x || x >= scRect->x+scRect->w) continue;
            if(y < scRect->y || y >= scRect->y+scRect->h) continue;
            
            Color base = At[y*c->width + x];
            
            u8 sourceA = GetAlpha(textColor);
            u8 dstA = glyph->data[eY*glyph->width + eX];
            
            f64 sA = (f64)sourceA / 255.0;
            f64 dA = (f64)dstA / 255.0;
            
            u8 Alpha = (sA * dA) * 255;
            
            Color blendedColor = ls_uiAlphaBlend(textColor, base, Alpha);
            At[y*c->width + x] = blendedColor;
        }
    }
}

void ls_uiGlyphStringFrag(UIContext *c, s32 xPos, s32 yPos, s32 oX, s32 oY, 
                          s32 minX, s32 minY, s32 maxX, s32 maxY, 
                          unistring text, Color textColor)
{
    //TODO: Pre-skip the glyphs that appear before (minX,minY)
    s32 currXPos = xPos;
    for(u32 i = 0; i < text.len; i++)
    {
        u32 indexInGlyphArray = text.data[i];
        AssertMsg(indexInGlyphArray <= c->currFont->maxCodepoint, "GlyphIndex OutOfBounds\n");
        
        UIGlyph *currGlyph = &c->currFont->glyph[indexInGlyphArray];
        ls_uiGlyphFrag(c, currXPos, yPos, oX, oY, minX, minY, maxX, maxY, currGlyph, textColor);
        
        s32 kernAdvance = 0;
        if(i < text.len-1) { kernAdvance = ls_uiGetKernAdvance(c, text.data[i], text.data[i+1]); }
        
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
s32 ls_uiSelectFontByFontSize(UIContext *cxt, UIFontSize fontSize)
{ cxt->currFont = &cxt->fonts[fontSize]; return cxt->currFont->pixelHeight; }

//TODO:Button autosizing width
//TODO:Menus use buttons, but also claim Focus, which means I can't use the global focus trick to avoid input
//     handling between overlapping elements.
//     I don't think the menu should deal with things like this [It shouldn't hold the close button first,
//     and it also shouldn't use 'normal' buttons for its drop down sub-menus, so... basically @MenuIsShit
//     and I wanna redo it completely.
b32 ls_uiButton(UIContext *c, UIButton *button, s32 xPos, s32 yPos, s32 w, s32 h, s32 zLayer = 0)
{
    b32 inputUse = FALSE;
    
    Color bkgColor = c->widgetColor;
    
    if(button->style == UIBUTTON_TEXT_NOBORDER) { bkgColor = c->backgroundColor; }
    
    if(MouseInRect(xPos, yPos, w, h) && ls_uiHasCapture(c, 0))// && ls_uiInFocus(cxt, 0))
    { 
        button->isHot = TRUE;
        bkgColor = c->highliteColor;
        
        //b32 noCapture = ls_uiHasCapture(cxt, 0);
        
        if(button->onClick && LeftClick)// && noCapture)
        {
            inputUse |= button->onClick(c, button->data);
        }
        if(LeftHold)//  && noCapture)
        {
            button->isHeld = TRUE;
            bkgColor = c->pressedColor;
            if(button->onHold) { inputUse |= button->onHold(c, button->data); }
        }
    }
    
    RenderCommand command = { UI_RC_BUTTON, xPos, yPos, w, h };
    command.button        = button;
    command.bkgColor      = bkgColor;
    command.textColor     = c->textColor;
    ls_uiPushRenderCommand(c, command, zLayer);
    
    return inputUse;
}

void ls_uiLabel(UIContext *cxt, unistring label, s32 xPos, s32 yPos)
{
    s32 strPixelHeight = ls_uiSelectFontByFontSize(cxt, FS_SMALL);
    
    s32 strWidth = ls_uiGlyphStringLen(cxt, label);
    
    ls_uiPushScissor(cxt, xPos, yPos-4, strWidth, strPixelHeight+8);
    
    ls_uiGlyphString(cxt, xPos, yPos, label, cxt->textColor);
    
    ls_uiPopScissor(cxt);
}

void ls_uiLabel(UIContext *cxt, const char32_t *label, s32 xPos, s32 yPos)
{
    unistring lab = ls_unistrConstant(label);
    ls_uiLabel(cxt, lab, xPos, yPos);
}

void ls_uiTextBoxClear(UIContext *cxt, UITextBox *box)
{
    ls_unistrClear(&box->text);
    
    box->caretIndex     = 0;
    box->isReadonly     = FALSE;
    box->selectBeginIdx = 0;
    box->selectEndIdx   = 0;
    box->isSelecting    = FALSE;
    box->viewBeginIdx   = 0;
    box->viewEndIdx     = 0;
}

void ls_uiTextBoxSet(UIContext *cxt, UITextBox *box, unistring s)
{
    ls_unistrSet(&box->text, s);
    box->viewEndIdx = box->text.len;
}

//TODO: Text Alignment
//TODO: Delete/Backspace don't respect selection
b32 ls_uiTextBox(UIContext *cxt, UITextBox *box, s32 xPos, s32 yPos, s32 w, s32 h)
{
    b32 inputUse = FALSE;
    
    if(LeftClickIn(xPos, yPos, w, h) && (box->isReadonly == FALSE) && ls_uiHasCapture(cxt, 0)) {
        cxt->currentFocus = (u64 *)box;
        cxt->focusWasSetThisFrame = TRUE;
        box->isCaretOn = TRUE; 
    }
    
    const s32 horzOff   = 4;
    s32 scissorWidth    = w - horzOff;
    s32 viewAddWidth    = scissorWidth - horzOff;
    //ls_uiPushScissor(cxt, xPos+4, yPos, scissorWidth, h);
    
    if(ls_uiInFocus(cxt, box))
    {
        if(box->preInput)
        { inputUse |= box->preInput(cxt, box->data); }
        
        //NOTE: Draw characters. (box->maxLen == 0 means there's no max len)
        if(HasPrintableKey() && (box->text.len < box->maxLen || box->maxLen == 0))
        {
            if(box->isSelecting) {
                s32 insertIdx = box->selectBeginIdx;
                s32 selectRange = box->selectEndIdx - box->selectBeginIdx;
                s32 endIdx = box->selectEndIdx-1;
                
                if(box->caretIndex > insertIdx) { box->caretIndex -= selectRange; }
                
                if(ls_uiGlyphStringLen(cxt, box->text) <= viewAddWidth)
                { box->viewEndIdx -= selectRange; }
                
                ls_unistrRmSubstr(&box->text, insertIdx, endIdx);
                
                //NOTE: Because te view region gets invalidated by scrolled replacements
                //      We need to re-adjust it!
                s32 diff = (s32)box->text.len - (s32)box->viewEndIdx;
                if(diff < 0) { box->viewEndIdx += diff; box->viewBeginIdx += diff; }
                
                box->isSelecting = FALSE;
            }
            
            if(box->caretIndex == box->text.len) { ls_unistrAppendChar(&box->text, GetPrintableKey()); }
            else { ls_unistrInsertChar(&box->text, GetPrintableKey(), box->caretIndex); }
            
            box->caretIndex += 1;
            box->viewEndIdx += 1;
            
            if(ls_uiGlyphStringLen(cxt, box->text) > viewAddWidth)
            { box->viewBeginIdx += 1; }
            
            inputUse = TRUE;
        }
        if(KeyPressOrRepeat(keyMap::Backspace) && box->text.len > 0 && box->caretIndex > 0) 
        {
            if(box->isSelecting) {
                AssertMsg(FALSE, "Not implemented yet\n");
            }
            
            
            if(box->caretIndex == box->text.len) { ls_unistrTrimRight(&box->text, 1); }
            else { ls_unistrRmIdx(&box->text, box->caretIndex-1); }
            
            box->caretIndex -= 1;
            box->viewEndIdx -= 1;
            if(box->viewBeginIdx != 0) { box->viewBeginIdx -= 1; }
            
            inputUse = TRUE;
        }
        if(KeyPressOrRepeat(keyMap::Delete) && box->text.len > 0 && box->caretIndex < box->text.len)
        {
            if(box->isSelecting) {
                AssertMsg(FALSE, "Not implemented yet\n");
            }
            
            if(box->caretIndex == box->text.len-1) { ls_unistrTrimRight(&box->text, 1); }
            else { ls_unistrRmIdx(&box->text, box->caretIndex); }
            
            if(box->text.len < box->viewEndIdx) { box->viewEndIdx -= 1; }
            
            
            if(box->text.len == 0 && box->isSelecting) { box->isSelecting = FALSE; }
            
            inputUse = TRUE;
        }
        
        if(KeyPressOrRepeat(keyMap::LArrow) && box->caretIndex > 0)
        { 
            if(KeyHeld(keyMap::Shift))
            {
                if(!box->isSelecting) 
                { 
                    box->selectEndIdx   = box->caretIndex;
                    box->selectBeginIdx = box->caretIndex - 1;
                    box->isSelecting    = TRUE;
                }
                else
                {
                    if(box->caretIndex == box->selectBeginIdx)
                    { box->selectBeginIdx -= 1; }
                    else if(box->caretIndex == box->selectEndIdx)
                    { box->selectEndIdx   -= 1;}
                    else
                    { AssertMsg(FALSE, "LArrow -> Caret is not aligned with select anymore\n"); }
                    
                    if(box->selectBeginIdx == box->selectEndIdx) 
                    { box->isSelecting = FALSE; }
                }
            }
            else 
            { if(box->isSelecting) { box->isSelecting = FALSE; box->selectEndIdx = 0; box->selectBeginIdx = 0; } }
            
            box->isCaretOn = TRUE; box->dtCaret = 0; 
            box->caretIndex -= 1;
            if(box->caretIndex < box->viewBeginIdx) { box->viewBeginIdx -= 1; box->viewEndIdx -= 1; }
            
        }
        if(KeyPressOrRepeat(keyMap::RArrow) && box->caretIndex < box->text.len)
        { 
            if(KeyHeld(keyMap::Shift))
            {
                if(!box->isSelecting) 
                { 
                    box->selectEndIdx   = box->caretIndex + 1;
                    box->selectBeginIdx = box->caretIndex;
                    box->isSelecting    = TRUE;
                }
                else
                {
                    if(box->caretIndex == box->selectBeginIdx)
                    { box->selectBeginIdx += 1; }
                    else if(box->caretIndex == box->selectEndIdx)
                    { box->selectEndIdx   += 1;}
                    else
                    { AssertMsg(FALSE, "RArrow -> Caret is not aligned with select anymore\n"); }
                    
                    if(box->selectBeginIdx == box->selectEndIdx) 
                    { box->isSelecting = FALSE; }
                }
            }
            else 
            { if(box->isSelecting) { box->isSelecting = FALSE; box->selectEndIdx = 0; box->selectBeginIdx = 0; } }
            
            box->isCaretOn = TRUE; box->dtCaret = 0; 
            box->caretIndex += 1; 
            if(box->caretIndex > box->viewEndIdx) { box->viewBeginIdx += 1; box->viewEndIdx += 1; }
            
        }
        if(KeyPress(keyMap::Home)) 
        { 
            if(KeyHeld(keyMap::Shift))
            {
                if(!box->isSelecting) 
                { 
                    box->selectEndIdx   = box->caretIndex;
                    box->selectBeginIdx = 0;
                    box->isSelecting    = TRUE;
                }
                else
                {
                    if(box->caretIndex == box->selectBeginIdx)
                    { box->selectBeginIdx = 0; }
                    else if(box->caretIndex == box->selectEndIdx)
                    { box->selectEndIdx   = box->selectBeginIdx; box->selectBeginIdx = 0; }
                    else
                    { AssertMsg(FALSE, "Home -> Caret is not aligned with select anymore\n"); }
                    
                    if(box->selectBeginIdx == box->selectEndIdx) 
                    { box->isSelecting = FALSE; }
                }
            }
            else 
            { if(box->isSelecting) { box->isSelecting = FALSE; box->selectEndIdx = 0; box->selectBeginIdx = 0; } }
            
            box->isCaretOn = TRUE; box->dtCaret = 0; 
            box->caretIndex = 0;
            
            s32 vLen = box->viewEndIdx - box->viewBeginIdx;
            box->viewBeginIdx = 0;
            box->viewEndIdx = vLen;
        }
        if(KeyPress(keyMap::End)) 
        {
            if(KeyHeld(keyMap::Shift))
            {
                if(!box->isSelecting) 
                { 
                    box->selectEndIdx   = box->text.len;
                    box->selectBeginIdx = box->caretIndex;
                    box->isSelecting    = TRUE;
                }
                else
                {
                    if(box->caretIndex == box->selectBeginIdx)
                    { box->selectBeginIdx = box->selectEndIdx; box->selectEndIdx = box->text.len; }
                    else if(box->caretIndex == box->selectEndIdx)
                    { box->selectEndIdx   = box->text.len; }
                    else
                    { AssertMsg(FALSE, "End -> Caret is not aligned with select anymore\n"); }
                    
                    if(box->selectBeginIdx == box->selectEndIdx) 
                    { box->isSelecting = FALSE; }
                }
            }
            else 
            { if(box->isSelecting) { box->isSelecting = FALSE; box->selectEndIdx = 0; box->selectBeginIdx = 0; } }
            
            box->isCaretOn = TRUE; box->dtCaret = 0; 
            box->caretIndex = box->text.len;
            
            s32 vLen = box->viewEndIdx - box->viewBeginIdx;
            box->viewEndIdx = box->caretIndex;
            box->viewBeginIdx = box->viewEndIdx - vLen;
        }
        if(KeyHeld(keyMap::Control) && KeyPress(keyMap::A))
        {
            box->isSelecting    = TRUE;
            box->selectBeginIdx = 0;
            box->selectEndIdx   = box->text.len;
        }
        
        if(KeyHeld(keyMap::Control) && KeyPress(keyMap::V))
        {
            if(box->isSelecting)
            {
                AssertMsg(FALSE, "Not implemented yet.\n");
            }
            
            u32 buff[128] = {};
            u32 copiedLen = GetClipboard(buff, 128);
            
            u32 realCopyLen = copiedLen;
            
            //NOTE: maxLen == 0 means there's no max len.
            if(box->maxLen != 0) 
            {
                if(box->text.len + copiedLen > box->maxLen) {
                    if(box->text.len < box->maxLen) { realCopyLen = box->maxLen - box->text.len; }
                    else { realCopyLen = 0; }
                }
            }
            
            if(box->caretIndex == box->text.len) { ls_unistrAppendBuffer(&box->text, buff, realCopyLen); }
            else { ls_unistrInsertBuffer(&box->text, buff, realCopyLen, box->caretIndex); }
            
            box->caretIndex += copiedLen;
            
            inputUse = TRUE;
        }
        
        if(KeyHeld(keyMap::Control) && KeyPress(keyMap::C))
        { 
            if(box->isSelecting)
            {
                AssertMsg(FALSE, "Not implemented yet.\n");
            }
            
            SetClipboard(box->text.data, box->text.len); 
        }
        
        box->dtCaret += cxt->dt;
        if(box->dtCaret >= 400) { box->dtCaret = 0; box->isCaretOn = !box->isCaretOn; }
        
        
        if(box->postInput) 
        { inputUse |= box->postInput(cxt, box->data); }
    }
    
    RenderCommand command = {UI_RC_TEXTBOX, xPos, yPos, w, h, box, cxt->widgetColor, cxt->textColor};
    ls_uiPushRenderCommand(cxt, command, 0);
    
    return inputUse;
}


enum UIArrowSide { UIA_LEFT, UIA_RIGHT, UIA_UP, UIA_DOWN };
void ls_uiDrawArrow(UIContext *cxt, s32 x, s32 yPos, s32 w, s32 h, Color bkgColor, UIArrowSide s)
{
    UIScissor::UIRect *scRect = cxt->scissor.currRect;
    u32 *At = (u32 *)cxt->drawBuffer;
    
    s32 xPos = x-1;
    
    ls_uiBorderedRect(cxt, xPos, yPos, w, h, bkgColor);
    
    if(s == UIA_DOWN)
    {
        s32 arrowWidth = 8;
        s32 hBearing = (w - arrowWidth)/2;
        s32 xBase = xPos+hBearing;
        s32 xEnd  = xBase + arrowWidth;
        
        s32 arrowHeight = 4;
        s32 vBearing = (h - arrowHeight)/2;
        s32 yStart = (yPos + h - vBearing) - 1;
        s32 yEnd = yStart - arrowHeight;
        
        for(s32 y = yStart; y >= yEnd; y--)
        {
            for(s32 x = xBase; x < xEnd; x++)
            {
                if(x < 0 || x >= cxt->width)  continue;
                if(y < 0 || y >= cxt->height) continue;
                
                if(x < scRect->x || x >= scRect->x+scRect->w) continue;
                if(y < scRect->y || y >= scRect->y+scRect->h) continue;
                
                At[y*cxt->width + x] = cxt->borderColor;
            }
            
            xBase += 1;
            xEnd  -= 1;
        }
    }
    else if(s == UIA_RIGHT)
    {
        s32 arrowWidth  = 4;
        s32 hBearing    = (w - arrowWidth)/2;
        s32 xBase       = xPos + hBearing;
        s32 xEnd        = xBase+1;
        
        s32 arrowHeight = 8;
        s32 vBearing    = (h - arrowHeight)/2;
        s32 yStart1     = (yPos + h - vBearing) - 1;
        s32 yEnd1       = yStart1 - (arrowHeight/2);
        s32 yStart2     = yEnd1;
        s32 yEnd2       = yStart2 - arrowHeight;
        
        for(s32 y = yStart1; y >= yEnd1; y--)
        {
            for(s32 x = xBase; x < xEnd; x++)
            {
                if(x < 0 || x >= cxt->width)  continue;
                if(y < 0 || y >= cxt->height) continue;
                
                if(x < scRect->x || x >= scRect->x+scRect->w) continue;
                if(y < scRect->y || y >= scRect->y+scRect->h) continue;
                
                At[y*cxt->width + x] = RGBg(0x00);
            }
            
            xEnd  += 1;
        }
        
        xEnd -= 1;
        for(s32 y = yStart2; y >= yEnd2; y--)
        {
            for(s32 x = xBase; x < xEnd; x++)
            {
                if(x < 0 || x >= cxt->width)  continue;
                if(y < 0 || y >= cxt->height) continue;
                
                if(x < scRect->x || x >= scRect->x+scRect->w) continue;
                if(y < scRect->y || y >= scRect->y+scRect->h) continue;
                
                At[y*cxt->width + x] = RGBg(0x00);
            }
            
            xEnd  -= 1;
        }
    }
    else if(s == UIA_LEFT)
    {
        s32 arrowWidth  = 4;
        s32 hBearing    = (w - arrowWidth)/2;
        s32 xBase       = xPos + w - hBearing - 1;
        s32 xEnd        = xBase+1;
        
        s32 arrowHeight = 8;
        s32 vBearing    = (h - arrowHeight)/2;
        s32 yStart1     = (yPos + h - vBearing) - 1;
        s32 yEnd1       = yStart1 - (arrowHeight/2);
        s32 yStart2     = yEnd1;
        s32 yEnd2       = yStart2 - arrowHeight;
        
        for(s32 y = yStart1; y >= yEnd1; y--)
        {
            for(s32 x = xBase; x < xEnd; x++)
            {
                if(x < 0 || x >= cxt->width)  continue;
                if(y < 0 || y >= cxt->height) continue;
                
                if(x < scRect->x || x >= scRect->x+scRect->w) continue;
                if(y < scRect->y || y >= scRect->y+scRect->h) continue;
                
                At[y*cxt->width + x] = RGBg(0x00);
            }
            
            xBase -= 1;
        }
        
        xBase += 1;
        for(s32 y = yStart2; y >= yEnd2; y--)
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
        }
    }
}


void ls_uiLPane(UIContext *c, UILPane *pane, s32 xPos, s32 yPos, s32 w, s32 h)
{
    if(pane->isOpen)
    {
        s32 xArrowPos  = xPos + w - 1;
        s32 arrowWidth = 16;
        
        s32 arrowHeight = 14;
        s32 yArrowPos   = yPos + h - arrowHeight;
        
        //TODO: Fix Left Pane Arrow Color
        ls_uiDrawArrow(c, xArrowPos, yArrowPos, arrowWidth, arrowHeight, c->widgetColor, UIA_LEFT);
        
        if(LeftClickIn(xArrowPos, yArrowPos, arrowWidth, arrowHeight))
        { pane->isOpen = FALSE; }
        
        ls_uiBorderedRect(c, xPos-1, yPos, w, h, c->widgetColor);
    }
    else
    {
        s32 width = 0;
        if(pane->isOpening)
        {
            pane->dtOpen += c->dt;
            
            s32 maxDT = 120;
            f64 dtFract = (f64)pane->dtOpen / maxDT;
            width = w*dtFract;
            
            if(pane->dtOpen >= maxDT) { 
                pane->isOpen = TRUE; pane->isOpening = FALSE; pane->dtOpen = 0;
                ls_uiBorderedRect(c, xPos-1, yPos, w, h, c->widgetColor);
            }
            
            if(!pane->isOpen) { ls_uiBorderedRect(c, xPos-1, yPos, width, h, c->widgetColor); }
        }
        
        s32 xArrowPos  = xPos + width;
        s32 arrowWidth = 16;
        
        s32 arrowHeight = 14;
        s32 yArrowPos   = yPos + h - arrowHeight;
        
        ls_uiDrawArrow(c, xArrowPos, yArrowPos, arrowWidth, arrowHeight, c->widgetColor, UIA_RIGHT);
        
        if(LeftClickIn(xArrowPos, yArrowPos, arrowWidth, arrowHeight))
        { pane->isOpening = TRUE; }
    }
}


//NOTETODO: ListBox manages the data of the entry itself, even when a unistring is already passed.
//          This feels strange, and probably error-prone.

//TODO I really think ListBox shouldn't manage memory. Fix This.
inline u32 ls_uiListBoxAddEntry(UIContext *cxt, UIListBox *list, char *s)
{ 
    unistring text = ls_unistrFromAscii(s);
    UIListBoxItem item = { text, cxt->widgetColor, cxt->textColor };
    
    return list->list.push(item);
}

inline u32 ls_uiListBoxAddEntry(UIContext *cxt, UIListBox *list, unistring s)
{
    UIListBoxItem item = { s, cxt->widgetColor, cxt->textColor };
    return list->list.push(item); 
}

inline void ls_uiListBoxRemoveEntry(UIContext *cxt, UIListBox *list, u32 index)
{ 
    //NOTETODO: Is this good????
    if(list->selectedIndex == index) { list->selectedIndex = 0; }
    
    UIListBoxItem val = list->list[index];
    ls_unistrFree(&val.name);
    list->list.remove(index);
}

b32 ls_uiListBox(UIContext *c, UIListBox *list, s32 xPos, s32 yPos, s32 w, s32 h, u32 zLayer = 0)
{
    b32 inputUse = FALSE;
    
    const s32 arrowBoxWidth = 24;
    if(LeftClickIn(xPos+w, yPos, arrowBoxWidth, h) && ls_uiHasCapture(c, 0))
    {
        c->currentFocus = (u64 *)list;
        c->focusWasSetThisFrame = TRUE;
        
        if(list->isOpen) { list->isOpen = FALSE; }
        //else { list->isOpening = TRUE; } //NOTE:TODO: This is because Claudio wanted instant list open. 
        else { list->isOpen = TRUE; }
    }
    
    Color bkgColor = c->widgetColor;
    s32 arrowX = xPos + w - 1;
    if(MouseInRect(arrowX, yPos, arrowBoxWidth, h)) { bkgColor = c->highliteColor; }
    list->arrowBkg = bkgColor;
    
    if(list->isOpening)
    {
        list->dtOpen += c->dt;
        if(list->dtOpen > 70) { list->isOpen = TRUE; list->isOpening = FALSE; list->dtOpen = 0; }
    }
    
    if(ls_uiInFocus(c, list))
    {
        if(list->isOpen)
        {
            for(u32 i = 0; i < list->list.count; i++)
            {
                s32 currY = yPos - (h*(i+1));
                UIListBoxItem *currItem = list->list.getPointer(i);
                
                //TODO: The constant resetting is kinda stupid. Makes me think I should just not
                //      have listbox items with their own colors. Never even used that feature.
                currItem->bkgColor  = c->widgetColor;
                currItem->textColor = c->textColor;
                if(MouseInRect(xPos+1, currY+1, w-2, h-1)) 
                { 
                    currItem->bkgColor = c->highliteColor;
                    if(LeftClick) { 
                        c->mouseCapture = (u64 *)list;
                        
                        list->selectedIndex = i; list->isOpen = FALSE;
                        inputUse = TRUE;
                        if(list->onSelect) { list->onSelect(c, list->data); }
                    }
                    
                    //TODO: Lost the ability to hold because of mouse capture.
                    //      Should be able to regain it if capture is handled deferred?
                    //if(LeftHold) { currItem->bkgColor = cxt->pressedColor; }
                    //if(LeftUp) { ls_printf("here\n");  }
                }
            }
        }
    }
    
    RenderCommand list_command = { UI_RC_LISTBOX, xPos, yPos, w, h };
    list_command.listBox = list;
    ls_uiPushRenderCommand(c, list_command, zLayer);
    
    RenderCommand arr_command = { UI_RC_LISTBOX_ARR, xPos+w, yPos, arrowBoxWidth, h };
    arr_command.listBox = list;
    ls_uiPushRenderCommand(c, arr_command, zLayer);
    
    return inputUse;
}

UISlider ls_uiSliderInit(char32_t *name, s32 maxVal, s32 minVal, f64 currPos, SliderStyle s, Color l, Color r)
{
    UISlider Result = {};
    
    if(name) { Result.text = ls_unistrFromUTF32((const char32_t *)name); }
    else     { Result.text = ls_unistrAlloc(16); }
    
    Result.maxValue = maxVal;
    Result.minValue = minVal;
    Result.currPos  = currPos;
    Result.style    = s;
    Result.lColor   = l;
    Result.rColor   = r;
    
    return Result;
}

void ls_uiSliderChangeValueBy(UIContext *cxt, UISlider *f, s32 valueDiff)
{
    s32 newValue = f->currValue + valueDiff;
    
    if(newValue <= f->minValue) { newValue = f->minValue; }
    if(newValue >= f->maxValue) { newValue = f->maxValue; }
    
    s32 range = (f->maxValue - f->minValue);
    f->currPos = (f64)(newValue - f->minValue) / (f64)range;
    
    return;
}

s32 ls_uiSliderGetValue(UIContext *cxt, UISlider *f)
{
    return ((f->maxValue - f->minValue) * f->currPos) + f->minValue;
}

//TODO: The things are rendered in a logical order, but that makes the function's flow very annoying
//      going in and out of if blocks to check hot/held and style.
b32 ls_uiSlider(UIContext *cxt, UISlider *slider, s32 xPos, s32 yPos, s32 w, s32 h)
{
    if(LeftUp) { slider->isHeld = FALSE; }
    
    if(slider->style == SL_BOX)
    {
        slider->currValue = ((slider->maxValue - slider->minValue) * slider->currPos) + slider->minValue;
        s32 slidePos = w*slider->currPos;
        
        if(MouseInRect(xPos + slidePos-5, yPos, 10, h) && !(cxt->mouseCapture != 0 && cxt->mouseCapture != (u64 *)slider))
        {
            slider->isHot = TRUE;
            if(LeftHold) { slider->isHeld = TRUE; cxt->mouseCapture = (u64 *)slider; }
        }
    }
    else if(slider->style == SL_LINE)
    { AssertMsg(FALSE, "Slider style line is not implemented\n"); }
    
    b32 hasAnsweredToInput = FALSE;
    
    if(slider->isHeld) { 
        s32 deltaX = (UserInput.Mouse.prevPos.x - UserInput.Mouse.currPos.x);//*cxt->dt;
        
        f64 fractionMove = (f64)deltaX / (f64)w;
        
        slider->currPos -= fractionMove;
        
        slider->currPos = ls_mathClamp(slider->currPos, 1.0, 0.0);
        
        hasAnsweredToInput = TRUE;
    }
    
    RenderCommand command = { UI_RC_SLIDER, xPos, yPos, w, h };
    command.slider = slider;
    
    ls_uiPushRenderCommand(cxt, command, 0);
    
    return hasAnsweredToInput;
}

struct __UImenuDataPass { UIMenu *menu; s32 idx; };


b32 ls_uiMenuDefaultOnClick(UIContext *cxt, void *data)
{
    b32 inputUse = FALSE;
    
    __UImenuDataPass *pass = (__UImenuDataPass *)data;
    
    UIMenu *menu  = pass->menu;
    
    menu->isOpen  = !menu->isOpen;
    menu->openIdx = pass->idx;
    
    return inputUse;
}

void ls_uiMenuAddItem(UIMenu *menu, UIButton b)
{
    if(b.onClick == ls_uiMenuDefaultOnClick)
    {
        //TODO: I hate having to allocate data for this. I also need a way to deallocate it!
        //      @Leak @Memory
        __UImenuDataPass *pass = (__UImenuDataPass *)ls_alloc(sizeof(__UImenuDataPass));
        pass->menu = menu;
        pass->idx = menu->items.count;
        b.data = pass;
    }
    
    menu->items.push(b);
}

//TODO This is completely unnecessary
void ls_uiMenuAddSub(UIMenu *menu, UIMenu sub, s32 idx)
{ menu->sub[idx] = sub; }

//TODO: @MenuIsShit it shouldn't use buttons like this, and shouldn't hold the close button hostage.
b32 ls_uiMenu(UIContext *c, UIMenu *menu, s32 x, s32 y, s32 w, s32 h)
{
    b32 inputUse = FALSE;
    
    if(LeftClickIn(x, y, w, h)) {
        c->currentFocus = (u64 *)menu;
        c->focusWasSetThisFrame = TRUE;
    }
    
    if(c->currentFocus != (u64 *)menu) { menu->isOpen = FALSE; }
    
    RenderCommand commandFirst = { UI_RC_RECT, x, y, w, h };
    commandFirst.bkgColor  = c->backgroundColor;
    commandFirst.textColor = RGBg(110); //TODO: Hardcoded
    ls_uiPushRenderCommand(c, commandFirst, 1);
    
    ls_uiButton(c, &menu->closeWindow, x+w-20, y+2, 16, 16, 2);
    
    s32 xOff = 100;
    for(u32 i = 0; i < menu->items.count; i++)
    {
        UIButton *currItem = &menu->items[i];
        ls_uiButton(c, currItem, x+xOff, y+1, 100, 19, 2);
        xOff += 100;
    }
    
    if(menu->isOpen == TRUE)
    {
        UIButton *openItem = &menu->items[menu->openIdx];
        UIMenu *openSub = menu->sub + menu->openIdx;
        
        AssertMsg(openItem, "The item doesn't exist\n");
        AssertMsg(openSub, "The sub-menu doesn't exist\n");
        
        s32 yPos      = y-20;
        s32 height = openSub->items.count*20;
        
        RenderCommand command = { UI_RC_RECT, 100 + x + 100*menu->openIdx, y-height-2, 102, height+3 };
        command.bkgColor  = c->backgroundColor;
        command.textColor = c->borderColor;
        ls_uiPushRenderCommand(c, command, 1);
        
        //TODO: Store the xPos of every menu item.
        for(u32 i = 0; i < openSub->items.count; i++)
        {
            UIButton *currItem = &openSub->items[i];
            inputUse = ls_uiButton(c, currItem, 100 + 100*menu->openIdx, yPos, 100, 20, 2);
            yPos -= 21;
        }
    }
    
    return inputUse;
}

void ls_uiRender(UIContext *c)
{
    if(THREAD_COUNT == 0)
    {
        ls_uiRender__(c, 0);
        c->renderFunc();
        return;
    }
    
    WakeAllConditionVariable(&c->startRender);
    
    volatile b32 areAllDone = FALSE;
    while(areAllDone == FALSE)
    {
        volatile b32 allDone = TRUE;
        for(u32 i = 0; i < THREAD_COUNT; i++)
        {
            allDone &= c->renderGroups[i].isDone;
        }
        
        areAllDone = allDone;
    }
    
    for(u32 i = 0; i < THREAD_COUNT; i++)
    {
        c->renderGroups[i].isDone = FALSE;
    }
    
    c->renderFunc();
}

void ls_uiRender__(UIContext *c, u32 threadID)
{
    //NOTE: Render Layers in Z-order. Layer Zero is the first to be rendered, 
    //      so it's the one farther away from the screen
    for(u32 zLayer = 0; zLayer < UI_Z_LAYERS; zLayer++)
    {
        stack *currLayer = c->renderGroups[threadID].RenderCommands + zLayer;
        
        s32 count = currLayer->count;
        for(u32 commandIdx = 0; commandIdx < count; commandIdx++)
        {
            RenderCommand *curr = (RenderCommand *)ls_stackPop(currLayer);
            s32 xPos        = curr->x;
            s32 yPos        = curr->y;
            s32 w           = curr->w;
            s32 h           = curr->h;
            
            Color bkgColor  = curr->bkgColor;
            Color textColor = curr->textColor;
            
            switch(curr->type)
            {
                case UI_RC_TEXTBOX:
                {
                    UITextBox *box = curr->textBox;
                    
                    Color caretColor = textColor;
                    const s32 horzOff = 4;
                    
                    //TODO: Make the font selection more of a global thing??
                    s32 strPixelHeight = ls_uiSelectFontByFontSize(c, FS_SMALL);
                    
                    ls_uiBorderedRect(c, xPos, yPos, w, h, bkgColor);
                    
                    s32 vertOff = ((h - strPixelHeight) / 2) + 5; //TODO: @FontDescent
                    u32 viewLen = box->viewEndIdx - box->viewBeginIdx;
                    u32 actualViewLen = viewLen <= box->text.len ? viewLen : box->text.len;
                    unistring viewString = {box->text.data + box->viewBeginIdx, actualViewLen, actualViewLen};
                    
                    //NOTE: Finally draw the entire string.
                    ls_uiGlyphString(c, xPos + horzOff, yPos + vertOff, viewString, textColor);
                    
                    if(box->isSelecting)
                    {
                        //TODO: Draw this more efficiently by drawing text in 3 different non-overlapping calls??
                        s32 viewSelBegin = box->selectBeginIdx;
                        if(viewSelBegin <= box->viewBeginIdx) { viewSelBegin = box->viewBeginIdx; }
                        
                        s32 viewSelEnd = box->selectEndIdx;
                        if(box->viewEndIdx != 0 && viewSelEnd >= box->viewEndIdx) { viewSelEnd = box->viewEndIdx; }
                        
                        u32 selLen = viewSelEnd -  viewSelBegin;
                        unistring selString = {box->text.data + viewSelBegin, selLen, selLen};
                        s32 selStringWidth  = ls_uiGlyphStringLen(c, selString);
                        
                        u32 diffLen = 0;
                        if(box->selectBeginIdx > box->viewBeginIdx) { diffLen = box->selectBeginIdx - box->viewBeginIdx; }
                        
                        unistring diffString = { box->text.data + box->viewBeginIdx, diffLen, diffLen };
                        s32 diffStringWidth = ls_uiGlyphStringLen(c, diffString);
                        
                        ls_uiFillRect(c, xPos + horzOff + diffStringWidth, yPos+1, selStringWidth, h-2, c->invWidgetColor);
                        ls_uiGlyphString(c, xPos + horzOff + diffStringWidth, yPos + vertOff, selString, c->invTextColor);
                        
                        if(box->caretIndex == box->selectBeginIdx)
                        { caretColor = c->invTextColor; }
                    }
                    
                    //NOTE: Draw the Caret
                    if(box->isCaretOn && c->currentFocus == (u64 *)box)
                    {
                        UIGlyph *caretGlyph = &c->currFont->glyph['|'];
                        
                        u32 caretIndexInView = box->caretIndex - box->viewBeginIdx;
                        unistring tmp = {viewString.data, caretIndexInView, caretIndexInView};
                        
                        u32 stringLen = ls_uiGlyphStringLen(c, tmp);
                        
                        const s32 randffset = 4; //TODO: Maybe try to remove this?
                        ls_uiGlyph(c, xPos + horzOff + stringLen - randffset, yPos+vertOff, caretGlyph, caretColor);
                    }
                    
                } break;
                
                case UI_RC_LISTBOX:
                {
                    UIListBox *list = curr->listBox;
                    
                    s32 strHeight = ls_uiSelectFontByFontSize(c, FS_SMALL);
                    s32 vertOff = ((h - strHeight) / 2) + 4; //TODO: @FontDescent
                    
                    ls_uiBorderedRect(c, xPos, yPos, w, h);
                    
                    if(list->list.count)
                    {
                        unistring selected = list->list[list->selectedIndex].name;
                        ls_uiGlyphString(c, xPos+10, yPos + vertOff, selected, c->textColor);
                    }
                    
                    
                    s32 maxHeight = (list->list.count)*h;
                    if(list->isOpening)
                    {
                        s32 height = 0;
                        if(list->dtOpen > 17)  { height = maxHeight*0.10f; }
                        if(list->dtOpen > 34)  { height = maxHeight*0.35f; }
                        if(list->dtOpen > 52)  { height = maxHeight*0.70f; }
                        
                        if(!list->isOpen)
                        { ls_uiFillRect(c, xPos+1, yPos-height, w-2, height, c->widgetColor); }
                    }
                    
                    if(list->isOpen)
                    {
                        for(u32 i = 0; i < list->list.count; i++)
                        {
                            s32 currY = yPos - (h*(i+1));
                            UIListBoxItem *currItem = list->list.getPointer(i);
                            
                            ls_uiRect(c, xPos+1, currY, w-2, h, currItem->bkgColor);
                            ls_uiGlyphString(c, xPos+10, yPos + vertOff - (h*(i+1)),
                                             currItem->name, currItem->textColor);
                            
                        }
                        
                        ls_uiBorder(c, xPos, yPos-maxHeight, w, maxHeight+1);
                    }
                    
                } break;
                
                case UI_RC_LISTBOX_ARR:
                {
                    ls_uiDrawArrow(c, xPos, yPos, w, h, curr->listBox->arrowBkg, UIA_DOWN);
                } break;
                
                case UI_RC_BUTTON:
                {
                    UIButton *button = curr->button;
                    
                    s32 strHeight = ls_uiSelectFontByFontSize(c, FS_SMALL);
                    
                    if(button->style == UIBUTTON_TEXT)
                    {
                        ls_uiBorderedRect(c, xPos, yPos, w, h, bkgColor);
                        
                        s32 strWidth = ls_uiGlyphStringLen(c, button->name);
                        s32 xOff      = (w - strWidth) / 2; //TODO: What happens when the string is too long?
                        s32 strHeight = c->currFont->pixelHeight;
                        s32 yOff      = strHeight*0.25; //TODO: @FontDescent
                        
                        ls_uiGlyphString(c, xPos+xOff, yPos+yOff, button->name, c->textColor);
                    }
                    else if(button->style == UIBUTTON_TEXT_NOBORDER)
                    {
                        ls_uiRect(c, xPos, yPos, w, h, bkgColor);
                        
                        s32 strWidth = ls_uiGlyphStringLen(c, button->name);
                        s32 xOff      = (w - strWidth) / 2; //TODO: What happens when the string is too long?
                        s32 strHeight = c->currFont->pixelHeight;
                        s32 yOff      = strHeight*0.25; //TODO: @FontDescent
                        
                        ls_uiGlyphString(c, xPos+xOff, yPos+yOff, button->name, c->textColor);
                    }
                    else if(button->style == UIBUTTON_BMP)
                    {
                        ls_uiBitmap(c, xPos, yPos, (u32 *)button->bmpData, button->bmpW, button->bmpH);
                    }
                    else { AssertMsg(FALSE, "Unhandled button style"); }
                    
                } break;
                
                case UI_RC_SLIDER:
                {
                    UISlider *slider = curr->slider;
                    
                    //NOTE: Box Slider Branchless Opacity Check
                    //u8 opacity = 0xEE - (0xB0*slider->isHeld);
                    u8 opacity = 0xC0 - (0xB0*slider->isHeld);
                    
                    if(slider->style == SL_BOX)
                    {
                        s32 slideWidth = 3;
                        
                        ls_uiBorder(c, xPos, yPos, w, h);
                        
                        s32 slidePos = w*slider->currPos;
                        
                        ls_uiFillRect(c, xPos+1, yPos+1, slidePos, h-2, slider->lColor);
                        ls_uiFillRect(c, xPos+slidePos, yPos+1, w-slidePos-1, h-2, slider->rColor);
                        
                        unistring val = ls_unistrFromInt(slider->currValue);
                        
                        s32 strHeight = ls_uiSelectFontByFontSize(c, FS_SMALL);
                        
                        u32 textLen = ls_uiGlyphStringLen(c, val);
                        s32 strXPos = xPos + slidePos - textLen - 2;
                        Color textBkgC = slider->lColor;
                        
                        if(strXPos < xPos+1) { strXPos = xPos + slidePos + slideWidth + 2; textBkgC = slider->rColor; }
                        
                        Color valueColor = c->borderColor;
                        u8 alpha = 0x00 + (slider->isHeld*0xFF);
                        valueColor = SetAlpha(valueColor, alpha);
                        ls_uiGlyphString(c, strXPos, yPos + h - strHeight, val, valueColor);
                        
                        ls_unistrFree(&val);
                        
                        if(slider->isHot)
                        {
                            s32 actualX = (xPos + slidePos) - 1;
                            s32 actualY = yPos - 2;
                            
                            s32 actualWidth  = slideWidth+2;
                            s32 actualHeight = 4 + h;
                            
                            ls_uiFillRect(c, actualX, actualY, actualWidth, actualHeight, c->borderColor);
                        }
                        else
                        {
                            ls_uiFillRect(c, xPos+slidePos, yPos, slideWidth, h, c->borderColor);
                        }
                        
                    }
                    else if(slider->style == SL_LINE)
                    { AssertMsg(FALSE, "Slider style line is not implemented\n"); }
                    
                    //NOTE: Draw the displayed text, and hide through Alpha the slider info.
                    
                    Color rectColor = c->widgetColor;
                    rectColor = SetAlpha(rectColor, opacity);
                    ls_uiBorderedRect(c, xPos, yPos, w, h, rectColor);
                    
                    s32 strHeight = ls_uiSelectFontByFontSize(c, FS_SMALL);
                    
                    s32 strWidth  = ls_uiGlyphStringLen(c, slider->text);
                    s32 xOff      = (w - strWidth) / 2;
                    s32 yOff      = (h - strHeight) + 3; //TODO: @FontDescent
                    
                    Color textColor = c->textColor;
                    textColor = SetAlpha(textColor, opacity);
                    
                    ls_uiGlyphString(c, xPos+xOff, yPos + yOff, slider->text, textColor);
                    
                    //NOTETODO: The isHot is a hack to grow the slider as long as
                    //          the mouse is on top of it. Is it fine for logic to be here in render?
                    slider->isHot = FALSE;
                    
                } break;
                
                case UI_RC_RECT:
                {
                    //NOTE: here current text color is being used improperly for the border color
                    ls_uiBorderedRect(c, xPos, yPos, w, h, c->backgroundColor, curr->textColor);
                } break;
                
                case UI_RC_FRAG_BUTTON:
                {
                    UIButton *button       = curr->button;
                    RenderCommandExtra rce = curr->extra;
                    
                    s32 strHeight = ls_uiSelectFontByFontSize(c, FS_SMALL);
                    
                    if(button->style == UIBUTTON_TEXT)
                    {
                        ls_uiBorderedRectFrag(c, xPos, yPos, w, h, bkgColor, rce);
                        
                        s32 strWidth = ls_uiGlyphStringLen(c, button->name);
                        s32 xOff     = (curr->oW - strWidth) / 2; //TODO: What happens when string is too long?
                        s32 strHeight = c->currFont->pixelHeight;
                        s32 yOff      = strHeight*0.25; //TODO: @FontDescent
                        
                        ls_uiGlyphStringFrag(c, curr->oX+xOff, yPos+yOff, curr->oX, curr->oY,
                                             xPos, yPos, xPos+w, yPos+h, button->name, c->textColor);
                    }
                    else if(button->style == UIBUTTON_TEXT_NOBORDER)
                    {
                        ls_uiRect(c, xPos, yPos, w, h, bkgColor);
                        
                        s32 strWidth = ls_uiGlyphStringLen(c, button->name);
                        s32 xOff      = (curr->oW - strWidth) / 2; //TODO: What happens when the string is too long?
                        s32 strHeight = c->currFont->pixelHeight;
                        s32 yOff      = strHeight*0.25; //TODO: @FontDescent
                        
                        ls_uiGlyphStringFrag(c, curr->oX+xOff, yPos+yOff, curr->oX, curr->oY,
                                             xPos, yPos, xPos+w, yPos+h, button->name, c->textColor);
                    }
                    else if(button->style == UIBUTTON_BMP)
                    {
                        AssertMsg(FALSE, "button style BMP is not implemented yet.");
                        ls_uiBitmap(c, xPos, yPos, (u32 *)button->bmpData, button->bmpW, button->bmpH);
                    }
                    else { AssertMsg(FALSE, "Unhandled button style"); }
                    
                } break;
                
                case UI_RC_FRAG_TEXTBOX:
                {
                    UITextBox *box = curr->textBox;
                    
                    Color caretColor = textColor;
                    const s32 horzOff = 4;
                    
                    //TODO: Make the font selection more of a global thing??
                    s32 strPixelHeight = ls_uiSelectFontByFontSize(c, FS_SMALL);
                    
                    ls_uiBorderedRectFrag(c, xPos, yPos, w, h, bkgColor, curr->extra);
                    
                    s32 vertOff = ((h - strPixelHeight) / 2) + 5; //TODO: @FontDescent
                    u32 viewLen = box->viewEndIdx - box->viewBeginIdx;
                    u32 actualViewLen = viewLen <= box->text.len ? viewLen : box->text.len;
                    unistring viewString = {box->text.data + box->viewBeginIdx, actualViewLen, actualViewLen};
                    
                    //NOTE: Finally draw the entire string.
                    ls_uiGlyphStringFrag(c, curr->oX + horzOff, yPos + vertOff, curr->oX, curr->oY, 
                                         xPos, yPos, xPos+w, yPos+h, viewString, textColor);
                    
                    if(box->isSelecting)
                    {
                        //TODO: Draw this more efficiently by drawing text in 3 different non-overlapping calls??
                        s32 viewSelBegin = box->selectBeginIdx;
                        if(viewSelBegin <= box->viewBeginIdx) { viewSelBegin = box->viewBeginIdx; }
                        
                        s32 viewSelEnd = box->selectEndIdx;
                        if(box->viewEndIdx != 0 && viewSelEnd >= box->viewEndIdx) { viewSelEnd = box->viewEndIdx; }
                        
                        u32 selLen = viewSelEnd -  viewSelBegin;
                        unistring selString = {box->text.data + viewSelBegin, selLen, selLen};
                        s32 selStringWidth  = ls_uiGlyphStringLen(c, selString);
                        
                        u32 diffLen = 0;
                        if(box->selectBeginIdx > box->viewBeginIdx) 
                        { diffLen = box->selectBeginIdx - box->viewBeginIdx; }
                        
                        unistring diffString = { box->text.data + box->viewBeginIdx, diffLen, diffLen };
                        s32 diffStringWidth = ls_uiGlyphStringLen(c, diffString);
                        
                        //TODO: Fragged FillRect done better???
                        s32 startX = curr->oX + horzOff + diffStringWidth;
                        if((curr->extra == UI_RCE_LEFT) && (startX < (xPos + w)))
                        {
                            s32 fragWidth = selStringWidth;
                            if((startX + selStringWidth) > (xPos + w)) { fragWidth = (xPos + w) - startX; }
                            
                            ls_uiFillRect(c, startX, curr->oY+1, fragWidth, h-2, c->invWidgetColor);
                        }
                        else if((curr->extra == UI_RCE_RIGHT) && (startX+selStringWidth > xPos))
                        {
                            if(startX < xPos)
                            {
                                s32 fragWidth = selStringWidth - (xPos - startX);
                                ls_uiFillRect(c, xPos, curr->oY+1, fragWidth, h-2, c->invWidgetColor);
                            }
                            else
                            {
                                s32 fragWidth = selStringWidth;
                                ls_uiFillRect(c, startX, curr->oY+1, fragWidth, h-2, c->invWidgetColor);
                            }
                        }
                        
                        ls_uiGlyphStringFrag(c, curr->oX + horzOff + diffStringWidth, curr->oY + vertOff, curr->oX, curr->oY, xPos, yPos, xPos+w, yPos+h, selString, c->invTextColor);
                        
                        if(box->caretIndex == box->selectBeginIdx)
                        { caretColor = c->invTextColor; }
                    } 
                    
                    //NOTE: Draw the Caret
                    if(box->isCaretOn && c->currentFocus == (u64 *)box)
                    {
                        UIGlyph *caretGlyph = &c->currFont->glyph['|'];
                        
                        u32 caretIndexInView = box->caretIndex - box->viewBeginIdx;
                        unistring tmp = {viewString.data, caretIndexInView, caretIndexInView};
                        
                        u32 stringLen = ls_uiGlyphStringLen(c, tmp);
                        const s32 randffset = 4; //TODO: Maybe try to remove this?
                        
                        s32 finalXPos = curr->oX + horzOff + stringLen - randffset;
                        
                        ls_uiGlyphFrag(c, finalXPos, curr->oY+vertOff,
                                       curr->oX, curr->oY, xPos, yPos, xPos+w, yPos+h, caretGlyph, caretColor);
                    }
                    
                } break;
                
                case UI_RC_FRAG_SLIDER:
                {
                    UISlider *slider = curr->slider;
                    
                    //NOTE: Box Slider Branchless Opacity Check
                    //u8 opacity = 0xEE - (0xB0*slider->isHeld);
                    u8 opacity = 0xC0 - (0xB0*slider->isHeld);
                    
                    if(slider->style == SL_BOX)
                    {
                        s32 slideWidth = 3;
                        
                        ls_uiBorderFrag(c, xPos, yPos, w, h, curr->extra);
                        
                        s32 slidePos  = curr->oW*slider->currPos;
                        
                        if(curr->extra == UI_RCE_LEFT)
                        {
                            s32 startX = curr->oX+1;
                            s32 fragW  = (xPos+w) < curr->oX+slidePos ? w : slidePos;
                            
                            ls_uiFillRect(c, startX, curr->oY+1, fragW-1, h-2, slider->lColor);
                            
                            if((curr->oX+slidePos) < (xPos+w))
                            {
                                ls_uiFillRect(c, curr->oX+slidePos, curr->oY+1, w-slidePos, h-2, slider->rColor);
                            }
                        }
                        else if(curr->extra == UI_RCE_RIGHT)
                        {
                            if((curr->oX+slidePos) > (xPos+1))
                            {
                                s32 fragWLeft = curr->oX + slidePos - xPos;
                                ls_uiFillRect(c, xPos, curr->oY+1, fragWLeft, h-2, slider->lColor);
                            }
                            
                            s32 startX     = xPos > (curr->oX+slidePos) ? xPos : (curr->oX+slidePos);
                            s32 fragWRight = xPos > (curr->oX+slidePos) ? w : (w - (curr->oX+slidePos - xPos));
                            ls_uiFillRect(c, startX, curr->oY+1, fragWRight-1, h-2, slider->rColor);
                        }
                        
                        unistring val = ls_unistrFromInt(slider->currValue);
                        
                        s32 strHeight = ls_uiSelectFontByFontSize(c, FS_SMALL);
                        
                        u32 textLen = ls_uiGlyphStringLen(c, val);
                        s32 strXPos = curr->oX + slidePos - textLen - 2;
                        Color textBkgC = slider->lColor;
                        
                        if(strXPos < curr->oX+1)
                        { strXPos = curr->oX + slidePos + slideWidth + 2; textBkgC = slider->rColor; }
                        
                        Color valueColor = c->borderColor;
                        u8 alpha = 0x00 + (slider->isHeld*0xFF);
                        valueColor = SetAlpha(valueColor, alpha);
                        ls_uiGlyphStringFrag(c, strXPos, curr->oY + h - strHeight, curr->oX, curr->oY, xPos, yPos, xPos+w, yPos+h, val, valueColor);
                        
                        ls_unistrFree(&val);
                        
                        s32 actualX = (curr->oX + slidePos) - 1;
                        s32 actualY = curr->oY - 2;
                        
                        s32 actualWidth  = slideWidth+2;
                        s32 actualHeight = 4 + curr->oH;
                        
                        if(slider->isHot)
                        {
                            if((actualX-1) < (xPos+w))
                            { ls_uiFillRect(c, actualX-1, actualY, actualWidth+2, actualHeight, c->borderColor); }
                            
                        }
                        else
                        {
                            if(actualX < (xPos+w))
                            { ls_uiFillRect(c, actualX, curr->oY, actualWidth, curr->oH, c->borderColor); }
                        }
                        
                    }
                    else if(slider->style == SL_LINE)
                    { AssertMsg(FALSE, "Slider style line is not implemented\n"); }
                    
                    //NOTE: Draw the displayed text, and hide through Alpha the slider info.
                    
                    Color rectColor = c->widgetColor;
                    rectColor = SetAlpha(rectColor, opacity);
                    
                    if(curr->extra == UI_RCE_LEFT)       { ls_uiFillRect(c, xPos+1, yPos+1, w-1, h-2, rectColor); }
                    else if(curr->extra == UI_RCE_RIGHT) { ls_uiFillRect(c, xPos,   yPos+1, w-1, h-2, rectColor); }
                    
                    s32 strHeight = ls_uiSelectFontByFontSize(c, FS_SMALL);
                    
                    s32 strWidth  = ls_uiGlyphStringLen(c, slider->text);
                    s32 xOff      = (curr->oW - strWidth) / 2;
                    s32 yOff      = (curr->oH - strHeight) + 3; //TODO: @FontDescent
                    
                    Color textColor = c->textColor;
                    textColor = SetAlpha(textColor, opacity);
                    
                    ls_uiGlyphStringFrag(c, curr->oX+xOff, curr->oY + yOff, curr->oX, curr->oY, xPos, yPos, xPos+w, yPos+h, slider->text, textColor);
                    
                    //NOTETODO: The isHot is a hack to grow the slider as long as
                    //          the mouse is on top of it. Is it fine for logic to be here in render?
                    slider->isHot = FALSE;
                    
                } break;
                
                case UI_RC_FRAG_LISTBOX:
                {
                    UIListBox *list = curr->listBox;
                    
                    s32 strHeight = ls_uiSelectFontByFontSize(c, FS_SMALL);
                    s32 vertOff = ((curr->oH - strHeight) / 2) + 4; //TODO: @FontDescent
                    
                    ls_uiBorderedRectFrag(c, xPos, yPos, w, h, curr->extra);
                    
                    if(list->list.count)
                    {
                        unistring selected = list->list[list->selectedIndex].name;
                        ls_uiGlyphStringFrag(c, curr->oX+10, curr->oY + vertOff, curr->oX, curr->oY, xPos, yPos, xPos+w, yPos+h, selected, c->textColor);
                    }
                    
                    s32 maxHeight = (list->list.count)*h;
                    if(list->isOpening)
                    {
                        s32 height = 0;
                        if(list->dtOpen > 17)  { height = maxHeight*0.10f; }
                        if(list->dtOpen > 34)  { height = maxHeight*0.35f; }
                        if(list->dtOpen > 52)  { height = maxHeight*0.70f; }
                        
                        if(!list->isOpen)
                        { 
                            if(curr->extra == UI_RCE_LEFT)
                            { ls_uiFillRect(c, xPos+1, yPos-height, w, height, c->widgetColor); }
                            else if(curr->extra == UI_RCE_RIGHT)
                            { ls_uiFillRect(c, xPos, yPos-height, w-1, height, c->widgetColor); }
                        }
                    }
                    
                    if(list->isOpen)
                    {
                        for(u32 i = 0; i < list->list.count; i++)
                        {
                            s32 iOff = (h*(i+1));
                            s32 currY = yPos - iOff;
                            UIListBoxItem *currItem = list->list.getPointer(i);
                            
                            ls_uiFillRect(c, xPos, currY, w, h, currItem->bkgColor);
                            
                            ls_uiGlyphStringFrag(c, curr->oX+10, curr->oY + vertOff - iOff,
                                                 curr->oX, curr->oY - iOff, xPos, yPos - iOff, xPos+w, yPos+h,
                                                 currItem->name, currItem->textColor);
                            
                        }
                        
                        ls_uiBorderFrag(c, xPos, yPos-maxHeight, w, maxHeight+1, curr->extra);
                    }
                    
                } break;
                
                case UI_RC_FRAG_LISTBOX_ARR:
                {
                    UIScissor::UIRect *scRect = c->scissor.currRect;
                    u32 *At = (u32 *)c->drawBuffer;
                    
                    s32 arrowX = curr->oX - 1;
                    
                    ls_uiBorderedRectFrag(c, xPos-1, yPos, w, h, curr->listBox->arrowBkg, curr->extra);
                    
                    s32 arrowWidth = 8;
                    s32 hBearing = (curr->oW - arrowWidth)/2;
                    s32 xBase = arrowX + hBearing;
                    s32 xEnd  = xBase + arrowWidth;
                    
                    s32 arrowHeight = 4;
                    s32 vBearing = (curr->oH - arrowHeight)/2;
                    s32 yStart = (curr->oY + curr->oH - vBearing) - 1;
                    s32 yEnd = yStart - arrowHeight;
                    
                    for(s32 y = yStart; y >= yEnd; y--)
                    {
                        for(s32 x = xBase; x < xEnd; x++)
                        {
                            if((x < xPos-1) || (x > xPos+w)) continue;
                            if((y < yPos)   || (y > yPos+h)) continue;
                            
                            if(x < 0 || x >= c->width)  continue;
                            if(y < 0 || y >= c->height) continue;
                            
                            if(x < scRect->x || x >= scRect->x+scRect->w) continue;
                            if(y < scRect->y || y >= scRect->y+scRect->h) continue;
                            
                            At[y*c->width + x] = c->borderColor;
                        }
                        
                        xBase += 1;
                        xEnd  -= 1;
                    }
                    
                } break;
                
                case UI_RC_FRAG_RECT:
                {
                    //NOTE: here current text color is being used improperly for the border color
                    ls_uiBorderedRectFrag(c, xPos, yPos, w, h, c->backgroundColor, curr->textColor, curr->extra);
                } break;
                
                default: { AssertMsg(FALSE, "Unhandled Render Command Type\n"); } break;
            }
        }
    }
    
    c->renderGroups[threadID].isDone = TRUE;
    
    //c->renderFunc();
    
    return;
}

