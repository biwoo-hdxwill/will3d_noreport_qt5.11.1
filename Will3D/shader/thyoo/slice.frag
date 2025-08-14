#version 400

in vec3 TexCoord;

uniform sampler3D	VolumeTex; 

uniform float		WindowLevel;	
uniform float		WindowWidth;	
uniform int         Thickness; //must be grater than 1;
uniform vec3        TexelSize;
uniform vec3        UpVector;

layout (location = 0) out vec4 FragColor;

void main()
{
	float windowMin = WindowLevel - (WindowWidth * 0.5f);
	float windowMax = WindowLevel + (WindowWidth * 0.5f);

    float val = 0.0f;
	if(Thickness > 1){
        float haf_thickness =  (float(Thickness) * 0.5f);
        vec3 up_vector = UpVector*TexelSize;

        vec3 curr = TexCoord - up_vector*haf_thickness;
        for(int i = 0; i < Thickness; i++){
            val += texture(VolumeTex, curr).x * 65535.0f;
            curr += up_vector;
        }
        val /= float(Thickness);
        val = clamp(val,         
            min(windowMin, windowMax),
            max(windowMin, windowMax));
    } else{
        val = clamp(texture(VolumeTex, TexCoord).x * 65535.0f,         
            min(windowMin, windowMax),
            max(windowMin, windowMax));
    }

	val = (val - windowMin) / WindowWidth;

	FragColor = vec4(vec3(val), 1.0f);
}
