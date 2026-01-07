This aims to approximate an image using shapes

so a picture will be made out of rectangles, triagles, circles, ...

a python file will generate a config code, each bit is a valid shape.

it should support jpeg, png. export as png.

it should be a gpu compute shader, cause it will use random numbers for scale, position.

the color should be the average of the pixels a shape covers that it is in the actual image, so i dot enerate random numbers.

Alpha should depend on the shape. NOT RNG.
