#pragma once
/**=================================================================================================

Project: 			ResContainer
File:				resource_container.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-07-27
Last modify:		2017-07-27

 *===============================================================================================**/
#include <memory>
#include <mutex>

#include <QObject>
#include "rescontainer_global.h"

class CW3Image3D;
class FacePhotoResource;
class CW3ImageHeader;
class PlaneResource;
class PanoResource;
class NerveResource;
class CrossSectionResource;
class ImplantResource;
class TMJresource;
class SagittalResource;
class LightboxResource;
class CW3TF;

class RESCONTAINER_EXPORT ResourceContainer : public QObject {
	Q_OBJECT
public:
	static void SetInstance();
	static ResourceContainer* GetInstance();

	void Reset();

public:
	inline void SetMainVolumeResource(const std::shared_ptr<CW3Image3D>& res) { res_main_volume_ = res; }
	inline void SetSecondVolumeResource(const std::shared_ptr<CW3Image3D>& res) { res_second_volume_ = res; }
	inline void SetFacePhotoResource(const std::weak_ptr<FacePhotoResource>& res) { res_face_ = res; }
	inline void SetDicomHeaderResource(const std::weak_ptr<CW3ImageHeader>& header) { res_dicom_header_ = header; }
	inline void SetPlaneResource(const std::weak_ptr<PlaneResource>& res) { res_plane_ = res; }
	inline void SetPanoResource(const std::weak_ptr<PanoResource>& res) { res_pano_ = res; }
	inline void SetCrossSectionResource(const std::weak_ptr<CrossSectionResource>& res) { res_cross_section_ = res; }
	inline void SetNerveResource(const std::weak_ptr<NerveResource>& res) { res_nerve_ = res; }
	inline void SetImplantResource(const std::weak_ptr<ImplantResource>& res) { res_implant_ = res; }
	inline void SetTMJResource(const std::weak_ptr<TMJresource>& res) { res_tmj_ = res; }
	inline void SetSagittalResource(const std::weak_ptr<SagittalResource>& res) { res_sagittal_ = res; }
	inline void SetLightboxResource(const std::weak_ptr<LightboxResource>& res) { res_lightbox_ = res; }
	inline void SetTfResource(const std::weak_ptr<CW3TF>& res) { res_tf_ = res; }

	inline const CW3Image3D& GetMainVolume() const { return *(res_main_volume_.get()); }
	inline const CW3Image3D& GetSecondVolume() const { return *(res_second_volume_.get()); }
	inline FacePhotoResource* res_face() const { return res_face_.lock().get(); }
	inline const FacePhotoResource& GetFacePhotoResource() const { return *(res_face_.lock().get()); }
	inline const CW3ImageHeader& GetDicomHeaderResource() const { return *(res_dicom_header_.lock().get()); }
	inline const PlaneResource& GetPlaneResource() const { return*(res_plane_.lock().get()); }
	inline const PanoResource& GetPanoResource() const { return *(res_pano_.lock().get()); }
	inline const NerveResource& GetNerveResource() const { return *(res_nerve_.lock().get()); }
	inline const CrossSectionResource& GetCrossSectionResource() const { return *(res_cross_section_.lock().get()); }
	inline const ImplantResource& GetImplantResource() const { return *(res_implant_.lock().get()); }
	inline const TMJresource& GetTMJResource() const { return *(res_tmj_.lock().get()); }
	inline const SagittalResource& GetSagittalResource() const { return *(res_sagittal_.lock().get()); }
	inline const LightboxResource& GetLightboxResource() const { return *(res_lightbox_.lock().get()); }
	inline const CW3TF& GetTfResource() const { return *(res_tf_.lock().get()); }
private:
	ResourceContainer() = default;
	~ResourceContainer();
	ResourceContainer(const ResourceContainer&) = delete;
	ResourceContainer& operator=(const ResourceContainer&) = delete;

private:
	static ResourceContainer* instance_;
	static std::once_flag onceFlag_;

private:
	std::shared_ptr<CW3Image3D> res_main_volume_;
	std::shared_ptr<CW3Image3D> res_second_volume_;
	std::weak_ptr<FacePhotoResource> res_face_;
	std::weak_ptr<CW3ImageHeader> res_dicom_header_;
	std::weak_ptr<PlaneResource> res_plane_;
	std::weak_ptr<PanoResource> res_pano_;
	std::weak_ptr<NerveResource> res_nerve_;
	std::weak_ptr<CrossSectionResource> res_cross_section_;
	std::weak_ptr<ImplantResource> res_implant_;
	std::weak_ptr<TMJresource> res_tmj_;
	std::weak_ptr<SagittalResource> res_sagittal_;
	std::weak_ptr<LightboxResource> res_lightbox_;
	std::weak_ptr<CW3TF> res_tf_;
};
