#pragma once
#include "DrawDebugHelpers.h"

#define DRAW_SPHERE(location) if (GetWorld()) DrawDebugSphere(GetWorld(),location, 25.f, 12, FColor::Red, true);
#define DRAW_SPHERE_COLOR(location, color) if (GetWorld()) DrawDebugSphere(GetWorld(),location, 8.f, 12, color, false, 5.f);
#define DRAW_LINE(startLoc, endLoc) if (GetWorld()) DrawDebugLine(GetWorld(), startLoc, endLoc, FColor::Black, true, -1.f ,0 ,1.f);
#define DRAW_POINT(location) if(GetWorld()) DrawDebugPoint(GetWorld(), location, 25.f, FColor::Blue, true);
#define DRAW_VECTOR(startLoc, endLoc) \
	DRAW_LINE(startLoc, endLoc); \
	DRAW_POINT(endLoc);
#define DRAW_SPHERE_SINGLEFRAME(location) if (GetWorld()) DrawDebugSphere(GetWorld(),location, 25.f, 12, FColor::Red, false, -1.f);
#define DRAW_LINE_SINGLEFRAME(startLoc, endLoc) if (GetWorld()) DrawDebugLine(GetWorld(), startLoc, endLoc, FColor::Black, false, -1.f ,0 ,1.f);
#define DRAW_POINT_SINGLEFRAME(location) if(GetWorld()) DrawDebugPoint(GetWorld(), location, 25.f, FColor::Blue, false, -1.f);
#define DRAW_VECTOR_SINGLEFRAME(startLoc, endLoc) \
	DRAW_LINE_SINGLEFRAME(startLoc, endLoc); \
	DRAW_POINT_SINGLEFRAME(endLoc)

DECLARE_LOG_CATEGORY_EXTERN(LogWindless, Log, All);