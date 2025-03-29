#version 150

#define TWO_PI 6.283185307179586476925286766559

uniform sampler2DRect tex0[8];
in vec2 texCoordVarying;
out vec4 outputColor;
uniform vec2 window;
uniform float amplitude;
uniform vec4 parameterValues[16];
uniform vec4 filterVec;

void main()
{
    vec2 normalized = gl_FragCoord.xy / window;
    vec2 inverseNormalized = 1.0 - normalized;
    float phase = ((normalized.x * filterVec.x) + (inverseNormalized.x * filterVec.y) + (normalized.y * filterVec.z) + (inverseNormalized.y * filterVec.w)) * TWO_PI / (1.0 / (pow(amplitude, pow(window.x * window.y, 0.125)) + 1.0));
    vec3 feedbackColor = vec3(0.0);
    float floatCount = 0.0;
    for(int count = 0; count < 8; count++){
        floatCount++;
        vec3 lowPass = texture2DRect(tex0[count], texCoordVarying * parameterValues[count].yw + (((1.0 - parameterValues[count + 8].yw) * 2.0 - 1.0) * window)).rgb;
        vec3 highPass = 1.0 - texture2DRect(tex0[count], texCoordVarying * parameterValues[count].xz + (((1.0 - parameterValues[count + 8].xz) * 2.0 - 1.0) * window)).rgb;
        vec3 bandPass = lowPass * highPass;
        vec3 bandReject = 1.0 - bandPass;
        lowPass *= pow(filterVec.x * filterVec.y, 0.5);
        highPass *= pow(filterVec.z * filterVec.w, 0.5);
        bandPass *= pow(filterVec.x * filterVec.w, 0.25);
        bandReject *= pow(filterVec.y * filterVec.z, 0.25);
        feedbackColor += (lowPass + highPass + bandPass + bandReject) * 0.125 / (filterVec.x + filterVec.y + filterVec.z + filterVec.w);
    }
    vec3 newColor = vec3(sin(phase), sin(phase + (TWO_PI / 3.0)), sin(phase + (2.0 * TWO_PI / 3.0)));
    outputColor = vec4(mix(newColor, feedbackColor, abs(feedbackColor.r * feedbackColor.g * feedbackColor.b - 0.75)), 1.0);
}