#include "LandscapeEditorControl.h"

#include "ControlsFactory.h"

#include "EditorSettings.h"

#include "PaintAreaControl.h"

#include "EditorScene.h"
#include "CameraController.h"
#include "LandscapePropertyControl.h"


//***************    LandscapeEditorControl    **********************
LandscapeEditorControl::LandscapeEditorControl(const Rect & rect)
    :   UIControl(rect)
{
    ControlsFactory::CusomizeBottomLevelControl(this);
    
    CreateLeftPanel();
    CreatePaintAreaPanel();
    Create3D();
    
    fileSystemDialogOpMode = DIALOG_OPERATION_NONE;
    fileSystemDialog = new UIFileSystemDialog("~res:/Fonts/MyriadPro-Regular.otf");
    fileSystemDialog->SetDelegate(this);
    
    KeyedArchive *keyedArchieve = EditorSettings::Instance()->GetSettings();
    String path = keyedArchieve->GetString("3dDataSourcePath", "/");
    if(path.length())
    {
        fileSystemDialog->SetCurrentDir(path);   
    }
}


LandscapeEditorControl::~LandscapeEditorControl()
{
    SafeRelease(fileSystemDialog);
    
    Release3D();
    
    ReleasePaintAreaPanel();
    ReleaseLeftPanel();
}


void LandscapeEditorControl::CreateLeftPanel()
{
    Rect fullRect = GetRect();
    
    int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
    Rect leftRect = Rect(0, 0, leftSideWidth, fullRect.dy);
    leftPanel = ControlsFactory::CreatePanelControl(leftRect);
    AddControl(leftPanel);
    

    landscapeProperties = new LandscapePropertyControl(Rect(0, 0, leftRect.dx, leftRect.dy / 2), true);
    leftPanel->AddControl(landscapeProperties);

    
    propertyList = new PropertyList(Rect(0, leftRect.dy / 2, leftRect.dx, leftRect.dy / 2), this);
    leftPanel->AddControl(propertyList);
    
    propertyList->AddIntProperty("landscapeeditor.size", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetIntPropertyValue("landscapeeditor.size", 1024);
    
//    propertyList->AddFilepathProperty("TEXTURE_TEXTURE0", "", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
//    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE0", "");
//    propertyList->AddFilepathProperty("TEXTURE_TEXTURE1/TEXTURE_DETAIL", "", ".png", true, PropertyList::PROPERTY_IS_EDITABLE);
//    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL", "");
//    propertyList->AddFilepathProperty("LightMap", ".png", true, PropertyList::PROPERTY_IS_EDITABLE);
//    propertyList->SetFilepathPropertyValue("LightMap", "");

    String projectPath = EditorSettings::Instance()->GetDataSourcePath() + "Landscape/";
    propertyList->AddFilepathProperty("landscapeeditor.texture0", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetFilepathPropertyValue("landscapeeditor.texture0", projectPath + "Snow_Rock_N_1.png");
    propertyList->AddFilepathProperty("landscapeeditor.texture1", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetFilepathPropertyValue("landscapeeditor.texture1", projectPath + "Snow_sand_.png");
    
    propertyList->AddFilepathProperty("landscapeeditor.lightmap", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetFilepathPropertyValue("landscapeeditor.lightmap", "");
    
    propertyList->AddBoolProperty("landscapeeditor.showresult", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetBoolPropertyValue("landscapeeditor.showresult", false);
    
    propertyList->AddBoolProperty("landscapeeditor.maskred", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetBoolPropertyValue("landscapeeditor.maskred", true);
    propertyList->AddBoolProperty("landscapeeditor.maskgreen", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetBoolPropertyValue("landscapeeditor.maskgreen", true);
    propertyList->AddBoolProperty("landscapeeditor.maskblue", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetBoolPropertyValue("landscapeeditor.maskblue", false);
    propertyList->AddBoolProperty("landscapeeditor.maskalpha", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetBoolPropertyValue("landscapeeditor.maskalpha", false);
    
    propertyList->AddMessageProperty("landscapeeditor.savemask", 
                                     Message(this, &LandscapeEditorControl::OnSavePressed));
}

void LandscapeEditorControl::ReleaseLeftPanel()
{
    SafeRelease(propertyList);
    SafeRelease(leftPanel);
}

void LandscapeEditorControl::CreatePaintAreaPanel()
{
    Rect fullRect = GetRect();
    
    int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
    Rect toolsRect = Rect(leftSideWidth + OFFSET, 0, 
                          fullRect.dx - leftSideWidth - OFFSET*2, TOOLS_HEIGHT);
    toolsPanel = ControlsFactory::CreatePanelControl(toolsRect);
    AddControl(toolsPanel);
    
    selectedTool = NULL;
    
    const String sprites[] = 
    {
        "~res:/Gfx/LandscapeEditor/Tools/RadialGradientIcon",
        "~res:/Gfx/LandscapeEditor/Tools/SpikeGradientIcon",
        "~res:/Gfx/LandscapeEditor/Tools/CircleIcon",
        "~res:/Gfx/LandscapeEditor/Tools/NoiseIcon",
        "~res:/Gfx/LandscapeEditor/Tools/ErodeIcon",
        "~res:/Gfx/LandscapeEditor/Tools/WaterErodeIcon",
    };

    const float32 actualRadius[] = 
    {
        0.5f,
        0.35f,
        0.7f,
        0.5f,
        0.6f,
        0.6f,
    };

    
    int32 x = 0;
    int32 y = (TOOLS_HEIGHT - TOOL_BUTTON_SIDE) / 2;
    for(int32 i = 0; i < PaintTool::EBT_COUNT; ++i, x += (TOOL_BUTTON_SIDE + 1))
    {
        tools[i] = new PaintTool((PaintTool::eBrushType) i, sprites[i], actualRadius[i]);
        
        toolButtons[i] = new UIControl(Rect(x, y, TOOL_BUTTON_SIDE, TOOL_BUTTON_SIDE));
        toolButtons[i]->SetSprite(tools[i]->spriteName, 0);
        toolButtons[i]->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LandscapeEditorControl::OnToolSelected));

        toolsPanel->AddControl(toolButtons[i]);
    }
    
    radius = CreateSlider(Rect(toolsRect.dx - SLIDER_WIDTH, 0, SLIDER_WIDTH, TOOLS_HEIGHT / 2));
    radius->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeEditorControl::OnRadiusChanged));
    intension = CreateSlider(Rect(toolsRect.dx - SLIDER_WIDTH, TOOLS_HEIGHT / 2, SLIDER_WIDTH, TOOLS_HEIGHT / 2));
    intension->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeEditorControl::OnIntensionChanged));
    
    Rect zoomRect = radius->GetRect();
    zoomRect.x -= zoomRect.dx / 2;
    zoomRect.dx *= 2; // create longer zoom bar
    zoomRect.x -= zoomRect.dx; //
    
    zoom = CreateSlider(zoomRect);
    zoom->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeEditorControl::OnZoomChanged));
    
    toolsPanel->AddControl(radius);
    toolsPanel->AddControl(intension);
    toolsPanel->AddControl(zoom);
    
    AddSliderHeader(zoom, LocalizedString(L"landscapeeditor.zoom"));
    AddSliderHeader(radius, LocalizedString(L"landscapeeditor.radius"));
    AddSliderHeader(intension, LocalizedString(L"landscapeeditor.intension"));
    

    Rect paintRect = Rect(toolsRect.x, toolsRect.y + toolsRect.dy + OFFSET, 
                          toolsRect.dx, fullRect.dy - (toolsRect.y + toolsRect.dy + OFFSET*2));
    
    scrollView = NULL;
    paintArea = NULL;
//    scrollView = new UIScrollView(paintRect);
//    scrollView->SetContentSize(Vector2(1024, 1024));
//    AddControl(scrollView);
//    
//    paintArea = new PaintAreaControl(Rect(0, 0, 1024, 1024));
//    scrollView->AddControl(paintArea);
}

void LandscapeEditorControl::ReleasePaintAreaPanel()
{
    SafeRelease(zoom);
    SafeRelease(scrollView);
    SafeRelease(radius);
    SafeRelease(intension);
    
    
    for(int32 i = 0; i < PaintTool::EBT_COUNT; ++i)
    {
        SafeRelease(toolButtons[i]);
        SafeRelease(tools[i]);
    }
    
    SafeRelease(paintArea);
    SafeRelease(toolsPanel);
}

UISlider * LandscapeEditorControl::CreateSlider(const Rect & rect)
{
    UISlider *slider = new UISlider(rect);
    slider->SetMinMaxValue(0.f, 1.0f);
    slider->SetValue(0.5f);
    
    slider->SetMinSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 1);
    slider->SetMinDrawType(UIControlBackground::DRAW_STRETCH_HORIZONTAL);
    slider->SetMinLeftRightStretchCap(5);

    slider->SetMaxSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 0);
    slider->SetMaxDrawType(UIControlBackground::DRAW_STRETCH_HORIZONTAL);
    slider->SetMaxLeftRightStretchCap(5);

    slider->SetThumbSprite("~res:/Gfx/LandscapeEditor/Tools/polzunokCenter", 0);
    
    return slider;
}

void LandscapeEditorControl::AddSliderHeader(UISlider *slider, const WideString &text)
{
    Rect rect = slider->GetRect();
    rect.x -= rect.dx - OFFSET;
    
    UIStaticText *textControl = new UIStaticText(rect);
    textControl->SetText(text);
    textControl->SetFont(ControlsFactory::GetFontDark());
    textControl->SetAlign(ALIGN_VCENTER | ALIGN_RIGHT);
    toolsPanel->AddControl(textControl);
    SafeRelease(textControl);
}


void LandscapeEditorControl::WillAppear()
{
    if(scrollView)
    {
        scrollView->SetScale(zoom->GetValue() * PaintAreaControl::ZOOM_MULTIPLIER);
    }

    if(paintArea)
    {
        paintArea->SetTexture(PaintAreaControl::ETROID_LIGHTMAP_RGB, propertyList->GetFilepathPropertyValue("landscapeeditor.lightmap"));
        paintArea->SetTexture(PaintAreaControl::ETROID_TEXTURE_TEXTURE0, propertyList->GetFilepathPropertyValue("landscapeeditor.texture0"));
        paintArea->SetTexture(PaintAreaControl::ETROID_TEXTURE_TEXTURE1, propertyList->GetFilepathPropertyValue("landscapeeditor.texture1"));
        
        paintArea->ShowResultTexture(propertyList->GetBoolPropertyValue("landscapeeditor.showresult"));
        paintArea->SetDrawingMask(PaintAreaControl::EDM_NONE);
    }
    
    SetDrawingMask(PaintAreaControl::EDM_RED, propertyList->GetBoolPropertyValue("landscapeeditor.maskred"));
    SetDrawingMask(PaintAreaControl::EDM_GREEN, propertyList->GetBoolPropertyValue("landscapeeditor.maskgreen"));
    SetDrawingMask(PaintAreaControl::EDM_BLUE, propertyList->GetBoolPropertyValue("landscapeeditor.maskblue"));
    SetDrawingMask(PaintAreaControl::EDM_ALPHA, propertyList->GetBoolPropertyValue("landscapeeditor.maskalpha"));
    
    if(!selectedTool)
    {
        toolButtons[0]->PerformEvent(UIControl::EVENT_TOUCH_UP_INSIDE);
    }
    
    if(cameraController)
    {
        cameraController->SetSpeed(EditorSettings::Instance()->GetCameraSpeed());
    }

    if(landscapeProperties && workingLandscape)
    {
        landscapeProperties->ReadFrom(workingLandscape);
    }
    
    UIControl::WillAppear();
}

void LandscapeEditorControl::OnToolSelected(DAVA::BaseObject *object, void *userData, void *callerData)
{
    UIControl *button = (UIControl *)object;
    
    for(int32 i = 0; i < PaintTool::EBT_COUNT; ++i)
    {
        if(button == toolButtons[i])
        {
            selectedTool = tools[i];
            toolButtons[i]->SetDebugDraw(true);
            
            radius->SetValue(selectedTool->radius);
            intension->SetValue(selectedTool->intension);
            
            selectedTool->zoom = zoom->GetValue();
            
            if(paintArea)
            {
                paintArea->SetPaintTool(selectedTool);
            }
        }
        else
        {
            toolButtons[i]->SetDebugDraw(false);
        }
    }
}

void LandscapeEditorControl::OnRadiusChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(selectedTool)
    {
        selectedTool->radius = radius->GetValue();
    }
}

void LandscapeEditorControl::OnIntensionChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(selectedTool)
    {
        selectedTool->intension = intension->GetValue();
    }
}

void LandscapeEditorControl::OnZoomChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(scrollView)
    {
        scrollView->SetScale(zoom->GetValue() * PaintAreaControl::ZOOM_MULTIPLIER);
    }

    if(selectedTool)
    {
        selectedTool->zoom = zoom->GetValue();
    }
}



void LandscapeEditorControl::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
    if("landscapeeditor.size" == forKey)
    {
        Vector2 texSize(newValue, newValue);
        if(paintArea)
        {
            paintArea->SetTextureSideSize(texSize);
            paintArea->SetSize(texSize);
        }
        
        if(scrollView)
        {
            scrollView->SetContentSize(texSize);
        }
    }
}

void LandscapeEditorControl::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if(EditorSettings::IsValidPath(newValue))
    {
        if(paintArea)
        {
            if("landscapeeditor.lightmap" == forKey)
            {
                paintArea->SetTexture(PaintAreaControl::ETROID_LIGHTMAP_RGB, newValue);
            }
            else if("landscapeeditor.texture0" == forKey)
            {
                paintArea->SetTexture(PaintAreaControl::ETROID_TEXTURE_TEXTURE0, newValue);
            }
            else if("landscapeeditor.texture1" == forKey)
            {
                paintArea->SetTexture(PaintAreaControl::ETROID_TEXTURE_TEXTURE0, newValue);
            }
        }
    }
}

void LandscapeEditorControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if("landscapeeditor.showresult" == forKey)
    {
        if(paintArea)
        {
            paintArea->ShowResultTexture(propertyList->GetBoolPropertyValue(forKey));
        }
    }
    else if("landscapeeditor.maskred" == forKey)
    {
        SetDrawingMask(PaintAreaControl::EDM_RED, newValue);
    }
    else if("landscapeeditor.maskgreen" == forKey)
    {
        SetDrawingMask(PaintAreaControl::EDM_GREEN, newValue);
    }
    else if("landscapeeditor.maskblue" == forKey)
    {
        SetDrawingMask(PaintAreaControl::EDM_BLUE, newValue);
    }
    else if("landscapeeditor.maskalpha" == forKey)
    {
        SetDrawingMask(PaintAreaControl::EDM_ALPHA, newValue);
    }
}


void LandscapeEditorControl::SetDrawingMask(int32 flag, bool value)
{
    if(paintArea)
    {
        int32 drawingMask = paintArea->GetDrawingMask();
        if(value)
        {
            drawingMask |= flag;
        }
        else
        {
            drawingMask &= ~flag;
        }
        paintArea->SetDrawingMask(drawingMask);
    }
}

void LandscapeEditorControl::OnSavePressed(BaseObject * object, void * userData, void * callerData)
{
    if(!fileSystemDialog->GetParent())
    {
        fileSystemDialog->SetExtensionFilter(".png");
        fileSystemDialog->SetOperationType(UIFileSystemDialog::OPERATION_SAVE);
        
        fileSystemDialog->SetCurrentDir(EditorSettings::Instance()->GetDataSourcePath());
        
        fileSystemDialog->Show(this);
        fileSystemDialogOpMode = DIALOG_OPERATION_SAVE;
    }
}
    
void LandscapeEditorControl::OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile)
{
    switch (fileSystemDialogOpMode) 
    {
        case DIALOG_OPERATION_SAVE:
        {
            if(paintArea)
            {
                paintArea->SaveMask(pathToFile);
            }
            break;
        }
            
        default:
            break;
    }
    
    fileSystemDialogOpMode = DIALOG_OPERATION_NONE;
}

void LandscapeEditorControl::OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog)
{
    fileSystemDialogOpMode = DIALOG_OPERATION_NONE;
}

void LandscapeEditorControl::Create3D()
{
    Rect fullRect = GetRect();
    Rect toolsRect = toolsPanel->GetRect();
    
    Rect sceneRect = Rect(toolsRect.x, toolsRect.y + toolsRect.dy + OFFSET, 
                          toolsRect.dx, fullRect.dy - (toolsRect.y + toolsRect.dy + OFFSET*2));

    scene3dView = new UI3DView(sceneRect);

    scene3dView->SetDebugDraw(true);
    scene3dView->SetInputEnabled(false);
    AddControl(scene3dView);

    scene = new EditorScene();
    scene3dView->SetScene(scene);

    // Camera setup
    cameraController = new WASDCameraController(EditorSettings::Instance()->GetCameraSpeed());
    
    Camera * cam = new Camera(scene);
    cam->SetName("landscapeeditor.main-camera");
    cam->SetUp(Vector3(0.0f, 0.0f, 1.0f));
    cam->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    cam->SetTarget(Vector3(0.0f, 1.0f, 0.0f));
    
    cam->Setup(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f); 
    
    scene->AddNode(cam);
    scene->AddCamera(cam);
    scene->SetCurrentCamera(cam);
    cameraController->SetScene(scene);
    
    SafeRelease(cam);
        
    
    workingLandscape = new LandscapeNode(scene);
    scene->AddNode(workingLandscape);
}

void LandscapeEditorControl::Release3D()
{
    if(scene && workingLandscape)
    {
        scene->RemoveNode(workingLandscape);
    }
    
    SafeRelease(workingLandscape);
    SafeRelease(scene);
    SafeRelease(scene3dView);
    SafeRelease(cameraController);
}

void LandscapeEditorControl::Input(UIEvent * event)
{
    
    if(UIEvent::BUTTON_1 == event->tid)
    {
        Vector3 from, dir;
        GetCursorVectors(&from, &dir, event->point);
        Vector3 to = from + dir * 1000.0f;
        scene->TrySelection(from, to);				
//        selection = scene->GetProxy();
        
    }
    else 
    {
        if(cameraController)
        {
            cameraController->Input(event);
        }
    }
    
    UIControl::Input(event);
}

void LandscapeEditorControl::Update(float32 timeElapsed)
{
    if(cameraController)
    {
        cameraController->Update(timeElapsed);
    }
    
    UIControl::Update(timeElapsed);
}

void LandscapeEditorControl::GetCursorVectors(Vector3 * from, Vector3 * dir, const Vector2 &point)
{
    if(scene)
    {
        Camera * cam = scene->GetCurrentCamera();
        if (cam)
        {
            const Rect & rect = scene3dView->GetLastViewportRect();
            *from = cam->GetPosition();
            Vector3 to = cam->UnProject(point.x, point.y, 0, rect);
            to -= *from;
            *dir = to;
        }
    }
}
