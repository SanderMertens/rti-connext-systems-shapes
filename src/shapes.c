#include <include/shapes.h>
#include "src/gen/ShapeType.h"
#include "src/gen/ShapeTypeSupport.h"
#include <reflecs/components/transform/transform.h>

#define DOMAIN_ID (0)
#define TOPIC_NAME "Square"

typedef struct DdsEntities {
    DDS_DomainParticipant *dp;
    DDS_Publisher *pub;
    DDS_Topic *topic;
    DDS_DataWriter *dw;
} DdsEntities;

/* -- System that synchronizes entities (shapes) to DDS -- */
void DdsSync(EcsRows *rows) {
    DdsEntities *w = ecs_column(rows, NULL, 0);
    ShapeTypeExtendedDataWriter *dw = ShapeTypeExtendedDataWriter_narrow(w->dw);
    char *colors[] = {"PURPLE", "BLUE", "RED", "GREEN", "YELLOW", "CYAN", "MAGENTA", "ORANGE"};

    void *row;
    for (row = rows->first; row < rows->last; row = ecs_next(rows, row)) {
        EcsHandle entity = ecs_entity(row);
        EcsPosition2D *p = ecs_column(rows, row, 1);
        EcsRotation2D *r = ecs_column(rows, row, 2);
        Size *size = ecs_column(rows, row, 3);

        ShapeTypeExtended instance;
        instance.parent.color = colors[entity % (sizeof(colors) / sizeof(char*))];
        instance.parent.x = p->x;
        instance.parent.y = p->y;
        instance.parent.shapesize = *size;
        instance.angle = r->angle;
        instance.fillKind = 0;
        ShapeTypeExtendedDataWriter_write(dw, &instance, &DDS_HANDLE_NIL);
    }
}

/* -- DDS entity code -- */

void DdsDeinit(EcsRows *rows) {
    void *row;
    for (row = rows->first; row < rows->last; row = ecs_next(rows, row)) {
        DdsEntities *writer = ecs_column(rows, row, 0);
        if (writer->dp)
            DDS_DomainParticipant_delete_contained_entities(writer->dp);
    }
}

void DdsInit(EcsRows *rows) {
    void *row;

    for (row = rows->first; row < rows->last; row = ecs_next(rows, row)) {
        DdsEntities *writer = ecs_column(rows, row, 0);
        writer->dp = DDS_DomainParticipantFactory_create_participant(
            DDS_TheParticipantFactory, DOMAIN_ID, &DDS_PARTICIPANT_QOS_DEFAULT,
            NULL, DDS_STATUS_MASK_NONE);
        if (!writer->dp) break;

        writer->pub = DDS_DomainParticipant_create_publisher(
            writer->dp, &DDS_PUBLISHER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
        if (!writer->pub) break;

        const char *type_name = ShapeTypeExtendedTypeSupport_get_type_name();
        DDS_ReturnCode_t retcode = ShapeTypeExtendedTypeSupport_register_type(
            writer->dp, type_name);
        if (retcode != DDS_RETCODE_OK) break;

        writer->topic = DDS_DomainParticipant_create_topic(
            writer->dp, TOPIC_NAME, type_name, &DDS_TOPIC_QOS_DEFAULT, NULL,
            DDS_STATUS_MASK_NONE);
        if (writer->topic == NULL) break;

        writer->dw = DDS_Publisher_create_datawriter(
            writer->pub, writer->topic, &DDS_DATAWRITER_QOS_DEFAULT, NULL,
            DDS_STATUS_MASK_NONE);
        if (writer->dw == NULL) break;
    }
}

void EcsSystemsShapes(
    EcsWorld *world,
    int flags,
    void *handles_out)
{
    EcsSystemsShapesHandles *handles = handles_out;

    ECS_IMPORT(world, EcsComponentsTransform, ECS_2D);

    ECS_COMPONENT(world, DdsEntities);
    ECS_COMPONENT(world, Size);
    ECS_SYSTEM(world, DdsInit,   EcsOnAdd,   DdsEntities);
    ECS_SYSTEM(world, DdsDeinit, EcsOnRemove, DdsEntities);
    ECS_SYSTEM(world, DdsSync,   EcsOnFrame, SYSTEM.DdsEntities, EcsPosition2D, EcsRotation2D, Size);

    handles->DdsSync = DdsSync_h;
    handles->Size = Size_h;
}
