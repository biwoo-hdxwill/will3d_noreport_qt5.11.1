#include "pano_algorithm.h"

#include "../../Common/Common/W3Define.h"
#include "../../Resource/Resource/W3Image3D.h"

#include "../AutoCanal/AutoCanal.h"
#include "../AutoArch/W3AutoArch.h"
namespace {
	bool IsFreeFOV(const CW3Image3D* vol) {
		int size_slice_mm = (int)((float)(vol->width()*vol->height())*vol->pixelSpacing()*vol->pixelSpacing());
		return (size_slice_mm < common::dicom::kFreeFOVSliceMaximumSizeMM) ? true : false;
	}
}
void PanoAlgorithm::RunMandibleAutoArch(const CW3Image3D* vol, std::vector<glm::vec3>& points, const int target_slice_number)
{
	if (IsFreeFOV(vol))
		return;

	CW3AutoArch::runMandibleArch(vol, points, target_slice_number);
}

void PanoAlgorithm::RunMaxillaAutoArch(const CW3Image3D * vol, std::vector<glm::vec3>& points, const int target_slice_number) {
	if (IsFreeFOV(vol))
		return;

	CW3AutoArch::runMaxillaArch(vol, points, target_slice_number);
}

void PanoAlgorithm::runAutoCanal(unsigned char **out, glm::vec3 *coord, float pixelsize, CW3Image3D *pVolume)
{
	CW3AutoCanal::ActionEx(
		pVolume->width(), pVolume->height(), pVolume->depth(), pVolume->getData(),
		coord[0].x, coord[0].y, coord[0].z, coord[1].x, coord[1].y, coord[1].z,
		out, pixelsize, pVolume->getTissueBoneThreshold());

}

void PanoAlgorithm::runAutoCanal(std::vector<glm::vec3> *out, glm::vec3 *coord, float pixelsize, CW3Image3D *pVolume)
{
	/* smseo : unused functions */
	//printf("Auto Canal Started\n");
	//CW3AutoCanal::ActionEx(
	//	pVolume->width(), pVolume->height(), pVolume->depth(), pVolume->getData(),
	//	coord[0].x, coord[0].y, coord[0].z, coord[1].x, coord[1].y, coord[1].z,
	//	out, pixelsize, pVolume->getTissueBoneThreshold());
}

void PanoAlgorithm::runAutoCanal(unsigned char **out, glm::vec3 *coord, float pixelsize, CW3Image3D* pVolume, int *startZ, int *endZ)
{
	CW3AutoCanal::ActionFrom2DEx(
		pVolume->width(), pVolume->height(), pVolume->depth(), pVolume->getData(),
		coord[0].x, coord[0].y, coord[1].x, coord[1].y,
		out, pixelsize, startZ, endZ, pVolume->getTissueBoneThreshold());
}
void PanoAlgorithm::runAutoCanal(std::vector<glm::vec3> *out, float pixelsize, CW3Image3D *pVolume)
{
	/* smseo : unused functions */
	//int startz, endz;
	//int startx = out->at(0).x;
	//int starty = out->at(0).y;
	//int endx = out->at(1).x;
	//int endy = out->at(1).y;

	//out->clear();

	//CW3AutoCanal::ActionFrom2DEx(
	//	pVolume->width(), pVolume->height(), pVolume->depth(), pVolume->getData(), 
	//	startx, starty, endx, endy, 
	//	out, pixelsize, &startz, &endz, pVolume->getTissueBoneThreshold());
}
