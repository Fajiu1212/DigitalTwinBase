/**
 * Desc: 统一的多播代理声明
 * Author: System
 * Date: 2025-12-10
 * 
 * 本文件集中管理项目中所有多播代理的声明，便于维护和管理
 * 
 * 优点：
 * - 集中管理，便于查找和维护
 * - 避免代理重复定义
 * - 减少头文件循环依赖
 * - 符合单一职责原则
 * 
 * 使用说明：
 * - 所有需要使用这些代理的类都应包含此头文件
 * - 代理命名遵循 UE 规范：FOn[事件名称]
 * - 使用 DECLARE_DYNAMIC_MULTICAST_DELEGATE 宏族定义动态多播代理
 */

#pragma once

#include "CoreMinimal.h"

// 前向声明
struct FJsonLibraryValue;

#pragma region WebUI Subsystem Delegates

/**
 * 测试环境热加载成功代理
 * 当测试环境资源热加载完成时触发
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTestEnvHotLoad);

/**
 * Web 函数调用代理
 * 用于 C++ 和 Web UI 之间的通信
 * @param Function - 要调用的函数名称
 * @param Data - JSON 格式的数据
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCallWebFunc, const FString&, Function, const FJsonLibraryValue&, Data);

/**
 * 原生选择变更代理
 * 当原生 UI 选择项发生变化时触发
 * @param Index - 新选择项的索引
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNativeSelectionChanged, int32, Index);

/**
 * 切换 POI Actor 可见性代理
 * 控制兴趣点 Actor 的显示/隐藏
 * @param Type - POI 类型标识
 * @param bShow - 是否显示
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTogglePOIActor, FString, Type, bool, bShow);

/**
 * 进度加载成功代理
 * 当开挖进度数据加载完成时触发
 * @param TargetModuleName - 目标模块名称
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProgressLoadSuccess, const FString&, TargetModuleName);

/**
 * 底板加载成功代理
 * 当底板或顶拱数据加载完成时触发
 * @param InSlabIndex - 底板索引
 * @param TargetModuleName - 目标模块名称
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlabLoadSuccess, int32, InSlabIndex, const FString&, TargetModuleName);

/**
 * 超前地质预报加载成功代理
 * 当地质预报数据加载完成时触发
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGeologicalForecastLoadSuccess);

/**
 * 模块切换代理
 * 当系统模式发生切换时触发
 * @param TargetModuleName - 目标模块名称
 * @param ExtraParam - 额外参数
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModuleSwitched, const FString&, TargetModuleName, const FString&, ExtraParam);

/**
 * 爆破动画代理
 * 触发隧道展开/爆破动画
 * @param TargetOwnerTunnelName - 目标隧道名称
 * @param bResetCamera - 是否重置摄像机位置
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExplosionAnimation, const FString&, TargetOwnerTunnelName, bool, bResetCamera);

/**
 * 剖面切割选中代理
 * 当选择隧道进行剖面切割时触发
 * @param TargetTunnelName - 目标隧道名称
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSectionCuttingSelected, const FString&, TargetTunnelName);

/**
 * 超欠挖加载成功代理
 * 当超欠挖数据加载完成时触发
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOverExcavationLoadSuccess);

/**
 * 围岩变形信息加载成功代理
 * 当围岩变形监测数据加载完成时触发
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSurfaceInfoLoadSuccess);

/**
 * 隧道车辆加载成功代理
 * 当隧道内车辆定位数据加载完成时触发
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTunnelVehicleLoadSuccess);

/**
 * 小地图切换点击代理
 * 控制小地图的显示/隐藏
 * @param bIsShow - 是否显示小地图
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMiniMapToggleClicked, bool, bIsShow);

/**
 * 小地图图标点击代理
 * 当点击小地图上的图标时触发
 * @param LocStr - 位置字符串
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMiniMapIconClicked, const FString&, LocStr);

/**
 * 监控概览页面变更代理
 * 当监控概览界面切换页面时触发
 * @param TargetPageIndex - 目标页面索引
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonitorOverviewPageChanged, int32, TargetPageIndex);

/**
 * 监控关闭代理
 * 当监控界面关闭时触发
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMonitorClosed);

/**
 * 禁用 WebUI 代理
 * 控制 WebUI 的启用/禁用状态
 * @param bDisable - 是否禁用
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCallDisableWebUI, bool, bDisable);

#pragma endregion
