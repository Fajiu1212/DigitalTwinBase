// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/ConfigManager.h"

void UConfigManager::InitTestConfig()
{
    if (!GConfig) return;

    FString Text;

    //配置文件路径
    FString FilePath = FPaths::SourceConfigDir() + ConfigName;

	Text = FString::Printf(TEXT("----Config----FilePath: %s"), *FilePath);
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, Text);

    //读取/写入 字符串
    FString testStr = "testTest";
	GConfig->SetString(*TitleName, TEXT("testStr"), *testStr, FilePath);
    GConfig->GetString(*TitleName, TEXT("testStr"), testStr, FilePath);
 
	Text = FString::Printf(TEXT("----Config----testStr: %s"), *testStr);
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, Text);

    //读取写入float
    float  testFloat = 6.6f; 
    GConfig->SetFloat(*TitleName, TEXT("testFloat"), testFloat, FilePath);
	GConfig->GetFloat(*TitleName, TEXT("testFloat"), testFloat, FilePath);

	Text = FString::Printf(TEXT("----Config--------testFloat: %f"), testFloat);
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, Text);

    //读取写入int
    int32 testNum;
	testNum = 5;
	GConfig->SetInt(*TitleName, TEXT("testNum"), testNum, FilePath);
    GConfig->GetInt(*TitleName, TEXT("testNum"), testNum, FilePath);

    Text = FString::Printf(TEXT("----Config--------testNum: %i"), testNum);
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, Text);

    //读取写入bool
    bool testBool;
    GConfig->GetBool(*TitleName, TEXT("testBool"), testBool, FilePath);
	testBool =false;
	GConfig->SetBool(*TitleName, TEXT("testBool"), testBool, FilePath);

	Text = FString::Printf(TEXT("----Config--------testBool: %i"), testBool);
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, Text);

    //读取多行的Array  配置文件会分行显示 例：testArray=test1 testArray=test2
    TArray<FString> testArray;
    GConfig->GetArray(*TitleName, TEXT("testArray"), testArray, FilePath);
    testArray.Add("test1");
	GConfig->SetArray(*TitleName, TEXT("testArray"), testArray, FilePath);

    TArray<FString> testArray2;
    testArray2.Add("test111");
    testArray2.Add("test222");
    testArray2.Add("test333");
    //读取单行的Array 例：testArray2="test111 test222 test333 " 
	GConfig->SetSingleLineArray(*TitleName, TEXT("testArray2"), testArray2, FilePath);
    testArray2.Empty();
	GConfig->GetSingleLineArray(*TitleName, TEXT("testArray2"), testArray2, FilePath);

	Text = FString::Printf(TEXT("----Config--------testArray2: %i"), testArray2.Num());
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, Text);

    //读取/写入颜色 例：testColor=(R=15,G=12,B=14,A=13)
    FColor	Color1;
    FColor	Color2;
    Color1.G = 12;
    Color1.A = 13;
    Color1.B = 14;
    Color1.R = 15;
    GConfig->SetColor(*TitleName, TEXT("testColor"), Color1, FilePath);
    GConfig->GetColor(*TitleName, TEXT("testColor"), Color2, FilePath);

    //读取/写入 2d坐标  例：testVector2D=X=11.000 Y=11.000
    FVector2D Vector2D1 = FVector2D(11,11);
    FVector2D Vector2D2;
	GConfig->SetVector2D(*TitleName, TEXT("testVector2D"), Vector2D1, FilePath);
	GConfig->GetVector2D(*TitleName, TEXT("testVector2D"), Vector2D2, FilePath);

	//读取/写入 3d坐标  例：testVector=X=11.000 Y=11.000 Z=11.000
    FVector	Vector1= FVector(11, 11,11);
    FVector	Vector2;
	GConfig->SetVector(*TitleName, TEXT("testVector"), Vector1, FilePath);
	GConfig->GetVector(*TitleName, TEXT("testVector"), Vector2, FilePath);

    //读取/写入Vector4  例：testVector4=X=0.000 Y=0.000 Z=0.000 W=1.000
    /*
      Vector4既可以表示点也可以表示向量 例：
	  点，用Vector4表示就是(x，y，z，1)
      向量，用Vector4表示就是(x，y，z，0)
      向量是两个点相减组成，(x1，y1，z1，1)- (x2，y2，z2，1) =(x1 - x2， y1 - y2，z1 - z2，0)，
      表示两个点相减是一个向量。
    */
    FVector4 Value1 = FVector4(10,10,10,0);
    FVector4 Value2;
	GConfig->SetVector4(*TitleName, TEXT("testVector4"), Value1, FilePath);
	GConfig->GetVector4(*TitleName, TEXT("testVector4"), Value2, FilePath);

    //读取/写入Rotator  例：testRotator=P=20.000000 Y=20.000000 R=20.000000
    FRotator Rot1= FRotator(20,20,20);
    FRotator Rot2;
	GConfig->SetRotator(*TitleName, TEXT("testRotator"), Rot1, FilePath);
	GConfig->GetRotator(*TitleName, TEXT("testRotator"), Rot2, FilePath);

}


bool UConfigManager::GetIniConfigBool(const EProjType type, const FString key)
{
    if (!GConfig) return false;
    bool bValue;
	//配置文件路径
	FString FilePath = FPaths::SourceConfigDir() + ConfigName;
    switch (type)
    {
    case EProjType::ZHJZ:
		GConfig->GetBool(TEXT("ZHJZ") , *key, bValue, FilePath);
        break;
	case EProjType::EQ:
		GConfig->GetBool(TEXT("EQ"), *key, bValue, FilePath);
		break;
	case EProjType::SQ:
		GConfig->GetBool(TEXT("SQ"), *key, bValue, FilePath);
		break;
	case EProjType::MR:
		GConfig->GetBool(TEXT("MR"), *key, bValue, FilePath);
		break;
	case EProjType::LC:
		GConfig->GetBool(TEXT("LC"), *key, bValue, FilePath);
		break;
    default:
		GConfig->GetBool(TEXT("Test"), *key, bValue, FilePath);
        break;
    }
    return bValue;
}

int UConfigManager::GetIniConfigInt(const EProjType type, const FString key)
{
    if (!GConfig) return false;
    int iValue = -99999;
    //配置文件路径
    FString FilePath = FPaths::SourceConfigDir() + ConfigName;
    switch (type)
    {
    case EProjType::ZHJZ:
        GConfig->GetInt(TEXT("ZHJZ"), *key, iValue, FilePath);
        break;
    case EProjType::EQ:
        GConfig->GetInt(TEXT("EQ"), *key, iValue, FilePath);
        break;
    case EProjType::SQ:
        GConfig->GetInt(TEXT("SQ"), *key, iValue, FilePath);
        break;
    case EProjType::MR:
        GConfig->GetInt(TEXT("MR"), *key, iValue, FilePath);
        break;
    case EProjType::LC:
        GConfig->GetInt(TEXT("LC"), *key, iValue, FilePath);
        break;
    default:
        GConfig->GetInt(TEXT("Test"), *key, iValue, FilePath);
        break;
    }
    return iValue;
}

float UConfigManager::GetIniConfigFloat(const EProjType type, const FString key)
{
    if (!GConfig) return false;
    float fValue = 0.0f;
    //配置文件路径
    FString FilePath = FPaths::SourceConfigDir() + ConfigName;
    switch (type)
    {
    case EProjType::ZHJZ:
        GConfig->GetFloat(TEXT("ZHJZ"), *key, fValue, FilePath);
        break;
    case EProjType::EQ:
        GConfig->GetFloat(TEXT("EQ"), *key, fValue, FilePath);
        break;
    case EProjType::SQ:
        GConfig->GetFloat(TEXT("SQ"), *key, fValue, FilePath);
        break;
    case EProjType::MR:
        GConfig->GetFloat(TEXT("MR"), *key, fValue, FilePath);
        break;
    case EProjType::LC:
        GConfig->GetFloat(TEXT("LC"), *key, fValue, FilePath);
        break;
    default:
        GConfig->GetFloat(TEXT("Test"), *key, fValue, FilePath);
        break;
    }
    return fValue;
}

TArray<FString> UConfigManager::GetIniConfigArray(const EProjType type, const FString key)
{
    TArray<FString>  aValue;
	if (!GConfig) return aValue;

	//配置文件路径
	FString FilePath = FPaths::SourceConfigDir() + ConfigName;
	switch (type)
	{
	case EProjType::ZHJZ:
		GConfig->GetArray(TEXT("ZHJZ"), *key, aValue, FilePath);
		break;
	case EProjType::EQ:
		GConfig->GetArray(TEXT("EQ"), *key, aValue, FilePath);
		break;
	case EProjType::SQ:
		GConfig->GetArray(TEXT("SQ"), *key, aValue, FilePath);
		break;
	case EProjType::MR:
		GConfig->GetArray(TEXT("MR"), *key, aValue, FilePath);
		break;
	case EProjType::LC:
		GConfig->GetArray(TEXT("LC"), *key, aValue, FilePath);
		break;
	default:
		GConfig->GetArray(TEXT("Test"), *key, aValue, FilePath);
		break;
	}
	return aValue;
}

TArray<FString> UConfigManager::GetIniConfigSingleLineArray(const EProjType type, const FString key)
{
	TArray<FString>  aValue;
	if (!GConfig) return aValue;

	//配置文件路径
	FString FilePath = FPaths::SourceConfigDir() + ConfigName;
	switch (type)
	{
	case EProjType::ZHJZ:
		GConfig->GetSingleLineArray(TEXT("ZHJZ"), *key, aValue, FilePath);
		break;
	case EProjType::EQ:
		GConfig->GetSingleLineArray(TEXT("EQ"), *key, aValue, FilePath);
		break;
	case EProjType::SQ:
		GConfig->GetSingleLineArray(TEXT("SQ"), *key, aValue, FilePath);
		break;
	case EProjType::MR:
		GConfig->GetSingleLineArray(TEXT("MR"), *key, aValue, FilePath);
		break;
	case EProjType::LC:
		GConfig->GetSingleLineArray(TEXT("LC"), *key, aValue, FilePath);
		break;
	default:
		GConfig->GetSingleLineArray(TEXT("Test"), *key, aValue, FilePath);
		break;
	}
	return aValue;
}

FColor UConfigManager::GetIniConfigColor(const EProjType type, const FString key)
{
	FColor  cValue;
	if (!GConfig) return cValue;

	//配置文件路径
	FString FilePath = FPaths::SourceConfigDir() + ConfigName;
	switch (type)
	{
	case EProjType::ZHJZ:
		GConfig->GetColor(TEXT("ZHJZ"), *key, cValue, FilePath);
		break;
	case EProjType::EQ:
		GConfig->GetColor(TEXT("EQ"), *key, cValue, FilePath);
		break;
	case EProjType::SQ:
		GConfig->GetColor(TEXT("SQ"), *key, cValue, FilePath);
		break;
	case EProjType::MR:
		GConfig->GetColor(TEXT("MR"), *key, cValue, FilePath);
		break;
	case EProjType::LC:
		GConfig->GetColor(TEXT("LC"), *key, cValue, FilePath);
		break;
	default:
		GConfig->GetColor(TEXT("Test"), *key, cValue, FilePath);
		break;
	}
	return cValue;
}

FVector2D UConfigManager::GetIniConfigVector2D(const EProjType type, const FString key)
{
	FVector2D  vValue;
	if (!GConfig) return vValue;

	//配置文件路径
	FString FilePath = FPaths::SourceConfigDir() + ConfigName;
	switch (type)
	{
	case EProjType::ZHJZ:
		GConfig->GetVector2D(TEXT("ZHJZ"), *key, vValue, FilePath);
		break;
	case EProjType::EQ:
		GConfig->GetVector2D(TEXT("EQ"), *key, vValue, FilePath);
		break;
	case EProjType::SQ:
		GConfig->GetVector2D(TEXT("SQ"), *key, vValue, FilePath);
		break;
	case EProjType::MR:
		GConfig->GetVector2D(TEXT("MR"), *key, vValue, FilePath);
		break;
	case EProjType::LC:
		GConfig->GetVector2D(TEXT("LC"), *key, vValue, FilePath);
		break;
	default:
		GConfig->GetVector2D(TEXT("Test"), *key, vValue, FilePath);
		break;
	}
	return vValue;
}

FVector UConfigManager::GetIniConfigVector(const EProjType type, const FString key)
{
	FVector  vValue;
	if (!GConfig) return vValue;

	//配置文件路径
	FString FilePath = FPaths::SourceConfigDir() + ConfigName;
	switch (type)
	{
	case EProjType::ZHJZ:
		GConfig->GetVector(TEXT("ZHJZ"), *key, vValue, FilePath);
		break;
	case EProjType::EQ:
		GConfig->GetVector(TEXT("EQ"), *key, vValue, FilePath);
		break;
	case EProjType::SQ:
		GConfig->GetVector(TEXT("SQ"), *key, vValue, FilePath);
		break;
	case EProjType::MR:
		GConfig->GetVector(TEXT("MR"), *key, vValue, FilePath);
		break;
	case EProjType::LC:
		GConfig->GetVector(TEXT("LC"), *key, vValue, FilePath);
		break;
	default:
		GConfig->GetVector(TEXT("Test"), *key, vValue, FilePath);
		break;
	}
	return vValue;
}

FVector4 UConfigManager::GetIniConfigVector4(const EProjType type, const FString key)
{
	FVector4  vValue;
	if (!GConfig) return vValue;

	//配置文件路径
	FString FilePath = FPaths::SourceConfigDir() + ConfigName;
	switch (type)
	{
	case EProjType::ZHJZ:
		GConfig->GetVector4(TEXT("ZHJZ"), *key, vValue, FilePath);
		break;
	case EProjType::EQ:
		GConfig->GetVector4(TEXT("EQ"), *key, vValue, FilePath);
		break;
	case EProjType::SQ:
		GConfig->GetVector4(TEXT("SQ"), *key, vValue, FilePath);
		break;
	case EProjType::MR:
		GConfig->GetVector4(TEXT("MR"), *key, vValue, FilePath);
		break;
	case EProjType::LC:
		GConfig->GetVector4(TEXT("LC"), *key, vValue, FilePath);
		break;
	default:
		GConfig->GetVector4(TEXT("Test"), *key, vValue, FilePath);
		break;
	}
	return vValue;
}

FRotator UConfigManager::GetIniConfigRotator(const EProjType type, const FString key)
{
	FRotator  rValue;
	if (!GConfig) return rValue;

	//配置文件路径
	FString FilePath = FPaths::SourceConfigDir() + ConfigName;
	switch (type)
	{
	case EProjType::ZHJZ:
		GConfig->GetRotator(TEXT("ZHJZ"), *key, rValue, FilePath);
		break;
	case EProjType::EQ:
		GConfig->GetRotator(TEXT("EQ"), *key, rValue, FilePath);
		break;
	case EProjType::SQ:
		GConfig->GetRotator(TEXT("SQ"), *key, rValue, FilePath);
		break;
	case EProjType::MR:
		GConfig->GetRotator(TEXT("MR"), *key, rValue, FilePath);
		break;
	case EProjType::LC:
		GConfig->GetRotator(TEXT("LC"), *key, rValue, FilePath);
		break;
	default:
		GConfig->GetRotator(TEXT("Test"), *key, rValue, FilePath);
		break;
	}
	return rValue;
}
