---
comments: true
---

# Qt Desktop Water Ripple Effect

- GitHub Repository: https://github.com/Italink/DesktopWaveEffect.git

<p style="text-align:center"><iframe width="560" height="315" src="//player.bilibili.com/player.html?isOutside=true&aid=753422025&bvid=BV1yk4y1B72b&cid=197938359&p=1&autoplay=false" scrolling="no" border="0" frameborder="0" framespacing="0" allowfullscreen="true"></iframe></p>

![img](../../../../05-OpenSource/Qt/Resources/6c6942fc23e6d499a06203ccb8e73992.png)

![img](../../../../05-OpenSource/Qt/Resources/8289ad0b59540b647ca654aa30e74e5e.png)

## Implementation Tools

- Qt+OpenGL

## Implemented Features
- Water wave simulation: Generate water surface waveforms and complete light refraction calculations for the water surface
- Window embedding into the desktop
- Monitor whether the top-level window occludes the desktop; stop water ripple refreshing if it does
- Global mouse hook

## Principles
### Waveform Generation

![img](../../../../05-OpenSource/Qt/Resources/3341e5ac2f69635732774e4931be900e.png)


The water surface waveform can be approximately regarded as a sine wave with the distance from the vibration source (dis) as the parameter, where A is the maximum amplitude, F is the frequency, and dis is the distance between the current point (xy) and the vibration source. Essentially, this is a binary function. Using such a function, we can construct a surface with the following effect:

![img](../../../../05-OpenSource/Qt/Resources/586c6e1955cabe0fc4ee063b8a512ff3.png)

The half-section is a regular sine function:

![img](../../../../05-OpenSource/Qt/Resources/3c7dc1d9403c5a1a8e4f290f470508a1.png)

Is this the end of the water surface waveform calculation? Obviously not. The water surface waveform is not just a simple sine wave (the blogger also doesn't know the details), but by observing some water surface effects, we can find that water waves often only show a small ring-like waveform that spreads outward over time.

So what do we need to do?

We need to highlight a small segment of the sine wave and suppress other parts to almost zero. This highlighted segment will spread outward over time. We can achieve this effect by changing the maximum amplitude, meaning we need the amplitude to no longer be a constant but a function. Think carefully: what function can meet our needs?

Of course, it's the versatile Gaussian function.

![img](../../../../05-OpenSource/Qt/Resources/03f43d477268d22156de71a9e4cb4852.png)

In the Gaussian function:

a represents the maximum amplitude (peak value)
b represents the axis of symmetry
c is related to the width of the bell shape
When a=1, b=10, c=2, you get the following graph:

![img](../../../../05-OpenSource/Qt/Resources/467cf1d556f30a9694c0cb3a31b9cbf9.png)

With this Gaussian function, we use it as the maximum amplitude of the sine function (i.e., multiply the two):

![img](../../../../05-OpenSource/Qt/Resources/59231a71c5180e6267a135d411c7afe9.png)

Using the previous parameters, we will get:

![img](../../../../05-OpenSource/Qt/Resources/258d7e8ac6d5b6e82d8c349b0135e8f6.png)

Which is approximately the following waveform:

![img](../../../../05-OpenSource/Qt/Resources/ebd7858b760fbf032b0d6c093b153058.png)

To achieve spreading, we need to associate the axis of symmetry of the Gaussian function with time (T). As time passes, the axis of symmetry increases proportionally. We can add another variable V to control the spreading speed:

![img](../../../../05-OpenSource/Qt/Resources/967df372e35dc944ad44f0594aabf745.png)

We also use variable W to control the width of the ripple.

In summary, the surface equation for the water ripple we obtained is:

![img](../../../../05-OpenSource/Qt/Resources/fe7251b6ee301493c7bea2181085d5b1.png)

Where:

- A represents the maximum amplitude
- V represents the spreading speed
- T represents time
- W represents the width coefficient
- F represents the frequency

### Water Surface Refraction Calculation

In this step, we will calculate the actual color that should be output after water surface refraction for a certain pixel on the image. In OpenGL, this part of the code is implemented in the fragment shader (so the previous step is also in the fragment shader).

#### Normal Vector Calculation
We use a half-section of the water wave for demonstration. The line of sight is from top to bottom, and the observation model roughly looks like this (although light travels from water to air to the human eye, for derivation, we reverse it and consider the light as emitted from the human eye):

![img](../../../../05-OpenSource/Qt/Resources/cc5877c23a13299b58a318ab40f6c8c0.png)

We need to calculate which position on the texture image a certain pixel on the image actually corresponds to after water surface refraction.

We need to obtain the direction of the line of sight after refraction through the water surface. To calculate this, we must first get the normal vector of the plane corresponding to the line of sight on the water surface. With the normal vector, we can complete the calculation from the incident direction to the refraction direction.

Since the water surface is constructed using a function, theoretically, we can calculate two tangent vectors by taking partial derivatives of the function and then cross-multiply them to get the normal vector of the point plane. However, the function is actually very complex; after differentiation, the function becomes very large, making calculation difficult. Therefore, we use a clever method to calculate the normal vector of the point plane:

We calculate the heights of two non-collinear points near the point plane to form two direction vectors, then cross-multiply them to calculate the normal vector. Although the data obtained this way is not very precise, it is sufficient for our needs and has acceptable efficiency.

#### Refracted Ray Direction Calculation
GLSL provides a `refract` function (incident vector, normal vector, relative refraction coefficient) to calculate refraction.

Therefore, since we know the incident vector (0, 0, -1) and the normal vector, calculating the refraction vector is very simple—just call the function (the refraction coefficient from water to air is 4/3, approximately 1.33).

#### Coordinate Offset Calculation

![img](../../../../05-OpenSource/Qt/Resources/8dc1957a675635afc54577394646095c.png)

After obtaining the refraction vector, we only need to stretch the refraction vector so that its z-value equals the actual height (water surface height + waveform height) to get the xy offset value after water surface refraction. Adding the original coordinates to the offset coordinates gives the coordinates after water surface refraction. Additionally, since texture coordinates range from [0, 1], we need to normalize the coordinates based on the window width.

### Mission Accomplished!
Here's the fragment shader code implemented by the blogger:

``` glsl
#version 330 core
out vec4 FragColor;
 
uniform sampler2D texture;
 
uniform vec3 data[50];          //data passes (mouse x, mouse y, running time): supports multiple vibration sources
uniform int data_size;          //length of currently valid data
uniform vec2 screen_size;   //screen size
uniform float frequency;    //frequency
uniform float amplitude;    //maximum amplitude
uniform float wave_width;   //wave width
uniform float depth;        //depth of the water plane from the background image
uniform float speed;
 
in vec2 TexCoord;
 
void main()
{
    float height;
    float upHeight;
    float rightHeight;
    for(int i=0;i<data_size;i++){
        float time = data[i].z;    //time after mouse click
        float dis = distance(data[i].xy,gl_FragCoord.xy);  //distance from mouse position to current fragment position
 
        float amplit = amplitude*pow(2.74,-(dis-time*speed)*(dis-time*speed)/2/(wave_width*wave_width))*sin(dis*frequency);      //calculate amplitude of current fragment: use Gaussian function here to highlight the waveform displayed at the current time
 
        height += amplit*sin(dis*frequency);              //height of current waveform
 
        dis=distance(data[i].xy,gl_FragCoord.xy+vec2(0,1));
 
        upHeight += amplitude*pow(2.74,-(dis-time*speed)*(dis-time*speed)/2/(wave_width*wave_width))*sin(dis*frequency)*sin(dis*frequency);
 
        dis=distance(data[i].xy,gl_FragCoord.xy+vec2(1,0));
 
        rightHeight += amplitude*pow(2.74,-(dis-time*speed)*(dis-time*speed)/2/(wave_width*wave_width))*sin(dis*frequency)*sin(dis*frequency);
    }
 
 
    vec3 up = vec3(0,1,upHeight-height);
 
    vec3 right = vec3(1,0,rightHeight-height);
 
    vec3 normal = cross(up,right);
 
    vec3 view = vec3(0,0,-1);
 
    vec3 re = refract(view,normal,1.33);
 
    vec2 coordOffset = re.xy*((height+depth)/re.z)/screen_size;
 
    FragColor = texture2D(texture,TexCoord+coordOffset);
}
```