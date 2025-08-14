#pragma once
/*=========================================================================

File:			class ProjectIO
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-07-11
Last modify:	2016-07-11

=========================================================================*/
#include <H5Cpp.h>
#include <string>
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

namespace project {
namespace io {
const hsize_t kDimMatrix[] = {16};
const hsize_t kDimScalar[] = {1};
const hsize_t kDimPair[] = {2};
const hsize_t kDimVec3[] = {3};
const H5::DataSpace kDSMatrix(1, kDimMatrix);
const H5::DataSpace kDSScalar(1, kDimScalar);
const H5::DataSpace kDSPair(1, kDimPair);
const H5::DataSpace kDSVec3(1, kDimVec3);
}  // namespace io
}  // end of namespace project

class IOFunctions {
 public:
  IOFunctions() {}
  ~IOFunctions() {}

  IOFunctions(const IOFunctions&) = delete;
  IOFunctions& operator=(const IOFunctions&) = delete;

  static void WriteString(H5::Group& group, const std::string& dataset_name,
                          const std::string& string);
  static void WriteMatrix(H5::Group& group, const std::string& dataset_name,
                          const glm::mat4& mat);
  static void WriteVec2(H5::Group& group, const std::string& dataset_name, const glm::vec2& vec);
  static void WriteVec3(H5::Group& group, const std::string& dataset_name,
                        const glm::vec3& vec);
  static void WriteVec3List(H5::Group& group, const std::string& dataset_name,
                            const std::vector<glm::vec3>& vec);
  static void WriteVec2List(H5::Group& group, const std::string& dataset_name,
                            const std::vector<glm::vec2>& vec);
  static void WriteFloat(H5::Group& group, const std::string& dataset_name,
                         const float data);
  static void WriteDouble(H5::Group& group, const std::string& dataset_name,
                          const double data);
  static void WriteInt(H5::Group& group, const std::string& dataset_name,
                       const int data);
  static void WriteUInt(H5::Group& group, const std::string& dataset_name,
                        const unsigned int data);
  static void WriteBool(H5::Group& group, const std::string& dataset_name,
                        const bool data);

  static std::string ReadString(H5::Group& group,
                                const std::string& dataset_name);
  static glm::mat4 ReadMatrix(H5::Group& group,
                              const std::string& dataset_name);
  static glm::vec2 ReadVec2(H5::Group& group, const std::string& dataset_name);
  static glm::vec3 ReadVec3(H5::Group& group, const std::string& dataset_name);
  static std::vector<glm::vec3> ReadVec3List(H5::Group& group,
                                             const std::string& dataset_name);
  static std::vector<glm::vec2> ReadVec2List(H5::Group& group,
                                             const std::string& dataset_name);
  static float ReadFloat(H5::Group& group, const std::string& dataset_name);
  static double ReadDouble(H5::Group& group, const std::string& dataset_name);
  static int ReadInt(H5::Group& group, const std::string& dataset_name);
  static unsigned int ReadUInt(H5::Group& group,
                               const std::string& dataset_name);
  static bool ReadBool(H5::Group& group, const std::string& dataset_name);

  static bool ReadString(H5::Group& group, const std::string& dataset_name, std::string& buffer);
  static bool ReadMatrix(H5::Group& group, const std::string& dataset_name, glm::mat4& buffer);
  static bool ReadVec2(H5::Group& group, const std::string& dataset_name, glm::vec2& buffer);
  static bool ReadVec3(H5::Group& group, const std::string& dataset_name, glm::vec3& buffer);
  static bool ReadVec3List(H5::Group& group, const std::string& dataset_name, std::vector<glm::vec3>& buffer);
  static bool ReadVec2List(H5::Group& group, const std::string& dataset_name, std::vector<glm::vec2>& buffer);
  static bool ReadFloat(H5::Group& group, const std::string& dataset_name, float& buffer);
  static bool ReadDouble(H5::Group& group, const std::string& dataset_name, double& buffer);
  static bool ReadInt(H5::Group& group, const std::string& dataset_name, int& buffer);
  static bool ReadUInt(H5::Group& group, const std::string& dataset_name, unsigned int& buffer);
  static bool ReadBool(H5::Group& group, const std::string& dataset_name, bool& buffer);
};
