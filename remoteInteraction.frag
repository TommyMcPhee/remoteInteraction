#version 150

#define TWO_PI 6.283185307179586476925286766559

uniform sampler2DRect tex0[8];
in vec2 texCoordVarying;
out vec4 outputColor;
uniform vec2 window;
uniform float amplitude;
uniform vec4 parameterValues[16];
uniform vec4 filterVec;

float beam(float coordinates, float location, float power){
    float scale = 0.5 + abs(0.5 - location);
    return pow((scale - abs(coordinates - location)) * (1.0 / scale), power);
}

float oscillate(vec2 normalizedVec, float xLocation, float yLocation, float xPower, float yPower){
    return pow(beam(normalizedVec.x, xLocation, xPower) * beam(normalizedVec.y, yLocation, yPower), 0.5 * (1.0 - (xPower * yPower)));
}

float processColor(float unprocessedColor){
    return mod(unprocessedColor, 1.0);
}

void main()
{
    vec2 normalized = gl_FragCoord.xy / window;
    vec2 inverseNormalized = 1.0 - normalized;
    float phase = ((normalized.x * filterVec.x * filterVec.z) + (inverseNormalized.x * filterVec.y * filterVec.w) + (normalized.y * filterVec.z * filterVec.w) + (inverseNormalized.y * filterVec.x * filterVec.y)) * TWO_PI / (1.0 / (pow(amplitude, pow(window.x * window.y, 0.125)) + 1.0));
    vec3 feedbackColor = vec3(0.0);
    float floatCount = 0.0;
    for(int count = 0; count < 8; count++){
        floatCount++;
        vec3 lowPass = texture2DRect(tex0[count], texCoordVarying * parameterValues[count].yw + (((1.0 - parameterValues[count + 8].yw) * 2.0 - 1.0) * window)).rgb;
        vec3 highPass = 1.0 - texture2DRect(tex0[count], texCoordVarying * parameterValues[count].xz + (((1.0 - parameterValues[count + 8].xz) * 2.0 - 1.0) * window)).rgb;
        vec3 bandPass = lowPass * highPass;
        vec3 bandReject = 1.0 - bandPass;
        lowPass *= pow(filterVec.x * filterVec.y * filterVec.z, 0.25);
        highPass *= pow(filterVec.y * filterVec.z * filterVec.w, 0.25);
        bandPass *= pow(filterVec.x * filterVec.y * filterVec.w, 0.125);
        bandReject *= pow(filterVec.y * filterVec.z * filterVec.w, 0.5);
        vec3 feedbackLayer = lowPass + highPass + bandPass + bandReject;
        feedbackLayer.r *= oscillate(normalized, parameterValues[count].x * parameterValues[count].y, parameterValues[count].z * parameterValues[count].w, parameterValues[count + 8].x * parameterValues[count + 8].y, parameterValues[count + 8].z * parameterValues[count + 8].w);
        feedbackLayer.g *= oscillate(normalized, parameterValues[count].x * parameterValues[count].z, parameterValues[count].y * parameterValues[count].w, parameterValues[count + 8].x * parameterValues[count + 8].z, parameterValues[count + 8].y * parameterValues[count + 8].w);
        feedbackLayer.b *= oscillate(normalized, parameterValues[count].x * parameterValues[count].z, parameterValues[count].y * parameterValues[count].z, parameterValues[count + 8].x * parameterValues[count + 8].w, parameterValues[count + 8].y * parameterValues[count + 8].w);
        float secondaryLayer = (feedbackLayer.r * feedbackLayer.g) + (feedbackLayer.r * feedbackLayer.b) + (feedbackLayer.g * feedbackLayer.b) / 3.0;
        float primaryLayer = 1.0 - secondaryLayer;
        float bandLayer = pow(secondaryLayer * primaryLayer, 0.5);
        float notchLayer = 1.0 - bandLayer;
        feedbackLayer *= (secondaryLayer * parameterValues[count].y) + (primaryLayer * parameterValues[count].x) + (bandLayer * parameterValues[count].z) + (notchLayer * parameterValues[count].w);
        feedbackColor += feedbackLayer / amplitude;
    }
    feedbackColor = vec3(processColor(feedbackColor.r), processColor(feedbackColor.g), processColor(feedbackColor.b));
    vec3 newColor = vec3(sin(phase), sin(phase + (TWO_PI / 3.0)), sin(phase + (2.0 * TWO_PI / 3.0)));
    newColor *= mix(normalized.x * filterVec.x * filterVec.w, normalized.y * filterVec.y * filterVec.z, 0.5);
    outputColor = vec4(mix(newColor, feedbackColor, abs(feedbackColor.r * feedbackColor.g * feedbackColor.b - 0.75)), 1.0);
}