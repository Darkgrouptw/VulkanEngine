#extension GL_EXT_fragment_shader_barycentric : require														// Link: https://www.khronos.org/opengl/wiki/Core_Language_(GLSL)#Extensions
const vec4 BorderColor			                                    = vec4(0, 0, 0, 1);

float EdgeFactor()
{
	const vec3 baryCoord = gl_BaryCoordNoPerspEXT;
	vec3 d = fwidth(baryCoord);
	vec3 a3 = smoothstep(vec3(0.f), d * 0.8f, baryCoord);
	return min(min(a3.x, a3.y), a3.z);
}