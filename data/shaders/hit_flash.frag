uniform sampler2D texture;
uniform float flashAmount;

void main() {
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy) * gl_Color;
	vec3 flashed = mix(pixel.rgb, vec3(1.0), flashAmount);
	gl_FragColor = vec4(flashed, pixel.a);
}

