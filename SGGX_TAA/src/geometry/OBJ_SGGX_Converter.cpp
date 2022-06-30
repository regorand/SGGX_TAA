#include "OBJ_SGGX_Converter.h"

SGGX_Distribution convert_obj_to_SGGX(Mesh_Object_t object, glm::vec3 dimension)
{
	return SGGX_Distribution(100, object);
}
