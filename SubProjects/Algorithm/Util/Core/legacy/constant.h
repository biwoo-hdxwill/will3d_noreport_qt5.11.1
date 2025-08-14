#pragma once
#pragma message("# Util/Core/constant.h visited")
//#define CORP_HOME 0	//	0: corp	// now automatically done by visual studio configuration setup
						//	1: home	// now automatically done by visual studio configuration setup
//#define SOLUTION_DIR "C:\\Users\\HDXWILL\\Dropbox\\workspace\\cpp_workspace2\\space_qt_library_and_poisson_recon_corp\\SurfaceLibrary_~151125_work_backup_corp\\"
#define SOLUTION_DIR						"..\\"
#if CORP_HOME == 0
#	define LIB_DIR							"C:\\Users\\hosan\\Dropbox\\[1]corp_project\\cpp_workspace\\q2013\\SurfaceLibrary_151211~_inte\\"
#	define MATLAB_INPUT_PATH				"C:\\Users\\hosan\\Documents\\MATLAB\\input"
#	define MATLAB_SRC_PATH					"C:\\Users\\hosan\\Dropbox\\MATLAB_151204~\\3dmeshmove\\version3_5"
#	define RAW_PATH							"C:\\rawData"
#	define RENDERING						1
#elif CORP_HOME == 1
#	define LIB_DIR							"C:\\Users\\hosan\\Dropbox\\[1]corp_project\\cpp_workspace\\q2013\\SurfaceLibrary_151211~_inte\\"
#	define MATLAB_INPUT_PATH				"C:\\Users\\hosan\\Documents\\MATLAB\\input"
#	define MATLAB_SRC_PATH					"C:\\Users\\hosan\\Dropbox\\MATLAB_151204~\\3dmeshmove\\version3_5"
#	define RAW_PATH							"C:\\rawData"
#	define RENDERING						0
#endif

#define INPUT_PATH			SOLUTION_DIR	"Input"
#define TEXTURE_PATH		LIB_DIR			"Resources\\Texture"
#define SHADER_PATH			LIB_DIR			"Resources\\Shader2"
#define MESH_PATH			LIB_DIR			"Resources\\Mesh"
#define SAVE_PATH			SOLUTION_DIR	"Output\\Save"
#define OUTPUT_PATH			SOLUTION_DIR	"Output"
