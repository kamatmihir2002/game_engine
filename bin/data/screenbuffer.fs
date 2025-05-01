#version 330 core

in vec2 uv;
uniform sampler2D screentex;
uniform sampler2D depthtex;

vec4 invtex(sampler2D tex, vec2 _uv) {
    return texture(tex, vec2(_uv.x, (1.0 - _uv.y)));
}

float linearize_depth(float d,float zNear,float zFar)
{
    float z_n = 2.0 * d - 1.0;
    return 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}
void main() {
    // gl_FragColor = invtex(screentex, uv);

    float ofs = 0.55;
    vec4 s = vec4(0.0);
    vec4 tex = invtex(screentex, uv);
    float brightness = length(tex);
    const float PI2 = 3.14159 * 0.5;
    for (float i = -1.0; i <= 1.0; i+=0.25) {
        for (float j = -1.0; j <= 1.0; j+=0.25) {
            float coeff = 1.0 + length(vec2(cos(PI2 * i), cos(PI2 * j)));
            
            s += coeff * brightness * invtex(screentex, uv + ofs * 0.02 * vec2(i, j));
        }

    }
    
    const float p = 1.0 / 36.0;
    const float wt = 0.5;
    const float wt2 = 0.7;
    gl_FragColor = wt * s * p + wt2 * invtex(screentex, uv);
}