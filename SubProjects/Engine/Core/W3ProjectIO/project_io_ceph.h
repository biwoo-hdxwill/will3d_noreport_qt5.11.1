#pragma once
/*=========================================================================

File:			class ProjectIOCeph
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-07-19
Last modify:	2016-07-19

=========================================================================*/
#include <map>
#include <memory>
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif
#include <H5Cpp.h>
#include <qstring.h>

#include "../../Common/Common/W3Enum.h"
#include "datatypes.h"
#include "w3projectio_global.h"

class ProjectIOView;

class W3PROJECTIO_EXPORT ProjectIOCeph {
public:
	ProjectIOCeph(const project::Purpose& purpose,
				  const std::shared_ptr<H5::H5File>& file);
	~ProjectIOCeph();

	ProjectIOCeph(const ProjectIOCeph&) = delete;
	ProjectIOCeph& operator=(const ProjectIOCeph&) = delete;

	enum class ViewType {
		CEPH
	};

public:
	void InitCephTab();
	bool IsInit();
	ProjectIOView & GetViewIO();

	/*
		save functions
	*/
	// save ceph view
	void SaveIsSurgeryCut(const bool& maxilla, const bool& mandible,
						  const bool& chin);
	void SaveIsSurgeryAdjust(const bool& maxilla, const bool& mandible,
							 const bool& chin);
	void SaveIsSurgeryMove(const bool& maxilla, const bool& mandible,
						   const bool& chin);
	void SaveSurgeryCutItemCount(const int& cut_item_cnt);
	void SaveSurgeryCutItemPoints(const int& cut_item_id,
								  const std::vector<glm::vec3>& points);
	void SaveSrugeryCutItemMatrix(const int& cut_item_id,
								  const glm::mat4& trans, const glm::mat4& rot,
								  const glm::mat4& scale, const glm::mat4& arcball,
								  const glm::mat4& reori);
	void SaveLandmarks(const std::map<QString, glm::vec3>& landmarks);

	// save surgery bar
	void SaveSurgeryBtnStatusText(const int& button_id, const std::string& status_text);
	void SaveSrugeryParams(const std::vector<float>& params);
	void SaveSurgeryParamsPrev(const std::vector<float>& params);
	void SaveIsOutterEdit(const bool& edit);

	/*
		load functions
	*/
	// load ceph view
	void LoadIsSurgeryCut(bool& maxilla, bool& mandible, bool& chin);
	void LoadIsSurgeryAdjust(bool& maxilla, bool& mandible, bool& chin);
	void LoadIsSurgeryMove(bool& maxilla, bool& mandible, bool& chin);
	void LoadSurgeryCutItemCount(int& cut_item_cnt);
	void LoadSurgeryCutItemPoints(const int& cut_item_id,
								  std::vector<glm::vec3>& points);
	void LoadSrugeryCutItemMatrix(const int& cut_item_id,
								  glm::mat4& trans, glm::mat4& rot,
								  glm::mat4& scale, glm::mat4& arcball,
								  glm::mat4& reori);
	void LoadLandmarks(std::map<QString, glm::vec3>& landmarks);

	// load surgery bar
	void LoadSurgeryBtnStatusText(const int& button_id, std::string& status_text);
	void LoadSurgeryParams(std::vector<float>& params);
	void LoadSurgeryParamsPrev(std::vector<float>& params);
	void LoadIsOutterEdit(bool& edit);

private:
	std::shared_ptr<H5::H5File> file_;
	std::unique_ptr<ProjectIOView> view_io_;
	H5::CompType surgery_cut_flag_type_;
	ViewType curr_view_type_ = ViewType::CEPH;
};

