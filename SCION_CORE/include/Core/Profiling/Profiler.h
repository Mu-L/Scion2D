#pragma once

#ifdef TRACY_ENABLE

#include <tracy/Tracy.hpp>
#define SCION_PROFILE_SCOPE() ZoneScoped
#define SCION_PROFILE_SCOPE_N( name ) ZoneScopedN( name )
#define SCION_PROFILE_FRAME() FrameMark
#define SCION_PROFILE_FRAME_N( name ) FrameMarkNamed( name )
#define SCION_PROFILE_ALLOC( ptr, size ) TracyAlloc( ptr, size )
#define SCION_PROFILE_FREE( ptr ) TracyFree( ptr )

#else

#define SCION_PROFILE_SCOPE()
#define SCION_PROFILE_SCOPE_N( name )
#define SCION_PROFILE_FRAME()
#define SCION_PROFILE_FRAME_N( name )
#define SCION_PROFILE_ALLOC( ptr, size )
#define SCION_PROFILE_FREE( ptr )

#endif
