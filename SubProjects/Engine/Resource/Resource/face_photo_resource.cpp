#include "face_photo_resource.h"

FacePhotoResource::FacePhotoResource() {}

FacePhotoResource::~FacePhotoResource() {}

void FacePhotoResource::clear() {
	points_.clear();
	points_after_.clear();
	tex_coords_.clear();
	indices_.clear();
	tex_handler_ = 0;
}
bool FacePhotoResource::IsSetFace() const {
	return (points_.size() > 0) ? true : false;
}
