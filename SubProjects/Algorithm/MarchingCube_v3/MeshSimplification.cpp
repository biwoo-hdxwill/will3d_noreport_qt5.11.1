#include "MeshSimplification.h"
#include <algorithm>
#include <exception>
#include <iostream>
#include <map>
#include <numeric>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <Util/PointProcessing/VertexToTriIdxMapper.h>
#include <Util/PointProcessing/EdgeToTriIdxMapper.h>
#include <Util/Core/Logger.hpp>
#include <Util/MeshGenerator/MeshCompacter.h>

using namespace std;

bool MeshSimplification::findConnectedComponents(std::vector<std::vector<int>>& connectedComponents,
												 const std::vector<Edge>& edges) {
	using namespace boost;
	typedef adjacency_list <vecS, vecS, undirectedS> Graph;
	typedef Graph::vertex_descriptor vertex_t;
	typedef Graph::edge_descriptor edge_t;
	typedef Graph::vertex_iterator vertex_it;
	typedef Graph::edge_iterator edge_it;
	try {
		/* find unique vertex set (v2b, b2v) */
		map<int, vertex_t> v2b;
		map<vertex_t, int> b2v;
		int cur = 0;
		for (const auto& e : edges) {
			if (v2b.find(e.first) == v2b.end()) { // e.first 가 아직 없으면 v2b
				v2b[e.first] = cur;
				b2v[cur] = e.first;
				cur++;
			}
			if (v2b.find(e.second) == v2b.end()) { // e.second 가 아직 없으면 v2b
				v2b[e.second] = cur;
				b2v[cur] = e.second;
				cur++;
			}
		}
		/* make bgl graph */
		Graph g;
		for (int i = 0; i < cur; i++) {
			add_vertex(g);
		}
		for (const auto& e : edges) {
			add_edge(v2b[e.first], v2b[e.second], g);
		}
		/* find connected components tag */
		int n = num_vertices(g);
		vector<int> cctag(n);
		int ncc = connected_components(g, cctag.data());
		connectedComponents.resize(ncc);
		for (int i = 0; i < n; i++) {
			connectedComponents[cctag[i]].push_back(b2v[i]);
		}

		return true;
	} catch (std::runtime_error& e) {
		lg << "MeshSimplification::findConnectedComponents: " << e << endl;
		return false;
	}
}

bool MeshSimplification::findConnectedComponents(std::vector<std::vector<int>>& connectedComponents, const std::vector<tora::Edge>& edges) {
	using namespace boost;
	typedef adjacency_list <vecS, vecS, undirectedS> Graph;
	typedef Graph::vertex_descriptor vertex_t;
	typedef Graph::edge_descriptor edge_t;
	typedef Graph::vertex_iterator vertex_it;
	typedef Graph::edge_iterator edge_it;
	try {
		/* find unique vertex set (v2b, b2v) */
		map<int, vertex_t> v2b;
		map<vertex_t, int> b2v;
		int cur = 0;
		for (const auto& e : edges) {
			if (v2b.find(e.v0) == v2b.end()) { // e.first 가 아직 없으면 v2b
				v2b[e.v0] = cur;
				b2v[cur] = e.v0;
				cur++;
			}
			if (v2b.find(e.v1) == v2b.end()) { // e.second 가 아직 없으면 v2b
				v2b[e.v1] = cur;
				b2v[cur] = e.v1;
				cur++;
			}
		}
		/* make bgl graph */
		Graph g;
		for (int i = 0; i < cur; i++) {
			add_vertex(g);
		}
		for (const auto& e : edges) {
			add_edge(v2b[e.v0], v2b[e.v1], g);
		}
		/* find connected components tag */
		int n = num_vertices(g);
		vector<int> cctag(n);
		int ncc = connected_components(g, cctag.data());
		connectedComponents.resize(ncc);
		for (int i = 0; i < n; i++) {
			connectedComponents[cctag[i]].push_back(b2v[i]);
		}

		return true;
	} catch (std::runtime_error& e) {
		lg << "MeshSimplification::findConnectedComponents: " << e << endl;
		return false;
	}
}

bool MeshSimplification::execute(vector<glm::vec3>& points, vector<vector<int>>& triangles, float arTol, float areaTol) {
	try {
		int nTri = triangles.size();
		int nPoint = points.size();
		vector<glm::vec3> pointsOut(nPoint);
		vector<vector<int>> trianglesOut(nTri);
		VertexToTriIdxMapper vtm(triangles);
		EdgeToTriIdxMapper etm(triangles);
		vector<Edge> edges;

		vector<char> triCpyFlag(nTri, 1);
		for (const auto& tri : triangles) {
			/* inspect aspect ratio and area */
			int vid0 = tri[0];
			int vid1 = tri[1];
			int vid2 = tri[2];
			const glm::vec3& p0 = points[vid0];
			const glm::vec3& p1 = points[vid1];
			const glm::vec3& p2 = points[vid2];
			float a = glm::length(p1 - p2);
			float b = glm::length(p0 - p2);
			float c = glm::length(p0 - p1);
			float s = (a + b + c) / 2;
			float area = sqrt(s*(s - a)*(s - b)*(s - c));
			float AR = (a + b - c)*(b + c - a)*(c + a - b) / (a*b*c); // Aspect Ratio: (degenerated) 0 <= 2r/R <= 1 (good)
			if (area < areaTol) {
				/*	tricpy[1], tricpy[0]
					tricpy[2], tricpy[0]
				*/
				edges.push_back(Edge(tri[1], tri[0]));
				edges.push_back(Edge(tri[2], tri[0]));
				/* 이 edge를 포함하는 삼각형들 삭제 */
				for (auto t : etm.findTriIdxByEdge(tora::Edge(tri[0], tri[1]))) {
					triCpyFlag[t] = 0;
				}
				for (auto t : etm.findTriIdxByEdge(tora::Edge(tri[1], tri[2]))) {
					triCpyFlag[t] = 0;
				}
				for (auto t : etm.findTriIdxByEdge(tora::Edge(tri[2], tri[0]))) {
					triCpyFlag[t] = 0;
				}
			} else if (AR < arTol) { // AR이 inf였으면 area = 0이라서 < areaTol 에서 걸림.
				if (a < b && a < c) { // min = a
					edges.push_back(Edge(vid1, vid2));
					for (auto t : etm.findTriIdxByEdge(tora::Edge(vid1, vid2))) {
						triCpyFlag[t] = 0;
					}
				} else if (b < a && b < c) { // min = b
					edges.push_back(Edge(vid0, vid2));
					for (auto t : etm.findTriIdxByEdge(tora::Edge(vid0, vid2))) {
						triCpyFlag[t] = 0;
					}
				} else { // min = c
					edges.push_back(Edge(vid0, vid1));
					for (auto t : etm.findTriIdxByEdge(tora::Edge(vid0, vid1))) {
						triCpyFlag[t] = 0;
					}
				}
			}
		}
		/* find connected components */
		vector<vector<int>> cc;
		findConnectedComponents(cc, edges);
		/* map1, triCpyFlag, pointCpyFlag */
		vector<char> pointCpyFlag(nPoint, 1);
		vector<int> map1(nPoint); // 하나로 모일 노드들 계산
		for (int i = 0; i < nPoint; i++) {
			map1[i] = i;
		}
		for (const auto& c : cc) {
			for (int i = 1; i < c.size(); i++) {
				map1[c[i]] = c[0];
				pointCpyFlag[c[i]] = 0;
			}
			glm::vec3 center(0, 0, 0);
			for (auto vid : c) {
				center += points[vid];
			}
			center /= c.size();
			points[c[0]] = center;
		}
		/* map2, ponits 복사 */
		int cntPoint = 0;
		vector<int> map2(nPoint, -1);
		for (int i = 0; i < nPoint; i++) {
			if (pointCpyFlag[i]) {
				map2[i] = cntPoint;
				pointsOut[cntPoint] = points[i];
				cntPoint++;
			}
		}
		pointsOut.resize(cntPoint);
		/* mapAll */
		vector<int> mapAll(nPoint);
		for (int i = 0; i < nPoint; i++) {
			mapAll[i] = map2[map1[i]];
		}
		/* triCpy */
		int cntTri = 0;
		for (int i = 0; i < nTri; i++) {
			if (triCpyFlag[i]) {
				auto tri = triangles[i];
				tri[0] = mapAll[tri[0]];
				tri[1] = mapAll[tri[1]];
				tri[2] = mapAll[tri[2]];
				if (!(tri[0] == tri[1] || tri[1] == tri[2] || tri[2] == tri[0])) {
					trianglesOut[cntTri] = tri;
					cntTri++;
				}
			}
		}
		trianglesOut.resize(cntTri);
		/* return */
		points = pointsOut;
		triangles = trianglesOut;

		return true;
	} catch (runtime_error& e) {
		lg << "MeshSimplification::execute: " << e << endl;
		return false;
	}
}
bool MeshSimplification::executeIsolatedComponentSelectMax(std::vector<glm::vec3>& points, std::vector<std::vector<int>>& triangles) {
	try {
		int nTri = triangles.size();
		int nPoint = points.size();
		vector<glm::vec3> pointsOut(nPoint);
		vector<vector<int>> trianglesOut(nTri);
		VertexToTriIdxMapper vtm(triangles);
		EdgeToTriIdxMapper etm(triangles);
		vector<tora::Edge> edges = etm.getEdges();

		/* find connected components */
		vector<vector<int>> cc;
		findConnectedComponents(cc, edges);

		vector<char> usedPoints(nPoint, 0);
		if (cc.empty()) {
			throw std::runtime_error("there is no connected component. is it empty mesh?");
		}

		int maxSize = 0;
		vector<int> maxComponent;
		for (const auto& c : cc) {
			int cSize = c.size();
			if (cSize > maxSize) {
				maxSize = cSize;
				maxComponent = c;
			}
		}

		for (int i : maxComponent) {
			usedPoints[i] = 1;
		}

		//cout << "		executeIsolatedComponentSelectMax : maxSize = " << maxSize  << endl;

		std::function<int(int)> l_remap;
		MeshCompacter::executeRemoveMeshByVertexCriteria(points, triangles, l_remap, [&usedPoints](int i)->bool {
			return !usedPoints[i]; // not used -> remove vertex
		});

		return true;
	} catch (std::runtime_error& e) {
		lg << "MeshSimplification::execute: " << e << endl;
		return false;
	}
}
bool MeshSimplification::executeIsolatedComponent(std::vector<glm::vec3>& points, std::vector<std::vector<int>>& triangles, int threshold) {
	try {
		int nTri = triangles.size();
		int nPoint = points.size();
		vector<glm::vec3> pointsOut(nPoint);
		vector<vector<int>> trianglesOut(nTri);
		VertexToTriIdxMapper vtm(triangles);
		EdgeToTriIdxMapper etm(triangles);
		vector<tora::Edge> edges = etm.getEdges();

		/* find connected components */
		vector<vector<int>> cc;
		findConnectedComponents(cc, edges);

		vector<char> usedPoints(nPoint, 0);
		if (cc.empty()) {
			throw std::runtime_error("there is no connected component. is it empty mesh?");
		}
		int maxSize = 0;
		for (const auto& c : cc) {
			int cSize = c.size();
			if (cSize > maxSize)
				maxSize = cSize;

			if (cSize > threshold) {
				for (int i : c) {
					usedPoints[i] = 1;
				}
			}
		}

		std::function<int(int)> l_remap;
		MeshCompacter::executeRemoveMeshByVertexCriteria(points, triangles, l_remap, [&usedPoints](int i)->bool {
			return !usedPoints[i]; // not used -> remove vertex
		});

		return true;
	} catch (std::runtime_error& e) {
		lg << "MeshSimplification::execute: " << e << endl;
		return false;
	}
}
bool MeshSimplification::executeIsolatedComponent(std::vector<glm::vec3>& points, std::vector<std::vector<int>>& triangles, std::function<int(int)>& remap, int threshold) {
	try {
		int nTri = triangles.size();
		int nPoint = points.size();
		vector<glm::vec3> pointsOut(nPoint);
		vector<vector<int>> trianglesOut(nTri);
		VertexToTriIdxMapper vtm(triangles);
		EdgeToTriIdxMapper etm(triangles);
		vector<tora::Edge> edges = etm.getEdges();

		/* find connected components */
		vector<vector<int>> cc;
		findConnectedComponents(cc, edges);

		vector<char> usedPoints(nPoint, 0);
		if (cc.empty()) {
			throw std::runtime_error("there is no connected component. is it empty mesh?");
		}
		for (const auto& c : cc) {
			if (c.size() > threshold) {
				for (int i : c) {
					usedPoints[i] = 1;
				}
			}
		}

		MeshCompacter::executeRemoveMeshByVertexCriteria(points, triangles, remap, [&usedPoints](int i)->bool {
			return !usedPoints[i]; // not used -> remove vertex
		});

		return true;
	} catch (std::runtime_error& e) {
		lg << "MeshSimplification::execute: " << e << endl;
		return false;
	}
}
void MeshSimplification::avgAspectRatio(float& avgAR, float& avgAreaAR, const std::vector<glm::vec3>& points, const std::vector<std::vector<int>>& triangles) {
	float sum_AR = 0;
	float sum_AreaAR = 0;
	float sum_area = 0;
	for (const auto& tri : triangles) {
		const glm::vec3& p0 = points[tri[0]];
		const glm::vec3& p1 = points[tri[1]];
		const glm::vec3& p2 = points[tri[2]];
		float a = glm::length(p1 - p2);
		float b = glm::length(p0 - p2);
		float c = glm::length(p0 - p1);
		float s = (a + b + c) / 2;
		float area = sqrt(s*(s - a)*(s - b)*(s - c));
		float AR = (a + b - c)*(b + c - a)*(c + a - b) / (a*b*c);
		sum_AR += AR;
		sum_AreaAR += AR * area;
		sum_area += area;
	}

	avgAR = sum_AR / triangles.size();
	avgAreaAR = sum_AreaAR / sum_area;
}
