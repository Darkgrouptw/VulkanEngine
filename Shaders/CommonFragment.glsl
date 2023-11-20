// BaryCoordPos
const vec4 BorderColor			                                    = vec4(0, 0, 0, 1);

float EdgeFactor(vec3 pBaryCoordpos)
{
	vec3 d = fwidth(pBaryCoordpos);
	vec3 a3 = smoothstep(vec3(0.f), d, pBaryCoordpos);
	return min(min(a3.x, a3.y), a3.z);
}