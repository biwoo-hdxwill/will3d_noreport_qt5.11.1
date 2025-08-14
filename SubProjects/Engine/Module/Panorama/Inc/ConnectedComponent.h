#pragma once

/*=========================================================================

File:			class CW3ConnectedComponent
Language:		C++11
Library:		-
Author:			Hong Jung
First date:		2016-02-03
Last date:		2016-02-03

=========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

class CW3ConnectedComponent
{
public:
	typedef struct {
		int label = 0;
		int pointsAboveLowThreshold = 0;
		int pointsAboveHighThreshold  = 0;
		char completelyProcessed = 0;
	} typeConnectedComponent;

	typedef struct {
		int label = 0;
		unsigned long size = 0;
	} typeCC_for_sort;

public:
	CW3ConnectedComponent();
	~CW3ConnectedComponent();

	/* Counts and labels connected components.
	 *
	 * DESCRIPTION:
	 * Extracts connected components from the input buffer.
	 * Only points with values >= 0 are considered.
	 *
	 * PARAMETERS:
	 * - bufferDims[0] is the dimension along X,
	 *   bufferDims[1] is the dimension along Y,
	 *   bufferDims[2] is the dimension along Y.
	 *
	 * RETURN:
	 * - the number of selected connected components if successful (may be 0)*/

	int CountConnectedComponents(unsigned short *bufferIn, unsigned short *bufferOut, int iSizeX, int iSizeY, int iSizeZ);

	/* Counts and labels connected components (without global parameters)
	 * Main differences with CountConnectedComponents()
	 *
	 * - the input image is binarized with the parameter 'threshold'
	 *   points with value greater or equal to the threshold are the
	 *   foreground (whom components will be extracted), while the others
	 *   are the background
	 *
	 * - by specifying a binary output (with a non null value of 'outputIsBinary')
	 *   one eliminates in a binary image the 'small' components. The binary value
	 *   is the maximal value with respect to the output type (e.g. 255 for unsigned char).
	 */

	int CountConnectedComponentsWithAllParams(unsigned short *bufferIn, unsigned short *bufferOut,
		int *bufferDims, double threshold, int connectivity, int minNumberOfPts,
		int maxNumberOfConnectedComponent);

private:

	/* Changes the labels of connected components to sort them by decreasing size
	 *
	 * DESCRIPTION:
	 * The connected components are sorted by deceasing size
	 * so that the new label of the largest connected component
	 * will be 1, of the second largest connected component 2,
	 * ... and the new label of the smallest connected component
	 * will be equal to the num,be of connected components
	 *
	 * RETURN:
	 *  1: success
	 * -1: error
	 */

	int RelabelConnectedComponentsByDecreasingSize(unsigned short *inputBuf, int *theDim);

	/* Set the connectivity for the extraction of connected components.
	 *
	 * DESCRIPTION:
	 * Admitted values are 4, 8, 6, 10, 18, and 26.
	 * The 2 first are 2D connectivities.
	 * The 4 last are 3D connectivities.
	 * The 10-connectivity is an anisotropic connectivity,
	 * a mixed-up of the 2D 8-connectivity and the 3D
	 * 6-connectivity.
	 *
	 */

	void Connexe_SetConnectivity(int c);

	/* Set the minimal size of the connected components to be kept.
	 *
	 * DESCRIPTION:
	 * The extracted components which contains less than
	 * this number of points are discarded.
	 */

	void Connexe_SetMinimumSizeOfComponents(int c);

	/* Set the maximum number of the connected components to be kept.
	 *
	 * DESCRIPTION:
	 * If this number is <= 0, all the valid connected components
	 * are kept.
	 * If this number is > 0, the connected components are sorted
	 * with respect to their size (number of points), and only
	 * the c largest components are kept.
	 *
	 * A side effect is (for CountConnectedComponents()):
	 * if c is larger than the number of valid connected
	 * connected, all the components are kept, but the labels are sorted,
	 * i.e. the largest connected component has label 1,
	 * the second largest connected component has label 2, etc.
	 *
	 */
	void Connexe_SetMaximumNumberOfComponents(int c);

	int CheckAndEvaluateConnectivity(int connectivity, int dimz);

	int RelabelConnectedComponents(unsigned short *inputBuf, int *theDim, unsigned short *labelsBuf,
		typeConnectedComponent *components);

	/**********************************************************************************************//**
	 * @fn	int CW3ConnectedComponent::InternalConnectedComponentsExtraction(unsigned short *inputBuf, int *theDim, typeConnectedComponent **theCc, int connectivity, int minNumberOfPtsAboveLow, int minNumberOfPtsAboveHigh, int maxNumberOfConnectedComponent);
	 *
	 * @brief	Internal connected components extraction.
	 *
	 * @author	Hong Jung
	 * @date	2016-02-03
	 *
	 * @param [in,out]	inputBuf					 	input buffer, should contains at most 3 values
	 * 													0, _low_value_, _hig_value_
	 * @param [in,out]	theDim						 	dimension of the input buffer
	 * @param [in,out]	theCc						 	equivalence array, should be already allocated
	 * @param 		  	connectivity				 	connectivity:
	 * 													in 2D: 4, 8;
	 * 													in 3D: 6, 10, 18, 26
	 * @param 		  	minNumberOfPtsAboveLow		 	requested minimum number of points
	 * 													above the low value to validate the connected component.
	 * @param 		  	minNumberOfPtsAboveHigh		 	requested minimum number of points
	 * 													above the high value to validate the connected component.
	 * @param 		  	maxNumberOfConnectedComponent	maximal number of connected components.
	 * 													If this value is <= 0 or >= _MAXIMAL_LABEL_VALUE_ (=65535),
	 * 													all the valid connected components are kept.
	 * 													For other values, the components are sorted with respect to
	 * 													their size and only the largest ones are kept.
	 * 													As a side effect, if the number of valid connected components
	 * 													is less than the maximal number of connected components,
	 * 													 the labeling gives the order with respect to the size.
	 *
	 * @return	An int.
	 **************************************************************************************************/

	int InternalConnectedComponentsExtraction(unsigned short *inputBuf, int *theDim,
		typeConnectedComponent **theCc, int connectivity, int minNumberOfPtsAboveLow,
		int minNumberOfPtsAboveHigh, int maxNumberOfConnectedComponent);
	int InitNeighborsOffsets(int array_offset[3][3][2], int offsets[13], int dimx,
		int dimxy, int connectivity);
	typeConnectedComponent* LabelsOverflowManagement(unsigned short *inputBuf, int *theDim,
		typeConnectedComponent *cc, int connectivity, int minNumberOfPtsAboveLow,
		int minNumberOfPtsAboveHigh, int *used_labels,
		int x, int y, int z);
	void SortConnectedComponents(typeConnectedComponent *tab, int left, int right);
	void SortCCWithRespectToSize(typeCC_for_sort *tab, int left, int right);

private:
	unsigned short m_low_value;
	unsigned short m_hig_value;

	int m_connectivity;
	int m_minimum_size_of_components;
	int m_maximum_number_of_components;
};
