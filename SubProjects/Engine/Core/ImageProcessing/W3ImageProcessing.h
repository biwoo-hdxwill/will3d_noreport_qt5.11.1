#ifndef W3IMAGEPROCESSING_H
#define W3IMAGEPROCESSING_H

#include "imageprocessing_global.h"

#include "../../Common/GLfunctions/W3GLTypes.h"

class IMAGEPROCESSING_EXPORT CW3ImageProcessing
{
public:
	CW3ImageProcessing();
	~CW3ImageProcessing();


public:
	static void GaussianBlur(ushort * buffer, int buffer_width, int buffer_height, SharpenLevel level);
	static void Sharpen(ushort* buffer, int buffer_width, int buffer_height, SharpenLevel level);
private:
	static void GetFilterParameter(const SharpenLevel& level, float* sigma, double* weight);
};

#endif // W3IMAGEPROCESSING_H
