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


#include "PreviewWidget.h"

#include "ScrollAreaController.h"

#include <QLineEdit>
#include <QScreen>
#include "UI/UIControl.h"
#include "UI/UIScreenManager.h"

#include "QtTools/DavaGLWidget/davaglwidget.h"

#include "Document.h"
#include "Systems/CanvasSystem.h"
#include "Systems/HUDSystem.h"

using namespace DAVA;

namespace
{
    QString ScaleFromInt(int scale)
    {
        return QString("%1 %").arg(scale);
    }
}

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent)
    , scrollAreaController(new ScrollAreaController(this))
{
    percentages << 10 << 25 << 50 << 75 << 100 << 125
        << 150 << 175 << 200 << 250 << 400 << 800;
    setupUi(this);
    davaGLWidget = new DavaGLWidget();
    frame->layout()->addWidget(davaGLWidget);

    davaGLWidget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    connect( davaGLWidget, &DavaGLWidget::Resized, this, &PreviewWidget::OnGLWidgetResized );

    // Setup the Scale Combo.
    for (auto percentage : percentages)
    {
        scaleCombo->addItem(ScaleFromInt(percentage));
    }
    connect(scrollAreaController, &ScrollAreaController::ViewSizeChanged, this, &PreviewWidget::UpdateScrollArea);
    connect(scrollAreaController, &ScrollAreaController::CanvasSizeChanged, this, &PreviewWidget::UpdateScrollArea);

    connect(scaleCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PreviewWidget::OnScaleByComboIndex);
    connect(scaleCombo->lineEdit(), &QLineEdit::editingFinished, this, &PreviewWidget::OnScaleByComboText);

    connect(verticalScrollBar, &QScrollBar::valueChanged, this, &PreviewWidget::OnVScrollbarMoved);
    connect(horizontalScrollBar, &QScrollBar::valueChanged, this, &PreviewWidget::OnHScrollbarMoved);

    connect(davaGLWidget->GetGLWindow(), &QWindow::screenChanged, this, &PreviewWidget::OnMonitorChanged);

    scaleCombo->setCurrentIndex(percentages.indexOf(100)); //100%
    scaleCombo->lineEdit()->setMaxLength(6); //3 digits + whitespace + % ?
    scaleCombo->setInsertPolicy(QComboBox::NoInsert);
    UpdateScrollArea();
}

DavaGLWidget *PreviewWidget::GetDavaGLWidget()
{
    return davaGLWidget;
}

ScrollAreaController* PreviewWidget::GetScrollAreaController()
{
    return scrollAreaController;
}

void PreviewWidget::OnDocumentChanged(Document *arg)
{
    scrollAreaController->GetBackgroundControl()->RemoveAllControls();
    document = arg;
    if (nullptr != document)
    {
        auto root = document->GetRootControl();
        DVASSERT(nullptr != root);
        scrollAreaController->GetBackgroundControl()->AddControl(root);
        scrollAreaController->SetNestedControl(root);
        
        scaleCombo->lineEdit()->setText(ScaleFromInt(document->GetScale()));
    }
}

void PreviewWidget::OnMonitorChanged()
{
    OnScaleByComboIndex(scaleCombo->currentIndex());
}

void PreviewWidget::UpdateScrollArea()
{
    QSize areaSize = scrollAreaController->GetViewSize();
    QSize contentSize = scrollAreaController->GetCanvasSize();

    verticalScrollBar->setPageStep(areaSize.height());
    horizontalScrollBar->setPageStep(areaSize.width());
    
    verticalScrollBar->setRange(0, contentSize.height() - areaSize.height());
    horizontalScrollBar->setRange(0, contentSize.width() - areaSize.width());

}

void PreviewWidget::OnScaleByZoom(int scaleDelta)
{
    //TODO: implement this method
}

void PreviewWidget::OnScaleByComboIndex(int index)
{   
    DVASSERT(index >= 0);
    int scale = percentages.at(index);
    scrollAreaController->SetScale(scale);
}

void PreviewWidget::OnScaleByComboText()
{
	// Firstly verify whether the value is already set.
	QString curTextValue = scaleCombo->currentText().trimmed();
	int scaleValue = 0;
	if (curTextValue.endsWith(" %"))
	{
		int endCharPos = curTextValue.lastIndexOf(" %");
		QString remainderNumber = curTextValue.left(endCharPos);
		scaleValue = remainderNumber.toInt();
	}
	else
	{
		// Try to parse the value.
		scaleValue = curTextValue.toFloat();
	}
    scrollAreaController->SetScale(scaleValue);
}

void PreviewWidget::OnZoomInRequested()
{
	OnScaleByZoom(10);
}

void PreviewWidget::OnZoomOutRequested()
{
	OnScaleByZoom(-10);
}

void PreviewWidget::OnGLWidgetResized(int width, int height, int dpr)
{
    scrollAreaController->SetViewSize(QSize(width * dpr, height * dpr));
    UpdateScrollArea();
}

void PreviewWidget::OnVScrollbarMoved(int vPosition)
{
    QPoint canvasPosition = scrollAreaController->GetPosition();
    canvasPosition.setY(-vPosition);
    scrollAreaController->SetPosition(canvasPosition);
}

void PreviewWidget::OnHScrollbarMoved(int hPosition)
{
    QPoint canvasPosition = scrollAreaController->GetPosition();
    canvasPosition.setX(-hPosition);
    scrollAreaController->SetPosition(canvasPosition);
}
