---
comments: true
---

# Qt Screen Color Picker

- Repository: https://github.com/Italink/ColorPicker.git

![img](../../../../05-OpenSource/Qt/Resources/c42c52b30c698cbbd842c6c8f6167e12.gif)

## Program Details

- Automatically adjusts the magnifier display position
- Automatically switches the border color

## Principles

- First, cover the entire screen with a fully transparent borderless window (here I use the color (255, 255, 255) with transparency set to 1 (range [0, 255])).
- Update the mouse position in real time via a timer or thread. Note: Do not use mouse move events, as color picking triggered by mouse movement cannot capture colors from dynamic videos.
- Use a screenshot function to capture the entire window. Select a rectangle centered on the mouse position; I chose a size of 10*7, while the magnifier is 100*70, so the magnification factor is 10. Place this rectangular area in the magnifier and select the pixel color at the mouse position.
- The selected pixel color will have deviations due to the transparent window, so the original color needs to be restored using a transparency algorithm.
- Transparency principle: Assume B is the transparent color, a is the transparency, A is the underlying color, and C is the final displayed color. The formula is 255*C = a*B + (255-a)*A.
- To restore the original color, simply solve for A using the formula.
- When the mouse is clicked, close the transparent window and emit the color signal.

## Usage Instructions

Just import the class files and create a ColorPicker instance in the desired window. Since it inherits from QWidget, call the show function to display it.

Obtain the selected color by connecting to the signal `void QColorPicker::colorSelect(const QColor&)`.

Note: Import the connection icon file; otherwise, color picking may not be accurate.

## Configuration Instructions

Located in mousedropper.cpp

``` c++
const QSize winSize(100,100);       //Window size
const int grabInterval=50;          //Refresh frequency
const int magnificationTimes=10;    //Magnification factor
const double split=0.7;             //Split ratio
const int sizeOfMouseIcon=20;       //Mouse icon size Icon material:
```