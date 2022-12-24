Order *GetOrderByID(s32 ID)
{
    s32 visibleMobs   = State.Init->Mobs.selectedIndex;
    s32 visibleAllies = State.Init->Allies.selectedIndex;
    s32 visibleOrder  = visibleMobs + visibleAllies + PARTY_NUM - State.Init->orderAdjust;
    
    for(u32 i = 0; i < visibleOrder; i++)
    {
        Order *o = State.Init->OrderFields + i;
        if(o->ID == ID) { return o; }
    }
    
    return NULL;
}

InitField *GetInitFieldByID(s32 ID)
{
    s32 visibleMobs   = State.Init->Mobs.selectedIndex;
    s32 visibleAllies = State.Init->Allies.selectedIndex;
    
    for(u32 i = 0; i < visibleMobs; i++)
    {
        InitField *m = State.Init->MobFields + i;
        if(m->ID == ID) { return m; }
    }
    
    for(u32 i = 0; i < visibleAllies; i++)
    {
        InitField *a = State.Init->AllyFields + i;
        if(a->ID == ID) { return a; }
    }
    
    return NULL;
}

b32 selectStyleDefault(UIContext *c, void *data)
{
    currentStyle = INIT_STYLE_DEFAULT;
    return FALSE;
}

b32 selectStylePrana(UIContext *c, void *data)
{
    currentStyle = INIT_STYLE_PRANA;
    return FALSE;
}


b32 selectThemeDefault(UIContext *c, void *data)
{
    c->backgroundColor = RGBg(0x38);
    c->highliteColor   = RGBg(0x65);
    c->pressedColor    = RGBg(0x75);
    c->widgetColor     = RGBg(0x45);
    c->borderColor     = RGBg(0x22);
    c->textColor       = RGBg(0xCC);
    c->invWidgetColor  = RGBg(0xBA);
    c->invTextColor    = RGBg(0x33);
    
    return FALSE;
}

b32 selectThemeDarkNight(UIContext *c, void *data)
{
    c->backgroundColor = RGB(0x0C, 0x08, 0x0B);
    c->highliteColor   = RGB(0x14, 0x15, 0x13);
    c->pressedColor    = RGB(0x18, 0x20, 0x19);
    c->widgetColor     = RGB(0x1C, 0x18, 0x1B);
    c->borderColor     = RGB(0x60, 0x7B, 0x7D);
    c->textColor       = RGBg(0xCC);
    c->invWidgetColor  = RGB(0xE3, 0xE7, 0xE4);
    c->invTextColor    = RGB(0x7D, 0x71, 0x7D);
    
    return FALSE;
}


b32 CustomPlayerText(UIContext *c, void *data)
{
    Input *UserInput = &c->UserInput;
    b32 inputUse = FALSE;
    
    InitPage *Page = State.Init;
    
    UITextBox *f = (UITextBox *)data;
    
    if(c->lastFocus != (u64 *)f) { ls_uiTextBoxClear(c, f); inputUse = TRUE; }
    
    if(KeyPress(keyMap::Enter))
    {
        UITextBox *lastPlayerBox = Page->PlayerInit + (PARTY_NUM - 1);
        
        if(f == lastPlayerBox) { ls_uiFocusChange(c, 0x0); return TRUE; }
        
        ls_uiFocusChange(c, (u64 *)(f + 1));
        return TRUE;
    }
    
    return inputUse;
}

b32 CustomInitFieldText(UIContext *c, void *data)
{
    Input *UserInput = &c->UserInput;
    b32 inputUse = FALSE;
    
    CustomFieldTextHandler *f = (CustomFieldTextHandler *)data;
    
    InitPage *Page = State.Init;
    
    if((c->lastFocus != (u64 *)f->field) && (c->currentFocus == (u64 *)f->field))
    { ls_uiTextBoxClear(c, f->field); inputUse = TRUE; }
    
    InitField *lastMob  = State.Init->MobFields  + (MOB_NUM-1);
    InitField *lastAlly = State.Init->AllyFields + (ALLY_NUM-1);
    
    if(KeyPress(keyMap::Enter))
    {
        //NOTE: Clear up the '\n' printable key
        ClearPrintableKey();
        
        if(f->parent == lastMob)  { ls_uiFocusChange(c, 0x0); return inputUse; }
        if(f->parent == lastAlly) { ls_uiFocusChange(c, 0x0); return inputUse; }
        
        //NOTE: The maxLife field gets handled specially using the index 999
        if(f->if_idx == 999) {
            UITextBox *next = &((f->parent + 1)->maxLife);
            ls_uiTextBoxClear(c, next);
            ls_uiFocusChange(c, (u64 *)next);
            
            return TRUE;
        }
        
        AssertMsg(f->if_idx < INIT_FIELD_EDITFIELDS_NUM, "Out of bounds index\n");
        
        UITextBox *next = &((f->parent + 1)->editFields[f->if_idx]);
        ls_uiTextBoxClear(c, next);
        ls_uiFocusChange(c, (u64 *)next);
        
        return TRUE;
    }
    
    UITextBox *parentFields = f->parent->editFields;
    
    if(KeyPress(keyMap::DArrow))
    {
        if(f->parent == lastMob)  { ls_uiFocusChange(c, 0x0); return inputUse; }
        if(f->parent == lastAlly) { ls_uiFocusChange(c, 0x0); return inputUse; }
        
        u32 numStrings = 0;
        utf32 *words = ls_utf32SeparateByNumber(parentFields[IF_IDX_NAME].text, &numStrings);
        
        AssertMsg(words, "The returned words array was null.\n");
        if(!words) { ls_uiFocusChange(c, 0x0); return inputUse; }
        
        if(numStrings != 2) { ls_uiFocusChange(c, 0x0); return inputUse; }
        
        s64 newNumber = ls_utf32ToInt(words[1]) + 1;
        ls_utf32FromInt_t(&words[1], newNumber);
        
        InitField *next = f->parent + 1;
        UITextBox *nextFields = next->editFields;
        
        ls_uiTextBoxClear(c, &nextFields[IF_IDX_NAME]);
        ls_utf32Set(&nextFields[IF_IDX_NAME].text, words[0]);
        ls_utf32Append(&nextFields[IF_IDX_NAME].text, words[1]);
        nextFields[IF_IDX_NAME].viewEndIdx = nextFields[IF_IDX_NAME].text.len;
        
        //NOTE: We start at i == 1, because the name field has already been handled,
        //      We stop at COUNT-1 because the last field is 'final' which should not be modified by this routine.
        for(u32 i = 1; i < IF_IDX_COUNT-1; i++)
        {
            ls_uiTextBoxClear(c, &nextFields[i]);
            ls_uiTextBoxSet(c, &nextFields[i], parentFields[i].text);
        }
        
        ls_uiTextBoxClear(c, &next->maxLife);
        ls_uiTextBoxSet(c, &next->maxLife, f->parent->maxLife.text);
        
        //NOTE: The maxLife field gets handled specially using the index 999
        if(f->if_idx == 999) { ls_uiFocusChange(c, (u64 *)&(next->maxLife)); return inputUse; }
        
        AssertMsg(f->if_idx < INIT_FIELD_EDITFIELDS_NUM, "Out of bounds index\n");
        ls_uiFocusChange(c, (u64 *)&(nextFields[f->if_idx]));
        
        return TRUE;
    }
    
    return inputUse;
}


b32 CustomMobLifeField(UIContext *c, void *data)
{
    Input *UserInput = &c->UserInput;
    b32 inputUse = FALSE;
    
    MobLifeHandler *h = (MobLifeHandler *)data;
    UITextBox *f = h->parent;
    
    if(!State.inBattle) {
        CustomFieldTextHandler dummy = {h->mob, h->parent, 999};
        inputUse |= CustomInitFieldText(c, &dummy);
    };
    
    if((c->lastFocus != (u64 *)f) && h->isEditing) {
        //NOTE: We lost focus, let's reset the box
        //TODO: Should we also reset what was being written?
        
        h->isEditing = FALSE;
    }
    
    if(LeftClick && !h->isEditing && State.inBattle)
    { 
        ls_utf32Set(&h->previous, f->text);
        ls_uiTextBoxClear(c, f);
        
        h->isEditing = TRUE;
        inputUse = TRUE;
    }
    
    if(h->isEditing && KeyPress(keyMap::Enter))
    {
        Order *order = GetOrderByID(h->mob->ID);
        AssertMsg(order, "Could not find order by ID. Fucked up ID?\n");
        
        s32 diff = ls_utf32ToInt(f->text);
        
        ls_uiSliderChangeValueBy(c, &order->field, diff);
        
        s32 currVal = ls_uiSliderGetValue(c, &order->field);
        if(currVal == 0)
        { order->field.rColor = ls_uiAlphaBlend(RGBA(0xFF, 0x97, 0x12, 0x99), c->widgetColor); }
        
        else if(currVal < 0)
        { order->field.rColor = ls_uiAlphaBlend(RGBA(0xDD, 0x10, 0x20, 0x99), c->widgetColor); }
        
        else if(currVal > 0)
        { order->field.rColor = ls_uiAlphaBlend(RGBA(0xF0, 0xFF, 0x3D, 0x99), c->widgetColor); }
        
        ls_uiTextBoxClear(c, f);
        ls_uiTextBoxSet(c, f, h->previous);
        
        h->isEditing = FALSE;
        ls_uiFocusChange(c, NULL);
        
        inputUse = TRUE;
    }
    
    return inputUse;
}

b32 ChangeOrder(UIContext *c, void *data)
{
    Input *UserInput = &c->UserInput;
    b32 inputUse = FALSE;
    
    OrderHandler *h = (OrderHandler *)data;
    UITextBox *f = h->parent;
    
    AssertMsg(State.inBattle, "How did we even get here?\n");
    
    if(LeftClick && !h->isEditing)
    { 
        ls_utf32Set(&h->previous, f->text);
        ls_uiTextBoxClear(c, f);
        
        h->isEditing = TRUE;
        inputUse = TRUE;
    }
    
    
    if(h->isEditing && KeyPress(keyMap::Enter))
    {
        InitPage *Init = State.Init;
        
        s32 visibleMobs   = Init->Mobs.selectedIndex;
        s32 visibleAllies = Init->Allies.selectedIndex;
        s32 visibleOrder  = visibleMobs + visibleAllies + PARTY_NUM - Init->orderAdjust;
        
        Order *order = h->order;
        
        s32 newPosition = ls_utf32ToInt(f->text);
        s32 oldPosition = ls_utf32ToInt(h->previous);
        
        if((newPosition >= visibleOrder) || (newPosition == oldPosition) || (newPosition < 0))
        {
            ls_uiTextBoxClear(c, f);
            ls_uiTextBoxSet(c, f, h->previous);
            
            h->isEditing = FALSE;
            ls_uiFocusChange(c, NULL);
            return TRUE;
        }
        
        
        //-------------------------------
        //TODO: Horrible temp saving...
        //
        Order *oldOrder = &Init->OrderFields[oldPosition];
        
        utf32 oldName = {}; 
        ls_utf32Set(&oldName, oldOrder->field.text);
        
        s32 oldID = oldOrder->ID;
        
        s32 slideCurr = oldOrder->field.currValue;
        s32 slideMax = oldOrder->field.maxValue;
        s32 slideMin = oldOrder->field.minValue;
        f64 slidePos = oldOrder->field.currPos;
        //
        //TODO: Horrible temp saving...
        //-------------------------------
        
        //TODO: Make branchless by getting the diff (new-old) and obtaining either -1 or +1 for the j offset.
        //      Problem with <= / >= newPosition???
        if(newPosition > oldPosition)
        {
            for(u32 j = oldPosition; j <= newPosition; j++)
            {
                UISlider *curr = &Init->OrderFields[j].field;
                UISlider *next = &Init->OrderFields[j+1].field;
                
                ls_utf32Set(&curr->text, next->text);
                Init->OrderFields[j].ID = Init->OrderFields[j+1].ID;
                
                curr->currValue = next->currValue;
                curr->maxValue  = next->maxValue;
                curr->minValue  = next->minValue;
                curr->currPos   = next->currPos;
            }
        }
        
        if(newPosition < oldPosition)
        {
            for(u32 j = oldPosition; j > newPosition; j--)
            {
                UISlider *curr = &Init->OrderFields[j].field;
                UISlider *prev = &Init->OrderFields[j-1].field;
                
                ls_utf32Set(&curr->text, prev->text);
                Init->OrderFields[j].ID = Init->OrderFields[j-1].ID;
                
                curr->currValue = prev->currValue;
                curr->maxValue  = prev->maxValue;
                curr->minValue  = prev->minValue;
                curr->currPos   = prev->currPos;
            }
        }
        
        ls_utf32Set(&Init->OrderFields[newPosition].field.text, oldName);
        
        Init->OrderFields[newPosition].ID = oldID;
        Init->OrderFields[newPosition].field.currValue = slideCurr;
        Init->OrderFields[newPosition].field.maxValue  = slideMax;
        Init->OrderFields[newPosition].field.minValue  = slideMin;
        Init->OrderFields[newPosition].field.currPos   = slidePos;
        
        //NOTE: Reset the position to actual ordinal
        ls_uiTextBoxClear(c, f);
        ls_uiTextBoxSet(c, f, h->previous);
        
        //NOTE: Update "Current" TextBox to reflect change if there was any
        //NOTE:TODO: Do I actually like this? Do we want this?
        if(Init->currIdx == newPosition)
        { ls_uiTextBoxSet(c, &Init->Current, oldName); }
        
        ls_utf32Free(&oldName); //TODO: yuck... look upward at horrible temp saving.
        
        h->isEditing = FALSE;
        ls_uiFocusChange(c, NULL);
        
        return TRUE;
    }
    
    return inputUse;
}

b32 ResetOnClick(UIContext *, void *);
void OnEncounterSelect(UIContext *c, void *data)
{
    UIListBox *b = (UIListBox *)data;
    
    u32 idx = b->selectedIndex;
    
    if(idx == 0) { ResetOnClick(c, 0); return; }
    
    Encounter *e = &State.encounters.Enc[idx-1];
    
    State.Init->Mobs.selectedIndex = e->numMobs;
    State.Init->Allies.selectedIndex = e->numAllies;
    
    for(u32 i = 0; i < e->numMobs; i++)
    {
        InitField *m = State.Init->MobFields + i;
        
        for(u32 j = 0; j < IF_IDX_COUNT; j++)
        { ls_uiTextBoxSet(c, &m->editFields[j], e->mob[i][j]); }
        
        ls_uiTextBoxSet(c, &m->maxLife, e->mob[i][MOB_INIT_ENC_FIELDS-1]);
        
    }
    
    for(u32 i = 0; i < e->numAllies; i++)
    {
        InitField *a = State.Init->AllyFields + i;
        
        ls_uiTextBoxSet(c, &a->editFields[IF_IDX_NAME],  e->allyName[i]);
        ls_uiTextBoxSet(c, &a->editFields[IF_IDX_BONUS], e->allyBonus[i]);
        ls_uiTextBoxSet(c, &a->editFields[IF_IDX_FINAL], e->allyFinal[i]);
    }
    
    for(u32 i = 0; i < THROWER_NUM; i++)
    {
        DiceThrow *t = State.Init->Throwers + i;
        
        ls_uiTextBoxClear(c, &t->name);
        ls_uiTextBoxClear(c, &t->toHit);
        ls_uiTextBoxClear(c, &t->hitRes);
        ls_uiTextBoxClear(c, &t->damage);
        ls_uiTextBoxClear(c, &t->dmgRes);
        
        ls_utf32Set(&t->name.text, e->throwerName[i]);     t->name.viewEndIdx   = t->name.text.len;
        
        ls_utf32Set(&t->toHit.text,  e->throwerHit[i]);    t->toHit.viewEndIdx  = t->toHit.text.len;
        ls_utf32Set(&t->damage.text, e->throwerDamage[i]); t->damage.viewEndIdx = t->damage.text.len;
    }
    
    //NOTE: This is to avoid the program never being reset. I don't think it's necessary,
    //      but for "security" reasons we'll have it.
    addID = 1000;
    
    return;
}

b32 RequestUndoOnClick(UIContext *c, void *data)
{
    undoRequest = TRUE;
    return FALSE;
}

b32 RequestRedoOnClick(UIContext *c, void *data)
{
    redoRequest = TRUE;
    return FALSE;
}

//TODO: Adding a new one while selecting an old one displays an incorrect name in the list box,
//      But it's only visual. On program restart the names are correct... wtf???
b32 SaveEncounterOnClick(UIContext *c, void *data)
{
    s32 visibleMobs   = State.Init->Mobs.selectedIndex;
    s32 visibleAllies = State.Init->Allies.selectedIndex;
    s32 visibleOrder  = visibleMobs + visibleAllies + PARTY_NUM - State.Init->orderAdjust;
    
    u32 numEncounters = State.encounters.numEncounters;
    Encounter *curr = State.encounters.Enc + numEncounters;
    
    ls_utf32Set(&curr->name, State.Init->EncounterName.text);
    
    curr->numMobs = visibleMobs;
    curr->numAllies = visibleAllies;
    
    for(u32 i = 0; i < visibleMobs; i++)
    {
        for(u32 j = 0; j < MOB_INIT_ENC_FIELDS; j++)
        { ls_utf32Set(&curr->mob[i][j], State.Init->MobFields[i].editFields[j].text); }
        
        ls_utf32Set(&curr->mob[i][MOB_INIT_ENC_FIELDS-1], State.Init->MobFields[i].maxLife.text);
    }
    
    for(u32 i = 0; i < visibleAllies; i++)
    {
        ls_utf32Set(&curr->allyName[i],  State.Init->AllyFields[i].editFields[IF_IDX_NAME].text);
        ls_utf32Set(&curr->allyBonus[i], State.Init->AllyFields[i].editFields[IF_IDX_BONUS].text);
        ls_utf32Set(&curr->allyFinal[i], State.Init->AllyFields[i].editFields[IF_IDX_FINAL].text);
    }
    
    for(u32 i = 0; i < THROWER_NUM; i++)
    {
        ls_utf32Set(&curr->throwerName[i],   State.Init->Throwers[i].name.text);
        ls_utf32Set(&curr->throwerHit[i],    State.Init->Throwers[i].toHit.text);
        ls_utf32Set(&curr->throwerDamage[i], State.Init->Throwers[i].damage.text);
    }
    
    State.encounters.numEncounters += 1;
    
    State.Init->EncounterSel.selectedIndex =
        ls_uiListBoxAddEntry(c, &State.Init->EncounterSel, State.Init->EncounterName.text);
    
    return FALSE;
}

b32 RemoveEncounterOnClick(UIContext *c, void *data)
{
    b32 inputUse = FALSE;
    
    u32 idx = State.Init->EncounterSel.selectedIndex;
    if(idx == 0) { return FALSE; }
    
    u32 lastIdx = State.Init->EncounterSel.list.count-1;
    
    Encounter *selected = State.encounters.Enc + (idx-1);
    Encounter *last = State.encounters.Enc + (lastIdx-1);
    
    //NOTE:      When idx == lastIdx we just decrease the numEncounters
    //
    //TODO:      Is it fine to keep the old stuff allocated? ls_unistrSet only allocates if data is null
    //           So adding new things on it shouldn't leak memory (and it gets reset on program startup anyway)
    //           Also everything gets overwritten, so there should be no problem of old data hanging.
    
    if(idx != lastIdx)
    {
        //NOTE: In this case I have to free memory, else I will leak
        //TODO: Should I change how Encounters are stored to avoid this annoyance?
        
        //TODO: Why don't I clear instead of free??????
        for(u32 i = 0; i < MOB_NUM; i++)
        {
            for(u32 j = 0; j < MOB_INIT_ENC_FIELDS; j++)
            { ls_utf32Free(&selected->mob[i][j]); }
        }
        
        for(u32 i = 0; i < ALLY_NUM; i++)
        {
            ls_utf32Free(&selected->allyName[i]);
            ls_utf32Free(&selected->allyBonus[i]);
            ls_utf32Free(&selected->allyFinal[i]);
        }
        
        for(u32 i = 0; i < THROWER_NUM; i++)
        {
            ls_utf32Free(&selected->throwerName[i]);
            ls_utf32Free(&selected->throwerHit[i]);
            ls_utf32Free(&selected->throwerDamage[i]);
        }
        
        ls_memcpy(last, selected, sizeof(Encounter));
    }
    
    State.encounters.numEncounters -= 1;
    ls_uiListBoxRemoveEntry(c, &State.Init->EncounterSel, idx);
    inputUse |= ResetOnClick(c, NULL);
    
    return inputUse;
}

b32 ThrowDiceOnClick(UIContext *c, void *data)
{
    UIButton *f = (UIButton *)data;
    
    if(f == &State.Init->GeneralThrower.throwDie)
    {
        char toThrow[128] = {};
        s32 len = ls_utf32ToAscii_t(&State.Init->GeneralThrower.toHit.text, toThrow, 128);
        s32 result = (s32)diceRoll(toThrow, len);
        
        ls_utf32FromInt_t(&State.Init->GeneralThrower.hitRes.text, result);
        State.Init->GeneralThrower.hitRes.viewEndIdx = State.Init->GeneralThrower.hitRes.text.len;
    }
    else
    {
        u32 idx = 9999;
        for(u32 i = 0; i < THROWER_NUM; i++) {
            if(f == &State.Init->Throwers[i].throwDie) { idx = i; }
        }
        AssertMsg(idx != 9999, "Couldn't find thrower\n");
        
        DiceThrow *f = State.Init->Throwers + idx;
        
        char toThrow[128] = {};
        s32 len = ls_utf32ToAscii_t(&f->toHit.text, toThrow, 128);
        s32 result = (s32)diceRoll(toThrow, len);
        
        ls_utf32FromInt_t(&f->hitRes.text, result);
        f->hitRes.viewEndIdx = f->hitRes.text.len;
        
        char dmgThrow[128] = {};
        len = ls_utf32ToAscii_t(&f->damage.text, dmgThrow, 128);
        result = (s32)diceRoll(dmgThrow, len);
        
        ls_utf32FromInt_t(&f->dmgRes.text, result);
        f->dmgRes.viewEndIdx = f->dmgRes.text.len;
    }
    
    return TRUE;
}

s32 sortTmpOrder(void *a, void *b)
{
    tmp_order *ordA = (tmp_order *)a;
    tmp_order *ordB = (tmp_order *)b;
    
    if(ordA->init < ordB->init) { return -1; }
    else if(ordA->init == ordB->init) { return 0; }
    else { return 1; }
}

b32 RollOnClick(UIContext *c, void *data)
{
    InitPage *Page = State.Init;
    
    s32 visibleMobs   = Page->Mobs.selectedIndex;
    s32 visibleAllies = Page->Allies.selectedIndex;
    
    for(u32 i = 0; i < visibleMobs; i++)
    {
        InitField *f = Page->MobFields + i;
        
        s32 finalVal = ls_utf32ToInt(f->editFields[IF_IDX_FINAL].text);
        if(finalVal != 0) { continue; }
        
        s32 die = pcg32_bounded(&pcg32_global, 20) + 1;
        s32 bonus = ls_utf32ToInt(f->editFields[IF_IDX_BONUS].text);
        
        ls_utf32Clear(&f->editFields[IF_IDX_FINAL].text);
        ls_utf32FromInt_t(&f->editFields[IF_IDX_FINAL].text, bonus + die);
        f->editFields[IF_IDX_FINAL].viewEndIdx = f->editFields[IF_IDX_FINAL].text.len;
    }
    
    for(u32 i = 0; i < visibleAllies; i++)
    {
        InitField *f = Page->AllyFields + i;
        
        s32 finalVal = ls_utf32ToInt(f->editFields[IF_IDX_FINAL].text);
        if(finalVal != 0) { continue; }
        
        s32 die = pcg32_bounded(&pcg32_global, 20) + 1;
        s32 bonus = ls_utf32ToInt(f->editFields[IF_IDX_BONUS].text);
        
        ls_utf32Clear(&f->editFields[IF_IDX_FINAL].text);
        ls_utf32FromInt_t(&f->editFields[IF_IDX_FINAL].text, bonus + die);
        f->editFields[IF_IDX_FINAL].viewEndIdx = f->editFields[IF_IDX_FINAL].text.len;
    }
    
    return TRUE;
}

b32 SetOnClick(UIContext *c, void *data)
{
    AssertMsg(!State.inBattle, "Clicking this should be impossible while in Battle\n");
    
    InitPage *Page = State.Init;
    
    s32 visibleMobs   = Page->Mobs.selectedIndex;
    s32 visibleAllies = Page->Allies.selectedIndex;
    s32 visibleOrder  = visibleMobs + visibleAllies + PARTY_NUM - Page->orderAdjust;
    
    tmp_order ord[ORDER_NUM] = {};
    u32 idx = 0;
    
    for(u32 i = 0; i < visibleMobs; i++)
    {
        InitField *f = Page->MobFields + i;
        
        ord[idx].init          = ls_utf32ToInt(f->editFields[IF_IDX_FINAL].text);
        ord[idx].name          = &f->editFields[IF_IDX_NAME].text;
        ord[idx].maxLife       = ls_utf32ToInt(f->maxLife.text);
        ord[idx].compendiumIdx = f->compendiumIdx;
        ord[idx].ID            = f->ID;
        
        idx += 1;
    }
    
    for(u32 i = 0; i < visibleAllies; i++)
    {
        InitField *f = Page->AllyFields + i;
        
        ord[idx].init          = ls_utf32ToInt(f->editFields[IF_IDX_FINAL].text);
        ord[idx].name          = &f->editFields[IF_IDX_NAME].text;
        ord[idx].maxLife       = 0;
        ord[idx].compendiumIdx = f->compendiumIdx;
        ord[idx].ID            = f->ID;
        
        idx += 1;
    }
    
    for(u32 i = 0; i < PARTY_NUM; i++)
    {
        ord[idx].init    = ls_utf32ToInt(Page->PlayerInit[i].text);
        ord[idx].name    = (utf32 *)(PartyNameUTF32 + i);
        ord[idx].maxLife = 0;
        ord[idx].ID      = i;
        
        idx += 1;
    }
    
    ls_quicksortCustom(ord, sizeof(tmp_order), visibleOrder, sortTmpOrder);
    
    for(u32 i = 0, j = visibleOrder - 1; i < visibleOrder; i++, j--)
    {
        Order *f = Page->OrderFields + i;
        
        ls_utf32Set(&f->field.text, *ord[j].name);
        f->field.maxValue = ord[j].maxLife;
        f->compendiumIdx  = ord[j].compendiumIdx;
        f->ID             = ord[j].ID;
        
        //TODO: Make this just a reference to Order[i].field.text ??
        if(i == 0) 
        { 
            ls_utf32Set(&Page->Current.text, *ord[j].name); 
            Page->Current.viewEndIdx = Page->Current.text.len;
        }
    }
    
    for(u32 i = 0; i < PARTY_NUM; i++)
    { Page->PlayerInit[i].isReadonly = TRUE; }
    
    for(u32 i = 0; i < MOB_NUM; i++)
    {
        InitField *f = Page->MobFields + i;
        
        for(u32 j = 0; j < IF_IDX_COUNT; j++)
        { f->editFields[j].isReadonly = TRUE; }
    }
    
    for(u32 i = 0; i < ALLY_NUM; i++) { 
        Page->AllyFields[i].editFields[IF_IDX_NAME].isReadonly  = TRUE;
        Page->AllyFields[i].editFields[IF_IDX_BONUS].isReadonly = TRUE;
        Page->AllyFields[i].editFields[IF_IDX_FINAL].isReadonly = TRUE;
    }
    
    for(u32 i = 0; i < ORDER_NUM; i++)
    { Page->OrderFields[i].pos.isReadonly = FALSE; }
    
    Page->turnsInRound = visibleOrder;
    Page->currIdx      = 0;
    State.inBattle     = TRUE;
    
    return TRUE;
}

b32 ResetOnClick(UIContext *c, void *data)
{
    InitPage *Page = State.Init;
    
    Page->Mobs.selectedIndex   = 0;
    Page->Allies.selectedIndex = 0;
    Page->selectedMobIndex     = -1;
    
    State.inBattle = FALSE;
    
    utf32 zeroUTF32 = { (u32 *)U"0", 1, 1 };
    
    for(u32 i = 0; i < PARTY_NUM; i++) 
    { 
        ls_uiTextBoxClear(c, Page->PlayerInit + i);
        ls_utf32Set(&Page->PlayerInit[i].text, zeroUTF32);
        Page->PlayerInit[i].viewEndIdx = Page->PlayerInit[i].text.len;
    }
    
    s32 currID = PARTY_NUM;
    for(u32 i = 0; i < MOB_NUM; i++)   
    { 
        InitField *f = Page->MobFields + i;
        
        ls_uiTextBoxClear(c, &f->editFields[IF_IDX_NAME]);
        ls_uiTextBoxSet(c, &f->editFields[IF_IDX_NAME], ls_utf32Constant(MobName[i]));
        
        for(u32 j = 1; j < IF_IDX_COUNT; j++)
        {
            if(j == IF_IDX_EXTRA) { ls_uiTextBoxClear(c, &f->editFields[j]); continue; }
            
            ls_uiTextBoxClear(c, &f->editFields[j]);
            ls_uiTextBoxSet(c, &f->editFields[j], zeroUTF32);
        }
        
        ls_uiTextBoxClear(c, &f->maxLife);
        ls_uiTextBoxSet(c, &f->maxLife, zeroUTF32);
        
        f->compendiumIdx = -1;
        f->ID            = currID;
        currID          += -1;
    }
    
    for(u32 i = 0; i < ALLY_NUM; i++)  
    { 
        InitField *f = Page->AllyFields + i;
        
        ls_uiTextBoxClear(c, &f->editFields[IF_IDX_NAME]);
        ls_uiTextBoxSet(c, &f->editFields[IF_IDX_NAME], ls_utf32Constant(AllyName[i]));
        
        ls_uiTextBoxClear(c, &f->editFields[IF_IDX_BONUS]);
        ls_uiTextBoxSet(c, &f->editFields[IF_IDX_BONUS], zeroUTF32);
        
        ls_uiTextBoxClear(c, &f->editFields[IF_IDX_FINAL]);
        ls_uiTextBoxSet(c, &f->editFields[IF_IDX_FINAL], zeroUTF32);
        
        f->compendiumIdx = -1;
        f->ID            = currID;
        currID          += -1;
    }
    
    for(u32 i = 0; i < ORDER_NUM; i++)
    {
        Order *f = Page->OrderFields + i;
        
        f->pos.isReadonly = TRUE;
        
        ls_utf32Clear(&f->field.text);
        f->field.maxValue = 100;
        f->field.minValue = -30;
        f->field.currPos  = 1.0;
        f->compendiumIdx  = -1;
        f->ID             = -1;
    }
    
    for(u32 i = 0; i < COUNTER_NUM; i++)
    {
        Counter *f = Page->Counters + i;
        
        ls_uiTextBoxClear(c, &f->name);
        ls_uiTextBoxClear(c, &f->rounds);
        f->roundsLeft      = 0;
        f->startIdxInOrder = 0;
        f->turnCounter     = 0;
        f->isActive        = FALSE;
    }
    
    for(u32 i = 0; i < THROWER_NUM; i++)
    {
        DiceThrow *f = Page->Throwers + i;
        
        ls_uiTextBoxClear(c, &f->name);
        ls_uiTextBoxClear(c, &f->toHit);
        ls_uiTextBoxClear(c, &f->hitRes);
        ls_uiTextBoxClear(c, &f->damage);
        ls_uiTextBoxClear(c, &f->dmgRes);
    }
    
    // General Thrower
    {
        ls_uiTextBoxClear(c, &Page->GeneralThrower.toHit);
        ls_uiTextBoxClear(c, &Page->GeneralThrower.hitRes);
    }
    
    Page->EncounterSel.selectedIndex = 0;
    
    ls_utf32Clear(&Page->Current.text);
    
    ls_utf32Set(&Page->RoundCounter.text, zeroUTF32);
    Page->RoundCounter.viewEndIdx = Page->RoundCounter.text.len;
    Page->roundCount = 0;
    
    Page->orderAdjust = 0;
    
    addID = 1000;
    
    return TRUE;
}

b32 NextOnClick(UIContext *c, void *data)
{
    InitPage *Page = State.Init;
    
    s32 visibleMobs   = Page->Mobs.selectedIndex;
    s32 visibleAllies = Page->Allies.selectedIndex;
    s32 visibleOrder  = visibleMobs + visibleAllies + PARTY_NUM - Page->orderAdjust;
    
    Page->currIdx = (Page->currIdx + 1) % visibleOrder;
    
    //NOTE: Increase the round
    if(Page->currIdx == 0)
    {
        Page->roundCount += 1;
        ls_utf32FromInt_t(&Page->RoundCounter.text, Page->roundCount);
        Page->RoundCounter.viewEndIdx = Page->RoundCounter.text.len;
    }
    
    //NOTE: Set the Current
    ls_utf32Set(&Page->Current.text, Page->OrderFields[Page->currIdx].field.text);
    Page->Current.viewEndIdx = Page->Current.text.len;
    
    //NOTE: Advance the Counters
    for(u32 i = 0; i < COUNTER_NUM; i++)
    {
        Counter *C = Page->Counters + i;
        
        if(C->isActive == TRUE)
        {
            C->turnCounter += 1;
            
            if(C->turnCounter >= Page->turnsInRound)
            { 
                C->turnCounter = 0;
                C->roundsLeft -= 1;
                
                AssertMsg(C->roundsLeft >= 0, "Rounds Left became negative");
                
                if(C->roundsLeft == 0)
                {
                    ls_uiTextBoxClear(c, &C->name);
                    ls_uiTextBoxClear(c, &C->rounds);
                    
                    C->turnCounter = 0;
                    C->isActive     = FALSE;
                    
                    continue;
                }
                
                ls_utf32FromInt_t(&C->rounds.text, C->roundsLeft);
                C->rounds.viewEndIdx = C->rounds.text.len;
                
                continue;
            }
            
            continue;
        }
    }
    
    return TRUE;
}

void CopyOrder(Order *From, Order *To)
{
    // Copy Slider
    To->field.isHot  = From->field.isHot;
    To->field.isHeld = From->field.isHeld;
    
    ls_utf32Set(&To->field.text, From->field.text);
    
    To->field.currValue = From->field.currValue;
    To->field.maxValue  = From->field.maxValue;
    To->field.minValue  = From->field.minValue;
    
    To->field.currPos   = From->field.currPos;
    
    To->field.style     = From->field.style;
    To->field.lColor    = From->field.lColor;
    To->field.rColor    = From->field.rColor;
    
    //NOTE: Don't need to copy the textbox, since the position of an order field is fixed.
    //NOTE: Don't need to copy the button
    
    To->ID = From->ID;
}

void CopyInitField(UIContext *c, InitField *From, InitField *To)
{
    for(u32 i = 0; i < IF_IDX_COUNT; i++)
    { ls_uiTextBoxSet(c, &To->editFields[i], From->editFields[i].text); }
    
    ls_uiTextBoxSet(c, &To->maxLife, From->maxLife.text);
    
    To->ID = From->ID;
}

b32 RemoveOrderOnClick(UIContext *c, void *data)
{
    if(State.inBattle == FALSE) { return FALSE; }
    
    u32 index = (u32)((u64)data);
    
    InitPage *Page = State.Init;
    Order *f = Page->OrderFields + index;
    
    s32 removeID = f->ID;
    
    s32 visibleMobs   = Page->Mobs.selectedIndex;
    s32 visibleAllies = Page->Allies.selectedIndex;
    s32 visibleOrder  = visibleMobs + visibleAllies + PARTY_NUM - Page->orderAdjust;
    
    if(removeID < PARTY_NUM)
    {
        Page->orderAdjust += 1;
        
        for(u32 i = index; i < (visibleOrder-1); i++)
        {
            Order *A = Page->OrderFields + i;
            Order *B = Page->OrderFields + (i+1);
            
            CopyOrder(B, A);
        }
        
        goto exit;
    }
    else
    {
        for(u32 i = 0; i < ALLY_NUM; i++) 
        {
            InitField *ally = Page->AllyFields + i;
            if(removeID == ally->ID) 
            {
                //NOTE: We remove the ally from the allies list by moving the last one in its place
                InitField *B = Page->AllyFields + (visibleAllies - 1);
                CopyInitField(c, B, ally);
                
                ls_uiTextBoxClear(c, &B->editFields[IF_IDX_NAME]);
                ls_uiTextBoxClear(c, &B->editFields[IF_IDX_BONUS]);
                ls_uiTextBoxClear(c, &B->editFields[IF_IDX_FINAL]);
                B->ID = 0;
                
                Page->Allies.selectedIndex -= 1;
                
                for(u32 i = index; i < (visibleOrder-1); i++)
                {
                    Order *A = Page->OrderFields + i;
                    Order *B = Page->OrderFields + (i+1);
                    
                    CopyOrder(B, A);
                }
                
                goto exit;
            }
        }
        
        for(u32 i = 0; i < MOB_NUM; i++)
        {
            InitField *mob = Page->MobFields + i;
            if(removeID == mob->ID)
            {
                //NOTE: We remove the Mob from the mobs list by moving the last one in its place
                InitField *B = Page->MobFields + (visibleMobs - 1);
                CopyInitField(c, B, mob);
                
                for(u32 j = 0; j < IF_IDX_COUNT; j++)
                { ls_uiTextBoxClear(c, &B->editFields[j]); }
                
                ls_uiTextBoxClear(c, &B->maxLife);
                
                B->ID = 0;
                
                Page->Mobs.selectedIndex -= 1;
                
                for(u32 i = index; i < (visibleOrder-1); i++)
                {
                    Order *A = Page->OrderFields + i;
                    Order *B = Page->OrderFields + (i+1);
                    
                    CopyOrder(B, A);
                }
                
                goto exit;
            }
        }
    }
    
    
    exit:
    
    Page->turnsInRound -= 1;
    
    //NOTE: We won't move the 'Current' field if you remove the 'Current' from the order.
    //      Because of that, Counters will be one count extra on the first lap after the remove.
    //      So we decrease them by one.
    if(Page->currIdx == index)
    {
        for(u32 i = 0; i < COUNTER_NUM; i++)
        {
            if(Page->Counters[i].isActive) 
            { Page->Counters[i].turnCounter -= 1; }
        }
    }
    
    //NOTE: Because we push everything up in the order (they go towards index 0)
    //      If the current index is larger then the removed idx, the current will have been
    //      pushed up, hence we need to readjust it.
    if(Page->currIdx >= index) { Page->currIdx -= 1; }
    
    return TRUE;
}

b32 StartCounterOnClick(UIContext *c, void *data)
{
    Counter *C = (Counter *)data;
    
    if(State.inBattle == FALSE) { return FALSE; }
    
    s32 val = ls_utf32ToInt(C->rounds.text);
    if(val <= 0) { return FALSE; }
    
    C->name.isReadonly   = TRUE;
    C->rounds.isReadonly = TRUE;
    
    C->isActive        = TRUE;
    C->turnCounter     = 0;
    C->roundsLeft      = val;
    C->startIdxInOrder = State.Init->currIdx;
    
    return TRUE;
}

b32 PlusOneCounterOnClick(UIContext *c, void *data)
{
    Counter *C = (Counter *)data;
    
    C->roundsLeft += 1;
    ls_utf32FromInt_t(&C->rounds.text, C->roundsLeft);
    
    return TRUE;
}

b32 StopCounterOnClick(UIContext *c, void *data)
{
    Counter *C = (Counter *)data;
    
    ls_uiTextBoxClear(c, &C->name);
    ls_uiTextBoxClear(c, &C->rounds);
    
    C->name.isReadonly   = FALSE;
    C->rounds.isReadonly = FALSE;
    
    C->isActive        = FALSE;
    C->turnCounter     = 0;
    C->roundsLeft      = 0;
    C->startIdxInOrder = 0;
    
    return TRUE;
}

b32 AddNewInitOnClick(UIContext *c, void *data)
{
    InitField *f = (InitField *)data;
    f->isAdding = TRUE;
    
    return TRUE;
}

//TODO: Right now we are not allowing to give an Initiative Value and put the new 
//      Field in the right place in Order.
//      If we allow it, the counter checker WILL HAVE to probably fix C->startIdxInOrder to work.
b32 AddConfirmOnClick(UIContext *c, void *data)
{
    InitField *f = (InitField *)data;
    
    s32 visibleMobs   = State.Init->Mobs.selectedIndex;
    s32 visibleAllies = State.Init->Allies.selectedIndex;
    s32 visibleOrder = visibleMobs + visibleAllies + PARTY_NUM - State.Init->orderAdjust;
    
    //NOTETODO: This is a really dumb way to determine if it's an ally or a mob...
    //          I should pass as user data to the hook a special struct which contains all relevant info.
    b32 isMob = FALSE;
    if(State.Init->AllyFields <= f && f <= (State.Init->AllyFields + visibleAllies))
    { State.Init->Allies.selectedIndex += 1; }
    else
    { State.Init->Mobs.selectedIndex += 1; isMob = TRUE; }
    
    State.Init->turnsInRound += 1;
    
    for(u32 i = 0; i < COUNTER_NUM; i++)
    {
        Counter *C = State.Init->Counters + i;
        if(C->isActive)
        { 
            AssertMsg(State.Init->turnsInRound >= C->startIdxInOrder, "startIdxInOrder is not realiable anymore\n");
            
            //NOTE: This counter missed counting the new addition this round,
            //      So we manually count up by one.
            if(State.Init->currIdx < C->startIdxInOrder)
            { C->turnCounter += 1; }
        }
    }
    
    Order *o = State.Init->OrderFields + visibleOrder;
    
    ls_utf32Set(&o->field.text, f->addName.text);
    ls_uiTextBoxSet(c, &f->editFields[IF_IDX_NAME], f->addName.text);
    
    f->editFields[IF_IDX_NAME].isReadonly  = TRUE;
    f->editFields[IF_IDX_BONUS].isReadonly = TRUE;
    f->editFields[IF_IDX_FINAL].isReadonly = TRUE;
    
    if(isMob)
    {
        //NOTE: We are skipping Name, Bonus and Final, by starting at 2 and ending 1 earlier.
        for(u32 i = 2; i < IF_IDX_COUNT-1; i++)
        { f->editFields[i].isReadonly = TRUE; }
        
        o->field.maxValue = ls_utf32ToInt(f->maxLife.text);
    }
    
    o->compendiumIdx = f->compendiumIdx;
    
    //NOTE: Because IDs in the InitFields get overwritten when removing from order 
    //      we can't reliably re-use them, unless we create a system to dispense Unique IDs.
    //      So for simplicy we are just starting from 1000 and every single Added Init will just the next one.
    //      AddID is reset during ResetOnClick
    f->ID = addID;
    o->ID = addID;
    addID += 1;
    
    f->isAdding = FALSE;
    return TRUE;
}

void InitFieldInit(UIContext *c, InitField *f, s32 *currID, const char32_t *name)
{
    //TODO: I hate the init fields, and how we have a different CustomFieldTextHandler for every piece
    //      of the init fields.
    CustomFieldTextHandler *textHandler = 
        (CustomFieldTextHandler *)ls_alloc(sizeof(CustomFieldTextHandler)*INIT_FIELD_EDITFIELDS_NUM);
    for(u32 i = 0; i < IF_IDX_COUNT; i++)
    {
        textHandler[i].parent = f;
        textHandler[i].field = f->editFields + i;
        textHandler[i].if_idx = i;
    }
    
    utf32 zeroUTF32 = { (u32 *)U"0", 1, 1 };
    
    ls_uiTextBoxSet(c, &f->editFields[IF_IDX_NAME], ls_utf32Constant(name));
    f->editFields[IF_IDX_NAME].preInput     = CustomInitFieldText;
    f->editFields[IF_IDX_NAME].data         = &textHandler[0];
    f->editFields[IF_IDX_NAME].isSingleLine = TRUE;
    
    ls_uiTextBoxSet(c, &f->editFields[IF_IDX_BONUS], zeroUTF32);
    f->editFields[IF_IDX_BONUS].maxLen       = 2;
    f->editFields[IF_IDX_BONUS].preInput     = CustomInitFieldText;
    f->editFields[IF_IDX_BONUS].data         = &textHandler[1];
    f->editFields[IF_IDX_BONUS].isSingleLine = TRUE;
    
    f->editFields[IF_IDX_EXTRA].text         = ls_utf32Alloc(16);
    f->editFields[IF_IDX_EXTRA].viewEndIdx   = f->editFields[IF_IDX_EXTRA].text.len;
    f->editFields[IF_IDX_EXTRA].preInput     = CustomInitFieldText;
    f->editFields[IF_IDX_EXTRA].data         = &textHandler[2];
    f->editFields[IF_IDX_EXTRA].isSingleLine = TRUE;
    
    MobLifeHandler *handler = (MobLifeHandler *)ls_alloc(sizeof(MobLifeHandler));
    handler->parent   = &f->maxLife;
    handler->mob      = f;
    handler->previous = ls_utf32Alloc(16);
    
    ls_uiTextBoxSet(c, &f->maxLife, zeroUTF32);
    f->maxLife.maxLen       = 4;
    f->maxLife.preInput     = CustomMobLifeField;
    f->maxLife.data         = handler;
    f->maxLife.isSingleLine = TRUE;
    
    ls_uiTextBoxSet(c, &f->editFields[IF_IDX_TOTALAC], zeroUTF32);
    f->editFields[IF_IDX_TOTALAC].maxLen       = 2;
    f->editFields[IF_IDX_TOTALAC].preInput     = CustomInitFieldText;
    f->editFields[IF_IDX_TOTALAC].data         = &textHandler[3];
    f->editFields[IF_IDX_TOTALAC].isSingleLine = TRUE;
    
    ls_uiTextBoxSet(c, &f->editFields[IF_IDX_TOUCHAC], zeroUTF32);
    f->editFields[IF_IDX_TOUCHAC].maxLen       = 2;
    f->editFields[IF_IDX_TOUCHAC].preInput     = CustomInitFieldText;
    f->editFields[IF_IDX_TOUCHAC].data         = &textHandler[4];
    f->editFields[IF_IDX_TOUCHAC].isSingleLine = TRUE;
    
    ls_uiTextBoxSet(c, &f->editFields[IF_IDX_FLATAC], zeroUTF32);
    f->editFields[IF_IDX_FLATAC].maxLen       = 2;
    f->editFields[IF_IDX_FLATAC].preInput     = CustomInitFieldText;
    f->editFields[IF_IDX_FLATAC].data         = &textHandler[5];
    f->editFields[IF_IDX_FLATAC].isSingleLine = TRUE;
    
    ls_uiTextBoxSet(c, &f->editFields[IF_IDX_LOWAC], zeroUTF32);
    f->editFields[IF_IDX_LOWAC].maxLen       = 2;
    f->editFields[IF_IDX_LOWAC].preInput     = CustomInitFieldText;
    f->editFields[IF_IDX_LOWAC].data         = &textHandler[6];
    f->editFields[IF_IDX_LOWAC].isSingleLine = TRUE;
    
    ls_uiTextBoxSet(c, &f->editFields[IF_IDX_CONSAVE], zeroUTF32);
    f->editFields[IF_IDX_CONSAVE].maxLen       = 2;
    f->editFields[IF_IDX_CONSAVE].preInput     = CustomInitFieldText;
    f->editFields[IF_IDX_CONSAVE].data         = &textHandler[7];
    f->editFields[IF_IDX_CONSAVE].isSingleLine = TRUE;
    
    ls_uiTextBoxSet(c, &f->editFields[IF_IDX_DEXSAVE], zeroUTF32);
    f->editFields[IF_IDX_DEXSAVE].maxLen       = 2;
    f->editFields[IF_IDX_DEXSAVE].preInput     = CustomInitFieldText;
    f->editFields[IF_IDX_DEXSAVE].data         = &textHandler[8];
    f->editFields[IF_IDX_DEXSAVE].isSingleLine = TRUE;
    
    ls_uiTextBoxSet(c, &f->editFields[IF_IDX_WISSAVE], zeroUTF32);
    f->editFields[IF_IDX_WISSAVE].maxLen       = 2;
    f->editFields[IF_IDX_WISSAVE].preInput     = CustomInitFieldText;
    f->editFields[IF_IDX_WISSAVE].data         = &textHandler[9];
    f->editFields[IF_IDX_WISSAVE].isSingleLine = TRUE;
    
    ls_uiTextBoxSet(c, &f->editFields[IF_IDX_FINAL], zeroUTF32);
    f->editFields[IF_IDX_FINAL].maxLen       = 2;
    f->editFields[IF_IDX_FINAL].preInput     = CustomInitFieldText;
    f->editFields[IF_IDX_FINAL].data         = &textHandler[10];
    f->editFields[IF_IDX_FINAL].isSingleLine = TRUE;
    
    
    f->addName.text         = ls_utf32Alloc(16);
    f->addName.isSingleLine = TRUE;
    f->addInit.text         = ls_utf32Alloc(16);
    f->addInit.maxLen       = 2;
    f->addInit.isSingleLine = TRUE;
    
    f->addNew.style       = UIBUTTON_TEXT;
    f->addNew.name        = ls_utf32FromUTF32(U"+");
    f->addNew.onClick     = AddNewInitOnClick;
    f->addNew.data        = f;
    f->addNew.onHold      = 0x0;
    
    f->addConfirm.style   = UIBUTTON_TEXT;
    f->addConfirm.name    = ls_utf32FromUTF32(U"Ok");
    f->addConfirm.onClick = AddConfirmOnClick;
    f->addConfirm.data    = f;
    f->addConfirm.onHold  = 0x0;
    
    f->compendiumIdx      = -1;
    
    f->ID = *currID;
    *currID += 1;
}

void SetInitTab(UIContext *c, ProgramState *PState)
{
    InitPage *Page = PState->Init;
    
    Page->selectedMobIndex = -1;
    
    for(u32 i = 0; i < MOB_NUM + 1; i++) { ls_uiListBoxAddEntry(c, &Page->Mobs, (char *)Enemies[i]); }
    for(u32 i = 0; i < ALLY_NUM + 1; i++) { ls_uiListBoxAddEntry(c, &Page->Allies, (char *)Allies[i]); }
    
    for(u32 i = 0; i < PARTY_NUM; i++) 
    { 
        UITextBox *f = Page->PlayerInit + i;
        ls_uiTextBoxSet(c, f, ls_utf32Constant(U"0"));
        f->maxLen       = 2;
        f->preInput     = CustomPlayerText;
        f->data         = f;
        f->isSingleLine = TRUE;
    }
    
    s32 currID = PARTY_NUM;
    for(u32 i = 0; i < MOB_NUM; i++)   
    { 
        InitField *f = Page->MobFields + i;
        
        InitFieldInit(c, f, &currID, MobName[i]);
    }
    
    for(u32 i = 0; i < ALLY_NUM; i++)  
    { 
        InitField *f = Page->AllyFields + i;
        
        InitFieldInit(c, f, &currID, AllyName[i]);
    }
    
    for(u32 i = 0; i < ORDER_NUM; i++)
    {
        Order *f = Page->OrderFields + i;
        
        Color lColor      = ls_uiAlphaBlend(RGBA(0x10, 0xDD, 0x20, 0x99), c->widgetColor);
        Color rColor      = ls_uiAlphaBlend(RGBA(0xF0, 0xFF, 0x3D, 0x99), c->widgetColor);
        f->field          = ls_uiSliderInit(NULL, 100, -30, 1.0, SL_BOX, lColor, rColor);
        
        OrderHandler *orderHandler = (OrderHandler *)ls_alloc(sizeof(OrderHandler));
        orderHandler->parent = &f->pos;
        orderHandler->order  = f;
        
        //TODO: Use a constant increasing number table. No point int allocating data for numbers in the [0-24] range
        f->pos.text         = ls_utf32FromInt(i);
        f->pos.viewEndIdx   = f->pos.text.len;
        f->pos.maxLen       = 2;
        f->pos.preInput     = ChangeOrder;
        f->pos.data         = orderHandler;
        f->pos.isReadonly   = TRUE;
        f->pos.isSingleLine = TRUE;
        
        f->remove.style   = UIBUTTON_TEXT;
        f->remove.name    = ls_utf32FromUTF32(U"X");
        f->remove.onClick = RemoveOrderOnClick;
        f->remove.data    = (void *)((u64)i);
        f->remove.onHold  = 0x0;
    }
    
    //TODO: Many of these strings should be constant, rather than allocated.
    for(u32 i = 0; i < COUNTER_NUM; i++)
    {
        Counter *f = Page->Counters + i;
        
        f->name.text        = ls_utf32Alloc(16);
        f->name.isSingleLine = TRUE;
        
        f->rounds.text         = ls_utf32Alloc(16);
        f->rounds.maxLen       = 2;
        f->rounds.isSingleLine = TRUE;
        
        f->start.style   = UIBUTTON_TEXT;
        f->start.name    = ls_utf32FromUTF32(U"Start");
        f->start.onClick = StartCounterOnClick;
        f->start.data    = (void *)f;
        f->start.onHold  = 0x0;
        
        f->plusOne.style   = UIBUTTON_TEXT;
        f->plusOne.name    = ls_utf32FromUTF32(U"+1");
        f->plusOne.onClick = PlusOneCounterOnClick;
        f->plusOne.data    = (void *)f;
        f->plusOne.onHold  = 0x0;
        
        f->stop.style   = UIBUTTON_TEXT;
        f->stop.name    = ls_utf32FromUTF32(U"Stop");
        f->stop.onClick = StopCounterOnClick;
        f->stop.data    = (void *)f;
        f->stop.onHold  = 0x0;
    }
    
    for(u32 i = 0; i < THROWER_NUM; i++)
    {
        DiceThrow *f = Page->Throwers + i;
        
        f->name.text   = ls_utf32Alloc(32);
        f->toHit.text  = ls_utf32Alloc(32);
        f->hitRes.text = ls_utf32Alloc(16);
        f->damage.text = ls_utf32Alloc(32);
        f->dmgRes.text = ls_utf32Alloc(16);
        
        f->name.isSingleLine   = TRUE;
        f->toHit.isSingleLine  = TRUE;
        f->damage.isSingleLine = TRUE;
        
        f->hitRes.maxLen       = 4;
        f->hitRes.isReadonly   = TRUE;
        f->hitRes.isSingleLine = TRUE;
        
        f->dmgRes.maxLen       = 4;
        f->dmgRes.isReadonly   = TRUE;
        f->dmgRes.isSingleLine = TRUE;
        
        f->throwDie.style   = UIBUTTON_TEXT;
        f->throwDie.name    = ls_utf32FromUTF32(U"Go");
        f->throwDie.onClick = ThrowDiceOnClick;
        f->throwDie.data    = &f->throwDie;
        f->throwDie.onHold  = 0x0;
    }
    
    {
        // General Thrower
        Page->GeneralThrower.name.text     = ls_utf32Alloc(32);
        Page->GeneralThrower.toHit.text    = ls_utf32Alloc(32);
        Page->GeneralThrower.hitRes.text   = ls_utf32Alloc(16);
        Page->GeneralThrower.damage.text   = ls_utf32Alloc(32);
        Page->GeneralThrower.dmgRes.text   = ls_utf32Alloc(16);
        
        Page->GeneralThrower.name.isSingleLine  = TRUE;
        Page->GeneralThrower.toHit.isSingleLine = TRUE;
        
        Page->GeneralThrower.hitRes.maxLen       = 4;
        Page->GeneralThrower.hitRes.isReadonly   = TRUE;
        Page->GeneralThrower.hitRes.isSingleLine = TRUE;
        
        Page->GeneralThrower.dmgRes.maxLen       = 4;
        Page->GeneralThrower.dmgRes.isReadonly   = TRUE;
        Page->GeneralThrower.dmgRes.isSingleLine = TRUE;
        
        Page->GeneralThrower.throwDie.style   = UIBUTTON_TEXT;
        Page->GeneralThrower.throwDie.name    = ls_utf32FromUTF32(U"Go");
        Page->GeneralThrower.throwDie.onClick = ThrowDiceOnClick;
        Page->GeneralThrower.throwDie.data    = &Page->GeneralThrower.throwDie;
        Page->GeneralThrower.throwDie.onHold  = 0x0;
    }
    
    //Encounter Selector
    {
        Page->EncounterSel.onSelect = OnEncounterSelect;
        Page->EncounterSel.data = &Page->EncounterSel;
        ls_uiListBoxAddEntry(c, &Page->EncounterSel, ls_utf32Constant(NoEncounterStr));
        for(u32 i = 0; i < PState->encounters.numEncounters; i++)
        { ls_uiListBoxAddEntry(c, &Page->EncounterSel, PState->encounters.Enc[i].name); }
        
        Page->EncounterName.text         = ls_utf32Alloc(16);
        Page->EncounterName.isSingleLine = TRUE;
    }
    
    
    Page->SaveEnc.style     = UIBUTTON_TEXT;
    Page->SaveEnc.name      = ls_utf32FromUTF32(U"Save");
    Page->SaveEnc.onClick   = SaveEncounterOnClick;
    Page->SaveEnc.data      = 0x0;
    
    Page->RemoveEnc.style   = UIBUTTON_TEXT;
    Page->RemoveEnc.name    = ls_utf32FromUTF32(U"X");
    Page->RemoveEnc.onClick = RemoveEncounterOnClick;
    
    Page->Current.text         = ls_utf32Alloc(16);
    Page->Current.isReadonly   = TRUE;
    Page->Current.isSingleLine = TRUE;
    
    Page->RoundCounter.text         = ls_utf32FromUTF32(U"0");
    Page->RoundCounter.viewEndIdx   = Page->RoundCounter.text.len;
    Page->RoundCounter.isReadonly   = TRUE;
    Page->RoundCounter.isSingleLine = TRUE;
    
    Page->Roll.style    = UIBUTTON_TEXT;
    Page->Roll.name     = ls_utf32FromUTF32(U"Roll");
    Page->Roll.onClick  = RollOnClick;
    Page->Roll.onHold   = 0x0;
    
    Page->Set.style     = UIBUTTON_TEXT;
    Page->Set.name      = ls_utf32FromUTF32(U"Set");
    Page->Set.onClick   = SetOnClick;
    Page->Set.onHold    = 0x0;
    
    Page->Reset.style   = UIBUTTON_TEXT;
    Page->Reset.name    = ls_utf32FromUTF32(U"Reset");
    Page->Reset.onClick = ResetOnClick;
    Page->Reset.onHold  = 0x0;
    
    Page->Next.style    = UIBUTTON_TEXT;
    Page->Next.name     = ls_utf32FromUTF32(U"Next");
    Page->Next.onClick  = NextOnClick;
    Page->Next.onHold   = 0x0;
    
    Page->Undo.style    = UIBUTTON_TEXT;
    Page->Undo.name     = ls_utf32FromUTF32(U"<-");
    Page->Undo.onClick  = RequestUndoOnClick;
    Page->Undo.onHold   = 0x0;
    
    Page->Redo.style    = UIBUTTON_TEXT;
    Page->Redo.name     = ls_utf32FromUTF32(U"->");
    Page->Redo.onClick  = RequestRedoOnClick;
    Page->Redo.onHold   = 0x0;
}

b32 DrawInitExtra(UIContext *c, InitField *F, s32 baseX, s32 y)
{
    b32 inputUse = FALSE;
    
    Color base = c->widgetColor;
    s32 x = baseX;
    
    ls_uiLabel(c, F->editFields[IF_IDX_NAME].text, x-30, y+45);
    
    c->widgetColor = ls_uiAlphaBlend(RGBA(0x1B, 0x18, 0x14, 150), base);
    ls_uiLabel(c, U"PF", x-30, y+5);
    inputUse |= ls_uiTextBox(c, &F->maxLife, x, y, 62, 20);
    
    y -= 40;
    c->widgetColor = ls_uiAlphaBlend(RGBA(0x61, 0x3B, 0x09, 150), base);
    ls_uiLabel(c, U"CA", x-30, y+5);
    inputUse |= ls_uiTextBox(c, &F->editFields[IF_IDX_TOTALAC], x, y, 26, 20); x += 130;
    
    c->widgetColor = ls_uiAlphaBlend(RGBA(0x9C, 0x43, 0x8B, 150), base);
    ls_uiLabel(c, U"Contatto", x-76, y+5);
    inputUse |= ls_uiTextBox(c, &F->editFields[IF_IDX_TOUCHAC], x, y, 26, 20); x += 152;
    
    c->widgetColor = ls_uiAlphaBlend(RGBA(0xD5, 0xCB, 0x35, 150), base);
    ls_uiLabel(c, U"Impreparato", x-100, y+5);
    inputUse |= ls_uiTextBox(c, &F->editFields[IF_IDX_FLATAC],  x, y, 26, 20); x += 100;
    
    c->widgetColor = ls_uiAlphaBlend(RGBA(0x75, 0x46, 0x46, 150), base);
    ls_uiLabel(c, U"Base", x-46, y+5);
    inputUse |= ls_uiTextBox(c, &F->editFields[IF_IDX_LOWAC],   x, y, 26, 20); x += 140;
    
    y -= 40;
    x  = baseX;
    c->widgetColor = ls_uiAlphaBlend(RGBA(0xDD, 0x10, 0x20, 150), base);
    ls_uiLabel(c, U"Tempra", x-62, y+5);
    inputUse |= ls_uiTextBox(c, &F->editFields[IF_IDX_CONSAVE], x, y, 26, 20); x += 130;
    
    c->widgetColor = ls_uiAlphaBlend(RGBA(0x2C, 0x80, 0x34, 150), base);
    ls_uiLabel(c, U"Riflessi", x-76, y+5);
    inputUse |= ls_uiTextBox(c, &F->editFields[IF_IDX_DEXSAVE], x, y, 26, 20); x += 152;
    
    c->widgetColor = ls_uiAlphaBlend(RGBA(0x2C, 0x3D, 0x80, 150), base);
    ls_uiLabel(c, U"Volont\U000000E0", x-70, y+5);
    inputUse |= ls_uiTextBox(c, &F->editFields[IF_IDX_WISSAVE], x, y, 26, 20); x += 130;
    
    c->widgetColor = base;
    
    y -= 380;
    x  = baseX;
    inputUse |= ls_uiTextBox(c, &F->editFields[IF_IDX_EXTRA], x, y, 408, 360); x += 96;
    
    return inputUse;
}

b32 DrawInitField(UIContext *c, InitField *F, s32 baseX, s32 y, u32 posIdx)
{
    b32 inputUse = FALSE;
    
    s32 w = 120;
    
    InitPage *Page = State.Init;
    
    s32 x = baseX;
    inputUse |= ls_uiTextBox(c, &F->editFields[IF_IDX_NAME],  x         , y, w, 20);
    inputUse |= ls_uiTextBox(c, &F->editFields[IF_IDX_BONUS], x + w     , y, 26, 20);
    inputUse |= ls_uiTextBox(c, &F->editFields[IF_IDX_FINAL], x + w + 26, y, 26, 20);
    
    Input *UserInput = &c->UserInput;
    if(RightClickIn(x, y, w+26, 19)) { Page->selectedMobIndex = (s32)posIdx; }
    
    
    return inputUse;
}

b32 DrawOrderField(UIContext *c, Order *f, s32 xPos, s32 yPos, u32 posIdx)
{
    b32 inputUse = FALSE;
    
    inputUse |= ls_uiSlider(c, &f->field, xPos + 50, yPos, 120, 20);
    if(inputUse)
    {
        s32 currVal = ls_uiSliderGetValue(c, &f->field);
        
        if(currVal == 0)
        { f->field.rColor = ls_uiAlphaBlend(RGBA(0xFF, 0x97, 0x12, 0x99), c->widgetColor); }
        
        else if(currVal < 0)
        { f->field.rColor = ls_uiAlphaBlend(RGBA(0xDD, 0x10, 0x20, 0x99), c->widgetColor); }
        
        else if(currVal > 0)
        { f->field.rColor = ls_uiAlphaBlend(RGBA(0xF0, 0xFF, 0x3D, 0x99), c->widgetColor); }
    }
    
    inputUse |= ls_uiTextBox(c, &f->pos, xPos + 25, yPos, 25, 20);
    inputUse |= ls_uiButton(c, &f->remove, xPos, yPos, 20, 20);
    
    Input *UserInput = &c->UserInput;
    if(RightClickIn(xPos + 50, yPos, 120, 20)) State.Init->selectedMobIndex = (s32)posIdx;
    
    return inputUse;
}

b32 DrawDefaultStyle(UIContext *c)
{
    InitPage *Page = State.Init;
    
    s32 visibleMobs   = Page->Mobs.selectedIndex;
    s32 visibleAllies = Page->Allies.selectedIndex;
    s32 visibleOrder  = visibleMobs + visibleAllies + PARTY_NUM - Page->orderAdjust;
    
    b32 inputUse = FALSE;
    
    //NOTE: Z Layer 1 Input
    {
        if(!State.inBattle)
        {
            inputUse |= ls_uiListBox(c, &Page->EncounterSel,  480, 718, 120, 20);
            inputUse |= ls_uiTextBox(c, &Page->EncounterName, 624, 718, 100, 20);
            
            inputUse |= ls_uiButton(c, &Page->SaveEnc, 601, 740, 44, 20);
            inputUse |= ls_uiButton(c, &Page->RemoveEnc, 455, 718, 24, 20);
            
            inputUse |= ls_uiListBox(c, &Page->Mobs,   406, 678, 100, 20);
            inputUse |= ls_uiListBox(c, &Page->Allies, 641, 498, 100, 20);
            
            inputUse |= ls_uiButton(c, &Page->Roll, 556, 678, 48, 20);
            inputUse |= ls_uiButton(c, &Page->Set,  780, 678, 48, 20);
        }
    }
    
    //NOTE: Z Layer 0 Input
    
    // Party
    s32 yPos = 638;
    for(u32 i = 0; i < PARTY_NUM; i++)
    {
        ls_uiLabel(c, PartyName[i], 650, yPos+6);
        inputUse |= ls_uiTextBox(c, Page->PlayerInit + i, 732, yPos, 32, 20);
        yPos -= 20;
    }
    
    // Allies
    yPos = 458;
    for(u32 i = 0; i < visibleAllies; i++)
    {
        inputUse |= DrawInitField(c, Page->AllyFields + i, 616, yPos, i);
        yPos -= 20;
    }
    
    // Enemies
    yPos = 638;
    for(u32 i = 0; i < visibleMobs; i++)
    {
        InitField *f = Page->MobFields + i;
        
        inputUse |= DrawInitField(c, Page->MobFields + i, 378, yPos, i);
        yPos -= 20;
    }
    
    // Order
    yPos = 638;
    for(u32 i = 0; i < visibleOrder; i += 2)
    {
        inputUse |= DrawOrderField(c, Page->OrderFields + i, 870, yPos, i);
        
        if((i+1) < visibleOrder)
        { inputUse |= DrawOrderField(c, Page->OrderFields + (i+1), 1056, yPos, i); }
        
        yPos -= 20;
    }
    
    // Counters
    if(!Page->InfoPane.isOpen)
    {
        yPos = 638;
        for(u32 i = 0; i < DEF_COUNTER_NUM; i++)
        {
            Counter *f = Page->Counters + i;
            
            ls_uiLabel(c, ls_utf32Constant(CounterNames[i]), 20, yPos+24);
            
            inputUse |= ls_uiTextBox(c, &f->name, 20, yPos, 100, 20);
            inputUse |= ls_uiTextBox(c, &f->rounds, 125, yPos, 36, 20);
            
            if(!f->isActive) { ls_uiButton(c, &f->start, 166, yPos, 48, 20); }
            else
            {
                inputUse |= ls_uiButton(c, &f->plusOne, 166, yPos, 48, 20);
                inputUse |= ls_uiButton(c, &f->stop, 113, yPos+22, 48, 20);
            }
            
            yPos -= 44;
        }
    }
    
    // Mob Info Left Pane
    //TODO:RECOVER nocheckin
    //_ls_uiLPane(cxt, &Page->InfoPane, 0, 180, 360, 580);
    
    //Mob Info
    if(Page->InfoPane.isOpen)
    {
        u32 infoX = 2;
        u32 infoY = 742;
        for(u32 i = 0; i < visibleMobs; i++)
        {
            InitField *f = Page->MobFields + i;
            
            ls_uiLabel(c, f->editFields[IF_IDX_NAME].text, infoX, infoY);
            
            Color base = c->widgetColor;
            
            c->widgetColor = ls_uiAlphaBlend(RGBA(0x61, 0x3B, 0x09, 150), base);
            inputUse |= ls_uiTextBox(c, &f->editFields[IF_IDX_TOTALAC], infoX + 88, infoY-6, 26, 20);
            
            c->widgetColor = ls_uiAlphaBlend(RGBA(0x9C, 0x43, 0x8B, 150), base);
            inputUse |= ls_uiTextBox(c, &f->editFields[IF_IDX_TOUCHAC], infoX + 116, infoY-6, 26, 20);
            
            c->widgetColor = ls_uiAlphaBlend(RGBA(0xD5, 0xCB, 0x35, 150), base);
            inputUse |= ls_uiTextBox(c, &f->editFields[IF_IDX_FLATAC], infoX + 144, infoY-6, 26, 20);
            
            c->widgetColor = ls_uiAlphaBlend(RGBA(0x75, 0x46, 0x46, 150), base);
            inputUse |= ls_uiTextBox(c, &f->editFields[IF_IDX_LOWAC], infoX + 144, infoY-6, 26, 20);
            
            c->widgetColor = ls_uiAlphaBlend(RGBA(0x1B, 0x18, 0x14, 150), base);
            inputUse |= ls_uiTextBox(c, &f->maxLife, infoX + 182, infoY-6, 42, 20);
            
            c->widgetColor = base;
            
            infoY -= 24;
        }
    }
    
    // Dice Throwers
    s32 xPos = 20;
    yPos = 90;
    for(u32 i = 0; i < THROWER_NUM; i++)
    {
        DiceThrow *f = Page->Throwers + i;
        
        inputUse |= ls_uiTextBox(c, &f->name,   xPos,       yPos,      120, 20);
        inputUse |= ls_uiTextBox(c, &f->toHit,  xPos,       yPos + 24, 138, 20);
        inputUse |= ls_uiTextBox(c, &f->hitRes, xPos + 138, yPos + 24, 36,  20);
        inputUse |= ls_uiTextBox(c, &f->damage, xPos,       yPos + 48, 138, 20);
        inputUse |= ls_uiTextBox(c, &f->dmgRes, xPos + 138, yPos + 48, 36,  20);
        
        inputUse |= ls_uiButton(c, &f->throwDie, xPos + 126, yPos, 48, 20);
        
        xPos += 212;
    }
    
    xPos = 20;
    yPos = 42;
    // General Thrower
    {
        inputUse |= ls_uiTextBox(c, &Page->GeneralThrower.toHit,  xPos,       yPos, 168, 20);
        inputUse |= ls_uiTextBox(c, &Page->GeneralThrower.hitRes, xPos + 168, yPos, 36,  20);
        
        inputUse |= ls_uiButton(c, &Page->GeneralThrower.throwDie, xPos + 210, yPos, 48, 20);
    }
    
    inputUse |= ls_uiButton(c, &Page->Reset, 670, 678, 48, 20);
    
    if(State.inBattle)
    {
        inputUse |= ls_uiTextBox(c, &Page->Current,      1010, 668, 100, 20);
        inputUse |= ls_uiTextBox(c, &Page->RoundCounter, 1230, 720, 30, 20);
        
        inputUse |= ls_uiButton(c, &Page->Next, 1036, 698, 48, 20);
        
        if(visibleAllies < ALLY_NUM)
        {
            InitField *f = Page->AllyFields + visibleAllies;
            
            u32 addY = 454 - (20*visibleAllies);
            if(!f->isAdding)
            {
                inputUse |= ls_uiButton(c, &f->addNew, 678, addY, 36, 20);
            }
            else
            {
                inputUse |= ls_uiTextBox(c, &f->addName, 616, addY+4, 120, 20);
                inputUse |= ls_uiTextBox(c, &f->addInit, 736, addY+4, 26, 20);
                
                inputUse |= ls_uiButton(c, &f->addConfirm, 762, addY+4, 26, 20);
            }
        }
        
        if(visibleMobs < MOB_NUM)
        {
            InitField *f = Page->MobFields + visibleMobs;
            
            u32 addY = 634 - (20*visibleMobs);
            if(!f->isAdding)
            {
                inputUse |= ls_uiButton(c, &f->addNew, 428, addY, 36, 20);
            }
            else
            {
                inputUse |= ls_uiTextBox(c, &f->addName, 378, addY+4, 120, 20);
                inputUse |= ls_uiTextBox(c, &f->addInit, 498, addY+4, 26, 20);
                
                inputUse |= ls_uiButton(c, &f->addConfirm, 524, addY+4, 26, 20);
            }
        }
    }
    
    return inputUse;
}

b32 DrawPranaStyle(UIContext *c)
{
    InitPage *Page = State.Init;
    
    s32 visibleMobs   = Page->Mobs.selectedIndex;
    s32 visibleAllies = Page->Allies.selectedIndex;
    s32 visibleOrder  = visibleMobs + visibleAllies + PARTY_NUM - Page->orderAdjust;
    
    b32 inputUse = FALSE;
    
    ls_uiSelectFontByFontSize(c, FS_SMALL);
    
    s32 yPos = 780;
    //NOTE: Z Layer 1 Input
    {
        if(!State.inBattle)
        {
            inputUse |= ls_uiListBox(c, &Page->EncounterSel,  495, yPos, 120, 20, 2);
            inputUse |= ls_uiTextBox(c, &Page->EncounterName, 644, yPos, 120, 20);
            
            inputUse |= ls_uiButton(c, &Page->SaveEnc, 617, yPos+22, 44, 20);
            inputUse |= ls_uiButton(c, &Page->RemoveEnc, 455, yPos, 24, 20);
            
            inputUse |= ls_uiListBox(c, &Page->Mobs,   76, yPos-65, 100, 20, 1);
            inputUse |= ls_uiListBox(c, &Page->Allies, 1080, yPos-225, 100, 20, 1);
            
            inputUse |= ls_uiButton(c, &Page->Roll, 536, yPos-40, 48, 20);
            inputUse |= ls_uiButton(c, &Page->Set,  698, yPos-40, 48, 20);
            inputUse |= ls_uiButton(c, &Page->Reset, 616, yPos-40, 48, 20);
        }
        
        //NOTE: We hijack the globals to know when to show the buttons.
        if(distanceFromOld != 0) { ls_uiButton(c, &Page->Undo, 1180, yPos+22, 24, 20); }
        if(distanceFromNow != 0) { ls_uiButton(c, &Page->Redo, 1220, yPos+22, 24, 20); }
    }
    
    
    yPos = 678;
    //NOTE: Z Layer 0 Input
    if(!State.inBattle)
    {
        // Party
        for(u32 i = 0; i < PARTY_NUM; i++)
        {
            ls_uiLabel(c, PartyName[i], 1085, yPos+6);
            inputUse |= ls_uiTextBox(c, Page->PlayerInit + i, 1170, yPos, 32, 20);
            yPos -= 20;
        }
        
        yPos = 678;
        for(u32 i = 0; i < visibleAllies; i++)
        {
            inputUse |= DrawInitField(c, Page->AllyFields + i, 1056, yPos-160, i);
            yPos -= 20;
        }
        
        // Enemies
        yPos = 678;
        for(u32 i = 0; i < visibleMobs; i++)
        {
            inputUse |= DrawInitField(c, Page->MobFields + i, 52, yPos, i);
            yPos -= 20;
        }
        
        yPos = 678;
        if(Page->selectedMobIndex >= 0)
        {
            AssertMsg(Page->selectedMobIndex < MOB_NUM, "Selected Mob Index is out of bounds\n");
            
            InitField *mob = Page->MobFields + Page->selectedMobIndex;
            if(mob->compendiumIdx == -1) { inputUse |= DrawInitExtra(c, mob, 436, yPos); }
            else
            { 
                static UIScrollableRegion mobViewScroll = { 260, 218, 780, 478, 0, 0, 998, 218};
                
                if(mainCachedPage.pageIndex != mob->compendiumIdx)
                { 
                    PageEntry pEntry = compendium.codex.pages[compendium.viewIndices[mob->compendiumIdx]];
                    CachePage(pEntry, mob->compendiumIdx, &mainCachedPage);
                    mobViewScroll = { 260, 218, 780, 478, 0, 0, 998, 218};
                }
                
                ls_uiStartScrollableRegion(c, &mobViewScroll);
                mobViewScroll.minY = DrawPage(c , &mainCachedPage, 260, 676, 998, 218);
                ls_uiEndScrollableRegion(c);
                
                ls_uiRect(c, 260, 218, 780, 478, RGBg(0x33), RGBg(0x11));
            }
        }
    }
    
    if(State.inBattle)
    {
        inputUse |= ls_uiTextBox(c, &Page->Current,      992, 668, 100, 20);
        inputUse |= ls_uiTextBox(c, &Page->RoundCounter, 1150, 698, 30, 20);
        inputUse |= ls_uiButton(c, &Page->Next, 1016, 698, 48, 20);
        
        // Order
        yPos = 638;
        for(u32 i = 0; i < visibleOrder; i += 2)
        {
            inputUse |= DrawOrderField(c, Page->OrderFields + i, 862, yPos, i);
            
            if((i+1) < visibleOrder)
            { inputUse |= DrawOrderField(c, Page->OrderFields + (i+1), 1048, yPos, i+1); }
            
            yPos -= 20;
        }
        
        yPos = 678;
        if(Page->selectedMobIndex >= 0)
        {
            AssertMsg(Page->selectedMobIndex < visibleOrder, "Selected Order Index is out of bounds\n");
            Order *ord = Page->OrderFields + Page->selectedMobIndex;
            InitField *mob = GetInitFieldByID(ord->ID);
            
            if(mob && ord->compendiumIdx == -1) { inputUse |= DrawInitExtra(c, mob, 66, yPos); }
            else
            { 
                static UIScrollableRegion viewScroll = { 40, 218, 760, 478, 0, 0, 758, 218};
                
                if(mainCachedPage.pageIndex != ord->compendiumIdx)
                { 
                    PageEntry pEntry = compendium.codex.pages[compendium.viewIndices[ord->compendiumIdx]];
                    CachePage(pEntry, ord->compendiumIdx, &mainCachedPage);
                    viewScroll = { 40, 218, 760, 478, 0, 0, 758, 218};
                }
                
                ls_uiStartScrollableRegion(c, &viewScroll);
                viewScroll.minY = DrawPage(c , &mainCachedPage, 40, 676, 758, 218);
                ls_uiEndScrollableRegion(c);
                
                ls_uiRect(c, 40, 218, 760, 478, RGBg(0x33), RGBg(0x11));
            }
        }
        inputUse |= ls_uiButton(c, &Page->Reset, 1212, 718, 48, 20);
    }
    
    // Counters
    yPos = 124;
    s32 xPos = 20;
    s32 xDiff = 0;
    for(u32 i = 0; i < PRANA_COUNTER_NUM; i++)
    {
        Counter *f = Page->Counters + i;
        
        if((i % 3) == 0) { yPos = 124; }
        if((i % 3) == 1) { yPos = 76; }
        if((i % 3) == 2) { yPos = 28; xDiff = 216; }
        
        ls_uiLabel(c, ls_utf32Constant(CounterNames[i]), xPos, yPos+24);
        
        inputUse |= ls_uiTextBox(c, &f->name, xPos, yPos, 100, 20);
        inputUse |= ls_uiTextBox(c, &f->rounds, xPos + 105, yPos, 36, 20);
        
        if(!f->isActive) { inputUse |= ls_uiButton(c, &f->start, xPos + 146, yPos, 48, 20); }
        else
        {
            inputUse |= ls_uiButton(c, &f->plusOne, xPos + 146, yPos, 48, 20);
            inputUse |= ls_uiButton(c, &f->stop, xPos + 93, yPos+22, 48, 20);
        }
        
        xPos += xDiff;
        xDiff = 0;
    }
    
    // Dice Throwers
    xPos = 512;
    yPos = 100;
    for(u32 i = 0; i < PRANA_THROWER_NUM; i++)
    {
        DiceThrow *f = Page->Throwers + i;
        
        if((i % 2) == 0) { yPos = 100; }
        if((i % 2) == 1) { yPos = 20; xDiff = 186; }
        
        
        inputUse |= ls_uiTextBox(c, &f->name,    xPos,      yPos + 40, 98, 20);
        inputUse |= ls_uiButton(c, &f->throwDie, xPos + 98, yPos + 40, 36, 20);
        
        inputUse |= ls_uiTextBox(c, &f->damage, xPos,       yPos + 20, 134, 20);
        inputUse |= ls_uiTextBox(c, &f->dmgRes, xPos + 134, yPos + 20, 36,  20);
        
        inputUse |= ls_uiTextBox(c, &f->toHit,  xPos,       yPos, 134, 20);
        inputUse |= ls_uiTextBox(c, &f->hitRes, xPos + 134, yPos, 36,  20);
        
        xPos += xDiff;
        xDiff = 0;
    }
    
    return inputUse;
}

b32 DrawInitTab(UIContext *c)
{
    switch(currentStyle)
    {
        case INIT_STYLE_DEFAULT: return DrawDefaultStyle(c); break;
        case INIT_STYLE_PRANA:   return DrawPranaStyle(c); break;
        
        default: AssertMsg(FALSE, "Unhandled Init Style\n"); return  FALSE;
    }
    
    AssertMsg(FALSE, "Unreachable code\n");
    return FALSE;
}