#include "Enemy/AI/MonsterArchetypeRow.h"
#include "Enemy/AI/EnemyAIDataValidation.h"

void FMonsterArchetypeRow::OnPostDataImport(const UDataTable* inDataTable, const FName inRowName, TArray<FString>& outCollectedImportProblems)
{
	FEnemyAIValidationResult result;
	EnemyAIDataValidation::ValidateArchetypeUsingSettings(*this, inRowName.ToString(), result);
	result.AppendTo(outCollectedImportProblems);
}

#if WITH_EDITOR
EDataValidationResult FMonsterArchetypeRow::IsDataValid(FDataValidationContext& context) const
{
	FEnemyAIValidationResult result;
	EnemyAIDataValidation::ValidateArchetypeUsingSettings(*this, monsterId, result);
	return result.ReportTo(context);
}
#endif