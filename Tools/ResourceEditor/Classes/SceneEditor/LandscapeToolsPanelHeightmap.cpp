#include "LandscapeToolsPanelHeightmap.h"
#include "ControlsFactory.h"
#include "LandscapeTool.h"

#pragma mark  --LandscapeToolsPanelHeightmap
LandscapeToolsPanelHeightmap::LandscapeToolsPanelHeightmap(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect)
    :   LandscapeToolsPanel(newDelegate, rect)
{
    Rect sizeRect(rect.dx - ControlsFactory::BUTTON_HEIGHT - TEXTFIELD_WIDTH, 
                  0, TEXTFIELD_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2);
    sizeValue = CreateTextField(sizeRect);
    AddControl(sizeValue);
    
    Rect strengthRect(sizeRect);
    strengthRect.y = sizeRect.y + sizeRect.dy;
    strengthValue = CreateTextField(strengthRect);
    AddControl(strengthValue);
    
    Rect lineRect;
    lineRect.x = strengthSlider->GetRect().x + strengthSlider->GetRect().dx/2;
    lineRect.dx = 2;
    lineRect.y = strengthSlider->GetRect().y - 2;
    lineRect.dy = strengthSlider->GetRect().dy + 4;
    UIControl *line = ControlsFactory::CreateLine(lineRect);
    AddControl(line);
    SafeRelease(line);

    int32 x = ControlsFactory::TOOLS_HEIGHT/2 + OFFSET + TEXT_WIDTH;
    relative = CreateCkeckbox(Rect(x, ControlsFactory::TOOLS_HEIGHT, 
                                   ControlsFactory::TOOLS_HEIGHT/2, ControlsFactory::TOOLS_HEIGHT/2), 
                              LocalizedString(L"landscapeeditor.relative"));
    x += (ControlsFactory::TOOLS_HEIGHT/2 + OFFSET + TEXT_WIDTH);
    average = CreateCkeckbox(Rect(x, ControlsFactory::TOOLS_HEIGHT, 
                                  ControlsFactory::TOOLS_HEIGHT/2, ControlsFactory::TOOLS_HEIGHT/2), 
                              LocalizedString(L"landscapeeditor.average"));
    
    Rect heightRect;
    heightRect.x = average->GetRect().x + average->GetRect().dx + TEXT_WIDTH + OFFSET + ControlsFactory::OFFSET/2.0;
    heightRect.y = average->GetRect().y;
    heightRect.dx = TEXTFIELD_WIDTH;
    heightRect.dy = average->GetRect().dy;
    heightValue = CreateTextField(Rect(heightRect));
    heightValue->SetText(L"0");
    AddControl(heightValue);
    
    heightRect.x = heightRect.x + heightRect.dx + ControlsFactory::OFFSET/2.0;
    heightRect.dx = TEXT_WIDTH;
    
    UIStaticText *textControl = new UIStaticText(heightRect);
    textControl->SetText(LocalizedString(L"landscapeeditor.height"));
    textControl->SetFont(ControlsFactory::GetFontLight());
    textControl->SetAlign(ALIGN_VCENTER | ALIGN_LEFT);
    AddControl(textControl);
    SafeRelease(textControl);
    
    averageStrength = CreateSlider(Rect(heightRect.x + heightRect.dx + TEXT_WIDTH, heightRect.y, 
                                        SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));
    averageStrength->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeToolsPanelHeightmap::OnAverageSizeChanged));
    AddControl(averageStrength);
    AddSliderHeader(averageStrength, LocalizedString(L"landscapeeditor.averagestrength"));

    
    showGrid = CreateCkeckbox(Rect(0, ControlsFactory::TOOLS_HEIGHT, ControlsFactory::TOOLS_HEIGHT/2, ControlsFactory::TOOLS_HEIGHT/2), 
                              LocalizedString(L"landscapeeditor.showgrid"));

    
    dropperTool = new LandscapeTool(-1, LandscapeTool::TOOL_DROPPER, "~res:/LandscapeEditor/SpecialTools/dropper.png");
    dropperTool->size = 1.0f;
    dropperTool->height = prevHeightValue = 0.f;
    
    Rect dropperRect = brushIcon->GetRect();
    dropperRect.x = (dropperRect.x + dropperRect.dx + ControlsFactory::OFFSET);
    dropperIcon = new UIControl(dropperRect);
    dropperIcon->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LandscapeToolsPanelHeightmap::OnDropperTool));
    dropperIcon->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
    dropperIcon->SetSprite(dropperTool->sprite, 0);
    AddControl(dropperIcon);
    
    
    sizeValue->SetText(Format(L"%.3f", LandscapeTool::SizeHeightMax()));
    strengthValue->SetText(Format(L"%.3f", LandscapeTool::StrengthHeightMax()));
    
    relative->SetChecked(true, false);

    sizeSlider->SetMinMaxValue(0.f, LandscapeTool::SizeHeightMax());
    sizeSlider->SetValue(LandscapeTool::DefaultSizeHeight());

    strengthSlider->SetMinMaxValue(-LandscapeTool::StrengthHeightMax(), LandscapeTool::StrengthHeightMax());
    strengthSlider->SetValue(LandscapeTool::DefaultStrengthHeight());
    
    averageStrength->SetMinMaxValue(0.f, 1.f);
    averageStrength->SetValue(0.5f);
}


LandscapeToolsPanelHeightmap::~LandscapeToolsPanelHeightmap()
{
    SafeRelease(dropperIcon);
    SafeRelease(dropperTool);

    SafeRelease(heightValue);
    
    SafeRelease(average);
    SafeRelease(relative);
    
    SafeRelease(sizeValue);
    SafeRelease(strengthValue);
}

void LandscapeToolsPanelHeightmap::WillAppear()
{
    LandscapeToolsPanel::WillAppear();
        
    showGrid->SetChecked(showGrid->Checked(), true);
}


UITextField *LandscapeToolsPanelHeightmap::CreateTextField(const Rect &rect)
{
    Font * font = ControlsFactory::GetFontLight();
    UITextField *tf = new UITextField(rect);
    ControlsFactory::CustomizeEditablePropertyCell(tf);
    tf->SetFont(font);
    tf->SetDelegate(this);
    
    return tf;
}

void LandscapeToolsPanelHeightmap::OnDropperTool(DAVA::BaseObject *object, void *userData, void *callerData)
{
    selectedTool = dropperTool;
    if(delegate)
    {
        delegate->OnToolSelected(selectedTool);
    }
    
    if(average->Checked())
    {
        average->SetChecked(false, true);
    }
    if(relative->Checked())
    {
        relative->SetChecked(false, true);
    }
    
    ToolIconSelected(dropperIcon);
}

void LandscapeToolsPanelHeightmap::OnAverageSizeChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    selectedTool->averageStrength = averageStrength->GetValue();
}

void LandscapeToolsPanelHeightmap::Update(float32 timeElapsed)
{
    if(selectedTool->height != prevHeightValue)
    {
        prevHeightValue = selectedTool->height;
        heightValue->SetText(Format(L"%.3f", selectedTool->height));
        
        if(selectedBrushTool)
        {
            selectedBrushTool->height = selectedTool->height;
        }
        dropperTool->height = selectedTool->height;
    }
    
    LandscapeToolsPanel::Update(timeElapsed);
}

void LandscapeToolsPanelHeightmap::ToolIconSelected(UIControl *focused)
{
    dropperIcon->SetDebugDraw(focused == dropperIcon);
    LandscapeToolsPanel::ToolIconSelected(focused);
}


#pragma mark  --UITextFieldDelegate
void LandscapeToolsPanelHeightmap::TextFieldShouldReturn(UITextField * textField)
{
    textField->ReleaseFocus();
}

void LandscapeToolsPanelHeightmap::TextFieldShouldCancel(UITextField * textField)
{
    textField->ReleaseFocus();
}

void LandscapeToolsPanelHeightmap::TextFieldLostFocus(UITextField * textField)
{
    if(textField == sizeValue)
    {
        float32 value = atof(WStringToString(sizeValue->GetText()).c_str());
        
        sizeSlider->SetMinMaxValue(0.f, value);
        selectedBrushTool->maxSize = value;
        if(value < selectedBrushTool->size)
        {
            selectedBrushTool->size = value;
        }
        sizeSlider->SetValue(selectedBrushTool->size);
    }
    else if(textField == strengthValue)
    {
        float32 value = fabsf(atof(WStringToString(strengthValue->GetText()).c_str()));

        strengthSlider->SetMinMaxValue(-value, value);
        selectedBrushTool->maxStrength = value;
        if(value < selectedBrushTool->strength)
        {
            selectedBrushTool->strength = value;
        }
        else if(selectedBrushTool->strength < -value)
        {
            selectedBrushTool->strength = -value;
        }
        strengthSlider->SetValue(selectedBrushTool->strength);
    }
    else if(textField == heightValue)
    {
        prevHeightValue = fabsf(atof(WStringToString(heightValue->GetText()).c_str()));
        dropperTool->height = prevHeightValue;

        if(selectedBrushTool)
        {
            selectedBrushTool->height = prevHeightValue;
        }
    }
}

bool LandscapeToolsPanelHeightmap::TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString)
{
    if (replacementLength < 0) 
    {
        return true;
    }

    WideString newText = textField->GetAppliedChanges(replacementLocation, replacementLength, replacementString);
    bool allOk;
    int pointsCount = 0;
    for (int i = 0; i < newText.length(); i++) 
    {
        allOk = false;
        if (newText[i] == L'-' && i == 0)
        {
            allOk = true;
        }
        else if(newText[i] >= L'0' && newText[i] <= L'9')
        {
            allOk = true;
        }
        else if(newText[i] == L'.' && pointsCount == 0)
        {
            allOk = true;
            pointsCount++;
        }
        if (!allOk) 
        {
            return false;
        }
    }
    return true;
};

#pragma mark  --UICheckBoxDelegate
void LandscapeToolsPanelHeightmap::ValueChanged(UICheckBox *forCheckbox, bool newValue)
{
    LandscapeToolsPanel::ValueChanged(forCheckbox, newValue);

    if(forCheckbox == average)
    {
        if(newValue)
        {
            relative->SetChecked(false, false);
        }
    }
    else if(forCheckbox == relative)
    {
        if(newValue)
        {
            average->SetChecked(false, false);
        }
    }
    else if(forCheckbox == showGrid)
    {
        if(delegate)
        {
            delegate->OnShowGrid(showGrid->Checked());
        }
    }

    if(selectedBrushTool)
    {
        selectedBrushTool->averageDrawing = average->Checked();
        selectedBrushTool->relativeDrawing = relative->Checked();
    }
}

#pragma mark  --LandscapeToolsSelectionDelegate
void LandscapeToolsPanelHeightmap::OnToolSelected(LandscapeToolsSelection * forControl, LandscapeTool *newTool)
{
    newTool->height = atof(WStringToString(heightValue->GetText()).c_str());
    newTool->averageDrawing = average->Checked();
    newTool->relativeDrawing = relative->Checked();
    
    newTool->maxStrength = fabsf(atof(WStringToString(strengthValue->GetText()).c_str()));
    newTool->maxSize = fabsf(atof(WStringToString(sizeValue->GetText()).c_str()));
    newTool->averageStrength = averageStrength->GetValue();

    LandscapeToolsPanel::OnToolSelected(forControl, newTool);
}

