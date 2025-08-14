__kernel void uTransformCL(__global unsigned short *Ref, __global float *IsIn, __global float *TM,
						   float tranX, uint nx, uint ny, uint nz, float centerx, float centery, float centerz,
						   __read_only image3d_t VOL, __local float *localTM, uint nTMele)
{
	uint idx = get_global_id(0);
	uint iy = get_global_id(1);

	if(get_local_id(0) < nTMele && get_local_id(1) == 0)
	{
		localTM[get_local_id(0)] = TM[get_local_id(0)];
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if(idx < nx && iy < ny*nz)
	{
		const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

		uint idz = iy/ny;
		uint idy = iy - idz*ny;

		float idxf = idx - centerx;
		float idyf = idy - centery;
		float idzf = idz - centerz;

		float4 coord;

		coord.x = (idxf*localTM[0] + idyf*localTM[1] + idzf*localTM[2] + tranX) + centerx + 0.5f;
		coord.y = (idxf*localTM[3] + idyf*localTM[4] + idzf*localTM[5]) + centery + 0.5f;
		coord.z = (idxf*localTM[6] + idyf*localTM[7] + idzf*localTM[8]) + centerz + 0.5f;

		uint id = iy*nx + idx;

		if(coord.x >= 0.5f && coord.x <= nx - 0.5f && coord.y >= 0.5f && coord.y <= ny - 0.5f && coord.z >= 0.5f && coord.z <= nz - 0.5f)
		{	
			Ref[id] = read_imageui(VOL, sampler, coord).w;
			IsIn[id] = 1.0f;
		}
		else
		{
			Ref[id] = 0;
			IsIn[id] = 0.0f;
		}


	}

}

__kernel void sTransformCL(__global short *Ref, __global float *IsIn, __global float *TM,
						   float tranX, uint nx, uint ny, uint nz, float centerx, float centery, float centerz,
						   __read_only image3d_t VOL, __local float *localTM, uint nTMele)
{
	uint idx = get_global_id(0);
	uint iy = get_global_id(1);

	if(get_local_id(0) < nTMele && get_local_id(1) == 0)
	{
		localTM[get_local_id(0)] = TM[get_local_id(0)];
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if(idx < nx && iy < ny*nz)
	{
		const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

		uint idz = iy/ny;
		uint idy = iy - idz*ny;

		float idxf = idx - centerx;
		float idyf = idy - centery;
		float idzf = idz - centerz;

		float4 coord;

		coord.x = (idxf*localTM[0] + idyf*localTM[1] + idzf*localTM[2] + tranX) + centerx + 0.5f;
		coord.y = (idxf*localTM[3] + idyf*localTM[4] + idzf*localTM[5]) + centery + 0.5f;
		coord.z = (idxf*localTM[6] + idyf*localTM[7] + idzf*localTM[8]) + centerz + 0.5f;

		uint id = iy*nx + idx;

		float4 zeroc;
		zeroc.x = 0.5f;
		zeroc.y = 0.5f;
		zeroc.z = 0.5f;


		if(coord.x >= 0.5f && coord.x <= nx - 0.5f && coord.y >= 0.5f && coord.y <= ny - 0.5f && coord.z >= 0.5f && coord.z <= nz - 0.5f)
		{	
			Ref[id] = read_imagei(VOL, sampler, coord).w;
			IsIn[id] = 1.0f;
		}
		else
		{
			Ref[id] = read_imagei(VOL, sampler, zeroc).w;
			IsIn[id] = 0.0f;
		}


	}

}

__kernel void uTransform3DCL(__global unsigned short *Ref, __global float *IsIn, __global float *TM,
	float tranX, float tranY, float tranZ, uint nx, uint ny, uint nz, float centerx, float centery, float centerz,
	__read_only image3d_t VOL, __local float *localTM, uint nTMele)
{
	uint idx = get_global_id(0);
	uint iy = get_global_id(1);

	if (get_local_id(0) < nTMele && get_local_id(1) == 0)
	{
		localTM[get_local_id(0)] = TM[get_local_id(0)];
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if (idx < nx && iy < ny*nz)
	{
		const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

		uint idz = iy / ny;
		uint idy = iy - idz*ny;

		float idxf = idx - centerx;
		float idyf = idy - centery;
		float idzf = idz - centerz;

		float4 coord;

		coord.x = (idxf*localTM[0] + idyf*localTM[1] + idzf*localTM[2] + tranX) + centerx + 0.5f;
		coord.y = (idxf*localTM[3] + idyf*localTM[4] + idzf*localTM[5] + tranY) + centery + 0.5f;
		coord.z = (idxf*localTM[6] + idyf*localTM[7] + idzf*localTM[8] + tranZ) + centerz + 0.5f;

		uint id = iy*nx + idx;

		if (coord.x >= 0.5f && coord.x <= nx - 0.5f && coord.y >= 0.5f && coord.y <= ny - 0.5f && coord.z >= 0.5f && coord.z <= nz - 0.5f)
		{
			Ref[id] = read_imageui(VOL, sampler, coord).w;
			IsIn[id] = 1.0f;
		}
		else
		{
			Ref[id] = 0;
			IsIn[id] = 0.0f;
		}


	}

}

__kernel void sTransform3DCL(__global short *Ref, __global float *IsIn, __global float *TM,
	float tranX, float tranY, float tranZ, uint nx, uint ny, uint nz, float centerx, float centery, float centerz,
	__read_only image3d_t VOL, __local float *localTM, uint nTMele)
{
	uint idx = get_global_id(0);
	uint iy = get_global_id(1);

	if (get_local_id(0) < nTMele && get_local_id(1) == 0)
	{
		localTM[get_local_id(0)] = TM[get_local_id(0)];
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if (idx < nx && iy < ny*nz)
	{
		const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

		uint idz = iy / ny;
		uint idy = iy - idz*ny;

		float idxf = idx - centerx;
		float idyf = idy - centery;
		float idzf = idz - centerz;

		float4 coord;

		coord.x = (idxf*localTM[0] + idyf*localTM[1] + idzf*localTM[2] + tranX) + centerx + 0.5f;
		coord.y = (idxf*localTM[3] + idyf*localTM[4] + idzf*localTM[5] + tranY) + centery + 0.5f;
		coord.z = (idxf*localTM[6] + idyf*localTM[7] + idzf*localTM[8] + tranZ) + centerz + 0.5f;

		uint id = iy*nx + idx;

		float4 zeroc;
		zeroc.x = 0.5f;
		zeroc.y = 0.5f;
		zeroc.z = 0.5f;


		if (coord.x >= 0.5f && coord.x <= nx - 0.5f && coord.y >= 0.5f && coord.y <= ny - 0.5f && coord.z >= 0.5f && coord.z <= nz - 0.5f)
		{
			Ref[id] = read_imagei(VOL, sampler, coord).w;
			IsIn[id] = 1.0f;
		}
		else
		{
			Ref[id] = read_imagei(VOL, sampler, zeroc).w;
			IsIn[id] = 0.0f;
		}


	}

}

__kernel void uTransformedDifferenceCL(__global float *Difference, __global uint *boneId, __global float *IsIn, __global float *TM,
	float tranX, float tranY, float tranZ, uint nx, uint ny, uint nz, uint Nid, float centerx, float centery, float centerz, float centerxM, float centeryM, float centerzM,
	__read_only image3d_t ref, __read_only image3d_t moving, __local float *localTM, uint nTMele)
{
	uint id = get_global_id(0);

	if (get_local_id(0) < nTMele)
	{
		localTM[get_local_id(0)] = TM[get_local_id(0)];
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if (id < Nid)
	{
		uint idx, idy, idz;

		idy = nx*ny;

		idz = boneId[id] / idy;
		idx = boneId[id] - idz*idy;
		idy = idx / nx;
		idx = idx - idy*nx;

		const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

		float idxf = idx - centerx;
		float idyf = idy - centery;
		float idzf = idz - centerz;	// reference volume coordinate

		float4 coordRef;
		coordRef.x = idx + 0.5f;
		coordRef.y = idy + 0.5f;
		coordRef.z = idz + 0.5f;

		float4 coord;	// moving volume coordinate

		coord.x = (idxf*localTM[0] + idyf*localTM[1] + idzf*localTM[2] + tranX) + centerxM + 0.5f;
		coord.y = (idxf*localTM[3] + idyf*localTM[4] + idzf*localTM[5] + tranY) + centeryM + 0.5f;
		coord.z = (idxf*localTM[6] + idyf*localTM[7] + idzf*localTM[8] + tranZ) + centerzM + 0.5f;

		//IsIn[id] = 1.0f;
		//Difference[id] = read_imagei(ref, sampler, coordRef).w - read_imagei(moving, sampler, coordRef).w;
		if (coord.x >= 0.5f && coord.x <= nx - 0.5f && coord.y >= 0.5f && coord.y <= ny - 0.5f && coord.z >= 0.5f && coord.z <= nz - 0.5f)
		{
			Difference[id] = abs(read_imageui(ref, sampler, coordRef).w - read_imageui(moving, sampler, coord).w);
			//Difference[id] = abs(read_imagei(ref, sampler, coordRef).w - read_imagei(moving, sampler, coordRef).w);
			IsIn[id] = 1.0f;
		}
		else
		{
			Difference[id] = 0.0f;
			IsIn[id] = 0.0f;
		}


	}

}

__kernel void sTransformedDifferenceCL(__global float *Difference, __global uint *boneId, __global float *IsIn, __global float *TM,
	float tranX, float tranY, float tranZ, uint nx, uint ny, uint nz, uint Nid, float centerx, float centery, float centerz, float centerxM, float centeryM, float centerzM,
	__read_only image3d_t ref, __read_only image3d_t moving, __local float *localTM, uint nTMele)
{
	uint id = get_global_id(0);

	if (get_local_id(0) < nTMele)
	{
		localTM[get_local_id(0)] = TM[get_local_id(0)];
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if (id < Nid)
	{
		uint idx, idy, idz;

		idy = nx*ny;

		idz = boneId[id] / idy;
		idx = boneId[id] - idz*idy;
		idy = idx / nx;
		idx = idx - idy*nx;

		const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

		float idxf = idx - centerx;
		float idyf = idy - centery;
		float idzf = idz - centerz;	// reference volume coordinate

		float4 coordRef;
		coordRef.x = idx + 0.5f;
		coordRef.y = idy + 0.5f;
		coordRef.z = idz + 0.5f;

		float4 coord;	// moving volume coordinate

		coord.x = (idxf*localTM[0] + idyf*localTM[1] + idzf*localTM[2] + tranX) + centerxM + 0.5f;
		coord.y = (idxf*localTM[3] + idyf*localTM[4] + idzf*localTM[5] + tranY) + centeryM + 0.5f;
		coord.z = (idxf*localTM[6] + idyf*localTM[7] + idzf*localTM[8] + tranZ) + centerzM + 0.5f;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// by jdk 170524 temp
		float4 ref_, moving_;

		ref_ = read_imagef(ref, sampler, coordRef);
		moving_ = read_imagef(moving, sampler, coord);
		
		Difference[id] = fabs(ref_.w - moving_.w);
		IsIn[id] = 1.0f;
		//

		/* org - 1st : normal, 2nd : move -> ok / 1st : move, 2nd : normal -> error
		if (coord.x >= 0.5f && coord.x <= nx - 0.5f && coord.y >= 0.5f && coord.y <= ny - 0.5f && coord.z >= 0.5f && coord.z <= nz - 0.5f)
		{
			Difference[id] = abs(read_imagei(ref, sampler, coordRef).w - read_imagei(moving, sampler, coord).w);
			//Difference[id] = abs(read_imagei(ref, sampler, coordRef).w - read_imagei(moving, sampler, coordRef).w);
			IsIn[id] = 1.0f;
		}
		else
		{
			Difference[id] = 0.0f;
			IsIn[id] = 0.0f;
		}
		*/
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}
}

__kernel void uDifferenceXReflectionCL(__global float *Difference, __global float *IsInCount, __global unsigned short *Ref, __global float *IsIn,
									  uint nxhalf, uint nx, uint ny, uint nz)
{
	uint idx = get_global_id(0);
	uint idy = get_global_id(1);

	if (idx < nxhalf && idy < ny*nz)
	{
		uint id = idy*nx;
		uint left = id + idx;
		uint right = id + nx - 1 - idx;

		id = idy*nxhalf + idx;

		if (IsIn[left] == 0.0f && IsIn[right] == 0.0f)
		{
			Difference[id] = 0.0f;
			IsInCount[id] = 0.0f;
		}
		else
		{
			Difference[id] = abs(Ref[left] - Ref[right]);
			IsInCount[id] = 1.0f;
		}

	}

}


__kernel void sDifferenceXReflectionCL(__global float *Difference, __global float *IsInCount, __global short *Ref, __global float *IsIn,
									  uint nxhalf, uint nx, uint ny, uint nz)
{
	uint idx = get_global_id(0);
	uint idy = get_global_id(1);

	if(idx < nxhalf && idy < ny*nz)
	{
		uint id = idy*nx;
		uint left = id + idx;
		uint right = id + nx - 1 - idx;

		id = idy*nxhalf + idx;

		if( IsIn[left] == 0.0f && IsIn[right] == 0.0f)
		{
			Difference[id] = 0.0f;
			IsInCount[id] = 0.0f;
		}
		else
		{
			Difference[id] = abs(Ref[left] - Ref[right]);
			IsInCount[id] = 1.0f;
		}

	}

}


__kernel void uTransform2CL(__global unsigned short *Ref, __global float *TM,
						   float tranX, uint nx, uint ny, uint nz, float centerx, float centery, float centerz,
						   __read_only image3d_t VOL, __local float *localTM, uint nTMele)
{
	uint idx = get_global_id(0);
	uint iy = get_global_id(1);

	if(get_local_id(0) < nTMele && get_local_id(1) == 0)
	{
		localTM[get_local_id(0)] = TM[get_local_id(0)];
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if(idx < nx && iy < ny*nz)
	{
		const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

		uint idz = iy/ny;
		uint idy = iy - idz*ny;

		float idxf = idx - centerx;
		float idyf = idy - centery;
		float idzf = idz - centerz;

		float4 coord;

		coord.x = (idxf*localTM[0] + idyf*localTM[1] + idzf*localTM[2] + tranX) + centerx + 0.5f;
		coord.y = (idxf*localTM[3] + idyf*localTM[4] + idzf*localTM[5]) + centery + 0.5f;
		coord.z = (idxf*localTM[6] + idyf*localTM[7] + idzf*localTM[8]) + centerz + 0.5f;

		uint id = iy*nx + idx;

		if(coord.x >= 0.5f && coord.x <= nx - 0.5f && coord.y >= 0.5f && coord.y <= ny - 0.5f && coord.z >= 0.5f && coord.z <= nz - 0.5f)
		{	
			Ref[id] = read_imageui(VOL, sampler, coord).w;
		}
		else
		{
			Ref[id] = 0;
		}


	}
}


__kernel void sTransform2CL(__global short *Ref, __global float *TM,
						   float tranX, uint nx, uint ny, uint nz, float centerx, float centery, float centerz,
						   __read_only image3d_t VOL, __local float *localTM, uint nTMele)
{
	uint idx = get_global_id(0);
	uint iy = get_global_id(1);

	if(get_local_id(0) < nTMele && get_local_id(1) == 0)
	{
		localTM[get_local_id(0)] = TM[get_local_id(0)];
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if(idx < nx && iy < ny*nz)
	{
		const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

		uint idz = iy/ny;
		uint idy = iy - idz*ny;

		float idxf = idx - centerx;
		float idyf = idy - centery;
		float idzf = idz - centerz;

		float4 coord;

		coord.x = (idxf*localTM[0] + idyf*localTM[1] + idzf*localTM[2] + tranX) + centerx + 0.5f;
		coord.y = (idxf*localTM[3] + idyf*localTM[4] + idzf*localTM[5]) + centery + 0.5f;
		coord.z = (idxf*localTM[6] + idyf*localTM[7] + idzf*localTM[8]) + centerz + 0.5f;

		float4 zeroc;
		zeroc.x = 0.5f;
		zeroc.y = 0.5f;
		zeroc.z = 0.5f;

		uint id = iy*nx + idx;

		if(coord.x >= 0.5f && coord.x <= nx - 0.5f && coord.y >= 0.5f && coord.y <= ny - 0.5f && coord.z >= 0.5f && coord.z <= nz - 0.5f)
		{	
			Ref[id] = read_imagei(VOL, sampler, coord).w;
		}
		else
		{
			Ref[id] = read_imagei(VOL, sampler, zeroc).w;
		}


	}
}


__kernel void Difference2DCL(__global float *diff, __global float *isIn, __read_only image2d_t ref, __read_only image2d_t moving, uint nx, uint ny)
{
	uint idx = get_global_id(0);
	uint idy = get_global_id(1);

	if(idx < nx && idy < ny)
	{
		const sampler_t sampler2D = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

		int2 coord2D;
		coord2D.x = idx;
		coord2D.y = idy;
		
		uint id = idy*nx + idx;

		float4 ref_, moving_;

		ref_ = read_imagef(ref, sampler2D, coord2D);
		moving_ = read_imagef(moving, sampler2D, coord2D);

		if(ref_.w > 0.5f && ref_.y < 0.0f && moving_.w > 0.5f)
		//if(ref_.w > 0.5f && ref_.y < 0.0f && moving_.w > 0.5f && moving_.y < 0.0f)
		//if(ref_.w > 0.5f && ref_.y < 0.0f)
		{
			diff[id] = length(ref_.xyz - moving_.xyz);
			isIn[id] = 1.0f;
		}
		else
		{
			diff[id] = 0.0f;
			isIn[id] = 0.0f;
		}


	}

}


__kernel void Test2DCL(__global float *diff, __global float *isIn, __read_only image2d_t ref, __read_only image2d_t moving, uint nx, uint ny)
{
	uint idx = get_global_id(0);
	uint idy = get_global_id(1);

	if(idx < nx && idy < ny)
	{
		const sampler_t sampler2D = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

		int2 coord2D;
		coord2D.x = idx;
		coord2D.y = idy;
		
		uint id = idy*nx + idx;

		float4 ref_, moving_;

		ref_ = read_imagef(ref, sampler2D, coord2D);
		moving_ = read_imagef(moving, sampler2D, coord2D);

		diff[id] = ref_.y;
		


	}

}