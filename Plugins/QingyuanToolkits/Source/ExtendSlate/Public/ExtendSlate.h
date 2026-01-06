#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FExtendSlateModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
private:

    //Tool bar menu
    void CreateToolBarMenu();
    void FillToolBarMenu(UToolMenu* Menu);
    
    static void ButtonAction_CameraData();
    void SpawnTab_CameraData();
};
