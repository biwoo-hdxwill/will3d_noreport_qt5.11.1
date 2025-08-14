#include "io_functions.h"

#include <H5Cpp.h>
using namespace H5;

void IOFunctions::WriteString(H5::Group& group, const std::string& dataset_name,
	const std::string& string)
{
	if (string.empty()) return;

	StrType type(PredType::C_S1, string.length());
	DataSet dataset =
		group.createDataSet(dataset_name, type, project::io::kDSScalar);
	dataset.write(string.c_str(), type);
}

void IOFunctions::WriteMatrix(H5::Group& group, const std::string& dataset_name,
	const glm::mat4& mat)
{
	DataSet dataset = group.createDataSet(dataset_name, PredType::NATIVE_FLOAT,
		project::io::kDSMatrix);
	dataset.write(&(mat[0][0]), PredType::NATIVE_FLOAT);
}

void IOFunctions::WriteVec2(H5::Group& group, const std::string& dataset_name, const glm::vec2& vec)
{
	DataSet dataset = group.createDataSet(dataset_name, PredType::NATIVE_FLOAT, project::io::kDSPair);
	dataset.write(&(vec[0]), PredType::NATIVE_FLOAT);
}

void IOFunctions::WriteVec3(H5::Group& group, const std::string& dataset_name,
	const glm::vec3& vec)
{
	DataSet dataset = group.createDataSet(dataset_name, PredType::NATIVE_FLOAT,
		project::io::kDSVec3);
	dataset.write(&(vec[0]), PredType::NATIVE_FLOAT);
}

void IOFunctions::WriteVec3List(H5::Group& group,
	const std::string& dataset_name,
	const std::vector<glm::vec3>& vec3_list)
{
	float* data = new float[vec3_list.size() * 3];
	for (int idx = 0; idx < vec3_list.size(); ++idx)
	{
		const glm::vec3& point = vec3_list[idx];
		data[idx * 3] = point.x;
		data[idx * 3 + 1] = point.y;
		data[idx * 3 + 2] = point.z;
	}
	hsize_t dim[] = { (hsize_t)vec3_list.size() * 3 };
	DataSpace space(1, dim);
	DataSet dataset =
		group.createDataSet(dataset_name, PredType::NATIVE_FLOAT, space);
	dataset.write(data, PredType::NATIVE_FLOAT);
	delete[] data;
	dataset.close();
}

void IOFunctions::WriteVec2List(H5::Group& group,
	const std::string& dataset_name,
	const std::vector<glm::vec2>& vec2_list)
{
	float* data = new float[vec2_list.size() * 2];
	for (int idx = 0; idx < vec2_list.size(); ++idx)
	{
		const glm::vec2& point = vec2_list[idx];
		data[idx * 2] = point.x;
		data[idx * 2 + 1] = point.y;
	}
	hsize_t dim[] = { (hsize_t)vec2_list.size() * 2 };
	DataSpace space(1, dim);
	DataSet dataset =
		group.createDataSet(dataset_name, PredType::NATIVE_FLOAT, space);
	dataset.write(data, PredType::NATIVE_FLOAT);
	delete[] data;
	dataset.close();
}

void IOFunctions::WriteFloat(H5::Group& group, const std::string& dataset_name,
	const float data)
{
	DataSet dataset = group.createDataSet(dataset_name, PredType::NATIVE_FLOAT,
		project::io::kDSScalar);
	dataset.write(&data, PredType::NATIVE_FLOAT);
	dataset.close();
}

void IOFunctions::WriteDouble(H5::Group& group, const std::string& dataset_name,
	const double data)
{
	DataSet dataset = group.createDataSet(dataset_name, PredType::NATIVE_DOUBLE,
		project::io::kDSScalar);
	dataset.write(&data, PredType::NATIVE_DOUBLE);
	dataset.close();
}

void IOFunctions::WriteInt(H5::Group& group, const std::string& dataset_name,
	const int data)
{
	DataSet dataset = group.createDataSet(dataset_name, PredType::NATIVE_INT,
		project::io::kDSScalar);
	dataset.write(&data, PredType::NATIVE_INT);
	dataset.close();
}

void IOFunctions::WriteUInt(H5::Group& group, const std::string& dataset_name,
	const unsigned int data)
{
	DataSet dataset = group.createDataSet(dataset_name, PredType::NATIVE_UINT,
		project::io::kDSScalar);
	dataset.write(&data, PredType::NATIVE_UINT);
	dataset.close();
}

void IOFunctions::WriteBool(H5::Group& group, const std::string& dataset_name,
	const bool data)
{
	DataSet dataset = group.createDataSet(dataset_name, PredType::NATIVE_HBOOL,
		project::io::kDSScalar);
	dataset.write(&data, PredType::NATIVE_HBOOL);
	dataset.close();
}

std::string IOFunctions::ReadString(H5::Group& group,
	const std::string& dataset_name)
{
	if (!group.exists(dataset_name)) return std::string();

	DataSet datset = group.openDataSet(dataset_name);
	DataType dtype = datset.getDataType();
	int dataset_size = dtype.getSize();
	char* buffer = new char[dataset_size + 1];
	buffer[dataset_size] = 0;
	datset.read(buffer, dtype);

	std::string result = std::string(buffer);
	delete[] buffer;
	return result;
}

glm::mat4 IOFunctions::ReadMatrix(H5::Group& group,
	const std::string& dataset_name)
{
	if (!group.exists(dataset_name)) return glm::mat4();

	DataSet datset = group.openDataSet(dataset_name);
	DataType dtype = datset.getDataType();
	float buffer[16] = { 0.0f };
	datset.read(buffer, dtype);

	glm::mat4 result;
	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			result[y][x] = buffer[4 * y + x];
		}
	}
	return result;
}

glm::vec2 IOFunctions::ReadVec2(H5::Group& group, const std::string& dataset_name)
{
	if (!group.exists(dataset_name))
	{
		return glm::vec2();
	}

	DataSet data_set = group.openDataSet(dataset_name);
	DataType dtype = data_set.getDataType();
	float buffer[2] = { 0.0f };
	data_set.read(buffer, dtype);
	return glm::vec2(buffer[0], buffer[1]);
}

glm::vec3 IOFunctions::ReadVec3(H5::Group& group,
	const std::string& dataset_name)
{
	if (!group.exists(dataset_name)) return glm::vec3();

	DataSet datset = group.openDataSet(dataset_name);
	DataType dtype = datset.getDataType();
	float buffer[3] = { 0.0f };
	datset.read(buffer, dtype);
	return glm::vec3(buffer[0], buffer[1], buffer[2]);
}

std::vector<glm::vec3> IOFunctions::ReadVec3List(
	H5::Group& group, const std::string& dataset_name)
{
	if (!group.exists(dataset_name)) return std::vector<glm::vec3>();

	DataSet dataset = group.openDataSet(dataset_name);
	DataSpace space = dataset.getSpace();
	hsize_t dims[1];
	space.getSimpleExtentDims(dims);  // 되는지 확인해봐야 함
	int sz = dims[0];

	FloatType f_type = dataset.getFloatType();
	float* data = new float[sz];
	dataset.read(data, PredType::NATIVE_FLOAT);

	std::vector<glm::vec3> result;
	result.reserve(sz / 3);

	for (int idx = 0; idx < sz / 3; ++idx)
	{
		result.push_back(
			glm::vec3(data[idx * 3], data[idx * 3 + 1], data[idx * 3 + 2]));
	}
	delete[] data;
	dataset.close();

	return result;
}

std::vector<glm::vec2> IOFunctions::ReadVec2List(
	H5::Group& group, const std::string& dataset_name)
{
	if (!group.exists(dataset_name)) return std::vector<glm::vec2>();

	DataSet dataset = group.openDataSet(dataset_name);
	DataSpace space = dataset.getSpace();
	hsize_t dims[1];
	space.getSimpleExtentDims(dims);  // 되는지 확인해봐야 함
	int sz = dims[0];

	float* data = new float[sz];
	dataset.read(data, PredType::NATIVE_FLOAT);

	std::vector<glm::vec2> result;
	result.reserve(sz / 2);

	for (int idx = 0; idx < sz / 2; ++idx)
	{
		result.push_back(glm::vec2(data[idx * 2], data[idx * 2 + 1]));
	}
	delete[] data;
	dataset.close();

	return result;
}

float IOFunctions::ReadFloat(H5::Group& group,
	const std::string& dataset_name)
{
	if (!group.exists(dataset_name)) return 0.0f;

	DataSet dataset = group.openDataSet(dataset_name);
	float data = 0.0f;
	dataset.read(&data, PredType::NATIVE_FLOAT);
	dataset.close();

	return data;
}

double IOFunctions::ReadDouble(H5::Group& group,
	const std::string& dataset_name)
{
	if (!group.exists(dataset_name)) return 0.0;

	DataSet dataset = group.openDataSet(dataset_name);
	double data = 0.0f;
	dataset.read(&data, PredType::NATIVE_DOUBLE);
	dataset.close();

	return data;
}

int IOFunctions::ReadInt(H5::Group& group, const std::string& dataset_name)
{
	if (!group.exists(dataset_name)) return 0;

	DataSet dataset = group.openDataSet(dataset_name);
	int data = 0;
	dataset.read(&data, PredType::NATIVE_INT);
	dataset.close();

	return data;
}

unsigned int IOFunctions::ReadUInt(H5::Group& group,
	const std::string& dataset_name)
{
	if (!group.exists(dataset_name)) return 0;

	DataSet dataset = group.openDataSet(dataset_name);
	unsigned int data = 0;
	dataset.read(&data, PredType::NATIVE_UINT);
	dataset.close();

	return data;
}

bool IOFunctions::ReadBool(H5::Group& group, const std::string& dataset_name)
{
	if (!group.exists(dataset_name)) return 0;

	DataSet dataset = group.openDataSet(dataset_name);
	bool data = 0;
	dataset.read(&data, PredType::NATIVE_HBOOL);
	dataset.close();

	return data;
}

bool IOFunctions::ReadString(H5::Group& group, const std::string& dataset_name, std::string& buffer)
{
	if (!group.exists(dataset_name))
	{
		return false;
	}

	DataSet datset = group.openDataSet(dataset_name);
	DataType dtype = datset.getDataType();
	int dataset_size = dtype.getSize();
	char* data = new char[dataset_size + 1];
	data[dataset_size] = 0;
	datset.read(data, dtype);

	buffer = std::string(data);
	delete[] data;

	return true;
}

bool IOFunctions::ReadMatrix(H5::Group& group, const std::string& dataset_name, glm::mat4& buffer)
{
	if (!group.exists(dataset_name))
	{
		return false;
	}

	DataSet datset = group.openDataSet(dataset_name);
	DataType dtype = datset.getDataType();
	float temp_buffer[16] = { 0.0f };
	datset.read(temp_buffer, dtype);

	glm::mat4 result;
	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			result[y][x] = temp_buffer[4 * y + x];
		}
	}

	return true;
}

bool IOFunctions::ReadVec2(H5::Group& group, const std::string& dataset_name, glm::vec2& buffer)
{
	if (!group.exists(dataset_name))
	{
		return false;
	}

	DataSet data_set = group.openDataSet(dataset_name);
	DataType dtype = data_set.getDataType();
	float temp_buffer[2] = { 0.0f };
	data_set.read(temp_buffer, dtype);

	buffer = glm::vec2(temp_buffer[0], temp_buffer[1]);

	return true;
}

bool IOFunctions::ReadVec3(H5::Group& group, const std::string& dataset_name, glm::vec3& buffer)
{
	if (!group.exists(dataset_name))
	{
		return false;
	}

	DataSet datset = group.openDataSet(dataset_name);
	DataType dtype = datset.getDataType();
	float temp_buffer[3] = { 0.0f };
	datset.read(temp_buffer, dtype);

	buffer = glm::vec3(temp_buffer[0], temp_buffer[1], temp_buffer[2]);

	return true;
}

bool IOFunctions::ReadVec3List(H5::Group& group, const std::string& dataset_name, std::vector<glm::vec3>& buffer)
{
	if (!group.exists(dataset_name))
	{
		return false;
	}

	DataSet dataset = group.openDataSet(dataset_name);
	DataSpace space = dataset.getSpace();
	hsize_t dims[1];
	space.getSimpleExtentDims(dims);  // 되는지 확인해봐야 함
	int sz = dims[0];

	FloatType f_type = dataset.getFloatType();
	float* data = new float[sz];
	dataset.read(data, PredType::NATIVE_FLOAT);

	buffer.reserve(sz / 3);

	for (int idx = 0; idx < sz / 3; ++idx)
	{
		buffer.push_back(glm::vec3(data[idx * 3], data[idx * 3 + 1], data[idx * 3 + 2]));
	}
	delete[] data;
	dataset.close();

	return true;
}

bool IOFunctions::ReadVec2List(H5::Group& group, const std::string& dataset_name, std::vector<glm::vec2>& buffer)
{
	if (!group.exists(dataset_name))
	{
		return false;
	}

	DataSet dataset = group.openDataSet(dataset_name);
	DataSpace space = dataset.getSpace();
	hsize_t dims[1];
	space.getSimpleExtentDims(dims);  // 되는지 확인해봐야 함
	int sz = dims[0];

	float* data = new float[sz];
	dataset.read(data, PredType::NATIVE_FLOAT);

	buffer.reserve(sz / 2);

	for (int idx = 0; idx < sz / 2; ++idx)
	{
		buffer.push_back(glm::vec2(data[idx * 2], data[idx * 2 + 1]));
	}
	delete[] data;
	dataset.close();

	return true;
}

bool IOFunctions::ReadFloat(H5::Group& group, const std::string& dataset_name, float& buffer)
{
	if (!group.exists(dataset_name))
	{
		return false;
	}

	DataSet dataset = group.openDataSet(dataset_name);
	float data = 0.0f;
	dataset.read(&data, PredType::NATIVE_FLOAT);
	buffer = data;
	dataset.close();

	return true;
}

bool IOFunctions::ReadDouble(H5::Group& group, const std::string& dataset_name, double& buffer)
{
	if (!group.exists(dataset_name))
	{
		return false;
	}

	DataSet dataset = group.openDataSet(dataset_name);
	double data = 0.0;
	dataset.read(&data, PredType::NATIVE_DOUBLE);
	buffer = data;
	dataset.close();

	return true;
}

bool IOFunctions::ReadInt(H5::Group& group, const std::string& dataset_name, int& buffer)
{
	if (!group.exists(dataset_name))
	{
		return false;
	}

	DataSet dataset = group.openDataSet(dataset_name);
	int data = 0;
	dataset.read(&data, PredType::NATIVE_INT);
	buffer = data;
	dataset.close();

	return true;
}

bool IOFunctions::ReadUInt(H5::Group& group, const std::string& dataset_name, unsigned int& buffer)
{
	if (!group.exists(dataset_name))
	{
		return false;
	}

	DataSet dataset = group.openDataSet(dataset_name);
	unsigned int data = 0;
	dataset.read(&data, PredType::NATIVE_UINT);
	buffer = data;
	dataset.close();

	return true;
}

bool IOFunctions::ReadBool(H5::Group& group, const std::string& dataset_name, bool& buffer)
{
	if (!group.exists(dataset_name))
	{
		return false;
	}

	DataSet dataset = group.openDataSet(dataset_name);
	bool data = 0;
	dataset.read(&data, PredType::NATIVE_HBOOL);
	buffer = data;
	dataset.close();

	return true;
}
