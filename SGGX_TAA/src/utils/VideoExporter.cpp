#include "VideoExporter.h"

void VideoExporter::startExport()
{
	// prob makes sense to do this -> test
	stbi_flip_vertically_on_write(1);
	
	m_frameNumber = 0;
	m_isExporting = true;
}

int VideoExporter::registerFrame(unsigned int height, unsigned int width, unsigned int num_channels, void* data)
{
	std::ostringstream string_out;
	string_out << std::internal << std::setfill('0') << std::setw(4) << std::to_string(m_frameNumber);
	const std::string file_name = "out_" + string_out.str() + "_.png";
	const std::string combined_path = FRAMES_OUT_DIR + "/" + file_name;

	int result = stbi_write_png(combined_path.c_str(), width, height, num_channels, data, 0);
	if (result) {
		m_frameNumber++;
	}
	// TODO, simply dont accept a frame if it didnt work
	return result;
}

bool VideoExporter::exportStarted()
{
	return m_isExporting;
}

void VideoExporter::endExport()
{
	m_isExporting = false;
}
