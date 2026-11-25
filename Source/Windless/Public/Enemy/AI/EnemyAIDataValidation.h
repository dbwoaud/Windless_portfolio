#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

struct FEnemyAttackRow;
struct FMonsterArchetypeRow;

/* 데이터 검사 결과 */
struct WINDLESS_API FEnemyAIValidationResult
{
	TArray<FString> errors;   // 에러
	TArray<FString> warnings; // 경고

	bool HasErrors() const { return errors.Num() > 0; } // 에러가 있는지 확인하는 함수 

	void AddError(const TCHAR* tag, const FString& owner, const FString& message);   // 에러를 출력하는 함수
	void AddWarning(const TCHAR* tag, const FString& owner, const FString& message); // 경고를 출력하는 함수
	void AppendTo(TArray<FString>& outProblems) const; // 임포트 문제 목록에 붙이는 함수

#if WITH_EDITOR
	EDataValidationResult ReportTo(FDataValidationContext& context) const; // 에디터 데이터 검증에 보고하는 함수
#endif
};

/* 플레이어 기준값 */
struct FEnemyAIReferenceValues
{
	float playerToughness = 15.f;       // 플레이어 강인도
	float playerWeakStaggerTime = 0.6f; // 플레이어 약한 경직 시간
};

/* 적 AI 데이터 검사 */
namespace EnemyAIDataValidation
{
	using FAttackFinder = TFunctionRef<const FEnemyAttackRow* (FName)>; // 공격 행 이름으로 공격 데이터를 반환하는 함수

	void ValidateAttack(const FEnemyAttackRow& attack, const FString& owner, const FEnemyAIReferenceValues& reference, FEnemyAIValidationResult& out); // 공격을 검사하는 함수
	void ValidateArchetype(const FMonsterArchetypeRow& archetype, const FString& owner, FAttackFinder findAttack, FEnemyAIValidationResult& out); // 원형과 등급 규칙을 검사하는 함수
	void ValidateArchetypeUsingSettings(const FMonsterArchetypeRow& archetype, const FString& owner, FEnemyAIValidationResult& out); // 프로젝트 세팅의 공격 테이블로 원형을 검사하는 함수

	FEnemyAIValidationResult ValidateAllTables(int32& outArchetypeCount, int32& outAttackCount); // 프로젝트 세팅의 두 테이블을 모두 검사하는 함수
	FEnemyAIReferenceValues GetReferenceValues(); // 외부 참조 값을 반환하는 함수 

	FString BuildExpectedMonsterId(const FMonsterArchetypeRow& archetype); // 몬스터 ID 앞부분을 만드는 함수
	bool MatchesMonsterId(const FString& actual, const FString& expected); // 몬스터 ID가 맞는지 확인하는 함수
} 