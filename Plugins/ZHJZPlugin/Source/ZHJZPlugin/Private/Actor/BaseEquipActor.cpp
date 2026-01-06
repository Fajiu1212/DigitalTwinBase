#include "Actor/BaseEquipActor.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/AwsaPlayerPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AwsaWebUI/Public/AwsaWebUI_ActorDashboard.h"
#include "AwsaWebUI_Subsystem.h"

// Static ref path
const FString ABaseEquipActor::MonitorMeshPath =
	TEXT("/Script/Engine.StaticMesh'/Game/Blueprints/BP_Public/EquipActor/SM/Monitor/Jingjieji.Jingjieji'");
const FString ABaseEquipActor::HandleMeshPath =
	TEXT("/Script/Engine.StaticMesh'/Game/Blueprints/BP_Public/EquipActor/SM/Monitor/SM_JKZJ.SM_JKZJ'");
const FString ABaseEquipActor::MonitorRangeMeshPath =
	TEXT("/Script/Engine.StaticMesh'/Game/Blueprints/BP_Public/EquipActor/SM/Monitor/SM_JKFW_60.SM_JKFW_60'");

const FString ABaseEquipActor::DefaultIconPath =
	TEXT("/Script/Engine.Texture2D'/Game/Blueprints/BP_Public/EquipActor/Texture/JianKong_Online.JianKong_Online'");
const FString ABaseEquipActor::EquipUrlDataTablePath =
	TEXT("/Script/Engine.DataTable'/Game/Blueprints/BP_Public/EquipActor/DT/DT_EquipUrl.DT_EquipUrl'");

// Sets default values
ABaseEquipActor::ABaseEquipActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//init rootscene
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	//create into ui
	IntroUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("IntoUI"));
	IntroUI->SetupAttachment(RootScene);
	IntroUI->SetRelativeLocation(FVector(0, 0, 0));

	// //create main ui
	// MainUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("MainUI"));
	// MainUI->SetupAttachment(RootScene);
	// MainUI->SetRelativeLocation(FVector(-100, 0, 0));

	//create name ui
	NameUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameUI"));
	NameUI->SetupAttachment(IntroUI);
	NameUI->SetRelativeLocation(FVector(100, 0, 0));

	static ConstructorHelpers::FClassFinder<UUserWidget> IntoWidgetClassFinder(
		TEXT("WidgetBlueprint'/Game/Blueprints/BP_Public/EquipActor/ActorInto_UI.ActorInto_UI_C'"));
	if (IntoWidgetClassFinder.Succeeded())
	{
		DefaultIntroWidgetClass = IntoWidgetClassFinder.Class;
		IntroUI->SetWidgetClass(DefaultIntroWidgetClass);
		IntroUI->SetWidgetSpace(EWidgetSpace::World);
		IntroUI->SetPivot(FVector2D(0.21, 1));
		IntroUI->SetDrawSize(FVector2D(132, 300));
	}

	// static ConstructorHelpers::FClassFinder<UAwsaWebUI_ActorDashboard> MainWidgetClassFinder(
	// 	TEXT("WidgetBlueprint'/Game/Blueprints/BP_Public/EquipActor/ActorWebBox_UI.ActorWebBox_UI_C'"));
	// if (MainWidgetClassFinder.Succeeded())
	// {
	// 	DefaultMainWidgetClass = MainWidgetClassFinder.Class;
	// 	MainUI->SetWidgetClass(DefaultMainWidgetClass);
	// 	MainUI->SetWidgetSpace(EWidgetSpace::Screen);
	// 	MainUI->SetPivot(FVector2D(0.5, 0.5));
	// }

	static ConstructorHelpers::FClassFinder<UUserWidget> NameWidgetClassFinder(
		TEXT("WidgetBlueprint'/Game/Blueprints/BP_Public/EquipActor/ActorName_UI.ActorName_UI_C'"));
	if (NameWidgetClassFinder.Succeeded())
	{
		DefaultNameWidgetClass = NameWidgetClassFinder.Class;
		NameUI->SetWidgetClass(DefaultNameWidgetClass);
		NameUI->SetWidgetSpace(EWidgetSpace::Screen);
		NameUI->SetPivot(FVector2D(0, 0));
		NameUI->SetDrawSize(FVector2D(190, 15));
	}

	//Default icon
	static ConstructorHelpers::FObjectFinder<UTexture2D> IconFinder(*DefaultIconPath);
	if (IconFinder.Succeeded())
	{
		EquipIcon = IconFinder.Object;
	}
#pragma region Monitor
	MonitorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MonitorMesh"));
	MonitorMesh->SetupAttachment(RootComponent);
	MonitorMesh->SetRelativeLocation(FVector(0, 0, 60));
	MonitorMesh->SetVisibility(false);
	MonitorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HandleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandleMesh"));
	HandleMesh->SetupAttachment(MonitorMesh);
	HandleMesh->SetRelativeLocation(FVector(0, 0, 70.0));
	HandleMesh->SetVisibility(false);
	HandleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MonitorRangeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MonitorRangeMesh"));
	MonitorRangeMesh->SetupAttachment(RootComponent);
	MonitorRangeMesh->SetRelativeLocation(FVector(0, 0, -260.0));
	MonitorRangeMesh->SetVisibility(false);
	MonitorRangeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(
		*MonitorMeshPath);
	if (MeshFinder.Succeeded())
	{
		MonitorMesh->SetStaticMesh(MeshFinder.Object);
	}
	static ConstructorHelpers::FObjectFinder<UStaticMesh> HandleFinder(*HandleMeshPath);
	if (MeshFinder.Succeeded())
	{
		HandleMesh->SetStaticMesh(HandleFinder.Object);
	}
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RangeFinder(*MonitorRangeMeshPath);
	if (MeshFinder.Succeeded())
	{
		MonitorRangeMesh->SetStaticMesh(RangeFinder.Object);
	}
#pragma endregion

	static ConstructorHelpers::FObjectFinder<UDataTable> UrlStrFinder(*EquipUrlDataTablePath);
	if (UrlStrFinder.Succeeded())
	{
		EquipUrlDataTable = UrlStrFinder.Object;
	}
}


void ABaseEquipActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ABaseEquipActor::ReceiveFunction_Implementation(const FString& Name, const FString& Message)
{
	IAwsaWebUI_Interface::ReceiveFunction_Implementation(Name, Message);
}

#if WITH_EDITOR
void ABaseEquipActor::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ABaseEquipActor, EquipActorType))
	{
		UpdateMonitorMeshVisibility(EquipActorType == EEquipActorType::MonitorCamera);
	}
}
#endif

void ABaseEquipActor::UpdateMonitorMeshVisibility(const bool bShowMonitor)
{
	IntroUI->SetRelativeLocation(bShowMonitor ? FVector(0, 0, 200) : FVector(0, 0, 0));
	//MainUI->SetRelativeLocation(bShowMonitor ? FVector(0, 0, 100) : FVector(-100, 0, 0));
	if (MonitorMesh)
	{
		MonitorMesh->SetVisibility(bShowMonitor);
	}

	if (HandleMesh)
	{
		HandleMesh->SetVisibility(bShowMonitor);
	}

	if (MonitorRangeMesh)
	{
		MonitorRangeMesh->SetVisibility(bShowMonitor);
	}
}

void ABaseEquipActor::BeginPlay()
{
	Super::BeginPlay();

	//MainUI->SetDrawSize(MainUISize);
}

void ABaseEquipActor::ChangeRotAndScale()
{
	APlayerCameraManager* CameraManger = UGameplayStatics::GetPlayerCameraManager(GetWorld(),0);
	if (!CameraManger) return;
	FVector IntroLoc = IntroUI->GetComponentLocation();
	FVector CameraLoc = UGameplayStatics::GetPlayerCameraManager(GetWorld(),0)->GetCameraLocation();
	FRotator TargetRot = FRotationMatrix::MakeFromX(CameraLoc - IntroLoc).Rotator();
	float ClampRoll = FMath::Clamp(TargetRot.Roll, 0.0, 45.0);
	float ClampPitch = FMath::Clamp(TargetRot.Pitch, 0.0, 45.0);
	IntroUI->SetWorldRotation(FRotator(ClampPitch, TargetRot.Yaw, ClampRoll));

	if (!PlayerPawn)
	{
		PlayerPawn = Cast<AAwsaPlayerPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
		if (!PlayerPawn) return;
	}
	float CurrArmLength = PlayerPawn->TargetArmLength;
	//if (LastArmLength == CurrArmLength) return;
	LastArmLength = CurrArmLength;
	AdjustedScale = LastArmLength / RefArmLength;
	float ClampedScale = FMath::Clamp(AdjustedScale, ClampMinScale, ClampMaxScale);
	IntroUI->SetWorldScale3D(FVector(1, ClampedScale, ClampedScale));
}


void ABaseEquipActor::ChangeIntroScale()
{
	AAwsaPlayerPawn* awsaPawn = Cast<AAwsaPlayerPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (awsaPawn)
	{
		float armLength = awsaPawn->SpringArm->TargetArmLength / 3000;
		if (armLength == OriginScale)
		{
			return;
		}
		float yz = FMath::Clamp(armLength * 1.8, 2.5, 7.0);

		IntroUI->SetWorldScale3D(FVector(1.0, yz, yz));
		OriginScale = armLength;
	}
}

void ABaseEquipActor::OnOffMainUI(bool bIsOn)
{
	//MainUI->SetHiddenInGame(!bIsOn);
	IntroUI->SetHiddenInGame(bIsOn);
	ECollisionEnabled::Type NewType = bIsOn ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryOnly;
	IntroUI->SetCollisionEnabled(NewType);
}

void ABaseEquipActor::GetURL(FString& Url) const
{
	if (!Dashboard_Url.IsValid())
	{
		return;
	}
	
	if (DeviceData.IconURL != TEXT(""))
	{
		Url = DeviceData.IconURL;
		UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("EquipURL:::%s"),*Url));
	}
}


FString ABaseEquipActor::ConvertTMapToQueryString(const TMap<FString, FString>& InMap) const
{
	FString Result;
	for (const TPair<FString, FString>& Pair : InMap)
	{
		if (!Result.IsEmpty())
		{
			Result += "&";
		}
		Result += FString::Printf(TEXT("%s=%s"), *Pair.Key, *Pair.Value);
	}
	return Result;
}

// Called every frame
void ABaseEquipActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseEquipActor::UpdateRoiMonitorCameraData()
{
	FString EnvUrl = TEXT("");
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UAwsaWebUI_Subsystem* Subsystem = GameInstance->GetSubsystem<UAwsaWebUI_Subsystem>())
			{
				FString TagName = FString::Printf(TEXT("Env.%s.Production"),
				                                  Subsystem->CheckTestEnv()
					                                  ? TEXT("Test")
					                                  : TEXT("Official"));
				EnvUrl = Subsystem->GetEnvUrl(FGameplayTag::RequestGameplayTag(*TagName));
			}
		}
	}
	FString RoiConfigURL = EnvUrl + FString(TEXT(""));
	FString RoiConfigContent = TEXT("");
	FString RoiConfigVerb = TEXT("");

	UAutoParse* Instance = UAutoParse::AutoHttpRequest(RoiConfigURL, RoiConfigContent, RoiConfigVerb,
	                                                   TMap<FString, FString>(), FString(), ERequestType::HttpOnly);
	Instance->OnSuccess.AddDynamic(this, &ABaseEquipActor::OnRoiMonitorConfigRequestCompleted);
}

void ABaseEquipActor::OnRoiMonitorConfigRequestCompleted(const FJsonStruct& JsonStruct, const FString& WebMsg,
                                                         const FString& StructName)
{
	FAutoParseJsonObject MsgObj;
	UAutoParse::GetJsonObjectFromJsonStr(WebMsg, MsgObj);
	TArray<FAutoParseJsonObject> DataArray;
	if (UAutoParse::GetObjArrayValueFormObject(TEXT("data"), MsgObj, DataArray))
	{
		FAutoParseJsonObject TargetObj = DataArray[0];

		UAutoParse::GetInt32ValueFormObject(TEXT("relatedMonitoring"), TargetObj, RelatedMonitoring);
		UAutoParse::GetInt32ValueFormObject(TEXT("relatedMonitoring"), TargetObj, RelatedMonitoring);
	}
}

void ABaseEquipActor::InitDeviceData(FDeviceData TargetDeviceData)
{
	DeviceData = MoveTemp(TargetDeviceData);
}
