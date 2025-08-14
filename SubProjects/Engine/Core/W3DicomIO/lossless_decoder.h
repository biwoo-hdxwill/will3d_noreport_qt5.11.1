#pragma once
/*=========================================================================

File:			lossless_decoder.h
Language:		C++11
Library:		Qt 5.8.0, Standard C++ Library
Author:			
First Date:		2020-01-08
Modify Date:	2020-01-13

Copyright (c) 2020 All rights reserved by HDXWILL.

=========================================================================*/

#include <QObject>

class LosslessDecoder : public QObject
{
	Q_OBJECT

public:
	LosslessDecoder(QObject *parent = nullptr);
	~LosslessDecoder();

	bool DecodeGrayscale(const QString& path, uchar *data);
	bool Decode(const QString& path, uchar *data);

	struct ScanComponent {
		int acTabSel;
		int dcTabSel;
		int scanCompSel;
	};

	struct ComponentSpec {
		int hSamp;
		int quantTableSel;
		int vSamp;
	};


private:
	int IDCT_P[64] = { 0, 5, 40, 16, 45, 2, 7, 42, 21, 56, 8, 61, 18, 47, 1, 4, 41, 23, 58, 13, 32, 24, 37, 10, 63, 17, 44, 3, 6, 43, 20,
		57, 15, 34, 29, 48, 53, 26, 39, 9, 60, 19, 46, 22, 59, 12, 33, 31, 50, 55, 25, 36, 11, 62, 14, 35, 28, 49, 52, 27, 38, 30, 51, 54 };
	int TABLE[64] = { 0, 1, 5, 6, 14, 15, 27, 28, 2, 4, 7, 13, 16, 26, 29, 42, 3, 8, 12, 17, 25, 30, 41, 43, 9, 11, 18, 24, 31, 40, 44, 53,
		10, 19, 23, 32, 39, 45, 52, 54, 20, 22, 33, 38, 46, 51, 55, 60, 21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48, 49, 57, 58, 62, 63 };

	int HuffTab[4][2][50 * 256] = { 0 };
	int IDCT_Source[64] = { 0 };
	int nBlock[10] = { 0 };
	ScanComponent *scanComps = nullptr;

	bool restarting;
	int dataBufferIndex;
	int marker;
	int markerIndex = 0;
	int numComp;
	int restartInterval;
	int selection;
	int xDim, yDim;
	int xLoc;
	int yLoc;
	int mask;
	int* outputData = nullptr;
	int* outputRedData = nullptr;
	int* outputGreenData = nullptr;
	int* outputBlueData = nullptr;

	int RESTART_MARKER_BEGIN = 0xFFD0;
	int RESTART_MARKER_END = 0xFFD7;
	int MAX_HUFFMAN_SUBTREE = 50;
	int MSB = 0x80000000;

	int precision;
	ComponentSpec* components = nullptr;
	int temp_value = 0;
	int temp_index = 0;
	int temp_pred = 0;

	int rgb_pred[3] = { 0 };

private:
	bool ReadHuffmanTable(QDataStream& data);
	bool ReadFrame(QDataStream& data);
	void WriteData(const int pred);
	void WriteRGBData();
	int DecodeSingle(QDataStream& data);
	int DecodeRGB(QDataStream& data);
	int GetHuffmanValue(QDataStream& data);
	int GetRGBHuffmanValue(QDataStream& data, int ctrC, int type);
	int GetN(QDataStream& data, int n);
	int ReadApp(QDataStream& data);
	int ScanHeader(QDataStream& data);
};
