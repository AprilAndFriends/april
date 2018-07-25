/// @version 5.1

#define MAKE_DESATURATE(color) dot(color.rgb, float3(0.2125, 0.7154, 0.0721))
#define MAKE_SEPIA_R(color) (color.r * 0.393 + color.g * 0.769 + color.b * 0.189)
#define MAKE_SEPIA_G(color) (color.r * 0.349 + color.g * 0.686 + color.b * 0.168)
#define MAKE_SEPIA_B(color) (color.r * 0.272 + color.g * 0.534 + color.b * 0.131)
#define MAKE_SEPIA(color) MAKE_SEPIA_R(color), MAKE_SEPIA_G(color), MAKE_SEPIA_B(color)
#define MAKE_SEPIA_FLOAT3(color) float3(MAKE_SEPIA(color))

