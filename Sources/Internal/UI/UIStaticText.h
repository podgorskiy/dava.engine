/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#ifndef __DAVAENGINE_UI_STATIC_TEXT_H__
#define __DAVAENGINE_UI_STATIC_TEXT_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
//#include "FTFont.h"
//#include "Texture.h"
//#include "Sprite.h"
#include "Render/2D/TextBlock.h"

namespace DAVA 
{
class UIStaticText : public UIControl 
{
public:
	
	UIStaticText(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false); 
	virtual ~UIStaticText();

	//if requested size is 0 - text creates in the rect with size of the drawRect on draw phase
	//if requested size is >0 - text creates int the rect with the requested size
	//if requested size in <0 - rect creates for the all text size	
	void SetText(const WideString & string, const Vector2 &requestedTextRectSize = Vector2(0,0));
	void SetFont(Font * font);
    void SetTextColor(const Color& color);

	void SetShadowColor(const Color &color);
	void SetShadowOffset(const Vector2 &offset);

	void SetMultiline(bool isMultilineEnabled, bool bySymbol = false);
	bool GetMultiline() const;
	bool GetMultilineBySymbol() const;

	void SetFittingOption(int32 fittingType);//may be FITTING_DISABLED, FITTING_ENLARGE, FITTING_REDUCE, FITTING_ENLARGE | FITTING_REDUCE
	
	//for background sprite
	virtual void SetAlign(int32 _align);
	virtual int32 GetAlign() const;

	virtual void SetTextAlign(int32 _align);
	virtual int32 GetTextAlign() const;

	const Vector2 &GetTextSize();
	
	inline void PrepareSprite()
	{
		if (textBlock->IsSpriteReady())
		{
			shadowBg->SetSprite(textBlock->GetSprite(), 0);
			textBg->SetSprite(textBlock->GetSprite(), 0);
		}
		else 
		{
			shadowBg->SetSprite(NULL, 0);
			textBg->SetSprite(NULL, 0);
		}
	}

	
	const WideString & GetText();
    const Vector<WideString> & GetMultilineStrings();
	
	Font * GetFont() { return textBlock->GetFont(); }
	
	virtual UIControl *Clone();
	virtual void CopyDataFrom(UIControl *srcControl);
	UIStaticText *CloneStaticText();
	TextBlock * GetTextBlock() { return textBlock; }
	const Color &GetTextColor() const;
	const Color &GetShadowColor() const;
	const Vector2 &GetShadowOffset() const;

	// Animation methods for Text Color and Shadow Color.
	virtual Animation *	ColorAnimation(const Color & finalColor, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
	virtual Animation* ShadowColorAnimation(const Color & finalColor, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 1);

protected:
	Color textColor;
	TextBlock *textBlock;
	Vector2 tempSize;
	Vector2 shadowOffset;
	Color shadowColor;
	UIControlBackground *shadowBg;
	UIControlBackground *textBg;
	
	virtual void Draw(const UIGeometricData &geometricData);
	
public:
	void LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader);
	virtual YamlNode * SaveToYamlNode(UIYamlLoader * loader);
};
};

#endif //__DAVAENGINE_UI_STATIC_TEXT_H__
