#pragma once

#include <filesystem>
#include <string>
#include <sstream>

#include "../3rd_party/stb_image/stb_image_write.h"	

class VideoExporter
{
private:
	
	unsigned int m_frameNumber = 0;
	bool m_isExporting = false;


	const std::string FRAMES_OUT_DIR = "out/tmp_frames";
public:
	VideoExporter() {};

	void startExport();
	int registerFrame(unsigned int height, unsigned int width, unsigned int num_channels, void* data);
	bool exportStarted();
	
	void endExport();
};


