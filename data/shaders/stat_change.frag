uniform sampler2D texture;
uniform vec3 tintColour;
uniform float opacity;
uniform float scrollOffset;
uniform float rowCount;
uniform float thickness;

// The half-width of the V
uniform float chevronWidth;

float ChevronMask(vec2 uv)
{
	// tile vertically, scroll over time
	float row = fract(uv.y * rowCount + scrollOffset) - 0.5;

	// centre x, build a V shape: distance from the two diagonal legs
	float x = (uv.x - 0.5) * 2.0;
	float legDist = abs(abs(x) * chevronWidth - abs(row)) ;

	float mask = 1.0 - smoothstep(0.0, thickness, legDist);

	// fade out near the top/bottom of texcoord range so it doesn't wrap ugly
	float edgeFade = smoothstep(0.0, 0.05, uv.y) * smoothstep(1.0, 0.95, uv.y);

	return mask * edgeFade;
}

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec4 pixel = texture2D(texture, uv) * gl_Color;

	float arrowMask = ChevronMask(uv);

	vec3 finalColor = mix(pixel.rgb, tintColour, arrowMask * opacity * pixel.a);
	gl_FragColor = vec4(finalColor, pixel.a);
}
