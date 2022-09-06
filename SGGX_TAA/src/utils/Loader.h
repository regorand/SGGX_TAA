#pragma once

#include <filesystem>
#include <thread>
#include <queue>
#include <mutex>

#include "../Renderer/SceneObject.h"

enum class LoadJobStatus { SUCCESS, FAIL };
enum class LoaderThreadStatus { IDLE, LOADING, FINISHED };

class Loader
{
private:
	std::atomic<LoaderThreadStatus> mStatus;
	std::atomic<bool> mRunning;
	std::atomic<LoadJobStatus> mLoadJobStatus;

	std::string mLoadJobPath;
	SceneObject* mLoadBuffer;

	std::mutex mMutex;
	std::condition_variable mCond;

	std::thread mLoaderThread;
public:
	Loader();
	~Loader();

	void loaderThreadMain();

	bool loadSceneObjectAsynch(std::string object_path, SceneObject *scene_object);

	bool loadSceneObjectSynchronous(std::string object_path, SceneObject *scene_object);
	
	const LoaderThreadStatus getThreadStatus();
	const LoadJobStatus getJobStatus();

	void receivedResult();
	
	bool initVoxels(Mesh_Object_t& obj, unsigned int dimension, std::shared_ptr<VoxelGrid>& voxels);

	std::shared_ptr<RasterizationObject> registerMeshObject(Mesh_Object_t& source, std::shared_ptr<Shader> shader, glm::mat4 model_matrix);
};


