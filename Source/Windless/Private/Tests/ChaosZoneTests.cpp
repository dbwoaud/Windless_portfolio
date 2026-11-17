#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/AttributeComponent.h"

/* 혼돈 게이지: 그늘, 어둠 상한, 빛 하한 (DEV-1~3) */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChaosZoneTest, "Windless.Chaos.Zones",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FChaosZoneTest::RunTest(const FString& Parameters)
{
	UAttributeComponent* attributes = NewObject<UAttributeComponent>();
	auto run = [attributes](float start, EChaosZone zone, float limit, float seconds)
	{
		attributes->DebugSetChaos(start);
		for (float t = 0.f; t < seconds; t += 0.1f)
			attributes->TickChaos(0.1f, zone, limit);
		return attributes->GetChaos();
	};

	TestEqual(TEXT("그늘에서는 그대로"), run(10.f, EChaosZone::Shade, 0.f, 60.f), 10.f, 0.01f);
	TestEqual(TEXT("어둠은 초당 +2 (정신력 저항만큼 줄어듦)"), run(0.f, EChaosZone::Dark, 50.f, 5.f), 10.f * (1.f - attributes->GetWillPowerResistRatio()), 0.3f);
	TestEqual(TEXT("얕은 어둠은 50에서 멈춤"), run(0.f, EChaosZone::Dark, 50.f, 60.f), 50.f, 0.01f);
	TestEqual(TEXT("깊은 어둠은 100까지"), run(0.f, EChaosZone::Dark, 100.f, 60.f), 100.f, 0.01f);
	TestEqual(TEXT("상한 위에서 들어오면 내리지 않음"), run(70.f, EChaosZone::Dark, 50.f, 10.f), 70.f, 0.01f);
	TestEqual(TEXT("빛은 초당 -1.5"), run(30.f, EChaosZone::Light, 0.f, 4.f), 24.f, 0.3f);
	TestEqual(TEXT("세이브포인트 빛은 0에서 멈춤"), run(30.f, EChaosZone::Light, 0.f, 60.f), 0.f, 0.01f);
	TestEqual(TEXT("횃불은 -21에서 멈춤"), run(30.f, EChaosZone::Light, -21.f, 60.f), -21.f, 0.01f);
	TestEqual(TEXT("하한 아래에서 들어오면 올리지 않음"), run(-40.f, EChaosZone::Light, -21.f, 10.f), -40.f, 0.01f);
	attributes->SetLanternOn(true);
	TestEqual(TEXT("등불을 켜면 어둠 증가가 절반"), run(0.f, EChaosZone::Dark, 50.f, 5.f), 5.f * (1.f - attributes->GetWillPowerResistRatio()), 0.2f);
	TestEqual(TEXT("등불은 빛의 감소를 바꾸지 않음"), run(30.f, EChaosZone::Light, 0.f, 4.f), 24.f, 0.3f);
	return true;
}

#endif
