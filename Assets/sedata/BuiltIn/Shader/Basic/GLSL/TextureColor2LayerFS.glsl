precision mediump float;

uniform sampler2D uTexDiffuse1;
uniform sampler2D uTexDiffuse2;
uniform float uLightMapEnable;
uniform vec4 uSelectedColor;
uniform float uSelected;
in vec2 varTexCoord0;
in vec2 varTexCoord1;
in vec4 varColor;
in vec4 pos;
out vec4 FragColor;

void main(void)
{
	gl_FragDepth = gl_FragCoord.z;
	vec4 color1 = texture(uTexDiffuse1, varTexCoord0.xy);

	vec4 color2 = texture(uTexDiffuse2, varTexCoord1.xy);
	vec4 color3 = color1 *color2 * varColor;
	color3.a=color1.a*varColor.a;

	if(round(uLightMapEnable)==0) {
		color3 = color1 * varColor;
	}
	
	FragColor =color3;
	if(round(uSelected)==1) {
		FragColor=mix(color3,uSelectedColor,uSelectedColor.a);
		FragColor.a= color3.a;
	}
}