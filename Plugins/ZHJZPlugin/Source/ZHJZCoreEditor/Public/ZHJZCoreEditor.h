#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FZHJZCoreEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
    //Tool bar menu
    void RegisterToolBarMenu();
    void RegisterZHJZEventTagsTable();
};
