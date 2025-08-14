#include "project_io_general.h"
#include <H5Cpp.h>

#include "../../Common/Common/W3Define.h"

#include "io_functions.h"
#include "project_path_info.h"

using namespace H5;
using namespace project;

ProjectIOGeneral::ProjectIOGeneral(const project::Purpose& purpose,
								   const std::shared_ptr<H5::H5File>& file) :
	file_(file) {
	if (purpose == project::Purpose::SAVE) {
		DataSet dataset = file_->createDataSet(ds::kVersionInfo, PredType::NATIVE_INT,
											   project::io::kDSScalar);
		dataset.write(&common::kVersionCurrent, PredType::NATIVE_INT);
		dataset.close();
	}
}

ProjectIOGeneral::~ProjectIOGeneral() {}
