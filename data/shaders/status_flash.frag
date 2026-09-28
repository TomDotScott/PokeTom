uniform sampler2D texture;
uniform float flashAmount;
uniform vec3 flashColour;

void main() {
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy) * gl_Color;
	vec3 flashed = mix(pixel.rgb, flashColour, flashAmount);
	gl_FragColor = vec4(flashed, pixel.a);
}
