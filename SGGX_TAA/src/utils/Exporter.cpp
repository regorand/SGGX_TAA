#include "Exporter.h"

Exporter::Exporter() {
	stbi_flip_vertically_on_write(1);
}

void Exporter::startVideoExport()
{	
	m_frameNumber = 0;
	m_isExporting = true;
}

int Exporter::registerVideoFrame(unsigned int height, unsigned int width, unsigned int num_channels, void* data)
{
	std::ostringstream string_out;
	string_out << std::internal << std::setfill('0') << std::setw(4) << std::to_string(m_frameNumber);
	const std::string file_name = "out_" + string_out.str() + "_.png";
	const std::string combined_path = VIDEO_FRAMES_OUT_DIR + "/" + file_name;

	int result = stbi_write_png(combined_path.c_str(), width, height, num_channels, data, 0);
	if (result) {
		m_frameNumber++;
	}
	// TODO, simply dont accept a frame if it didnt work
	return result;
}

bool Exporter::videoExportStarted()
{
	return m_isExporting;
}

void Exporter::endVideoExport()
{
	m_isExporting = false;
}

int Exporter::exportImage(unsigned int height, unsigned int width, unsigned int num_channels, void* data, unsigned long time)
{
	const std::string file_name = std::to_string(time) + ".png";
	const std::string combined_path = IMAGE_OUT_DIR + "/" + file_name;

	return stbi_write_png(combined_path.c_str(), width, height, num_channels, data, 0); 
}
