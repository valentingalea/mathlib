#include "../../vml/vector.h"
#include "../../vml/matrix.h"

using  vec4 = vml::vector<float, 0, 1, 2, 3>;
using  vec3 = vml::vector<float, 0, 1, 2>;
using  vec2 = vml::vector<float, 0, 1>;
using   _01 = vml::indices_pack<0, 1>;
using  _012 = vml::indices_pack<0, 1, 2>;
using _0123 = vml::indices_pack<0, 1, 2, 3>;
using  mat2 = vml::matrix<float, vml::vector, _01, _01>;
using  mat3 = vml::matrix<float, vml::vector, _012, _012>;
using  mat4 = vml::matrix<float, vml::vector, _0123, _0123>;

namespace sandbox { // to isolate against std:: funcs potentially found by ADL (ex: min/max)

#include "../../vml/vector_functions.h"

namespace ref
{
	typedef vec2& vec2;
	typedef vec3& vec3;
	typedef vec4& vec4;
}

namespace in
{
	typedef const ::vec2& vec2;
	typedef const ::vec3& vec3;
	typedef const ::vec4& vec4;
}

struct fragment_shader
{
	vec2 gl_FragCoord;
	vec4 gl_FragColor;
	void mainImage(vec4 &fragColor, vec2 fragCoord);
};

#define uniform extern
#define in
#define out ref::
#define inout ref::
#define mainImage fragment_shader::mainImage

// verbatim from Shadertoy.com
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iTime;                 // shader playback time (in seconds)
uniform float     iTimeDelta;            // render time (in seconds)
uniform int       iFrame;                // shader playback frame
uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform vec4      iDate;                 // (year, month, day, time in seconds)
uniform float     iSampleRate;           // sound sample rate (i.e., 44100)
float &			  iGlobalTime = iTime;	 // old name

/***** SHADERBOX *************************************************************/
#if defined(APP_EGG)
#include <app_egg.h>
#elif defined(APP_RAYTRACER)
#include <app_raytracer.h>
#elif defined(APP_SDF_AO)
#include <app_sdf_ao.h>
#elif defined(APP_CLOUDS)
//#include <app_clouds.h>
#include <app_clouds_best.h>
#elif defined(APP_ATMOSPHERE)
#include <app_atmosphere.h>
#elif defined(APP_2D)
#include <app_2d.h>
#elif defined(APP_PLANET)
#include <app_planet.h>
#elif defined(APP_FUNC)
#include <app_func.h>
#elif defined(APP_VINYL)
#include <app_vinyl.h>
#else
#include "ref/default.h"
#endif
/*****************************************************************************/

#undef in
#undef out
#undef inout
#undef uniform
#undef main

} // namespace sandbox