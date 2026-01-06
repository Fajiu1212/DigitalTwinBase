#include "QingyuanToolkits.h"

#define LOCTEXT_NAMESPACE "FQingyuanToolkitsModule"

void FQingyuanToolkitsModule::StartupModule()
{
	IModuleInterface::StartupModule();
}

void FQingyuanToolkitsModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQingyuanToolkitsModule, QingyuanToolkits)