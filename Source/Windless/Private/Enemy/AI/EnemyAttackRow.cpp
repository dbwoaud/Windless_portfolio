#include "Enemy/AI/EnemyAttackRow.h"
#include "Enemy/AI/EnemyAIDataValidation.h"

bool FEnemyAttackRow::UsesMultiHitTracking() const
{
	for (int32 i = 1; i < GetHitCount(); ++i)
	{
		if (GetTrackProfile(i) == EEnemyTrackProfile::ETP_Strong)
			return true;
	}
	return false;
}

void FEnemyAttackRow::OnPostDataImport(const UDataTable* inDataTable, const FName inRowName, TArray<FString>& outCollectedImportProblems)
{
	FEnemyAIValidationResult result;
	EnemyAIDataValidation::ValidateAttack(*this, inRowName.ToString(), EnemyAIDataValidation::GetReferenceValues(), result);
	result.AppendTo(outCollectedImportProblems);
}

#if WITH_EDITOR
EDataValidationResult FEnemyAttackRow::IsDataValid(FDataValidationContext& context) const
{
	FEnemyAIValidationResult result;
	EnemyAIDataValidation::ValidateAttack(*this, attackName.ToString(), EnemyAIDataValidation::GetReferenceValues(), result);
	return result.ReportTo(context);
}
#endif