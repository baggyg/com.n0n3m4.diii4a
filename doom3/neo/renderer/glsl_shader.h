#ifndef _KARIN_GLSL_SHADER_H
#define _KARIN_GLSL_SHADER_H

// Unuse C++11 raw string literals for Traditional C++98

// diffuse map
static const char DEFAULT_VERT[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"attribute lowp vec4 attr_Color;\n"
"attribute vec4 attr_TexCoord;\n"
"attribute highp vec4 attr_Vertex;\n"
"\n"
"uniform highp mat4 u_modelViewProjectionMatrix;\n"
"uniform mat4 u_textureMatrix;\n"
"uniform lowp vec4 u_colorAdd;\n"
"uniform lowp vec4 u_colorModulate;\n"
"\n"
"varying vec2 var_TexDiffuse;\n"
"varying lowp vec4 var_Color;\n"
"\n"
"void main(void)\n"
"{\n"
	"var_TexDiffuse = (attr_TexCoord * u_textureMatrix).xy;\n"
"\n"
	"var_Color = (attr_Color / 255.0) * u_colorModulate + u_colorAdd;\n"
"\n"
	"gl_Position = u_modelViewProjectionMatrix * attr_Vertex;\n"
"}\n"
;
static const char DEFAULT_FRAG[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"uniform sampler2D u_fragmentMap0;\n"
"uniform lowp vec4 u_glColor;\n"
"\n"
"varying vec2 var_TexDiffuse;\n"
"varying lowp vec4 var_Color;\n"
"\n"
"void main(void)\n"
"{\n"
	"gl_FragColor = texture2D(u_fragmentMap0, var_TexDiffuse) * u_glColor * var_Color;\n"
"}\n"
;

// shadow
static const char SHADOW_VERT[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"attribute highp vec4 attr_Vertex;\n"
"\n"
"uniform highp mat4 u_modelViewProjectionMatrix;\n"
"uniform lowp vec4 u_glColor;\n"
"uniform vec4 u_lightOrigin;\n"
"\n"
"varying lowp vec4 var_Color;\n"
"\n"
"void main(void)\n"
"{\n"
	"gl_Position = u_modelViewProjectionMatrix * (attr_Vertex.w * u_lightOrigin + attr_Vertex - u_lightOrigin);\n"
"\n"
	"var_Color = u_glColor;\n"
"}\n"
;
static const char SHADOW_FRAG[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision lowp float;\n"
"\n"
"varying lowp vec4 var_Color;\n"
"\n"
"void main(void)\n"
"{\n"
	"gl_FragColor = var_Color;\n"
"}\n"
;

// UNUSED
static const char HEATHAZE_VERT[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"attribute vec3 attr_Bitangent;\n"
"attribute vec3 attr_Normal;\n"
"attribute vec3 attr_Tangent;\n"
"attribute lowp vec4 attr_Color;\n"
"attribute vec4 attr_TexCoord;\n"
"attribute highp vec4 attr_Vertex;\n"
"\n"
"uniform highp mat4 u_modelViewProjectionMatrix;\n"
"\n"
"void main(void)\n"
"{\n"
	"gl_Position = u_modelViewProjectionMatrix * attr_Vertex;\n"
"}\n"
;
static const char HEATHAZE_FRAG[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"uniform sampler2D u_fragmentMap0;\n"
"uniform sampler2D u_fragmentMap1;\n"
"uniform sampler2D u_fragmentMap2;\n"
"uniform sampler2D u_fragmentMap3;\n"
"\n"
"void main(void)\n"
"{\n"
	"//gl_FragColor = texture2D(u_fragmentMap0, vec2(0.5, 0.5));\n"
	"gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
"}\n"
;

// interaction
static const char INTERACTION_VERT[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"//#define BLINN_PHONG\n"
"\n"
"varying vec2 var_TexDiffuse;\n"
"varying vec2 var_TexNormal;\n"
"varying vec2 var_TexSpecular;\n"
"varying vec4 var_TexLight;\n"
"varying lowp vec4 var_Color;\n"
"varying vec3 var_L;\n"
"varying vec3 var_V;\n"
"#if defined(BLINN_PHONG)\n"
"varying vec3 var_H;\n"
"#endif\n"
"\n"
"attribute vec4 attr_TexCoord;\n"
"attribute vec3 attr_Tangent;\n"
"attribute vec3 attr_Bitangent;\n"
"attribute vec3 attr_Normal;\n"
"attribute highp vec4 attr_Vertex;\n"
"attribute lowp vec4 attr_Color;\n"
"\n"
"uniform vec4 u_lightProjectionS;\n"
"uniform vec4 u_lightProjectionT;\n"
"uniform vec4 u_lightFalloff;\n"
"uniform vec4 u_lightProjectionQ;\n"
"uniform lowp vec4 u_colorModulate;\n"
"uniform lowp vec4 u_colorAdd;\n"
"uniform lowp vec4 u_glColor;\n"
"\n"
"uniform vec4 u_lightOrigin;\n"
"uniform vec4 u_viewOrigin;\n"
"\n"
"uniform vec4 u_bumpMatrixS;\n"
"uniform vec4 u_bumpMatrixT;\n"
"uniform vec4 u_diffuseMatrixS;\n"
"uniform vec4 u_diffuseMatrixT;\n"
"uniform vec4 u_specularMatrixS;\n"
"uniform vec4 u_specularMatrixT;\n"
"\n"
"uniform highp mat4 u_modelViewProjectionMatrix;\n"
"\n"
"void main(void)\n"
"{\n"
	"mat3 M = mat3(attr_Tangent, attr_Bitangent, attr_Normal);\n"
"\n"
	"var_TexNormal.x = dot(u_bumpMatrixS, attr_TexCoord);\n"
	"var_TexNormal.y = dot(u_bumpMatrixT, attr_TexCoord);\n"
"\n"
	"var_TexDiffuse.x = dot(u_diffuseMatrixS, attr_TexCoord);\n"
	"var_TexDiffuse.y = dot(u_diffuseMatrixT, attr_TexCoord);\n"
"\n"
	"var_TexSpecular.x = dot(u_specularMatrixS, attr_TexCoord);\n"
	"var_TexSpecular.y = dot(u_specularMatrixT, attr_TexCoord);\n"
"\n"
	"var_TexLight.x = dot(u_lightProjectionS, attr_Vertex);\n"
	"var_TexLight.y = dot(u_lightProjectionT, attr_Vertex);\n"
	"var_TexLight.z = dot(u_lightFalloff, attr_Vertex);\n"
	"var_TexLight.w = dot(u_lightProjectionQ, attr_Vertex);\n"
"\n"
	"vec3 L = u_lightOrigin.xyz - attr_Vertex.xyz;\n"
	"vec3 V = u_viewOrigin.xyz - attr_Vertex.xyz;\n"
"#if defined(BLINN_PHONG)\n"
	"vec3 H = normalize(L) + normalize(V);\n"
"#endif\n"
"\n"
	"var_L = L * M;\n"
	"var_V = V * M;\n"
"#if defined(BLINN_PHONG)\n"
	"var_H = H * M;\n"
"#endif\n"
"\n"
	"var_Color = (attr_Color / 255.0) * u_colorModulate + u_colorAdd;\n"
"\n"
	"gl_Position = u_modelViewProjectionMatrix * attr_Vertex;\n"
"}\n"
;
static const char INTERACTION_FRAG[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"//#define BLINN_PHONG\n"
"\n"
"//#define HALF_LAMBERT\n"
"\n"
"varying vec2 var_TexDiffuse;\n"
"varying vec2 var_TexNormal;\n"
"varying vec2 var_TexSpecular;\n"
"varying vec4 var_TexLight;\n"
"varying lowp vec4 var_Color;\n"
"varying vec3 var_L;\n"
"#if defined(BLINN_PHONG)\n"
"varying vec3 var_H;\n"
"#else\n"
"varying vec3 var_V;\n"
"#endif\n"
"\n"
"uniform vec4 u_diffuseColor;\n"
"uniform vec4 u_specularColor;\n"
"uniform float u_specularExponent;\n"
"\n"
"uniform sampler2D u_fragmentMap0;	/* u_bumpTexture */\n"
"uniform sampler2D u_fragmentMap1;	/* u_lightFalloffTexture */\n"
"uniform sampler2D u_fragmentMap2;	/* u_lightProjectionTexture */\n"
"uniform sampler2D u_fragmentMap3;	/* u_diffuseTexture */\n"
"uniform sampler2D u_fragmentMap4;	/* u_specularTexture */\n"
"uniform sampler2D u_fragmentMap5;	/* u_specularFalloffTexture */\n"
"\n"
"void main(void)\n"
"{\n"
	"//float u_specularExponent = 4.0;\n"
"\n"
	"vec3 L = normalize(var_L);\n"
"#if defined(BLINN_PHONG)\n"
	"vec3 H = normalize(var_H);\n"
	"vec3 N = 2.0 * texture2D(u_fragmentMap0, var_TexNormal.st).agb - 1.0;\n"
"#else\n"
	"vec3 V = normalize(var_V);\n"
	"vec3 N = normalize(2.0 * texture2D(u_fragmentMap0, var_TexNormal.st).agb - 1.0);\n"
"#endif\n"
"\n"
	"float NdotL = clamp(dot(N, L), 0.0, 1.0);\n"
"#if defined(HALF_LAMBERT)\n"
	"NdotL *= 0.5;\n"
	"NdotL += 0.5;\n"
	"NdotL = NdotL * NdotL;\n"
"#endif\n"
"#if defined(BLINN_PHONG)\n"
	"float NdotH = clamp(dot(N, H), 0.0, 1.0);\n"
"#endif\n"
"\n"
	"vec3 lightProjection = texture2DProj(u_fragmentMap2, var_TexLight.xyw).rgb;\n"
	"vec3 lightFalloff = texture2D(u_fragmentMap1, vec2(var_TexLight.z, 0.5)).rgb;\n"
	"vec3 diffuseColor = texture2D(u_fragmentMap3, var_TexDiffuse).rgb * u_diffuseColor.rgb;\n"
	"vec3 specularColor = 2.0 * texture2D(u_fragmentMap4, var_TexSpecular).rgb * u_specularColor.rgb;\n"
"\n"
"#if defined(BLINN_PHONG)\n"
	"float specularFalloff = pow(NdotH, u_specularExponent);\n"
"#else\n"
	"vec3 R = -reflect(L, N);\n"
	"float RdotV = clamp(dot(R, V), 0.0, 1.0);\n"
	"float specularFalloff = pow(RdotV, u_specularExponent);\n"
"#endif\n"
"\n"
	"vec3 color;\n"
	"color = diffuseColor;\n"
	"color += specularFalloff * specularColor;\n"
	"color *= NdotL * lightProjection;\n"
	"color *= lightFalloff;\n"
"\n"
	"gl_FragColor = vec4(color, 1.0) * var_Color;\n"
"}\n"
;
static const char INTERACTION_ETC_FRAG[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"//#define BLINN_PHONG\n"
"\n"
"//#define HALF_LAMBERT\n"
"\n"
"varying vec2 var_TexDiffuse;\n"
"varying vec2 var_TexNormal;\n"
"varying vec2 var_TexSpecular;\n"
"varying vec4 var_TexLight;\n"
"varying lowp vec4 var_Color;\n"
"varying vec3 var_L;\n"
"#if defined(BLINN_PHONG)\n"
"varying vec3 var_H;\n"
"#else\n"
"varying vec3 var_V;\n"
"#endif\n"
"\n"
"uniform vec4 u_diffuseColor;\n"
"uniform vec4 u_specularColor;\n"
"uniform float u_specularExponent;\n"
"\n"
"uniform sampler2D u_fragmentMap0;	/* u_bumpTexture */\n"
"uniform sampler2D u_fragmentMap1;	/* u_lightFalloffTexture */\n"
"uniform sampler2D u_fragmentMap2;	/* u_lightProjectionTexture */\n"
"uniform sampler2D u_fragmentMap3;	/* u_diffuseTexture */\n"
"uniform sampler2D u_fragmentMap4;	/* u_specularTexture */\n"
"uniform sampler2D u_fragmentMap5;	/* u_specularFalloffTexture */\n"
"\n"
"void main(void)\n"
"{\n"
	"//float u_specularExponent = 4.0;\n"
"\n"
	"vec3 L = normalize(var_L);\n"
"#if defined(BLINN_PHONG)\n"
	"vec3 H = normalize(var_H);\n"
	"vec3 N = 2.0 * texture2D(u_fragmentMap0, var_TexNormal.st).rgb - 1.0;\n"
"#else\n"
	"vec3 V = normalize(var_V);\n"
	"vec3 N = normalize(2.0 * texture2D(u_fragmentMap0, var_TexNormal.st).rgb - 1.0);\n"
"#endif\n"
"\n"
	"float NdotL = clamp(dot(N, L), 0.0, 1.0);\n"
"#if defined(HALF_LAMBERT)\n"
	"NdotL *= 0.5;\n"
	"NdotL += 0.5;\n"
	"NdotL = NdotL * NdotL;\n"
"#endif\n"
"#if defined(BLINN_PHONG)\n"
	"float NdotH = clamp(dot(N, H), 0.0, 1.0);\n"
"#endif\n"
"\n"
	"vec3 lightProjection = texture2DProj(u_fragmentMap2, var_TexLight.xyw).rgb;\n"
	"vec3 lightFalloff = texture2D(u_fragmentMap1, vec2(var_TexLight.z, 0.5)).rgb;\n"
	"vec3 diffuseColor = texture2D(u_fragmentMap3, var_TexDiffuse).rgb * u_diffuseColor.rgb;\n"
	"vec3 specularColor = 2.0 * texture2D(u_fragmentMap4, var_TexSpecular).rgb * u_specularColor.rgb;\n"
"\n"
"#if defined(BLINN_PHONG)\n"
	"float specularFalloff = pow(NdotH, u_specularExponent);\n"
"#else\n"
	"vec3 R = -reflect(L, N);\n"
	"float RdotV = clamp(dot(R, V), 0.0, 1.0);\n"
	"float specularFalloff = pow(RdotV, u_specularExponent);\n"
"#endif\n"
"\n"
	"vec3 color;\n"
	"color = diffuseColor;\n"
	"color += specularFalloff * specularColor;\n"
	"color *= NdotL * lightProjection;\n"
	"color *= lightFalloff;\n"
"\n"
	"gl_FragColor = vec4(color, 1.0) * var_Color;\n"
"}\n"
;

// z-fill
static const char ZFILL_VERT[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"attribute vec4 attr_TexCoord;\n"
"attribute highp vec4 attr_Vertex;\n"
"\n"
"uniform mat4 u_textureMatrix;\n"
"uniform highp mat4 u_modelViewProjectionMatrix;\n"
"\n"
"varying vec2 var_TexDiffuse;\n"
"\n"
"void main(void)\n"
"{\n"
"	// var_TexDiffuse = attr_TexCoord.xy; // orig\n"
"	var_TexDiffuse = (attr_TexCoord * u_textureMatrix).xy;\n"
"\n"
"	gl_Position = u_modelViewProjectionMatrix * attr_Vertex;\n"
"}\n"
;
static const char ZFILL_FRAG[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"uniform sampler2D u_fragmentMap0;\n"
"uniform lowp float u_alphaTest;\n"
"uniform lowp vec4 u_glColor;\n"
"\n"
"varying vec2 var_TexDiffuse;\n"
"\n"
"void main(void)\n"
"{\n"
	"if (u_alphaTest > texture2D(u_fragmentMap0, var_TexDiffuse).a) {\n"
		"discard;\n"
	"}\n"
"\n"
	"gl_FragColor = u_glColor;\n"
"}\n"
;

// cubemap
static const char CUBEMAP_VERT[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"attribute lowp vec4 attr_Color;\n"
"attribute vec4 attr_TexCoord;\n"
"attribute highp vec4 attr_Vertex;\n"
  "\n"
"uniform highp mat4 u_modelViewProjectionMatrix;\n"
"uniform mat4 u_textureMatrix;\n"
"uniform lowp vec4 u_colorAdd;\n"
"uniform lowp vec4 u_colorModulate;\n"
"uniform vec4 u_viewOrigin;\n"
"\n"
"varying vec3 var_TexCoord;\n"
"varying lowp vec4 var_Color;\n"
"\n"
"void main(void)\n"
"{\n"
"  var_TexCoord = (attr_TexCoord * u_textureMatrix).xyz;\n"
"\n"
  "var_Color = (attr_Color / 255.0) * u_colorModulate + u_colorAdd;\n"
"\n"
  "gl_Position = u_modelViewProjectionMatrix * attr_Vertex;\n"
"}\n"
;
static const char CUBEMAP_FRAG[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"varying vec3 var_TexCoord;\n"
"varying lowp vec4 var_Color;\n"
"\n"
"uniform samplerCube u_fragmentCubeMap0;\n"
"uniform lowp vec4 u_glColor;\n"
"\n"
"void main(void)\n"
"{\n"
  "gl_FragColor = textureCube(u_fragmentCubeMap0, var_TexCoord) * u_glColor * var_Color;\n"
"}\n"
;

// reflection cubemap
static const char REFLECTION_CUBEMAP_VERT[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"attribute highp vec4 attr_Vertex;\n"
"attribute lowp vec4 attr_Color;\n"
"attribute vec3 attr_TexCoord;\n"
"\n"
"uniform highp mat4 u_modelViewProjectionMatrix;\n"
"uniform mat4 u_modelViewMatrix;\n"
"uniform mat4 u_textureMatrix;\n"
"uniform lowp vec4 u_colorAdd;\n"
"uniform lowp vec4 u_colorModulate;\n"
"\n"
"varying vec3 var_TexCoord;\n"
"varying lowp vec4 var_Color;\n"
"\n"
"void main(void)\n"
"{\n"
  "var_TexCoord = (u_textureMatrix * reflect( normalize( u_modelViewMatrix * attr_Vertex ),\n"
                                            "// This suppose the modelView matrix is orthogonal\n"
                                            "// Otherwise, we should use the inverse transpose\n"
                                            "u_modelViewMatrix * vec4(attr_TexCoord,0.0) )).xyz ;\n"
"\n"
  "var_Color = (attr_Color / 255.0) * u_colorModulate + u_colorAdd;\n"
  "\n"
  "gl_Position = u_modelViewProjectionMatrix * attr_Vertex;\n"
"}\n"
;

// fog
static const char FOG_VERT[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"attribute highp vec4 attr_Vertex;      // input Vertex Coordinates\n"
"\n"
"uniform highp mat4 u_modelViewProjectionMatrix;\n"
"uniform mat4 u_fogMatrix;        // fogPlanes 0, 1, 3 (CATION: not 2!), 2\n"
"\n"
"varying vec2 var_TexFog;         // output Fog TexCoord\n"
"varying vec2 var_TexFogEnter;    // output FogEnter TexCoord\n"
"\n"
"void main(void)\n"
"{\n"
  "gl_Position = u_modelViewProjectionMatrix * attr_Vertex;\n"
"\n"
  "// What will be computed:\n"
  "//\n"
  "// var_TexFog.x      = dot(u_fogMatrix[0], attr_Vertex);\n"
  "// var_TexFog.y      = dot(u_fogMatrix[1], attr_Vertex);\n"
  "// var_TexFogEnter.x = dot(u_fogMatrix[2], attr_Vertex);\n"
  "// var_TexFogEnter.y = dot(u_fogMatrix[3], attr_Vertex);\n"
"\n"
  "// Optimized version:\n"
  "var_TexFog      = vec2(dot(u_fogMatrix[0], attr_Vertex),dot(u_fogMatrix[1], attr_Vertex));\n"
  "var_TexFogEnter = vec2(dot(u_fogMatrix[2], attr_Vertex),dot(u_fogMatrix[3], attr_Vertex));\n"
"}\n"
;
static const char FOG_FRAG[] =
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"varying vec2 var_TexFog;            // input Fog TexCoord\n"
"varying vec2 var_TexFogEnter;       // input FogEnter TexCoord\n"
"\n"
"uniform sampler2D u_fragmentMap0;   // Fog Image\n"
"uniform sampler2D u_fragmentMap1;   // Fog Enter Image\n"
"uniform lowp vec4 u_fogColor;       // Fog Color\n"
"\n"
"void main(void)\n"
"{\n"
  "gl_FragColor = texture2D( u_fragmentMap0, var_TexFog ) * texture2D( u_fragmentMap1, var_TexFogEnter ) * vec4(u_fogColor.rgb, 1.0);\n"
"}\n"
;

// blend light
static const char BLENDLIGHT_VERT[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"attribute highp vec4 attr_Vertex;\n"
"\n"
"uniform highp mat4 u_modelViewProjectionMatrix;\n"
"uniform mat4 u_fogMatrix;\n"
"\n"
"varying vec2 var_TexFog;\n"
"varying vec2 var_TexFogEnter;\n"
"\n"
"void main(void)\n"
"{\n"
  "gl_Position = u_modelViewProjectionMatrix * attr_Vertex;\n"
"\n"
  "// What will be computed:\n"
  "//\n"
  "// vec4 tc;\n"
  "// tc.x = dot( u_fogMatrix[0], attr_Vertex );\n"
  "// tc.y = dot( u_fogMatrix[1], attr_Vertex );\n"
  "// tc.z = 0.0;\n"
  "// tc.w = dot( u_fogMatrix[2], attr_Vertex );\n"
  "// var_TexFog.xy = tc.xy / tc.w;\n"
  "//\n"
  "// var_TexFogEnter.x = dot( u_fogMatrix[3], attr_Vertex );\n"
  "// var_TexFogEnter.y = 0.5;\n"
"\n"
  "// Optimized version:\n"
  "//\n"
  "var_TexFog = vec2(dot( u_fogMatrix[0], attr_Vertex ), dot( u_fogMatrix[1], attr_Vertex )) / dot( u_fogMatrix[2], attr_Vertex );\n"
  "var_TexFogEnter = vec2( dot( u_fogMatrix[3], attr_Vertex ), 0.5 );\n"
"}\n"
;

// z-fill with clip plane
static const char ZFILLCLIP_VERT[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"attribute highp vec4 attr_Vertex;\n"
"attribute vec4 attr_TexCoord;\n"
"\n"
"uniform highp mat4 u_modelViewProjectionMatrix;\n"
"uniform mat4 u_textureMatrix;\n"
"uniform vec4 u_clipPlane;\n"
"\n"
"varying vec2 var_TexDiffuse;\n"
"varying vec2 var_TexClip;\n"
"\n"
"void main(void)\n"
"{\n"
	"var_TexDiffuse = (attr_TexCoord * u_textureMatrix).xy;\n"
"\n"
  "var_TexClip = vec2( dot( u_clipPlane, attr_Vertex), 0.5 );\n"
"\n"
  "gl_Position = u_modelViewProjectionMatrix * attr_Vertex;\n"
"}\n"
;
static const char ZFILLCLIP_FRAG[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"varying vec2 var_TexDiffuse;\n"
"varying vec2 var_TexClip;\n"
"\n"
"uniform sampler2D u_fragmentMap0;\n"
"uniform sampler2D u_fragmentMap1;\n"
"uniform lowp float u_alphaTest;\n"
"uniform lowp vec4 u_glColor;\n"
"\n"
"void main(void)\n"
"{\n"
    "if (u_alphaTest > (texture2D(u_fragmentMap0, var_TexDiffuse).a * texture2D(u_fragmentMap1, var_TexClip).a) ) {\n"
      "discard;\n"
    "}\n"
	"\n"
  "gl_FragColor = u_glColor;\n"
"}\n"
;

// interaction(Blinn-phong)
static const char INTERACTION_BLINNPHONG_VERT[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"#define BLINN_PHONG\n"
"\n"
"varying vec2 var_TexDiffuse;\n"
"varying vec2 var_TexNormal;\n"
"varying vec2 var_TexSpecular;\n"
"varying vec4 var_TexLight;\n"
"varying lowp vec4 var_Color;\n"
"varying vec3 var_L;\n"
"varying vec3 var_V;\n"
"#if defined(BLINN_PHONG)\n"
"varying vec3 var_H;\n"
"#endif\n"
"\n"
"attribute vec4 attr_TexCoord;\n"
"attribute vec3 attr_Tangent;\n"
"attribute vec3 attr_Bitangent;\n"
"attribute vec3 attr_Normal;\n"
"attribute highp vec4 attr_Vertex;\n"
"attribute lowp vec4 attr_Color;\n"
"\n"
"uniform vec4 u_lightProjectionS;\n"
"uniform vec4 u_lightProjectionT;\n"
"uniform vec4 u_lightFalloff;\n"
"uniform vec4 u_lightProjectionQ;\n"
"uniform lowp vec4 u_colorModulate;\n"
"uniform lowp vec4 u_colorAdd;\n"
"uniform lowp vec4 u_glColor;\n"
"\n"
"uniform vec4 u_lightOrigin;\n"
"uniform vec4 u_viewOrigin;\n"
"\n"
"uniform vec4 u_bumpMatrixS;\n"
"uniform vec4 u_bumpMatrixT;\n"
"uniform vec4 u_diffuseMatrixS;\n"
"uniform vec4 u_diffuseMatrixT;\n"
"uniform vec4 u_specularMatrixS;\n"
"uniform vec4 u_specularMatrixT;\n"
"\n"
"uniform highp mat4 u_modelViewProjectionMatrix;\n"
"\n"
"void main(void)\n"
"{\n"
	"mat3 M = mat3(attr_Tangent, attr_Bitangent, attr_Normal);\n"
"\n"
	"var_TexNormal.x = dot(u_bumpMatrixS, attr_TexCoord);\n"
	"var_TexNormal.y = dot(u_bumpMatrixT, attr_TexCoord);\n"
"\n"
	"var_TexDiffuse.x = dot(u_diffuseMatrixS, attr_TexCoord);\n"
	"var_TexDiffuse.y = dot(u_diffuseMatrixT, attr_TexCoord);\n"
"\n"
	"var_TexSpecular.x = dot(u_specularMatrixS, attr_TexCoord);\n"
	"var_TexSpecular.y = dot(u_specularMatrixT, attr_TexCoord);\n"
"\n"
	"var_TexLight.x = dot(u_lightProjectionS, attr_Vertex);\n"
	"var_TexLight.y = dot(u_lightProjectionT, attr_Vertex);\n"
	"var_TexLight.z = dot(u_lightFalloff, attr_Vertex);\n"
	"var_TexLight.w = dot(u_lightProjectionQ, attr_Vertex);\n"
"\n"
	"vec3 L = u_lightOrigin.xyz - attr_Vertex.xyz;\n"
	"vec3 V = u_viewOrigin.xyz - attr_Vertex.xyz;\n"
"#if defined(BLINN_PHONG)\n"
	"vec3 H = normalize(L) + normalize(V);\n"
"#endif\n"
"\n"
	"var_L = L * M;\n"
	"var_V = V * M;\n"
"#if defined(BLINN_PHONG)\n"
	"var_H = H * M;\n"
"#endif\n"
"\n"
	"var_Color = (attr_Color / 255.0) * u_colorModulate + u_colorAdd;\n"
"\n"
	"gl_Position = u_modelViewProjectionMatrix * attr_Vertex;\n"
"}\n"
;
static const char INTERACTION_BLINNPHONG_FRAG[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"#define BLINN_PHONG\n"
"\n"
"//#define HALF_LAMBERT\n"
"\n"
"varying vec2 var_TexDiffuse;\n"
"varying vec2 var_TexNormal;\n"
"varying vec2 var_TexSpecular;\n"
"varying vec4 var_TexLight;\n"
"varying lowp vec4 var_Color;\n"
"varying vec3 var_L;\n"
"#if defined(BLINN_PHONG)\n"
"varying vec3 var_H;\n"
"#else\n"
"varying vec3 var_V;\n"
"#endif\n"
"\n"
"uniform vec4 u_diffuseColor;\n"
"uniform vec4 u_specularColor;\n"
"uniform float u_specularExponent;\n"
"\n"
"uniform sampler2D u_fragmentMap0;	/* u_bumpTexture */\n"
"uniform sampler2D u_fragmentMap1;	/* u_lightFalloffTexture */\n"
"uniform sampler2D u_fragmentMap2;	/* u_lightProjectionTexture */\n"
"uniform sampler2D u_fragmentMap3;	/* u_diffuseTexture */\n"
"uniform sampler2D u_fragmentMap4;	/* u_specularTexture */\n"
"uniform sampler2D u_fragmentMap5;	/* u_specularFalloffTexture */\n"
"\n"
"void main(void)\n"
"{\n"
	"//float u_specularExponent = 4.0;\n"
"\n"
	"vec3 L = normalize(var_L);\n"
"#if defined(BLINN_PHONG)\n"
	"vec3 H = normalize(var_H);\n"
	"vec3 N = 2.0 * texture2D(u_fragmentMap0, var_TexNormal.st).agb - 1.0;\n"
"#else\n"
	"vec3 V = normalize(var_V);\n"
	"vec3 N = normalize(2.0 * texture2D(u_fragmentMap0, var_TexNormal.st).agb - 1.0);\n"
"#endif\n"
"\n"
	"float NdotL = clamp(dot(N, L), 0.0, 1.0);\n"
"#if defined(HALF_LAMBERT)\n"
	"NdotL *= 0.5;\n"
	"NdotL += 0.5;\n"
	"NdotL = NdotL * NdotL;\n"
"#endif\n"
"#if defined(BLINN_PHONG)\n"
	"float NdotH = clamp(dot(N, H), 0.0, 1.0);\n"
"#endif\n"
"\n"
	"vec3 lightProjection = texture2DProj(u_fragmentMap2, var_TexLight.xyw).rgb;\n"
	"vec3 lightFalloff = texture2D(u_fragmentMap1, vec2(var_TexLight.z, 0.5)).rgb;\n"
	"vec3 diffuseColor = texture2D(u_fragmentMap3, var_TexDiffuse).rgb * u_diffuseColor.rgb;\n"
	"vec3 specularColor = 2.0 * texture2D(u_fragmentMap4, var_TexSpecular).rgb * u_specularColor.rgb;\n"
"\n"
"#if defined(BLINN_PHONG)\n"
	"float specularFalloff = pow(NdotH, u_specularExponent);\n"
"#else\n"
	"vec3 R = -reflect(L, N);\n"
	"float RdotV = clamp(dot(R, V), 0.0, 1.0);\n"
	"float specularFalloff = pow(RdotV, u_specularExponent);\n"
"#endif\n"
"\n"
	"vec3 color;\n"
	"color = diffuseColor;\n"
	"color += specularFalloff * specularColor;\n"
	"color *= NdotL * lightProjection;\n"
	"color *= lightFalloff;\n"
"\n"
	"gl_FragColor = vec4(color, 1.0) * var_Color;\n"
"}\n"
;
static const char INTERACTION_BLINNPHONG_ETC_FRAG[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"#define BLINN_PHONG\n"
"\n"
"//#define HALF_LAMBERT\n"
"\n"
"varying vec2 var_TexDiffuse;\n"
"varying vec2 var_TexNormal;\n"
"varying vec2 var_TexSpecular;\n"
"varying vec4 var_TexLight;\n"
"varying lowp vec4 var_Color;\n"
"varying vec3 var_L;\n"
"#if defined(BLINN_PHONG)\n"
"varying vec3 var_H;\n"
"#else\n"
"varying vec3 var_V;\n"
"#endif\n"
"\n"
"uniform vec4 u_diffuseColor;\n"
"uniform vec4 u_specularColor;\n"
"uniform float u_specularExponent;\n"
"\n"
"uniform sampler2D u_fragmentMap0;	/* u_bumpTexture */\n"
"uniform sampler2D u_fragmentMap1;	/* u_lightFalloffTexture */\n"
"uniform sampler2D u_fragmentMap2;	/* u_lightProjectionTexture */\n"
"uniform sampler2D u_fragmentMap3;	/* u_diffuseTexture */\n"
"uniform sampler2D u_fragmentMap4;	/* u_specularTexture */\n"
"uniform sampler2D u_fragmentMap5;	/* u_specularFalloffTexture */\n"
"\n"
"void main(void)\n"
"{\n"
	"//float u_specularExponent = 4.0;\n"
"\n"
	"vec3 L = normalize(var_L);\n"
"#if defined(BLINN_PHONG)\n"
	"vec3 H = normalize(var_H);\n"
	"vec3 N = 2.0 * texture2D(u_fragmentMap0, var_TexNormal.st).rgb - 1.0;\n"
"#else\n"
	"vec3 V = normalize(var_V);\n"
	"vec3 N = normalize(2.0 * texture2D(u_fragmentMap0, var_TexNormal.st).rgb - 1.0);\n"
"#endif\n"
"\n"
	"float NdotL = clamp(dot(N, L), 0.0, 1.0);\n"
"#if defined(HALF_LAMBERT)\n"
	"NdotL *= 0.5;\n"
	"NdotL += 0.5;\n"
	"NdotL = NdotL * NdotL;\n"
"#endif\n"
"#if defined(BLINN_PHONG)\n"
	"float NdotH = clamp(dot(N, H), 0.0, 1.0);\n"
"#endif\n"
"\n"
	"vec3 lightProjection = texture2DProj(u_fragmentMap2, var_TexLight.xyw).rgb;\n"
	"vec3 lightFalloff = texture2D(u_fragmentMap1, vec2(var_TexLight.z, 0.5)).rgb;\n"
	"vec3 diffuseColor = texture2D(u_fragmentMap3, var_TexDiffuse).rgb * u_diffuseColor.rgb;\n"
	"vec3 specularColor = 2.0 * texture2D(u_fragmentMap4, var_TexSpecular).rgb * u_specularColor.rgb;\n"
"\n"
"#if defined(BLINN_PHONG)\n"
	"float specularFalloff = pow(NdotH, u_specularExponent);\n"
"#else\n"
	"vec3 R = -reflect(L, N);\n"
	"float RdotV = clamp(dot(R, V), 0.0, 1.0);\n"
	"float specularFalloff = pow(RdotV, u_specularExponent);\n"
"#endif\n"
"\n"
	"vec3 color;\n"
	"color = diffuseColor;\n"
	"color += specularFalloff * specularColor;\n"
	"color *= NdotL * lightProjection;\n"
	"color *= lightFalloff;\n"
"\n"
	"gl_FragColor = vec4(color, 1.0) * var_Color;\n"
"}\n"
;

// diffuse dubemap(UNUSED in game)
static const char DIFFUSE_CUBEMAP_VERT[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
  "\n"
"// In\n"
"attribute highp vec4 attr_Vertex;\n"
"attribute lowp vec4 attr_Color;\n"
"attribute vec3 attr_TexCoord;\n"
  "\n"
"// Uniforms\n"
"uniform highp mat4 u_modelViewProjectionMatrix;\n"
"uniform mat4 u_textureMatrix;\n"
"uniform lowp float u_colorAdd;\n"
"uniform lowp float u_colorModulate;\n"
  "\n"
"// Out\n"
"// gl_Position\n"
"varying vec3 var_TexCoord;\n"
"varying lowp vec4 var_Color;\n"
  "\n"
"void main(void)\n"
"{\n"
  //"var_TexCoord = (u_textureMatrix * vec4(attr_TexCoord, 0.0)).xyz;\n"
	"var_TexCoord = (vec4(attr_TexCoord, 0.0) * u_textureMatrix).xyz;\n"
"\n"
	"var_Color = (attr_Color / 255.0) * u_colorModulate + u_colorAdd;\n"
"\n"
  "gl_Position = u_modelViewProjectionMatrix * attr_Vertex;\n"
"}\n"
;

// texgen(Only used in D3XP)
static const char TEXGEN_VERT[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"attribute highp vec4 attr_Vertex;\n"
"attribute lowp vec4 attr_Color;\n"
"\n"
"uniform lowp vec4 u_colorAdd;\n"
"uniform lowp vec4 u_colorModulate;\n"
"uniform vec4 u_texgenS;\n"
"uniform vec4 u_texgenT;\n"
"uniform vec4 u_texgenQ;\n"
"uniform mat4 u_textureMatrix;\n"
"uniform highp mat4 u_modelViewProjectionMatrix;\n"
"\n"
"varying vec4 var_TexCoord;\n"
"varying lowp vec4 var_Color;\n"
"\n"
"void main(void)\n"
"{\n"
	"gl_Position = u_modelViewProjectionMatrix * attr_Vertex;\n"
"\n"
	"vec4 texcoord0 = vec4(dot( u_texgenS, attr_Vertex ), dot( u_texgenT, attr_Vertex ), 0.0, dot( u_texgenQ, attr_Vertex )); \n"
"\n"
	"// multiply the texture matrix in\n"
	"var_TexCoord = vec4(dot( u_textureMatrix[0], texcoord0 ), dot( u_textureMatrix[1], texcoord0), texcoord0.z, texcoord0.w);\n"
"\n"
	"// compute vertex modulation\n"
	"var_Color = (attr_Color / 255.0) * u_colorModulate + u_colorAdd;\n"
"}\n"
;
static const char TEXGEN_FRAG[] = 
"#version 100\n"
"//#pragma optimize(off)\n"
"\n"
"precision mediump float;\n"
"\n"
"uniform sampler2D u_fragmentMap0;\n"
"uniform lowp vec4 u_glColor;\n"
"\n"
"varying vec4 var_TexCoord;\n"
"varying lowp vec4 var_Color;\n"
"\n"
"void main(void)\n"
"{\n"
	"// we always do a projective texture lookup so that we can support texgen\n"
	"// materials without a separate shader. Basic materials will have texture\n"
	"// coordinates with w = 1 which will result in a NOP projection when tex2Dproj\n"
	"// gets called.\n"
	"gl_FragColor = texture2DProj( u_fragmentMap0, var_TexCoord.xyw ) * u_glColor * var_Color;\n"
"}\n"
;

#endif
