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


#include "VisibilityToolPanel.h"
#include "Scene/SceneSignals.h"
#include "Scene/SceneEditor2.h"
#include "Tools/SliderWidget/SliderWidget.h"
#include "Tools/QtWaitDialog/QtWaitDialog.h"
#include "Constants.h"
#include "Main/QtUtils.h"
#include "Qt/DockLandscapeEditorControls/LandscapeEditorShortcutManager.h"

#include "Tools/PathDescriptor/PathDescriptor.h"


#include "QtTools/FileDialog/FileDialog.h"

#include <QLayout>
#include <QPushButton>
#include <QLabel>

VisibilityToolPanel::VisibilityToolPanel(QWidget* parent)
    : LandscapeEditorBasePanel(parent)
{
	InitUI();
	ConnectToSignals();
}

VisibilityToolPanel::~VisibilityToolPanel()
{
}

bool VisibilityToolPanel::GetEditorEnabled()
{
	SceneEditor2* sceneEditor = GetActiveScene();
	if (!sceneEditor)
	{
		return false;
	}

	return sceneEditor->visibilityToolSystem->IsLandscapeEditingEnabled();
}

void VisibilityToolPanel::SetWidgetsState(bool enabled)
{
	buttonSetVisibilityPoint->setEnabled(enabled);
	buttonSetVisibilityArea->setEnabled(enabled);
	buttonSaveTexture->setEnabled(enabled);
}

void VisibilityToolPanel::BlockAllSignals(bool block)
{
	buttonSetVisibilityPoint->blockSignals(block);
	buttonSetVisibilityArea->blockSignals(block);
	buttonSaveTexture->blockSignals(block);
}

void VisibilityToolPanel::InitUI()
{
	QVBoxLayout* layout = new QVBoxLayout(this);

	buttonSetVisibilityPoint = new QPushButton(this);
	buttonSetVisibilityArea = new QPushButton(this);
	buttonSaveTexture = new QPushButton(this);
	QSpacerItem* spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);

	layout->addWidget(buttonSetVisibilityPoint);
	layout->addWidget(buttonSetVisibilityArea);
	layout->addWidget(buttonSaveTexture);
	layout->addSpacerItem(spacer);

	setLayout(layout);

	SetWidgetsState(false);
	BlockAllSignals(true);

	buttonSetVisibilityPoint->setText(ResourceEditor::VISIBILITY_TOOL_SET_POINT_CAPTION.c_str());
	buttonSetVisibilityPoint->setCheckable(true);

    buttonSetVisibilityArea->setText(ResourceEditor::VISIBILITY_TOOL_COMPUTE_VISIBILITY_CAPTION.c_str());
    buttonSaveTexture->setText(ResourceEditor::VISIBILITY_TOOL_SAVE_TEXTURE_CAPTION.c_str());
}

void VisibilityToolPanel::ConnectToSignals()
{
    connect(SceneSignals::Instance(),
            SIGNAL(VisibilityToolStateChanged(SceneEditor2*, VisibilityToolSystem::State)),
            this,
            SLOT(SetVisibilityToolButtonsState(SceneEditor2*, VisibilityToolSystem::State)));
    connect(SceneSignals::Instance(), SIGNAL(VisibilityToolToggled(SceneEditor2*)),
			this, SLOT(EditorToggled(SceneEditor2*)));

	connect(buttonSaveTexture, SIGNAL(clicked()), this, SLOT(SaveTexture()));
	connect(buttonSetVisibilityPoint, SIGNAL(clicked()), this, SLOT(SetVisibilityPoint()));
	connect(buttonSetVisibilityArea, SIGNAL(clicked()), this, SLOT(SetVisibilityArea()));
}

void VisibilityToolPanel::StoreState()
{
}

void VisibilityToolPanel::RestoreState()
{
	SceneEditor2* sceneEditor = GetActiveScene();

	bool enabled = sceneEditor->visibilityToolSystem->IsLandscapeEditingEnabled();
    VisibilityToolSystem::State state = sceneEditor->visibilityToolSystem->GetState();

    SetWidgetsState(enabled);
	
	BlockAllSignals(true);
    buttonSetVisibilityPoint->setChecked(state == VisibilityToolSystem::State::SetPoint);
    BlockAllSignals(!enabled);
}

// these functions are designed to convert values from sliders in ui
// to the values suitable for visibility tool system
int32 VisibilityToolPanel::AreaSizeUIToSystem(int32 uiValue)
{
	int32 systemValue = uiValue * ResourceEditor::LANDSCAPE_BRUSH_SIZE_UI_TO_SYSTEM_COEF;
	return systemValue;
}

int32 VisibilityToolPanel::AreaSizeSystemToUI(int32 systemValue)
{
	int32 uiValue = systemValue / ResourceEditor::LANDSCAPE_BRUSH_SIZE_UI_TO_SYSTEM_COEF;
	return uiValue;
}
// end of convert functions ==========================

void VisibilityToolPanel::SetVisibilityToolButtonsState(SceneEditor2* scene, VisibilityToolSystem::State state)
{
	if (scene != GetActiveScene())
	{
		return;
	}

    bool b = buttonSetVisibilityPoint->signalsBlocked();
    buttonSetVisibilityPoint->blockSignals(true);
    buttonSetVisibilityPoint->setChecked(state == VisibilityToolSystem::State::SetPoint);
    buttonSetVisibilityPoint->blockSignals(b);
	
	b = buttonSetVisibilityArea->signalsBlocked();
	buttonSetVisibilityArea->blockSignals(true);
	buttonSetVisibilityArea->blockSignals(b);
}

void VisibilityToolPanel::SaveTexture()
{
	FilePath currentPath = FileSystem::Instance()->GetUserDocumentsPath();
	QString filePath = FileDialog::getSaveFileName(NULL,
													QString(ResourceEditor::VISIBILITY_TOOL_SAVE_CAPTION.c_str()),
													QString(currentPath.GetAbsolutePathname().c_str()),
													PathDescriptor::GetPathDescriptor(PathDescriptor::PATH_IMAGE).fileFilter);

	FilePath selectedPathname = PathnameToDAVAStyle(filePath);

	if(!selectedPathname.IsEmpty())
	{
		GetActiveScene()->visibilityToolSystem->SaveTexture(selectedPathname);
	}
}

void VisibilityToolPanel::SetVisibilityPoint()
{
	GetActiveScene()->visibilityToolSystem->SetVisibilityPoint();
}

void VisibilityToolPanel::SetVisibilityArea()
{
    QtWaitDialog* waitDialog = new QtWaitDialog(this);
    waitDialog->Show("Visibility", "Please wait while we computing visibility...", true, true);

    auto system = GetActiveScene()->visibilityToolSystem;

    system->ComputeVisibilityArea();

    while (system->GetState() == VisibilityToolSystem::State::ComputingVisibility)
    {
        if (waitDialog->WasCanceled())
        {
            system->CancelComputing();
        }
        else
        {
            waitDialog->SetValue(system->GetProgress());
        }
    }

    delete waitDialog;
}

void VisibilityToolPanel::ConnectToShortcuts()
{
	LandscapeEditorShortcutManager* shortcutManager = LandscapeEditorShortcutManager::Instance();

	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_VISIBILITY_TOOL_SET_POINT), SIGNAL(activated()),
			this, SLOT(SetVisibilityPoint()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_VISIBILITY_TOOL_SET_AREA), SIGNAL(activated()),
			this, SLOT(SetVisibilityArea()));

    shortcutManager->SetBrushSizeShortcutsEnabled(false);
    shortcutManager->SetVisibilityToolShortcutsEnabled(true);
}

void VisibilityToolPanel::DisconnectFromShortcuts()
{
	LandscapeEditorShortcutManager* shortcutManager = LandscapeEditorShortcutManager::Instance();

	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_VISIBILITY_TOOL_SET_POINT), SIGNAL(activated()),
			   this, SLOT(SetVisibilityPoint()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_VISIBILITY_TOOL_SET_AREA), SIGNAL(activated()),
			   this, SLOT(SetVisibilityArea()));
	
	shortcutManager->SetBrushSizeShortcutsEnabled(false);
	shortcutManager->SetVisibilityToolShortcutsEnabled(false);
}
