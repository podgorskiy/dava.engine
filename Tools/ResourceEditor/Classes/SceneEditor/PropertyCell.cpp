/*
 *  PropertyCell.cpp
 *  SniperEditorMacOS
 *
 *  Created by Alexey Prosin on 12/13/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "PropertyCell.h"
#include "PropertyCellData.h"
#include "ControlsFactory.h"

PropertyCell::PropertyCell(PropertyCellDelegate *propDelegate, const Rect &rect, PropertyCellData *prop)
:UIListCell(rect, GetTypeName(prop->cellType))
{
    ControlsFactory::CustomizePropertyCell(this, false);
    
    propertyDelegate = propDelegate;

    property = prop;
    keyName = new UIStaticText(Rect(0, 0, size.x, size.y));
    keyName->SetFont(ControlsFactory::CreateFontLight());
    AddControl(keyName);
}

PropertyCell::~PropertyCell()
{
    SafeRelease(keyName);
}


void PropertyCell::SetData(PropertyCellData *prop)
{
    property = prop;
    property->currentCell = this;
    keyName->SetText(StringToWString(prop->key) + L" :");
}

String PropertyCell::GetTypeName(int cellType)
{
    return Format("PropCellType%d", cellType);
}


PropertyTextCell::PropertyTextCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width)
: PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), prop)
{
    keyName->size.x = width/2;
  
    Font * font = ControlsFactory::CreateFontLight();
    editableText = new UITextField(Rect(width/2, 0, width/2, size.y));
    ControlsFactory::CustomizeEditablePropertyCell(editableText);
    editableText->SetFont(font);
    editableText->SetDelegate(this);
    
    uneditableTextContainer = new UIControl(Rect(width/2, 0, width/2, size.y));
    ControlsFactory::CustomizeUneditablePropertyCell(uneditableTextContainer);
    uneditableText = new UIStaticText(Rect(0, 0, uneditableTextContainer->size.x, uneditableTextContainer->size.y));
    uneditableText->SetFont(font);
    SafeRelease(font);
    uneditableTextContainer->AddControl(uneditableText);
    
    SetData(prop);
}

PropertyTextCell::~PropertyTextCell()
{
    SafeRelease(editableText);
    SafeRelease(uneditableTextContainer);
    SafeRelease(uneditableText);
}


void PropertyTextCell::DidAppear()
{
    editableText->ReleaseFocus();
}

void PropertyTextCell::SetData(PropertyCellData *prop)
{
    PropertyCell::SetData(prop);

    switch (prop->GetValueType())
    {
        case PropertyCellData::PROP_VALUE_STRING:
            editableText->SetText(StringToWString(prop->GetString()));
            uneditableText->SetText(StringToWString(prop->GetString()));
            break;
        case PropertyCellData::PROP_VALUE_INTEGER:
            editableText->SetText( StringToWString( Format("%d", prop->GetInt()) ) );
            uneditableText->SetText( StringToWString( Format("%d", prop->GetInt()) ) );
            break;
        case PropertyCellData::PROP_VALUE_FLOAT:
            editableText->SetText( StringToWString( Format("%.3f", prop->GetFloat()) ) );
            uneditableText->SetText( StringToWString( Format("%.3f", prop->GetFloat()) ) );
            break;
    }
    
    if (prop->isEditable) 
    {
        if (!editableText->GetParent()) 
        {
            RemoveControl(uneditableTextContainer);
            AddControl(editableText);
            editableText->ReleaseFocus();
        }
    }
    else 
    {
        if (!uneditableTextContainer->GetParent()) 
        {
            RemoveControl(editableText);
            AddControl(uneditableTextContainer);
            editableText->ReleaseFocus();
        }
    }
    
}

void PropertyTextCell::TextFieldShouldReturn(UITextField * textField)
{
    editableText->ReleaseFocus();
    switch (property->GetValueType())
    {
        case PropertyCellData::PROP_VALUE_STRING:
            property->SetString(WStringToString(editableText->GetText()));
            break;
        case PropertyCellData::PROP_VALUE_INTEGER:
            property->SetInt(atoi(WStringToString(editableText->GetText()).c_str()));
            break;
        case PropertyCellData::PROP_VALUE_FLOAT:
            property->SetFloat(atof(WStringToString(editableText->GetText()).c_str()));
            break;
    }
    propertyDelegate->OnPropertyChanged(property);
};

bool PropertyTextCell::TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString)
{
    if (replacementLength < 0) 
    {
        return true;
    }
    switch (property->GetValueType())
    {
        case PropertyCellData::PROP_VALUE_STRING:
            return true;
            break;
        case PropertyCellData::PROP_VALUE_INTEGER:
        {
            WideString newText = textField->GetAppliedChanges(replacementLocation, replacementLength, replacementString);
            bool allOk;
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
                if (!allOk) 
                {
                    return false;
                }
            }
            return true;
        }
            break;
        case PropertyCellData::PROP_VALUE_FLOAT:
        {
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
        }
            break;
    }
    return false;
};

float32 PropertyTextCell::GetHeightForWidth(float32 currentWidth)
{
    return 30.f;
}

//********************* PropertyBoolCell *********************
PropertyBoolCell::PropertyBoolCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width)
: PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), prop)
{
    keyName->size.x = width/2;
    
    float32 usedWidth = keyName->size.x;
    float32 checkBoxWidth = GetHeightForWidth(usedWidth);
    
    textContainer = new UIControl(Rect(usedWidth + checkBoxWidth, 0, usedWidth - checkBoxWidth, checkBoxWidth));
    ControlsFactory::CustomizeUneditablePropertyCell(textContainer);
    AddControl(textContainer);
    
    Font * font = ControlsFactory::CreateFontLight();
    falseText = new UIStaticText(Rect(0, 0, usedWidth - checkBoxWidth, checkBoxWidth));
    falseText->SetText(L"False");
    falseText->SetFont(font);
    
    trueText = new UIStaticText(Rect(0, 0, usedWidth - checkBoxWidth, checkBoxWidth));
    trueText->SetText(L"True");
    trueText->SetFont(font);
    SafeRelease(font);
    
    checkBox = new UICheckBox("~res:/Gfx/UI/chekBox", Rect(usedWidth, 0, checkBoxWidth, checkBoxWidth));
    checkBox->SetDelegate(this);
    AddControl(checkBox);
    
    SetData(prop);
}

PropertyBoolCell::~PropertyBoolCell()
{
    SafeRelease(textContainer);
    SafeRelease(checkBox);
    SafeRelease(falseText);
    SafeRelease(trueText);
}

void PropertyBoolCell::SetData(PropertyCellData *prop)
{
    PropertyCell::SetData(prop);
    
    switch (prop->GetValueType())
    {
        case PropertyCellData::PROP_VALUE_BOOL:
            checkBox->SetChecked(prop->GetBool());
            break;
            
        default:
            break;
    }
}

float32 PropertyBoolCell::GetHeightForWidth(float32 currentWidth)
{
    return 30.f;
}

void PropertyBoolCell::ValueChanged(bool newValue)
{
    property->SetBool(newValue);
    
    if(newValue)
    {
        if(falseText->GetParent())
        {
            textContainer->RemoveControl(falseText);
        }
        
        if(!trueText->GetParent())
        {
            textContainer->AddControl(trueText);
        }
    }
    else
    {
        if(trueText->GetParent())
        {
            textContainer->RemoveControl(trueText);
        }
        
        if(!falseText->GetParent())
        {
            textContainer->AddControl(falseText);
        }
    }
}