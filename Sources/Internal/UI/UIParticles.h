#ifndef __DAVAENGINE_UI_PARTICLES__
#define __DAVAENGINE_UI_PARTICLES__

#include "UI/UIControl.h"

namespace DAVA
{
class ParticleEffectComponent;
class ParticleEffectSystem;
class Camera;

class UIParticles : public UIControl, public TrackedObject
{
protected:
    virtual ~UIParticles();

public:
    UIParticles(const Rect& rect = Rect());

    void Draw(const UIGeometricData& geometricData) override;

    void OnActive() override;

    UIParticles* Clone() override;
    void CopyDataFrom(UIControl* srcControl) override;

    /*methods analogical to once in ParticleEffectComponent*/
    void Start();
    void Stop(bool isDeleteAllParticles = true);
    void Restart(bool isDeleteAllParticles = true);
    bool IsStopped() const;
    void Pause(bool isPaused = true);
    bool IsPaused() const;

    void SetEffectPath(const FilePath& path);
    const FilePath& GetEffectPath() const;
    void ReloadEffect();

    void SetAutostart(bool value);
    bool IsAutostart() const;

    // Start delay, in seconds.
    float32 GetStartDelay() const;
    void SetStartDelay(float32 value);

    //external
    void SetExtertnalValue(const String& name, float32 value);

protected:
    void LoadEffect(const FilePath& path);
    void UnloadEffect();

    // Start the playback in case Autostart flag is set.
    void HandleAutostart();

    // Handle the delayed action if requested.
    void HandleDelayedAction(float32 timeElapsed);

    // Start/Restart methods which can be called either immediately of after start delay.
    void DoStart();
    void DoRestart();

    enum eDelayedActionType
    {
        actionNone = 0,
        actionStart,
        actionRestart
    };

private:
    void Update(float32 timeElapsed);

    FilePath effectPath;
    bool isAutostart;
    float32 startDelay;

    ParticleEffectComponent* effect;
    ParticleEffectSystem* system;
    Matrix4 matrix;
    float32 updateTime;

    eDelayedActionType delayedActionType;
    float32 delayedActionTime;
    bool delayedDeleteAllParticles;
    bool needHandleAutoStart;

    static Camera* defaultCamera;

public:
    INTROSPECTION_EXTEND(UIParticles, UIControl,
                         PROPERTY("effectPath", "Effect path", GetEffectPath, SetEffectPath, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("autoStart", "Auto start", IsAutostart, SetAutostart, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("startDelay", "Start delay", GetStartDelay, SetStartDelay, I_SAVE | I_VIEW | I_EDIT)
                         );
};
};

#endif