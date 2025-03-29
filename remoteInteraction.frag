#version 150

#define TWO_PI 6.283185307179586476925286766559

uniform sampler2DRect tex0[8];
in vec2 texCoordVarying;
out vec4 outputColor;
uniform vec2 window;
uniform float recipriocalTime;
uniform float amplitude;
uniform vec4 parameterValues[16];
uniform vec4 timerValues[8];
uniform vec4 filterVec;

float beam(float coordinates, float location, float power){
    float adjustedLocation = pow(location, 0.5);
    float scale = 0.5 + abs(0.5 - adjustedLocation);
    return pow((scale - abs(coordinates - location)) * (1.0 / scale), power);
}

float beam2(vec2 normalizedVec, float xLocation, float yLocation, float xPower, float yPower){
    return pow(beam(normalizedVec.x, xLocation, xPower) * beam(normalizedVec.y, yLocation, yPower), 0.5 * (1.0 - (xPower * yPower)));
}

float modQuotient(float valueIn, float moduloIn){
    return mod(valueIn, moduloIn) / moduloIn;
}

float processColor(float unprocessedColor){
    return modQuotient(pow(unprocessedColor, 0.25), 0.5);
}

void main()
{
    float floatCount = 0.0;
    vec4 aggregatePhases = vec4(0.0);
    vec4 aggregateFrequencies = vec4(0.0);
    vec3 feedbackColor = vec3(0.0);
    for(int count = 0; count < 8; count++){
        floatCount++;
        vec4 adjustedParameters = vec4(0.0);
        for(int positionIndex = 0; positionIndex < 4; positionIndex++){
            aggregatePhases[positionIndex] += parameterValues[count][positionIndex];
            adjustedParameters[positionIndex] = pow(parameterValues[count][positionIndex], 1.0 / timerValues[count][positionIndex]);
            aggregateFrequencies[positionIndex] += adjustedParameters[positionIndex];
        }
        vec3 lowPass = texture2DRect(tex0[count], texCoordVarying * parameterValues[count].xw + ((parameterValues[count + 8].xz * 2.0 - 1.0) * window)).rgb;
        vec3 highPass = 1.0 - texture2DRect(tex0[count], texCoordVarying * parameterValues[count].yz + ((parameterValues[count + 8].yw * 2.0 - 1.0) * window)).rgb;
        vec3 bandPass = lowPass * highPass;
        vec3 bandReject = 1.0 - bandPass;
        for(int positionIndex = 0; positionIndex < 3; positionIndex++){
            lowPass[positionIndex] = pow(lowPass[positionIndex], 0.5);
            highPass[positionIndex] = pow(highPass[positionIndex], 0.5);
            bandPass[positionIndex] = pow(bandPass[positionIndex], 0.25);
        }
        lowPass *= filterVec.x;
        highPass *= filterVec.y;
        bandPass *= filterVec.z;
        bandReject *= filterVec.w;
        vec3 feedbackLayer = (lowPass + highPass + bandPass + bandReject);
        feedbackColor += feedbackLayer;
    }
    aggregatePhases /= 8.0;
    aggregateFrequencies /= 8.0;
    feedbackColor = vec3(processColor(feedbackColor.r), processColor(feedbackColor.g), processColor(feedbackColor.b));
    vec2 normalized = gl_FragCoord.xy / window;
    vec2 inverseNormalized = 1.0 - normalized;
    float phase = ((normalized.x * filterVec.x * filterVec.z) + (inverseNormalized.x * filterVec.y * filterVec.w) + (normalized.y * filterVec.z * filterVec.w) + (inverseNormalized.y * filterVec.x * filterVec.y)) * TWO_PI / (1.0 / (pow(amplitude, pow(window.x * window.y, 0.125)) + 1.0));
    vec3 osColor = vec3(sin(phase), sin(phase + (TWO_PI / 3.0)), sin(phase + (2.0 * TWO_PI / 3.0)));
    osColor *= mix(normalized.x * filterVec.x * filterVec.w, normalized.y * filterVec.y * filterVec.z, 0.5);
    vec3 beamColor = vec3(0.0);
    beamColor.r = beam2(normalized, aggregatePhases.x * aggregatePhases.y, aggregatePhases.z * aggregatePhases.w, 1.0 - (aggregateFrequencies.z * aggregateFrequencies.w), 1.0 - (aggregateFrequencies.x * aggregateFrequencies.y));
    beamColor.g = beam2(normalized, aggregatePhases.x * aggregatePhases.z, aggregatePhases.y * aggregatePhases.w, 1.0 - (aggregateFrequencies.y * aggregateFrequencies.w), 1.0 - (aggregateFrequencies.x * aggregateFrequencies.z));
    beamColor.b = beam2(normalized, aggregatePhases.x * aggregatePhases.w, aggregatePhases.y * aggregatePhases.z, 1.0 - (aggregateFrequencies.y * aggregateFrequencies.z), 1.0 - (aggregateFrequencies.x * aggregateFrequencies.w));
    vec3 newColor = osColor * beamColor;
    vec3 color = mix(newColor, feedbackColor, abs(feedbackColor.r * feedbackColor.g * feedbackColor.b - 0.75));
    float inverseAmplitude = 1.0 - amplitude;
    vec3 processedColor = vec3(modQuotient(processColor(color.r), inverseAmplitude), modQuotient(processColor(color.g), inverseAmplitude), modQuotient(processColor(color.b), inverseAmplitude));
    float white = processedColor.r * processedColor.g * processedColor.b;
    float secondary = modQuotient((color.r * color.g) + (color.r * color.b) + (color.g * color.b), recipriocalTime);
    float primary = 1.0 - secondary;
    float colorPallate = mix(secondary, primary, recipriocalTime * inverseAmplitude) - white;
    float red = pow(colorPallate * color.r, 0.03125);
    float green = pow(colorPallate * color.g, 0.03125);
    float blue = pow(colorPallate * color.b, 0.03125);
    outputColor = vec4(secondary * color, 1.0);
}