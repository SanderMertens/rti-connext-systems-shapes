#ifndef ECS_SYSTEMS_SHAPES_H
#define ECS_SYSTEMS_SHAPES_H

#include "bake_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef float Size;

typedef struct EcsSystemsShapesHandles {
    EcsHandle DdsSync;
    EcsHandle Size;
} EcsSystemsShapesHandles;

void EcsSystemsShapes(
    EcsWorld *world,
    int flags,
    void *handles_out);

#define EcsSystemsShapes_DeclareHandles(handles)\
    EcsDeclareHandle(handles, DdsSync);\
    EcsDeclareHandle(handles, Size);

#ifdef __cplusplus
}
#endif

#endif
