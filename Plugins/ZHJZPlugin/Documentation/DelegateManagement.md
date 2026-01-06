# 多播代理管理最佳实践

## 概述

本文档说明项目中多播代理（Multicast Delegates）的统一管理方案，符合 Unreal Engine 官方开发习惯。

## 背景

在项目初期，多播代理声明分散在 `AwsaWebUI_Subsystem.h` 和 `AwsaPlayerPawn.h` 等多个文件中。为了提高代码可维护性和团队协作效率，我们将所有代理声明统一整理到一个专门的头文件中。

## 统一管理方案

### 文件位置

所有多播代理声明现在集中在以下文件中：

```
/Plugins/ZHJZPlugin/Source/ZHJZPlugin/Public/Common/AwsaDelegates.h
```

### 设计原则

1. **单一职责原则**：代理声明文件只负责代理的定义，不包含其他逻辑
2. **命名规范**：遵循 UE 官方命名规范 `FOn[事件名称]`
3. **详细注释**：每个代理都有清晰的中文注释说明用途和参数
4. **模块化组织**：使用 `#pragma region` 将相关代理分组

## 优点分析

### ✅ 优点

1. **集中管理**
   - 所有代理定义在一个文件中，便于查找和维护
   - 团队成员可以快速了解项目中所有可用的代理

2. **避免重复定义**
   - 防止在不同文件中重复声明相同的代理
   - 确保代理签名的一致性

3. **减少头文件依赖**
   - 降低循环依赖的风险
   - 简化 #include 关系

4. **便于团队协作**
   - 新增代理时，所有人都知道在哪里添加
   - 代码审查更加简单明了

5. **符合 UE 最佳实践**
   - Epic Games 官方项目也采用类似的组织方式
   - 有利于项目长期维护

### ⚠️ 需要注意的方面

1. **额外的 #include**
   - 使用代理的文件需要显式包含 `Common/AwsaDelegates.h`
   - 解决方案：这是正常且必要的，明确的依赖关系有利于代码理解

2. **编译依赖**
   - 修改代理定义会导致依赖此文件的所有模块重新编译
   - 解决方案：代理定义应该相对稳定，不会频繁修改

## 使用方法

### 在类中使用代理

**步骤 1：包含头文件**

```cpp
// YourClass.h
#pragma once

#include "CoreMinimal.h"
#include "Common/AwsaDelegates.h"  // 包含统一的代理头文件
#include "YourClass.generated.h"
```

**步骤 2：声明代理成员变量**

```cpp
UCLASS()
class YOURMODULE_API UYourClass : public UObject
{
    GENERATED_BODY()

public:
    // 使用统一定义的代理类型
    UPROPERTY(BlueprintAssignable)
    FOnModuleSwitched OnModuleSwitched;
    
    UPROPERTY(BlueprintAssignable)
    FOnExplosionAnimation OnExplosionAnimation;
};
```

**步骤 3：绑定代理**

```cpp
// 在其他类中订阅代理
void AMyActor::BeginPlay()
{
    Super::BeginPlay();
    
    if (UAwsaWebUI_Subsystem* WebUISubsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
    {
        WebUISubsystem->OnModuleSwitched.AddDynamic(this, &AMyActor::OnModuleSwitched);
        WebUISubsystem->OnExplosionAnimation.AddDynamic(this, &AMyActor::OnExplosionAnimation);
    }
}
```

**步骤 4：广播代理**

```cpp
void UYourClass::SomeFunction()
{
    if (OnModuleSwitched.IsBound())
    {
        OnModuleSwitched.Broadcast(TEXT("NewModule"), TEXT("ExtraParam"));
    }
}
```

## 代理命名规范

遵循以下命名规范：

- **格式**：`FOn[事件名称]`
- **示例**：
  - `FOnModuleSwitched` - 模块切换时触发
  - `FOnExplosionAnimation` - 爆破动画时触发
  - `FOnProgressLoadSuccess` - 进度加载成功时触发

## 迁移指南

如果需要添加新的代理：

1. 在 `AwsaDelegates.h` 中添加代理声明
2. 添加详细的中文注释说明代理的用途和参数
3. 确保命名符合规范
4. 在需要使用的类中包含 `Common/AwsaDelegates.h`
5. 声明代理成员变量并实现相应的绑定和广播逻辑

## 当前代理列表

以下是目前项目中统一管理的所有代理：

| 代理名称 | 用途 | 参数 |
|---------|------|------|
| `FOnTestEnvHotLoad` | 测试环境热加载完成 | 无 |
| `FOnCallWebFunc` | Web 函数调用 | Function, Data |
| `FOnNativeSelectionChanged` | 原生选择变更 | Index |
| `FOnTogglePOIActor` | 切换 POI Actor | Type, bShow |
| `FOnProgressLoadSuccess` | 进度加载成功 | TargetModuleName |
| `FOnSlabLoadSuccess` | 底板加载成功 | InSlabIndex, TargetModuleName |
| `FOnGeologicalForecastLoadSuccess` | 地质预报加载成功 | 无 |
| `FOnModuleSwitched` | 模块切换 | TargetModuleName, ExtraParam |
| `FOnExplosionAnimation` | 爆破动画 | TargetOwnerTunnelName, bResetCamera |
| `FOnSectionCuttingSelected` | 剖面切割选中 | TargetTunnelName |
| `FOnOverExcavationLoadSuccess` | 超欠挖加载成功 | 无 |
| `FOnSurfaceInfoLoadSuccess` | 围岩信息加载成功 | 无 |
| `FOnTunnelVehicleLoadSuccess` | 隧道车辆加载成功 | 无 |
| `FOnMiniMapToggleClicked` | 小地图切换点击 | bIsShow |
| `FOnMiniMapIconClicked` | 小地图图标点击 | LocStr |
| `FOnMonitorOverviewPageChanged` | 监控概览页变更 | TargetPageIndex |
| `FOnMonitorClosed` | 监控关闭 | 无 |
| `FOnCallDisableWebUI` | 禁用 WebUI | bDisable |

## 参考资料

- [Unreal Engine Delegates Documentation](https://docs.unrealengine.com/en-US/ProgrammingAndScripting/ProgrammingWithCPP/UnrealArchitecture/Delegates/)
- [UE4 Code Style Guide](https://docs.unrealengine.com/en-US/ProductionPipelines/DevelopmentSetup/CodingStandard/)

## 版本历史

- **v1.0** (2025-12-10): 初始版本，统一管理所有多播代理
