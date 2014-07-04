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

#include "Render/2D/TextBlockDistanceRender.h"
#include "Render/RenderManager.h"
#include "Core/Core.h"
#include "Render/ShaderCache.h"

namespace DAVA 
{
    
    
static uint16* InitIndexBuffer()
{
    static uint16 buffer[DF_FONT_INDEX_BUFFER_SIZE];
    
    uint16 a = 0;
    for (int32 i = 0; i < DF_FONT_INDEX_BUFFER_SIZE;)
    {
        buffer[i] = buffer[i+3] = a;
        buffer[i+1] = a+1;
        buffer[i+2] = buffer[i+4] = a+2;
        buffer[i+5] = a+3;
        i+=6;
        a+=4;
    }
    return buffer;
}
    
uint16* TextBlockDistanceRender::indexBuffer = InitIndexBuffer();
	
FastName TextBlockDistanceRender::textureUniform("texture0");
FastName TextBlockDistanceRender::smoothingUniform("smoothing");
FastName TextBlockDistanceRender::colorUniform("color");

TextBlockDistanceRender::TextBlockDistanceRender(TextBlock* textBlock) :
	TextBlockRender(textBlock)
{
	charDrawed = 0;
	renderObject = new RenderDataObject();
	
	dfFont = (DFFont*)textBlock->font;
	
    shader = ShaderCache::Instance()->Get(FastName("~res:/Shaders/Default/df_font"), FastNameSet());
    shader->Retain();
}
	
TextBlockDistanceRender::~TextBlockDistanceRender()
{
    shader->Release();
    shader = NULL;

	SafeRelease(renderObject);
}
	
void TextBlockDistanceRender::Prepare()
{
	charDrawed = 0;
	renderRect = Rect(0, 0, 0, 0);
	DrawText();
    
	if (charDrawed == 0)
		return;
	
	renderObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, sizeof(DFFont::DFFontVertex), &vertexBuffer[0].position);
	renderObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, sizeof(DFFont::DFFontVertex), &vertexBuffer[0].texCoord);
	renderObject->BuildVertexBuffer(charDrawed * 4);
}

void TextBlockDistanceRender::Draw(const Color& textColor, const Vector2* offset)
{
	if (charDrawed == 0)
		return;
	
	int32 xOffset = textBlock->position.x - textBlock->pivotPoint.x;
	int32 yOffset = textBlock->position.y - textBlock->pivotPoint.y;

	if (offset)
	{
		xOffset += offset->x;
		yOffset += offset->y;
	}
	
	int32 align = textBlock->GetAlign();
	if (align & ALIGN_RIGHT)
	{
		xOffset += Max(0.f, textBlock->rectSize.dx - renderRect.dx);
	}
	else if (align & ALIGN_HCENTER)
	{
		xOffset += Max(0.f, (textBlock->rectSize.dx - renderRect.dx) * 0.5f);
	}
	
	if (align & ALIGN_BOTTOM)
	{
		yOffset += Max(0.f, textBlock->rectSize.dy - renderRect.dy);
	}
	else if ((align & ALIGN_VCENTER) || (align & ALIGN_HJUSTIFY))
	{
		yOffset += Max(0.f, (textBlock->rectSize.dy - renderRect.dy) * 0.5f);
	}
    RenderManager::Instance()->ClipPush();
    RenderManager::Instance()->ClipRect(Rect(xOffset, yOffset, textBlock->rectSize.dx, textBlock->rectSize.dy));
    
    RenderManager::Instance()->PushDrawMatrix();
    RenderManager::Instance()->SetDrawTranslate(Vector2(xOffset, yOffset));

    RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_2D_BLEND);
    RenderManager::Instance()->SetTextureState(dfFont->GetTextureHandler());
	RenderManager::Instance()->SetShader(shader);
    RenderManager::Instance()->SetRenderData(renderObject);
	RenderManager::Instance()->FlushState();
	RenderManager::Instance()->AttachRenderData();
    
    int idx;
    idx = shader->FindUniformIndexByName(textureUniform);
    shader->SetUniformValueByIndex(idx, 0);
    idx = shader->FindUniformIndexByName(smoothingUniform);
    shader->SetUniformValueByIndex(idx, dfFont->GetSpread());
    idx = shader->FindUniformIndexByName(colorUniform);
    shader->SetUniformColor4ByIndex(idx, textColor);
    
	RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, charDrawed * 6, EIF_16, this->indexBuffer);
    
    RenderManager::Instance()->PopDrawMatrix();
    RenderManager::Instance()->ClipPop();
}
	
Size2i TextBlockDistanceRender::DrawTextSL(const WideString& drawText, int32 x, int32 y, int32 w)
{
	return InternalDrawText(drawText, 0, 0, 0, 0);
}
		
Size2i TextBlockDistanceRender::DrawTextML(const WideString& drawText,
										   int32 x, int32 y, int32 w,
										   int32 xOffset, uint32 yOffset,
										   int32 lineSize)
{
	return InternalDrawText(drawText, xOffset, yOffset, w, lineSize);
}
	
Size2i TextBlockDistanceRender::InternalDrawText(const WideString& drawText, int32 x, int32 y, int32 w, int32 lineSize)
{
	if (drawText.empty())
		return Size2i(0, 0);
	
	int32 lastDrawed = 0;
	
	Size2i drawRect = dfFont->DrawStringToBuffer(drawText, x, y, vertexBuffer + (charDrawed * 4), lastDrawed, NULL, w, lineSize);
	if (drawRect.dx <= 0 && drawRect.dy <= 0)
		return drawRect;
	
	renderRect = renderRect.Combine(Rect(0, 0, drawRect.dx, drawRect.dy));
	this->charDrawed += lastDrawed;
	return drawRect;
}
	
};