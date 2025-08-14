#pragma once
#include <memory>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif
#include <QObject>

#include "uicomponent_global.h"

class MeshMove3d;
class DisplacementSurface2;
class DisplacementField;
class CW3TRDsurface;
class CW3ResourceContainer;
class CW3JobMgr;
class CW3VREngine;
class FacePhotoResource;
#ifndef WILL3D_VIEWER
class ProjectIOVTOSTO;
#endif

class UICOMPONENT_EXPORT CW3VTOSTO : public QObject {
	Q_OBJECT
public: /* members */
	/* engine */
	CW3VREngine* m_pgVREngine;
	CW3JobMgr* m_pgJobMgr;
	CW3ResourceContainer* m_pgRContainer;
	CW3TRDsurface* m_pFacePhoto3D = nullptr;
	/* data */
	float m_isoValue = -1;
	//std::vector<glm::vec3> m_facePoints;
	//std::vector<glm::vec3> m_facePointsAfter;
	//std::vector<glm::vec2> m_tex2dCoordsResult;
	//std::vector<std::vector<int>> m_faceIndices;

	std::vector<glm::vec3> m_headPoints;
	std::vector<std::vector<int>> m_headTriangles; // internal vector<int> 는 triangle index 3개
	std::vector<glm::vec3> m_modelPoints;
	std::vector<std::vector<int>> m_modelTetras; // internal vector<int> 는 tetra index 4개
	std::vector<std::vector<int>> m_modelTriangles; // internal vector<int> 는 triangle index 3개
	std::shared_ptr<MeshMove3d> m_meshMove;
	std::shared_ptr<DisplacementSurface2> m_dispSurf;
	std::shared_ptr<DisplacementField> m_dispField;
	std::vector<glm::vec3> m_tetraMoveResult;
	QString m_photoFilePath = "";
	QString m_trdFilePath = "";
	/* flags */
	struct Flag {
		int setIsoValue = 0;
		int generateHead = 0;
		int makeTetra = 0;
		int makeMeshMove = 0;
		int fixedIsoValueInSurgery = 0;
		int landmark = 0;
		int cutFace = 0;
		int doMapping = 0;
		int calcDisp = 0;
		int makeSurf = 0;
		int makeField = 0;
		int loadTRD = 0;
	};
	Flag flag;

public: /* methods */
	CW3VTOSTO(CW3VREngine* VREngine, CW3JobMgr *JobMgr,
			  CW3ResourceContainer* Rcontainer);
	~CW3VTOSTO();

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOVTOSTO& out);
	void importProject(ProjectIOVTOSTO& in);
#endif

	void reset();
	void saveTRD(const QString &path);
	void saveTRD(const QString &path, std::vector<glm::vec3> &points,
				 std::vector<glm::vec3> &normals, std::vector<glm::vec2> &texCoord,
				 std::vector<unsigned int> &indices, unsigned char *texImage,
				 const int &nsTex, const int &ntTex);
	bool SavePLY(const QString& path);
	bool SavePLY(const QString& path, std::vector<glm::vec3>& points,
		std::vector<glm::vec3>& normals, std::vector<glm::vec2>& tex_coords,
		std::vector<unsigned int>& indices, unsigned char* texture,
		const int& texture_ns, const int& texture_nt);
	bool SaveOBJ(const QString& path);
	bool SaveOBJ(const QString& base_path, std::vector<glm::vec3>& points,
		std::vector<glm::vec3>& normals, std::vector<glm::vec2>& tex_coords,
		std::vector<unsigned int>& indices, unsigned char* texture,
		const int& texture_ns, const int& texture_nt);

signals:
	void sigSetPhoto();
	void sigLoadPhoto(const QString& file);
	void sigChangeFaceAfterSurface();
	void sigSetFixedIsoValue(float);
	void sigPointModelLoadProject(glm::mat4 *);
	void sigUpdateMPRPhotoLoadProject();
	//void sigFaceDisabled(bool);

public slots:
	void slotLoadTRD(const QString& file);
	void slotLoadOnlyTRD(const QString& file);

private slots:
	void slotTransformedPhotoPoints(glm::mat4*);

public:
	CW3TRDsurface* getFacePhoto3D() { return m_pFacePhoto3D; }
	const FacePhotoResource* getResourceFacePhoto() { return m_pResFace.get(); }

	bool setIsoValue(float isoValue);
	bool setFixedIsoValue(float isoValue);
	bool genHeadAndMkTetra();
	bool generateHead();
	bool cutFace(const glm::vec4& plane);
	bool doMappingRunThread(
		//std::vector<glm::vec2>& tex2dCoordsResult,
		const std::vector<glm::vec3>& meshCtrlPoints,
		const std::vector<int>& meshCtrlTriangles,
		const std::vector<glm::vec2>& texCtrlPoints
	);
	bool doMappingFinal(const QImage& photoImage);

	bool makeTetra();
	bool makeMeshMove(const std::vector<int>& jointIdxs, float E = 1.f, float v = 0.47f);
	bool calcDisp(const std::vector<glm::vec3>& jointDispGroups);
	bool makeSurf();
	bool makeField();
	bool executeSurf();
	bool executeField(std::vector<glm::vec3>& queryPoints);
	//std::vector<glm::vec3> executeField(const std::vector<glm::vec3>& queryPoints);
	void depChainSetIsoValue();
	void depChainFixedIsoValueInSurgery();
	void depChainGenerateHead();
	void depChainMakeTetra();
	void depChainMakeMeshMove();
	void depChainCalcDisp();
	void depChainMakeSurf();
	void depChainMakeField();
	void depChainCutFace();
	void depChainDoMapping();

	const bool isAvailableFace() {
		if (flag.doMapping || flag.loadTRD)
			return true;
		else
			return false;
	}

	inline const bool isLoadTRD() const { return flag.loadTRD; }

	int getFacetexWidth();
	int getFacetexHeight();
	uchar* getFacetexData();

	unsigned int getFaceNindices();
	unsigned int *getFaceVBO();
	unsigned int getFaceTexHandler();

	bool m_bLoadFaceProject = false;

private:
	glm::mat4 m_modelPhotoToSurface;
	std::shared_ptr<FacePhotoResource> m_pResFace;
};
