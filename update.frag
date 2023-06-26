uniform sampler2D texture;
uniform vec2 resolution;
uniform float rowSize;
uniform float colSize;
float loop_mod(float dividend, float divisor);
bool isAlive(float row, float column);
int aliveCount(float row, float column);
vec3 dead=vec3(1,0,0);
void main()
{
    vec2 pixelSize=vec2(1.0,1.0)/resolution;
    // lookup the pixel in the texture
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
    // multiply it by the color
        gl_FragColor = gl_Color * pixel;
}
int aliveCount(float row, float column){
    int aliveCount;
     float r;
     aliveCount=0;
     r=0.0;
    for (r = row - 1.0; (r <= (row + 1.0)); r++)
    {
        float c;
        for ( c = column - 1.0; (c <= (column + 1.0)); c++)
        {
            if (isAlive(r, c) && !(r == row && c == column))
                aliveCount++;
        }
    }
    return aliveCount;
}
bool isAlive(float row, float column)
{
     float adjustedRow = loop_mod(row, rowSize);
     float adjustedCol = loop_mod(column, colSize);
    vec3 pixel = texture2D(texture, vec2(adjustedCol,adjustedRow)).xyz;
    if (pixel==dead)
        return false;
    else
        return true;
}
float loop_mod(float dividend, float divisor)
{
    float result=mod(dividend,divisor);
    if (result < 0.0)
    {
        result += divisor;
    }
    else if (result >= divisor)
    {
        result -= divisor;
    }
    return result;
}