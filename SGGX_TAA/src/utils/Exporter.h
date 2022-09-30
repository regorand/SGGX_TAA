#pragma once

#include <filesystem>
#include <string>
#include <sstream>

#include "../3rd_party/stb_image/stb_image_write.h"	

class Exporter
{
private:
	
	unsigned int m_frameNumber = 0;
	bool m_isExporting = false;


	const std::string VIDEO_FRAMES_OUT_DIR = "out/tmp_frames";

	const std::string IMAGE_OUT_DIR = "out/images";
public:
	Exporter();

	void startVideoExport();
	int registerVideoFrame(unsigned int height, unsigned int width, unsigned int num_channels, void* data);
	bool videoExportStarted();
	
	void endVideoExport();

	int exportImage(unsigned int height, unsigned int width, unsigned int num_channels, void *data, unsigned long time);
};


