#ifndef __COMMAND_ID_H__
#define __COMMAND_ID_H__

#include "Base/BaseTypes.h"

enum CommandID : DAVA::int32
{
    CMDID_UNKNOWN = -1,
    CMDID_BATCH = 0,

    CMDID_TRANSFORM,

    CMDID_ENTITY_ADD,
    CMDID_ENTITY_REMOVE,
    CMDID_ENTITY_CHANGE_PARENT,
    CMDID_ENTITY_LOCK,
    CMDID_PARTICLE_EMITTER_MOVE,
    CMDID_PARTICLE_LAYER_REMOVE,
    CMDID_PARTICLE_LAYER_MOVE,
    CMDID_PARTICLE_FORCE_REMOVE,
    CMDID_PARTICLE_FORCE_MOVE,
    CMDID_GROUP_ENTITIES_FOR_MULTISELECT,
    CMDID_LANDSCAPE_SET_HEIGHTMAP,
    CMDID_BAKE_GEOMERTY,

    CMDID_META_OBJ_MODIFY,
    CMDID_INSP_MEMBER_MODIFY,
    CMDID_INSP_DYNAMIC_MODIFY,

    CMDID_KEYEDARCHIVE_ADD_KEY,
    CMDID_KEYEDARCHIVE_REM_KEY,
    CMDID_KEYEDARCHIVE_SET_KEY,

    CMDID_CUSTOM_COLORS_ENABLE,
    CMDID_CUSTOM_COLORS_DISABLE,
    CMDID_CUSTOM_COLORS_MODIFY,
    CMDID_VISIBILITY_TOOL_ENABLE,
    CMDID_VISIBILITY_TOOL_SET_POINT,
    CMDID_VISIBILITY_TOOL_SET_AREA,
    CMDID_HEIGHTMAP_EDITOR_ENABLE,
    CMDID_HEIGHTMAP_MODIFY,
    CMDID_TILEMASK_EDITOR_ENABLE,
    CMDID_TILEMASK_MODIFY,
    CMDID_SET_TILE_COLOR,
    CMDID_RULER_TOOL_ENABLE,
    CMDID_NOT_PASSABLE_TERRAIN_ENABLE,
    CMDID_NOT_PASSABLE_TERRAIN_DISABLE,

    CMDID_PARTICLE_EFFECT_UPDATE,
    CMDID_PARTICLE_EMITTER_UPDATE,
    CMDID_PARTICLE_EMITTER_POSITION_UPDATE,
    CMDID_PARTICLE_LAYER_UPDATE,
    CMDID_PARTILCE_LAYER_UPDATE_TIME,
    CMDID_PARTICLE_LAYER_UPDATE_ENABLED,
    CMDID_PARTICLE_LAYER_UPDATE_LODS,
    CMDID_PARTICLE_LAYER_CHANGED_MATERIAL_VALUES,
    CMDID_PARTICLE_FORCE_UPDATE,

    CMDID_PARTICLE_EMITTER_ADD,

    CMDID_PARTICLE_EFFECT_START_STOP,
    CMDID_PARTICLE_EFFECT_RESTART,
    CMDID_PARTICLE_EFFECT_EMITTER_REMOVE,

    CMDID_PARTICLE_EMITTER_LAYER_ADD,
    CMDID_PARTICLE_EMITTER_LAYER_REMOVE,
    CMDID_PARTICLE_EMITTER_LAYER_CLONE,
    CMDID_PARTICLE_EMITTER_FORCE_ADD,
    CMDID_PARTICLE_EMITTER_FORCE_REMOVE,

    CMDID_PARTICLE_EMITTER_LOAD_FROM_YAML,
    CMDID_PARTICLE_EMITTER_SAVE_TO_YAML,

    CMDID_PARTICLE_INNER_EMITTER_LOAD_FROM_YAML,
    CMDID_PARTICLE_INNER_EMITTER_SAVE_TO_YAML,

    CMDID_DAE_CONVERT,
    CMDID_ENTITY_SAVE_AS,

    CMDID_CONVERT_TO_SHADOW,

    CMDID_LOD_DISTANCE_CHANGE,
    CMDID_LOD_CREATE_PLANE,
    CMDID_LOD_COPY_LAST_LOD, //TODO: remove after lod editing implementation
    CMDID_LOD_DELETE,

    CMDID_BEAST,

    CMDID_COMPONENT_ADD,
    CMDID_COMPONENT_REMOVE,

    CMDID_MATERIAL_SWITCH_PARENT,
    CMDID_MATERIAL_GLOBAL_SET,
    CMDID_MATERIAL_REMOVE_TEXTURE,
    CMDID_MATERIAL_REMOVE_CONFIG,
    CMDID_MATERIAL_CREATE_CONFIG,
    CMDID_MATERIAL_CHANGE_CURRENT_CONFIG,
    CMDID_MATERIAL_CHANGE_CONFIG_NAME,

    CMDID_REBUILD_TANGENT_SPACE,
    CMDID_DELETE_RENDER_BATCH,
    CMDID_IMAGE_REGION_COPY,

    CMDID_SOUND_ADD_EVENT,
    CMDID_SOUND_REMOVE_EVENT,
    CMDID_SOUND_SET_EVENT_FLAGS,

    CMDID_CLONE_LAST_BATCH,

    CMDID_PAINT_HEIGHT_DELTA,

    CMDID_EXPAND_PATH,
    CMDID_COLLAPSE_PATH,
    CMDID_ENABLE_WAYEDIT,
    CMDID_DISABLE_WAYEDIT,

    CMDID_SHOW_MATERIAL,

    CMDID_USER = 0xF000
};

#endif // __COMMAND_ID_H__
