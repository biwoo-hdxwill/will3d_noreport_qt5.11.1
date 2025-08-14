#include "lossless_decoder.h"

#include <QFile>
#include <QDataStream>
#include <QBuffer>

LosslessDecoder::LosslessDecoder(QObject *parent) : QObject(parent)
{
}

LosslessDecoder::~LosslessDecoder()
{
	delete[] outputData;
	delete[] outputRedData;
	delete[] outputGreenData;
	delete[] outputBlueData;
	delete[] components;
}

bool LosslessDecoder::DecodeGrayscale(const QString& path, uchar *data)
{
	QFile image_file(path);
	if (!image_file.open(QIODevice::ReadOnly))
	{
		return false;
	}

	QByteArray data_array = image_file.readAll();
	image_file.close();

	QBuffer data_buffer(&data_array);
	data_buffer.open(QBuffer::ReadOnly);
	QDataStream data_stream(&data_buffer);

	unsigned short current = 0;
	int scanNum = 0;
	int *pred = new int[10];

	xLoc = 0;
	yLoc = 0;
	data_stream >> current;

	bool is_jpeg = false;
	while (!data_stream.atEnd())
	{
		data_stream >> current;
		if (current == 0xFFD8)
		{
			is_jpeg = true;
			break;
		}
	}

	if (!is_jpeg)
	{
		// not jpeg
		return false;
	}

	data_stream >> current;

	int debug_value = 0;

	while (((current >> 4) != 0x0FFC) || (current == 0xFFC4))
	{ // SOF 0~15
		switch (current)
		{
		case 0xFFC4: // DHT
			ReadHuffmanTable(data_stream);
			break;
		case 0xFFCC: // DAC
			// Program doesn't support arithmetic coding
			return false;
		case 0xFFDB:
			//quantTable.read(this, TABLE); // WillMaster 압축 파일에는 없는 부분이라 Pass
			break;
		case 0xFFDD:
			//restartInterval = readNumber(); // WillMaster 압축 파일에는 없는 부분이라 Pass
			break;
		case 0xFFE0:
		case 0xFFE1:
		case 0xFFE2:
		case 0xFFE3:
		case 0xFFE4:
		case 0xFFE5:
		case 0xFFE6:
		case 0xFFE7:
		case 0xFFE8:
		case 0xFFE9:
		case 0xFFEA:
		case 0xFFEB:
		case 0xFFEC:
		case 0xFFED:
		case 0xFFEE:
		case 0xFFEF:
			//readApp(); // WillMaster 압축 파일에는 없는 부분이라 Pass
			ReadApp(data_stream);
			break;
		case 0xFFFE:
			//readComment(); // WillMaster 압축 파일에는 없는 부분이라 Pass
			break;
		default:
			if ((current >> 8) != 0xFF)
			{
				// Wrong value
			}
		}

		data_stream >> current;
	}

	if ((current < 0xFFC0) || (current > 0xFFC7))
	{
		// Could not handle arithmetic code
	}

	ReadFrame(data_stream);
	if (numComp > 1)
	{
		return false;
	}

	data_stream >> current;

	do
	{
		while (current != 0x0FFDA)
		{ //SOS
			switch (current)
			{
			case 0xFFC4: //DHT
				ReadHuffmanTable(data_stream);
				break;
			case 0xFFCC: //DAC
				// Program doesn't support arithmetic coding
				return false;
			case 0xFFDB:
				//quantTable.read(this, TABLE); // WillMaster 압축 파일에는 없는 부분이라 Pass
				break;
			case 0xFFDD:
				//restartInterval = readNumber(); // WillMaster 압축 파일에는 없는 부분이라 Pass
				break;
			case 0xFFE0:
			case 0xFFE1:
			case 0xFFE2:
			case 0xFFE3:
			case 0xFFE4:
			case 0xFFE5:
			case 0xFFE6:
			case 0xFFE7:
			case 0xFFE8:
			case 0xFFE9:
			case 0xFFEA:
			case 0xFFEB:
			case 0xFFEC:
			case 0xFFED:
			case 0xFFEE:
			case 0xFFEF:
				//readApp(); // WillMaster 압축 파일에는 없는 부분이라 Pass
				ReadApp(data_stream);
				break;
			case 0xFFFE:
				//readComment(); // WillMaster 압축 파일에는 없는 부분이라 Pass
				break;
			default:
				if ((current >> 8) != 0xFF)
				{
					// Wrong value
				}
			}

			data_stream >> current;
		}

		if (precision == 8)
		{
			mask = 0xFF;
		}
		else
		{
			mask = 0xFFFF;
		}

		ScanHeader(data_stream);

		int compN = 0;
		for (int i = 0; i < numComp; ++i)
		{
			compN = scanComps[i].scanCompSel;
			nBlock[i] = components[compN].vSamp * components[compN].hSamp;
		}

		scanNum++;

		temp_value = 0;
		temp_index = 0;

		outputData = new int[xDim * yDim];
		temp_pred = (1 << (precision - 1));

		current = DecodeSingle(data_stream);

		while ((current == 0) && ((xLoc < xDim) && (yLoc < yDim)))
		{
			WriteData(temp_pred);
			current = DecodeSingle(data_stream);
		}
	} while ((current != 0xFFD9) && ((xLoc < xDim) && (yLoc < yDim)) && (scanNum == 0));

	data_buffer.close();

	int min_int = 65535;
	int max_int = 0;
	for (uint idx = 0; idx < xDim * yDim; ++idx)
	{
		if (min_int > outputData[idx])
		{
			min_int = outputData[idx];
		}
		if (max_int < outputData[idx])
		{
			max_int = outputData[idx];
		}
		data[idx] = static_cast<uchar>(static_cast<double>(outputData[idx]) / mask * 255.0);
	}

	uchar min_uchar = 255;
	uchar max_uchar = 0;
	for (uint idx = 0; idx < xDim * yDim; ++idx)
	{
		if (min_uchar > data[idx])
		{
			min_uchar = data[idx];
		}
		if (max_uchar < data[idx])
		{
			max_uchar = data[idx];
		}
	}

	data_array.clear();
	delete[] outputData;
	outputData = nullptr;

	return true;
}

bool LosslessDecoder::Decode(const QString& path, uchar *data)
{
	QFile image_file(path);
	if (!image_file.open(QIODevice::ReadOnly))
	{
		return false;
	}

	QByteArray data_array = image_file.readAll();
	image_file.close();

	QBuffer data_buffer(&data_array);
	data_buffer.open(QBuffer::ReadOnly);
	QDataStream data_stream(&data_buffer);

	unsigned short current = 0;
	int scanNum = 0;
	int *pred = new int[10];

	xLoc = 0;
	yLoc = 0;
	data_stream >> current;

	bool is_jpeg = false;
	while (!data_stream.atEnd())
	{
		data_stream >> current;
		if (current == 0xFFD8)
		{
			is_jpeg = true;
			break;
		}
	}

	if (!is_jpeg)
	{
		// not jpeg
		return false;
	}

	data_stream >> current;

	int debug_value = 0;

	while (((current >> 4) != 0x0FFC) || (current == 0xFFC4))
	{ // SOF 0~15
		switch (current)
		{
		case 0xFFC4: // DHT
			ReadHuffmanTable(data_stream);
			break;
		case 0xFFCC: // DAC
			// Program doesn't support arithmetic coding
			return false;
		case 0xFFDB:
			//quantTable.read(this, TABLE); // WillMaster 압축 파일에는 없는 부분이라 Pass
			break;
		case 0xFFDD:
			//restartInterval = readNumber(); // WillMaster 압축 파일에는 없는 부분이라 Pass
			break;
		case 0xFFE0:
		case 0xFFE1:
		case 0xFFE2:
		case 0xFFE3:
		case 0xFFE4:
		case 0xFFE5:
		case 0xFFE6:
		case 0xFFE7:
		case 0xFFE8:
		case 0xFFE9:
		case 0xFFEA:
		case 0xFFEB:
		case 0xFFEC:
		case 0xFFED:
		case 0xFFEE:
		case 0xFFEF:
			//readApp(); // WillMaster 압축 파일에는 없는 부분이라 Pass
			ReadApp(data_stream);
			break;
		case 0xFFFE:
			//readComment(); // WillMaster 압축 파일에는 없는 부분이라 Pass
			break;
		default:
			if ((current >> 8) != 0xFF)
			{
				// Wrong value
			}
		}

		data_stream >> current;
	}

	if ((current < 0xFFC0) || (current > 0xFFC7))
	{
		// Could not handle arithmetic code
	}

	ReadFrame(data_stream);
	data_stream >> current;

	do
	{
		while (current != 0x0FFDA)
		{ //SOS
			switch (current)
			{
			case 0xFFC4: //DHT
				ReadHuffmanTable(data_stream);
				break;
			case 0xFFCC: //DAC
				// Program doesn't support arithmetic coding
				return false;
			case 0xFFDB:
				//quantTable.read(this, TABLE); // WillMaster 압축 파일에는 없는 부분이라 Pass
				break;
			case 0xFFDD:
				//restartInterval = readNumber(); // WillMaster 압축 파일에는 없는 부분이라 Pass
				break;
			case 0xFFE0:
			case 0xFFE1:
			case 0xFFE2:
			case 0xFFE3:
			case 0xFFE4:
			case 0xFFE5:
			case 0xFFE6:
			case 0xFFE7:
			case 0xFFE8:
			case 0xFFE9:
			case 0xFFEA:
			case 0xFFEB:
			case 0xFFEC:
			case 0xFFED:
			case 0xFFEE:
			case 0xFFEF:
				//readApp(); // WillMaster 압축 파일에는 없는 부분이라 Pass
				ReadApp(data_stream);
				break;
			case 0xFFFE:
				//readComment(); // WillMaster 압축 파일에는 없는 부분이라 Pass
				break;
			default:
				if ((current >> 8) != 0xFF)
				{
					// Wrong value
				}
			}

			data_stream >> current;
		}

		if (precision == 8)
		{
			mask = 0xFF;
		}
		else
		{
			mask = 0xFFFF;
		}

		ScanHeader(data_stream);

		int compN = 0;
		for (int i = 0; i < numComp; ++i)
		{
			compN = scanComps[i].scanCompSel;
			nBlock[i] = components[compN].vSamp * components[compN].hSamp;
		}

		scanNum++;

		temp_value = 0;
		temp_index = 0;

		if (numComp == 1)
		{
			outputData = new int[xDim * yDim];
			temp_pred = (1 << (precision - 1));

			current = DecodeSingle(data_stream);

			while ((current == 0) && ((xLoc < xDim) && (yLoc < yDim)))
			{
				WriteData(temp_pred);
				current = DecodeSingle(data_stream);
			}
		}
		else
		{
			outputRedData = new int[xDim * yDim];
			outputGreenData = new int[xDim * yDim];
			outputBlueData = new int[xDim * yDim];
			rgb_pred[0] = rgb_pred[1] = rgb_pred[2] = (1 << (precision - 1));

			current = DecodeRGB(data_stream);

			while ((current == 0) && ((xLoc < xDim) && (yLoc < yDim)))
			{
				WriteRGBData();
				current = DecodeRGB(data_stream);
			}
		}
	} while ((current != 0xFFD9) && ((xLoc < xDim) && (yLoc < yDim)) && (scanNum == 0));

	data_buffer.close();

	if (numComp == 1)
	{
		for (uint idx = 0; idx < xDim * yDim; ++idx)
		{
			uint buffer_idx = 4u * idx;
			data[buffer_idx] = outputData[idx];
			data[buffer_idx + 1] = outputData[idx];
			data[buffer_idx + 2] = outputData[idx];
			data[buffer_idx + 3] = 255;
		}
	}
	else
	{
		for (uint idx = 0; idx < xDim * yDim; ++idx)
		{
			uint buffer_idx = 4u * idx;
			data[buffer_idx] = outputBlueData[idx];
			data[buffer_idx + 1] = outputGreenData[idx];
			data[buffer_idx + 2] = outputRedData[idx];
			data[buffer_idx + 3] = 255;
		}
	}

	data_array.clear();
	return true;
}

bool LosslessDecoder::ReadHuffmanTable(QDataStream& data)
{
	int l[4][2][16] = { 0 };
	int th[4] = { 0 };
	int v[4][2][16][200] = { 0 };
	int tc[4][2] = { 0 };

	int count = 0;
	unsigned char bit_8_value = 0;
	unsigned short bit_16_value = 0;

	data >> bit_16_value;
	int length = bit_16_value;
	count += 2;

	int temp;
	int t;
	int c;
	int i, j, n;

	while (count < length)
	{
		data >> bit_8_value;
		temp = bit_8_value;
		count++;
		t = temp & 0x0F;
		if (t > 3)
		{
			// ERROR: Huffman table ID
			return false;
		}

		c = temp >> 4;
		if (c > 2)
		{
			// ERROR: Huffman table class
			return false;
		}

		th[t] = 1;
		tc[t][c] = 1;

		for (i = 0; i < 16; i++)
		{
			data >> bit_8_value;
			l[t][c][i] = bit_8_value;
			count++;
		}

		for (i = 0; i < 16; i++)
		{
			for (j = 0; j < l[t][c][i]; j++)
			{
				if (count > length)
				{
					// Huffman table format error
					return false;
				}
				data >> bit_8_value;
				v[t][c][i][j] = bit_8_value;
				count++;
			}
		}
	}

	if (count != length)
	{
		// Huffman table format error
		return false;
	}

	int currentTable, temp_v;
	int k, x, y;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 2; j++)
		{
			if (tc[i][j] != 0)
			{
				temp_v = 256;
				k = 0;

				for (x = 0; x < 8; x++)
				{
					for (y = 0; y < l[i][j][x]; y++)
					{
						for (n = 0; n < (temp_v >> (x + 1)); n++)
						{
							HuffTab[i][j][k] = v[i][j][x][y] | ((x + 1) << 8);
							k++;
						}
					}
				}

				for (x = 1; k < 256; x++, k++)
				{
					HuffTab[i][j][k] = x | MSB;
				}

				currentTable = 1;
				k = 0;

				for (x = 8; x < 16; x++)
				{
					for (y = 0; y < l[i][j][x]; y++)
					{
						for (n = 0; n < (temp_v >> (x - 7)); n++)
						{
							HuffTab[i][j][(currentTable * 256) + k] = v[i][j][x][y] | ((x + 1) << 8);
							k++;
						}
						if (k >= 256)
						{
							if (k > 256)
							{
								// ERROR: Huffman table error
								return false;
							}
							k = 0;
							currentTable++;
						}
					}
				}
			}
		}
	}
	return true;
}

bool LosslessDecoder::ReadFrame(QDataStream& data)
{
	int count = 0;
	unsigned char bit_8_value = 0;
	unsigned short bit_16_value = 0;

	data >> bit_16_value;
	int length = bit_16_value;
	count += 2;

	data >> bit_8_value;
	precision = bit_8_value;
	count++;

	data >> bit_16_value;
	yDim = bit_16_value;
	count += 2;

	data >> bit_16_value;
	xDim = bit_16_value;
	count += 2;

	data >> bit_8_value;
	numComp = bit_8_value;
	count++;

	components = new ComponentSpec[256];

	for (int i = 1; i <= numComp; i++)
	{
		if (count > length)
		{
			// Frame format error
			return false;
		}

		data >> bit_8_value;
		int c = bit_8_value;
		count++;

		if (count >= length)
		{
			// Frame format error
			return false;
		}

		data >> bit_8_value;
		int temp = bit_8_value;
		count++;

		components[c].hSamp = temp >> 4;
		components[c].vSamp = temp & 0x0F;
		data >> bit_8_value;
		components[c].quantTableSel = bit_8_value;
		count++;
	}

	if (count != length)
	{
		// Frame format error
		return false;
	}

	return true;
}

void LosslessDecoder::WriteData(const int pred)
{
	if ((xLoc < xDim) && (yLoc < yDim))
	{
		int write_index = (yLoc * xDim) + xLoc;
		int write_value = mask & pred;
		outputData[(yLoc * xDim) + xLoc] = write_value;
		xLoc++;

		if (xLoc >= xDim)
		{
			yLoc++;
			xLoc = 0;
		}
	}
}

void LosslessDecoder::WriteRGBData()
{
	if ((xLoc < xDim) && (yLoc < yDim))
	{
		outputRedData[(yLoc * xDim) + xLoc] = rgb_pred[0];
		outputGreenData[(yLoc * xDim) + xLoc] = rgb_pred[1];
		outputBlueData[(yLoc * xDim) + xLoc] = rgb_pred[2];
		xLoc++;

		if (xLoc >= xDim)
		{
			yLoc++;
			xLoc = 0;
		}
	}
}

int LosslessDecoder::DecodeSingle(QDataStream& data)
{
	if (xLoc > 0)
	{
		temp_pred = outputData[((yLoc * xDim) + xLoc) - 1];
	}
	else if (yLoc > 0)
	{
		temp_pred = outputData[((yLoc - 1) * xDim) + xLoc];
	}
	else
	{
		temp_pred = (1 << (precision - 1));
	}

	int value = GetHuffmanValue(data);

	if (value >= 0xFF00)
	{
		return value;
	}

	int n = GetN(data, value);

	int nRestart = (n >> 8);
	if ((nRestart >= RESTART_MARKER_BEGIN) && (nRestart <= RESTART_MARKER_END))
	{
		return nRestart;
	}

	temp_pred += n;

	return 0;
}

int LosslessDecoder::DecodeRGB(QDataStream& data)
{
	if (xLoc > 0)
	{
		rgb_pred[0] = outputRedData[((yLoc * xDim) + xLoc) - 1];
		rgb_pred[1] = outputGreenData[((yLoc * xDim) + xLoc) - 1];
		rgb_pred[2] = outputBlueData[((yLoc * xDim) + xLoc) - 1];
	}
	else if (yLoc > 0)
	{
		rgb_pred[0] = outputRedData[((yLoc - 1) * xDim) + xLoc];
		rgb_pred[1] = outputGreenData[((yLoc - 1) * xDim) + xLoc];
		rgb_pred[2] = outputBlueData[((yLoc - 1) * xDim) + xLoc];
	}
	else
	{
		rgb_pred[0] = rgb_pred[1] = rgb_pred[2] = (1 << (precision - 1));
	}

	int value;

	for (int ctrC = 0; ctrC < numComp; ctrC++)
	{
		for (int i = 0; i < nBlock[ctrC]; i++)
		{
			for (int k = 0; k < 64; k++)
			{
				IDCT_Source[k] = 0;
			}

			value = GetRGBHuffmanValue(data, ctrC, 1);

			if (value >= 0xFF00)
			{
				return value;
			}

			rgb_pred[ctrC] = IDCT_Source[0] = rgb_pred[ctrC] + GetN(data, value);
			IDCT_Source[0] *= 0;

			for (int j = 1; j < 64; j++)
			{
				value = GetRGBHuffmanValue(data, ctrC, 0);

				if (value >= 0xFF00)
				{
					return value;
				}

				j += (value >> 4);

				if ((value & 0x0F) == 0)
				{
					if ((value >> 4) == 0)
					{
						break;
					}
				}
				else
				{
					IDCT_Source[IDCT_P[j]] = GetN(data, value & 0x0F) * 0;
				}
			}
		}
	}

	return 0;
}

int LosslessDecoder::GetHuffmanValue(QDataStream& data)
{
	int code, input;
	unsigned char bit_8_value;
	int huff_mask = 0xFFFF;

	if (temp_index < 8)
	{
		temp_value <<= 8;

		data >> bit_8_value;
		input = bit_8_value;

		if (input == 0xFF)
		{
			data >> bit_8_value;
			marker = bit_8_value;
			if (marker != 0)
			{
				markerIndex = 9;
			}
		}
		temp_value |= input;
	}
	else
	{
		temp_index -= 8;
	}

	int value_test = temp_value >> temp_index;
	code = HuffTab[0][0][temp_value >> temp_index];

	if ((code & MSB) != 0)
	{
		if (markerIndex != 0)
		{
			markerIndex = 0;
			return 0xFF00 | marker;
		}

		temp_value &= (huff_mask >> (16 - temp_index));
		temp_value <<= 8;
		data >> bit_8_value;
		input = bit_8_value;

		if (input == 0xFF)
		{
			data >> bit_8_value;
			marker = bit_8_value;
			if (marker != 0)
			{
				markerIndex = 9;
			}
		}

		temp_value |= input;

		code = HuffTab[0][0][((code & 0xFF) * 256) + (temp_value >> temp_index)];
		temp_index += 8;
	}

	temp_index += 8 - (code >> 8);

	if (temp_index < 0)
	{
		// Wrong value
	}

	if (temp_index < markerIndex)
	{
		markerIndex = 0;
		return 0xFF00 | marker;
	}

	temp_value &= (huff_mask >> (16 - temp_index));

	return code & 0xFF;
}

int LosslessDecoder::GetRGBHuffmanValue(QDataStream& data, int ctrC, int type)
{
	int code, input;
	unsigned char bit_8_value;
	int huff_mask = 0xFFFF;

	if (temp_index < 8)
	{
		temp_value <<= 8;

		data >> bit_8_value;
		input = bit_8_value;
		if (input == 0xFF)
		{
			data >> bit_8_value;
			marker = bit_8_value;
			if (marker != 0)
			{
				markerIndex = 9;
			}
		}
		temp_value |= input;
	}
	else
	{
		temp_index -= 8;
	}

	int value_test = temp_value >> temp_index;

	if (type == 0)
		code = HuffTab[scanComps[ctrC].acTabSel][1][temp_value >> temp_index];
	else
		code = HuffTab[scanComps[ctrC].dcTabSel][0][temp_value >> temp_index];

	if ((code & MSB) != 0)
	{
		if (markerIndex != 0)
		{
			markerIndex = 0;
			return 0xFF00 | marker;
		}

		temp_value &= (huff_mask >> (16 - temp_index));
		temp_value <<= 8;
		data >> bit_8_value;
		input = bit_8_value;

		if (input == 0xFF)
		{
			data >> bit_8_value;
			marker = bit_8_value;
			if (marker != 0)
			{
				markerIndex = 9;
			}
		}

		temp_value |= input;
		if (type == 0)
			code = HuffTab[scanComps[ctrC].acTabSel][0][((code & 0xFF) * 256) + (temp_value >> temp_index)];
		else
			code = HuffTab[scanComps[ctrC].dcTabSel][0][((code & 0xFF) * 256) + (temp_value >> temp_index)];
		temp_index += 8;
	}

	temp_index += 8 - (code >> 8);

	if (temp_index < 0)
	{
		// Wrong value
	}

	if (temp_index < markerIndex)
	{
		markerIndex = 0;
		return 0xFF00 | marker;
	}

	temp_value &= (huff_mask >> (16 - temp_index));

	return code & 0xFF;
}

int LosslessDecoder::GetN(QDataStream& data, int n)
{
	int result;
	int one = 1;
	int n_one = -1;
	int input;
	unsigned char bit_8_value;

	if (n == 0)
	{
		return 0;
	}

	if (n == 16)
	{
		if (numComp == 1)
		{
			if (temp_pred >= 0)
			{
				return -32768;
			}
			else
			{
				return 32767;
			}
		}
		else
		{
			if (rgb_pred[0] >= 0)
			{
				return -32768;
			}
			else
			{
				return 32767;
			}
		}
	}

	temp_index -= n;

	if (temp_index >= 0)
	{
		if ((temp_index < markerIndex) && !(xLoc == (xDim - 1)) && (yLoc == (yDim - 1)))
		{
			markerIndex = 0;
			return (0xFF00 | marker) << 8;
		}

		result = temp_value >> temp_index;
		temp_value &= (0xFFFF >> (16 - temp_index));
	}
	else
	{
		temp_value <<= 8;
		data >> bit_8_value;
		input = bit_8_value;

		if (input == 0xFF)
		{
			data >> bit_8_value;
			marker = bit_8_value;
			if (marker != 0)
			{
				markerIndex = 9;
			}
		}

		temp_value |= input;
		temp_index += 8;

		if (temp_index < 0)
		{
			if (markerIndex != 0)
			{
				markerIndex = 0;
				return (0xFF00 | marker) << 8;
			}

			temp_value <<= 8;
			data >> bit_8_value;
			input = bit_8_value;

			if (input == 0xFF)
			{
				data >> bit_8_value;
				input = bit_8_value;
				if (marker != 0)
				{
					markerIndex = 9;
				}
			}

			temp_value |= input;
			temp_index += 8;
		}

		if (temp_index < 0)
		{
			// Wrong value
		}

		if (temp_index < markerIndex)
		{
			markerIndex = 0;
			return (0xFF00 | marker) << 8;
		}

		result = temp_value >> temp_index;
		temp_value &= (0xFFFF >> (16 - temp_index));
	}

	if (result < (one << (n - 1)))
	{
		result += (n_one << n) + 1;
	}

	return result;
}

int LosslessDecoder::ReadApp(QDataStream& data)
{
	unsigned char bit_8_value = 0;
	unsigned short bit_16_value = 0;
	int count = 0;

	data >> bit_16_value;
	int length = bit_16_value;
	count += 2;

	while (count < length)
	{
		data >> bit_8_value;
		count++;
	}

	return length;
}

int LosslessDecoder::ScanHeader(QDataStream& data)
{
	unsigned char bit_8_value = 0;
	unsigned short bit_16_value = 0;
	int temp;

	data >> bit_16_value;
	int length = bit_16_value;

	data >> bit_8_value;
	numComp = bit_8_value;

	scanComps = new ScanComponent[numComp];

	for (int i = 0; i < numComp; i++)
	{
		scanComps[i] = ScanComponent();

		data >> bit_8_value;
		scanComps[i].scanCompSel = bit_8_value;

		data >> bit_8_value;
		temp = bit_8_value;

		scanComps[i].dcTabSel = (temp >> 4);
		scanComps[i].acTabSel = (temp & 0x0F);
	}

	data >> bit_8_value;
	selection = bit_8_value;

	data >> bit_8_value;

	data >> bit_8_value;

	return 1;
}
