uniform sampler2D texture;
uniform sampler2D arrowTexture;
uniform vec2 arrowTiling;
uniform float opacity;
uniform float scrollOffset;

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec4 pixel = texture2D(texture, uv) * gl_Color;

	vec2 arrowUV = uv * arrowTiling + vec2(0.0, scrollOffset);
	vec3 arrowMask = texture2D(arrowTexture, fract(arrowUV)).rgb;

	vec3 finalColor = mix(pixel.rgb, arrowMask, opacity * pixel.a);
	gl_FragColor = vec4(finalColor, pixel.a);
}
