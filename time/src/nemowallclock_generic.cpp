
#include "nemowallclock_p.h"

WallClockPrivate *nemoCreateWallClockPrivate(WallClock *wc)
{
    return new WallClockPrivate(wc);
}

