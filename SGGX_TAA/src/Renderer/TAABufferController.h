#pragma once

#include <memory>

#include "../utils/GL_Utils.h"
#include "../gl_wrappers/TextureBuffer.h"

class TAABufferController
{
private:

	// Render into this
	std::shared_ptr<TextureBuffer> m_renderBuffer;

	// Holds color history values
	std::shared_ptr<TextureBuffer> m_historyBuffer;

	// TODO better name ? -> This stores octree node ids, if they dont fit with current -> history rejection
	std::shared_ptr<TextureBuffer> m_rejectionBuffer;

	std::shared_ptr<TextureBuffer> m_nodeHitBuffer;

	// TODO: This one can make nodeHitBuffer obsolete if we rebuild history rejection based on the world space coordinate of the 
	// voxel center, which will be stored here
	// Can even do parent based history rejection
	std::shared_ptr<TextureBuffer> m_spaceHitBuffer;

	// This stores an estimated coordinated offset in buffer coordinate space for where this pixels node was rendered last frame
	// r and g channel contain offset, b channel will set to 1 if offset is valid, 0 if no previous location could be found or on
	// similar errors -> will usually mean: no history can be used
	std::shared_ptr<TextureBuffer> m_motionVectorBuffer;

	std::shared_ptr<TextureBuffer> m_estimatedLodDiff;

	bool m_isValid = false;

public:
	TAABufferController();
	~TAABufferController();

	void registerBuffer(std::shared_ptr<TextureBuffer>& target,
		const size_t size,
		const unsigned int datatype,
		const unsigned int texture_target);

	void registerBuffers(const unsigned int width, const unsigned int height);

	bool isValid();

	void bindAll();
	void unbindAll();

};

