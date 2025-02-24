/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "tr_local.h"

shaderProgram_t	interactionShader;
shaderProgram_t	shadowShader;
shaderProgram_t	defaultShader;
shaderProgram_t	depthFillShader;
shaderProgram_t cubemapShader; //k: skybox shader
shaderProgram_t reflectionCubemapShader; //k: reflection shader
shaderProgram_t	depthFillClipShader; //k: z-fill clipped shader
shaderProgram_t	fogShader; //k: fog shader
shaderProgram_t	blendLightShader; //k: blend light shader
shaderProgram_t	interactionBlinnPhongShader; //k: BLINN-PHONG lighting model interaction shader
shaderProgram_t diffuseCubemapShader; //k: diffuse cubemap shader
shaderProgram_t texgenShader; //k: texgen shader

#include "glsl_shader.h"
#define HARM_INTERACTION_SHADER_PHONG "phong"
#define HARM_INTERACTION_SHADER_BLINNPHONG "blinn_phong"
const char *harm_r_lightModelArgs[]	= { HARM_INTERACTION_SHADER_PHONG, HARM_INTERACTION_SHADER_BLINNPHONG, NULL };
static idCVar harm_r_lightModel("harm_r_lightModel", harm_r_lightModelArgs[0], CVAR_RENDERER|CVAR_ARCHIVE, "[Harmattan]: Light model when draw interactions(`phong` - Phong(default), `blinn_phong` - Blinn-Phong.)", harm_r_lightModelArgs, idCmdSystem::ArgCompletion_String<harm_r_lightModelArgs>);
static idCVar harm_r_specularExponent("harm_r_specularExponent", "4.0", CVAR_FLOAT|CVAR_RENDERER|CVAR_ARCHIVE, "[Harmattan]: Specular exponent in interaction light model(default is 4.0.)");
#if 0
#define _GLPROGS "gl2progs"
#else
#define _GLPROGS "glslprogs"
#endif
static idCVar	harm_r_shaderProgramDir("harm_r_shaderProgramDir", "", CVAR_SYSTEM | CVAR_INIT | CVAR_SERVERINFO, "[Harmattan]: Special external GLSL shader program directory path(default is empty, means using `" _GLPROGS "`).");

static bool R_CreateShaderProgram(shaderProgram_t *shaderProgram, const char *vert, const char *frag , const char *name);

/*
=========================================================================================

GENERAL INTERACTION RENDERING

=========================================================================================
*/

/*
====================
GL_SelectTextureNoClient
====================
*/
static void GL_SelectTextureNoClient(int unit)
{
	backEnd.glState.currenttmu = unit;
	glActiveTexture(GL_TEXTURE0 + unit);
	RB_LogComment("glActiveTexture( %i )\n", unit);
}

/*
==================
RB_GLSL_DrawInteraction
==================
*/
void	RB_GLSL_DrawInteraction(const drawInteraction_t *din)
{
	static const float zero[4] = { 0, 0, 0, 0 };
	static const float one[4] = { 1, 1, 1, 1 };
	static const float negOne[4] = { -1, -1, -1, -1 };

	// load all the vertex program parameters
	GL_UniformMatrix4fv(offsetof(shaderProgram_t, textureMatrix), mat4_identity.ToFloatPtr());
	GL_Uniform4fv(offsetof(shaderProgram_t, localLightOrigin), din->localLightOrigin.ToFloatPtr());
	GL_Uniform4fv(offsetof(shaderProgram_t, localViewOrigin), din->localViewOrigin.ToFloatPtr());
	GL_Uniform4fv(offsetof(shaderProgram_t, lightProjectionS), din->lightProjection[0].ToFloatPtr());
	GL_Uniform4fv(offsetof(shaderProgram_t, lightProjectionT), din->lightProjection[1].ToFloatPtr());
	GL_Uniform4fv(offsetof(shaderProgram_t, lightProjectionQ), din->lightProjection[2].ToFloatPtr());
	GL_Uniform4fv(offsetof(shaderProgram_t, lightFalloff), din->lightProjection[3].ToFloatPtr());
	GL_Uniform4fv(offsetof(shaderProgram_t, bumpMatrixS), din->bumpMatrix[0].ToFloatPtr());
	GL_Uniform4fv(offsetof(shaderProgram_t, bumpMatrixT), din->bumpMatrix[1].ToFloatPtr());
	GL_Uniform4fv(offsetof(shaderProgram_t, diffuseMatrixS), din->diffuseMatrix[0].ToFloatPtr());
	GL_Uniform4fv(offsetof(shaderProgram_t, diffuseMatrixT), din->diffuseMatrix[1].ToFloatPtr());
	GL_Uniform4fv(offsetof(shaderProgram_t, specularMatrixS), din->specularMatrix[0].ToFloatPtr());
	GL_Uniform4fv(offsetof(shaderProgram_t, specularMatrixT), din->specularMatrix[1].ToFloatPtr());

	switch (din->vertexColor) {
		case SVC_IGNORE:
			GL_Uniform4fv(offsetof(shaderProgram_t, colorModulate), zero);
			GL_Uniform4fv(offsetof(shaderProgram_t, colorAdd), one);
			break;
		case SVC_MODULATE:
			GL_Uniform4fv(offsetof(shaderProgram_t, colorModulate), one);
			GL_Uniform4fv(offsetof(shaderProgram_t, colorAdd), zero);
			break;
		case SVC_INVERSE_MODULATE:
			GL_Uniform4fv(offsetof(shaderProgram_t, colorModulate), negOne);
			GL_Uniform4fv(offsetof(shaderProgram_t, colorAdd), one);
			break;
	}

	// set the constant colors
	GL_Uniform4fv(offsetof(shaderProgram_t, diffuseColor), din->diffuseColor.ToFloatPtr());
	GL_Uniform4fv(offsetof(shaderProgram_t, specularColor), din->specularColor.ToFloatPtr());

	// material may be NULL for shadow volumes
	float f;
	//k: dynamic setup user specular exponent by cvar
	f = harm_r_specularExponent.GetFloat();
	if(f <= 0.0f)

	switch (din->surf->material->GetSurfaceType()) {
		case SURFTYPE_METAL:
#if !defined(_HUMANHEAD)
		case SURFTYPE_RICOCHET:
#endif
			f = 4.0f;
			break;
		case SURFTYPE_STONE:
		case SURFTYPE_FLESH:
		case SURFTYPE_WOOD:
		case SURFTYPE_CARDBOARD:
		case SURFTYPE_LIQUID:
#if !defined(_HUMANHEAD)
		case SURFTYPE_GLASS:
		case SURFTYPE_PLASTIC:
#endif
		case SURFTYPE_NONE:
		default:
			f = 4.0f;
			break;
	}
	GL_Uniform1fv(offsetof(shaderProgram_t, specularExponent), &f);

	// set the textures

	// texture 0 will be the per-surface bump map
	GL_SelectTextureNoClient(0);
	din->bumpImage->Bind();

	// texture 1 will be the light falloff texture
	GL_SelectTextureNoClient(1);
	din->lightFalloffImage->Bind();

	// texture 2 will be the light projection texture
	GL_SelectTextureNoClient(2);
	din->lightImage->Bind();

	// texture 3 is the per-surface diffuse map
	GL_SelectTextureNoClient(3);
	din->diffuseImage->Bind();

	// texture 4 is the per-surface specular map
	GL_SelectTextureNoClient(4);
	din->specularImage->Bind();

	// draw it
	RB_DrawElementsWithCounters(din->surf->geo);
}


/*
=============
RB_GLSL_CreateDrawInteractions

=============
*/
void RB_GLSL_CreateDrawInteractions(const drawSurf_t *surf)
{
	if (!surf) {
		return;
	}

	// perform setup here that will be constant for all interactions
	GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | 
#if 1 //k: fix translucent interactions
			GLS_DEPTHMASK | 
#endif
			backEnd.depthFunc);

	// bind the vertex and fragment shader
	//k: dynamic choose blinn-phong / phong lighting model shader
	const char *lightModel = harm_r_lightModel.GetString();
	if(lightModel && !idStr::Icmp(HARM_INTERACTION_SHADER_BLINNPHONG, lightModel))
		GL_UseProgram(&interactionBlinnPhongShader);
	else
		GL_UseProgram(&interactionShader);

	// enable the vertex arrays
	GL_EnableVertexAttribArray(offsetof(shaderProgram_t, attr_TexCoord));
	GL_EnableVertexAttribArray(offsetof(shaderProgram_t, attr_Tangent));
	GL_EnableVertexAttribArray(offsetof(shaderProgram_t, attr_Bitangent));
	GL_EnableVertexAttribArray(offsetof(shaderProgram_t, attr_Normal));
	GL_EnableVertexAttribArray(offsetof(shaderProgram_t, attr_Vertex));	// gl_Vertex
	GL_EnableVertexAttribArray(offsetof(shaderProgram_t, attr_Color));	// gl_Color

	// texture 5 is the specular lookup table
	GL_SelectTextureNoClient(5);
	globalImages->specularTableImage->Bind();

	for (; surf ; surf=surf->nextOnLight) {
		// perform setup here that will not change over multiple interaction passes

		// set the modelview matrix for the viewer
		float   mat[16];
		myGlMultMatrix(surf->space->modelViewMatrix, backEnd.viewDef->projectionMatrix, mat);
		GL_UniformMatrix4fv(offsetof(shaderProgram_t, modelViewProjectionMatrix), mat);

		// set the vertex pointers
		idDrawVert	*ac = (idDrawVert *)vertexCache.Position(surf->geo->ambientCache);

		GL_VertexAttribPointer(offsetof(shaderProgram_t, attr_Normal), 3, GL_FLOAT, false, sizeof(idDrawVert), ac->normal.ToFloatPtr());
		GL_VertexAttribPointer(offsetof(shaderProgram_t, attr_Bitangent), 3, GL_FLOAT, false, sizeof(idDrawVert), ac->tangents[1].ToFloatPtr());
		GL_VertexAttribPointer(offsetof(shaderProgram_t, attr_Tangent), 3, GL_FLOAT, false, sizeof(idDrawVert), ac->tangents[0].ToFloatPtr());
		GL_VertexAttribPointer(offsetof(shaderProgram_t, attr_TexCoord), 2, GL_FLOAT, false, sizeof(idDrawVert), ac->st.ToFloatPtr());

		GL_VertexAttribPointer(offsetof(shaderProgram_t, attr_Vertex), 3, GL_FLOAT, false, sizeof(idDrawVert), ac->xyz.ToFloatPtr());
		GL_VertexAttribPointer(offsetof(shaderProgram_t, attr_Color), 4, GL_UNSIGNED_BYTE, false, sizeof(idDrawVert), ac->color);

		// this may cause RB_GLSL_DrawInteraction to be exacuted multiple
		// times with different colors and images if the surface or light have multiple layers
		RB_CreateSingleDrawInteractions(surf, RB_GLSL_DrawInteraction);
	}

	GL_DisableVertexAttribArray(offsetof(shaderProgram_t, attr_TexCoord));
	GL_DisableVertexAttribArray(offsetof(shaderProgram_t, attr_Tangent));
	GL_DisableVertexAttribArray(offsetof(shaderProgram_t, attr_Bitangent));
	GL_DisableVertexAttribArray(offsetof(shaderProgram_t, attr_Normal));
	GL_DisableVertexAttribArray(offsetof(shaderProgram_t, attr_Vertex));	// gl_Vertex
	GL_DisableVertexAttribArray(offsetof(shaderProgram_t, attr_Color));	// gl_Color

	// disable features
	GL_SelectTextureNoClient(5);
	globalImages->BindNull();

	GL_SelectTextureNoClient(4);
	globalImages->BindNull();

	GL_SelectTextureNoClient(3);
	globalImages->BindNull();

	GL_SelectTextureNoClient(2);
	globalImages->BindNull();

	GL_SelectTextureNoClient(1);
	globalImages->BindNull();

	backEnd.glState.currenttmu = -1;
	GL_SelectTexture(0);

	GL_UseProgram(NULL);
}

/*
==================
RB_GLSL_DrawInteractions
==================
*/
void RB_GLSL_DrawInteractions(void)
{
	viewLight_t		*vLight;
	const idMaterial	*lightShader;

	GL_SelectTexture(0);
	/*
	GL_DisableVertexAttribArray(offsetof(shaderProgram_t, attr_TexCoord));
	*/

	//
	// for each light, perform adding and shadowing
	//
	for (vLight = backEnd.viewDef->viewLights ; vLight ; vLight = vLight->next) 
	{
		backEnd.vLight = vLight;

		// do fogging later
		if (vLight->lightShader->IsFogLight()) {
			continue;
		}

		if (vLight->lightShader->IsBlendLight()) {
			continue;
		}

		if (!vLight->localInteractions && !vLight->globalInteractions
		    && !vLight->translucentInteractions) {
			continue;
		}

		lightShader = vLight->lightShader;

		// clear the stencil buffer if needed
		if ((vLight->globalShadows || vLight->localShadows)&&(r_shadows.GetBool())) {
			backEnd.currentScissor = vLight->scissorRect;

			if (r_useScissor.GetBool()) {
				glScissor(backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1,
				          backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
				          backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
				          backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1);
			}
			glClear(GL_STENCIL_BUFFER_BIT);
		} else {
			// no shadows, so no need to read or write the stencil buffer
			// we might in theory want to use GL_ALWAYS instead of disabling
			// completely, to satisfy the invarience rules
			if (r_shadows.GetBool())
			glStencilFunc(GL_ALWAYS, 128, 255);
		}
		GL_UseProgram(&shadowShader);
		RB_StencilShadowPass(vLight->globalShadows);
		RB_GLSL_CreateDrawInteractions(vLight->localInteractions);
		GL_UseProgram(&shadowShader);
		RB_StencilShadowPass(vLight->localShadows);
		RB_GLSL_CreateDrawInteractions(vLight->globalInteractions);
		GL_UseProgram(NULL);	// if there weren't any globalInteractions, it would have stayed on
		// translucent surfaces never get stencil shadowed
		if (r_skipTranslucent.GetBool()) {
			continue;
		}
		if (r_shadows.GetBool())
		glStencilFunc(GL_ALWAYS, 128, 255);

		backEnd.depthFunc = GLS_DEPTHFUNC_LESS;
		RB_GLSL_CreateDrawInteractions(vLight->translucentInteractions);

		backEnd.depthFunc = GLS_DEPTHFUNC_EQUAL;
	}

	// disable stencil shadow test
	if (r_shadows.GetBool())
	glStencilFunc(GL_ALWAYS, 128, 255);

	GL_SelectTexture(0);
	/*
	GL_EnableVertexAttribArray(offsetof(shaderProgram_t, attr_TexCoord));
	*/
}

//===================================================================================


/*
=================
R_LoadGLSLShader

loads GLSL vertex or fragment shaders
=================
*/
static void R_LoadGLSLShader(const char *name, shaderProgram_t *shaderProgram, GLenum type)
{
#if 0
	idStr	fullPath = "gl2progs/";
	fullPath += name;
#else	
	idStr	fullPath = cvarSystem->GetCVarString("harm_r_shaderProgramDir");
	if(fullPath.IsEmpty())
		fullPath = _GLPROGS;
	fullPath.AppendPath(name);
#endif
	char	*fileBuffer;
	char	*buffer;

	common->Printf("%s", fullPath.c_str());

	// load the program even if we don't support it, so
	// fs_copyfiles can generate cross-platform data dumps
	fileSystem->ReadFile(fullPath.c_str(), (void **)&fileBuffer, NULL);

	if (!fileBuffer) {
		common->Printf(": File not found\n");
		return;
	}

	// copy to stack memory and free
	buffer = (char *)_alloca(strlen(fileBuffer) + 1);
	strcpy(buffer, fileBuffer);
	fileSystem->FreeFile(fileBuffer);

	if (!glConfig.isInitialized) {
		return;
	}

	switch (type) {
		case GL_VERTEX_SHADER:
			// create vertex shader
			shaderProgram->vertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(shaderProgram->vertexShader, 1, (const GLchar **)&buffer, 0);
			glCompileShader(shaderProgram->vertexShader);
#ifdef _HARM_SHADER_NAME
			strncpy(shaderProgram->name, name, sizeof(shaderProgram->name));
#endif
			break;
		case GL_FRAGMENT_SHADER:
			// create fragment shader
			shaderProgram->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(shaderProgram->fragmentShader, 1, (const GLchar **)&buffer, 0);
			glCompileShader(shaderProgram->fragmentShader);
			break;
		default:
			common->Printf("R_LoadGLSLShader: no type\n");
			return;
	}

	common->Printf("\n");
}

/*
=================
R_LinkGLSLShader

links the GLSL vertex and fragment shaders together to form a GLSL program
=================
*/
static bool R_LinkGLSLShader(shaderProgram_t *shaderProgram, bool needsAttributes)
{
	char buf[BUFSIZ];
	int len;
	GLint status;
	GLint linked;

	shaderProgram->program = glCreateProgram();

	glAttachShader(shaderProgram->program, shaderProgram->vertexShader);
	glAttachShader(shaderProgram->program, shaderProgram->fragmentShader);

	if (needsAttributes) {
		glBindAttribLocation(shaderProgram->program, 8, "attr_TexCoord");
		glBindAttribLocation(shaderProgram->program, 9, "attr_Tangent");
		glBindAttribLocation(shaderProgram->program, 10, "attr_Bitangent");
		glBindAttribLocation(shaderProgram->program, 11, "attr_Normal");
		glBindAttribLocation(shaderProgram->program, 12, "attr_Vertex");
		glBindAttribLocation(shaderProgram->program, 13, "attr_Color");
	}

	glLinkProgram(shaderProgram->program);

	glGetProgramiv(shaderProgram->program, GL_LINK_STATUS, &linked);

	if (com_developer.GetBool()) {
		glGetShaderInfoLog(shaderProgram->vertexShader, sizeof(buf), &len, buf);
		common->Printf("VS:\n%.*s\n", len, buf);
		glGetShaderInfoLog(shaderProgram->fragmentShader, sizeof(buf), &len, buf);
		common->Printf("FS:\n%.*s\n", len, buf);
	}

	if (!linked) {
		//k: if load external shader fail, now will using internal shader, so do not call Error
#if 0
		common->Error("R_LinkGLSLShader: program failed to link\n");
#else
		common->Printf("R_LinkGLSLShader: program failed to link\n");
		glGetProgramInfoLog(shaderProgram->program, sizeof(buf), NULL, buf);
		common->Printf("R_LinkGLSLShader:\n%.*s\n", len, buf);
#endif
		return false;
	}

	return true;
}

/*
=================
R_ValidateGLSLProgram

makes sure GLSL program is valid
=================
*/
static bool R_ValidateGLSLProgram(shaderProgram_t *shaderProgram)
{
	GLint validProgram;

	glValidateProgram(shaderProgram->program);

	glGetProgramiv(shaderProgram->program, GL_VALIDATE_STATUS, &validProgram);

	if (!validProgram) {
		common->Printf("R_ValidateGLSLProgram: program invalid\n");
		return false;
	}

	return true;
}


static void RB_GLSL_GetUniformLocations(shaderProgram_t *shader)
{
	int	i;
	char	buffer[32];

	GL_UseProgram(shader);

	shader->localLightOrigin = glGetUniformLocation(shader->program, "u_lightOrigin");
	shader->localViewOrigin = glGetUniformLocation(shader->program, "u_viewOrigin");
	shader->lightProjectionS = glGetUniformLocation(shader->program, "u_lightProjectionS");
	shader->lightProjectionT = glGetUniformLocation(shader->program, "u_lightProjectionT");
	shader->lightProjectionQ = glGetUniformLocation(shader->program, "u_lightProjectionQ");
	shader->lightFalloff = glGetUniformLocation(shader->program, "u_lightFalloff");
	shader->bumpMatrixS = glGetUniformLocation(shader->program, "u_bumpMatrixS");
	shader->bumpMatrixT = glGetUniformLocation(shader->program, "u_bumpMatrixT");
	shader->diffuseMatrixS = glGetUniformLocation(shader->program, "u_diffuseMatrixS");
	shader->diffuseMatrixT = glGetUniformLocation(shader->program, "u_diffuseMatrixT");
	shader->specularMatrixS = glGetUniformLocation(shader->program, "u_specularMatrixS");
	shader->specularMatrixT = glGetUniformLocation(shader->program, "u_specularMatrixT");
	shader->colorModulate = glGetUniformLocation(shader->program, "u_colorModulate");
	shader->colorAdd = glGetUniformLocation(shader->program, "u_colorAdd");
	shader->diffuseColor = glGetUniformLocation(shader->program, "u_diffuseColor");
	shader->specularColor = glGetUniformLocation(shader->program, "u_specularColor");
	shader->glColor = glGetUniformLocation(shader->program, "u_glColor");
	shader->alphaTest = glGetUniformLocation(shader->program, "u_alphaTest");
	shader->specularExponent = glGetUniformLocation(shader->program, "u_specularExponent");

	shader->eyeOrigin = glGetUniformLocation(shader->program, "u_eyeOrigin");
	shader->localEyeOrigin = glGetUniformLocation(shader->program, "u_localEyeOrigin");
	shader->nonPowerOfTwo = glGetUniformLocation(shader->program, "u_nonPowerOfTwo");
	shader->windowCoords = glGetUniformLocation(shader->program, "u_windowCoords");

	shader->modelViewProjectionMatrix = glGetUniformLocation(shader->program, "u_modelViewProjectionMatrix");

	shader->modelMatrix = glGetUniformLocation(shader->program, "u_modelMatrix");
	shader->textureMatrix = glGetUniformLocation(shader->program, "u_textureMatrix");
	//k: add modelView matrix uniform
	shader->modelViewMatrix = glGetUniformLocation(shader->program, "u_modelViewMatrix");
	//k: add clip plane uniform
	shader->clipPlane = glGetUniformLocation(shader->program, "u_clipPlane");
	//k: add fog matrix uniform
	shader->fogMatrix = glGetUniformLocation(shader->program, "u_fogMatrix");
	//k: add fog color uniform
	shader->fogColor = glGetUniformLocation(shader->program, "u_fogColor");
	//k: add texgen S T Q uniform
	shader->texgenS = glGetUniformLocation(shader->program, "u_texgenS");
	shader->texgenT = glGetUniformLocation(shader->program, "u_texgenT");
	shader->texgenQ = glGetUniformLocation(shader->program, "u_texgenQ");

	shader->attr_TexCoord = glGetAttribLocation(shader->program, "attr_TexCoord");
	shader->attr_Tangent = glGetAttribLocation(shader->program, "attr_Tangent");
	shader->attr_Bitangent = glGetAttribLocation(shader->program, "attr_Bitangent");
	shader->attr_Normal = glGetAttribLocation(shader->program, "attr_Normal");
	shader->attr_Vertex = glGetAttribLocation(shader->program, "attr_Vertex");
	shader->attr_Color = glGetAttribLocation(shader->program, "attr_Color");

	for (i = 0; i < MAX_VERTEX_PARMS; i++) {
		idStr::snPrintf(buffer, sizeof(buffer), "u_vertexParm%d", i);
		shader->u_vertexParm[i] = glGetAttribLocation(shader->program, buffer);
	}

	for (i = 0; i < MAX_FRAGMENT_IMAGES; i++) {
		idStr::snPrintf(buffer, sizeof(buffer), "u_fragmentMap%d", i);
		shader->u_fragmentMap[i] = glGetUniformLocation(shader->program, buffer);
		glUniform1i(shader->u_fragmentMap[i], i);
	}

	//k: add cubemap texture units
	for ( i = 0; i < MAX_FRAGMENT_IMAGES; i++ ) {
		idStr::snPrintf(buffer, sizeof(buffer), "u_fragmentCubeMap%d", i);
		shader->u_fragmentCubeMap[i] = glGetUniformLocation(shader->program, buffer);
		glUniform1i(shader->u_fragmentCubeMap[i], i);
	}

	GL_CheckErrors();

	GL_UseProgram(NULL);
}

static bool RB_GLSL_InitShaders(void)
{
	memset(&interactionShader, 0, sizeof(shaderProgram_t));
	memset(&shadowShader, 0, sizeof(shaderProgram_t));
	memset(&defaultShader, 0, sizeof(shaderProgram_t));
	memset(&depthFillShader, 0, sizeof(shaderProgram_t));
	//k: add new shaders
	memset(&cubemapShader, 0, sizeof(shaderProgram_t));
	memset(&reflectionCubemapShader, 0, sizeof(shaderProgram_t));
	memset(&depthFillClipShader, 0, sizeof(shaderProgram_t));
	memset(&fogShader, 0, sizeof(shaderProgram_t));
	memset(&blendLightShader, 0, sizeof(shaderProgram_t));
	memset(&interactionBlinnPhongShader, 0, sizeof(shaderProgram_t));
	memset(&diffuseCubemapShader, 0, sizeof(shaderProgram_t));
	memset(&texgenShader, 0, sizeof(shaderProgram_t));

	// load interation shaders
	R_LoadGLSLShader("interaction.vert", &interactionShader, GL_VERTEX_SHADER);
	if (!glConfig.textureCompressionAvailable)
	{
	R_LoadGLSLShader("interaction_etc.frag", &interactionShader, GL_FRAGMENT_SHADER);
	}
	else
	{
	R_LoadGLSLShader("interaction.frag", &interactionShader, GL_FRAGMENT_SHADER);
	}

	if (!R_LinkGLSLShader(&interactionShader, true) && !R_ValidateGLSLProgram(&interactionShader)) {
		const char *frag = !glConfig.textureCompressionAvailable ? INTERACTION_ETC_FRAG : INTERACTION_FRAG;
		if(R_CreateShaderProgram(&interactionShader, INTERACTION_VERT, frag, !glConfig.textureCompressionAvailable ? "interaction_etc" : "interaction"))
		{
			common->Printf("[Harmattan]: RB_GLSL_InitShaders(%s) -> using internal\n", !glConfig.textureCompressionAvailable ? "interaction_etc" : "interaction");
		}
		else
		return false;
	} else {
		RB_GLSL_GetUniformLocations(&interactionShader);
	}

	// load stencil shadow extrusion shaders
	R_LoadGLSLShader("shadow.vert", &shadowShader, GL_VERTEX_SHADER);
	R_LoadGLSLShader("shadow.frag", &shadowShader, GL_FRAGMENT_SHADER);

	if (!R_LinkGLSLShader(&shadowShader, true) && !R_ValidateGLSLProgram(&shadowShader)) {
		if(R_CreateShaderProgram(&shadowShader, SHADOW_VERT, SHADOW_FRAG, "shadow"))
		{
			common->Printf("[Harmattan]: RB_GLSL_InitShaders(shadow) -> using internal\n");
		}
		else
		return false;
	} else {
		RB_GLSL_GetUniformLocations(&shadowShader);
	}

	// load default interation shaders
	R_LoadGLSLShader("default.vert", &defaultShader, GL_VERTEX_SHADER);
	R_LoadGLSLShader("default.frag", &defaultShader, GL_FRAGMENT_SHADER);

	if (!R_LinkGLSLShader(&defaultShader, true) && !R_ValidateGLSLProgram(&defaultShader)) {
		if(R_CreateShaderProgram(&defaultShader, DEFAULT_VERT, DEFAULT_FRAG, "diffuseMap"))
		{
			common->Printf("[Harmattan]: RB_GLSL_InitShaders(diffuseMap) -> using internal\n");
		}
		else
		return false;
	} else {
		RB_GLSL_GetUniformLocations(&defaultShader);
	}

	// load default interation shaders
	R_LoadGLSLShader("zfill.vert", &depthFillShader, GL_VERTEX_SHADER);
	R_LoadGLSLShader("zfill.frag", &depthFillShader, GL_FRAGMENT_SHADER);

	if (!R_LinkGLSLShader(&depthFillShader, true) && !R_ValidateGLSLProgram(&depthFillShader)) {
		if(R_CreateShaderProgram(&depthFillShader, ZFILL_VERT, ZFILL_FRAG, "zfill"))
		{
			common->Printf("[Harmattan]: RB_GLSL_InitShaders(zfill) -> using internal\n");
		}
		else
		return false;
	} else {
		RB_GLSL_GetUniformLocations(&depthFillShader);
	}

	//k: load new shaders
	//k: load cubemap shader
	R_LoadGLSLShader("cubemap.vert", &cubemapShader, GL_VERTEX_SHADER);
	R_LoadGLSLShader("cubemap.frag", &cubemapShader, GL_FRAGMENT_SHADER);

	if (!R_LinkGLSLShader(&cubemapShader, true) && !R_ValidateGLSLProgram(&cubemapShader)) {
		if(R_CreateShaderProgram(&cubemapShader, CUBEMAP_VERT, CUBEMAP_FRAG, "cubemap"))
		{
			common->Printf("[Harmattan]: RB_GLSL_InitShaders(cubemap) -> using internal\n");
		}
		else
		return false;
	} else {
		RB_GLSL_GetUniformLocations(&cubemapShader);
	}

	//k: load reflection shader
	R_LoadGLSLShader("reflectionCubemap.vert", &reflectionCubemapShader, GL_VERTEX_SHADER);
	R_LoadGLSLShader("reflectionCubemap.frag", &reflectionCubemapShader, GL_FRAGMENT_SHADER);

	if (!R_LinkGLSLShader(&reflectionCubemapShader, true) && !R_ValidateGLSLProgram(&reflectionCubemapShader)) {
		if(R_CreateShaderProgram(&reflectionCubemapShader, REFLECTION_CUBEMAP_VERT, CUBEMAP_FRAG, "reflectionCubemap"))
		{
			common->Printf("[Harmattan]: RB_GLSL_InitShaders(reflectionCubemap) -> using internal\n");
		}
		else
		return false;
	} else {
		RB_GLSL_GetUniformLocations(&reflectionCubemapShader);
	}

	//k: load z-fill clipped shader
	R_LoadGLSLShader("zfillClip.vert", &depthFillClipShader, GL_VERTEX_SHADER);
	R_LoadGLSLShader("zfillClip.frag", &depthFillClipShader, GL_FRAGMENT_SHADER);

	if (!R_LinkGLSLShader(&depthFillClipShader, true) && !R_ValidateGLSLProgram(&depthFillClipShader)) {
		if(R_CreateShaderProgram(&depthFillClipShader, ZFILLCLIP_VERT, ZFILLCLIP_FRAG, "zfillClip"))
		{
			common->Printf("[Harmattan]: RB_GLSL_InitShaders(zfillClip) -> using internal\n");
		}
		else
		return false;
	} else {
		RB_GLSL_GetUniformLocations(&depthFillClipShader);
	}

	//k: load fog shader
	R_LoadGLSLShader("fog.vert", &fogShader, GL_VERTEX_SHADER);
	R_LoadGLSLShader("fog.frag", &fogShader, GL_FRAGMENT_SHADER);

	if (!R_LinkGLSLShader(&fogShader, true) && !R_ValidateGLSLProgram(&fogShader)) {
		if(R_CreateShaderProgram(&fogShader, FOG_VERT, FOG_FRAG, "fog"))
		{
			common->Printf("[Harmattan]: RB_GLSL_InitShaders(fog) -> using internal\n");
		}
		else
		return false;
	} else {
		RB_GLSL_GetUniformLocations(&fogShader);
	}

	//k: load blend light shader
	R_LoadGLSLShader("blendLight.vert", &blendLightShader, GL_VERTEX_SHADER);
	R_LoadGLSLShader("blendLight.frag", &blendLightShader, GL_FRAGMENT_SHADER);

	if (!R_LinkGLSLShader(&blendLightShader, true) && !R_ValidateGLSLProgram(&blendLightShader)) {
		if(R_CreateShaderProgram(&blendLightShader, BLENDLIGHT_VERT, FOG_FRAG, "blendLight"))
		{
			common->Printf("[Harmattan]: RB_GLSL_InitShaders(blendLight) -> using internal\n");
		}
		else
		return false;
	} else {
		RB_GLSL_GetUniformLocations(&blendLightShader);
	}

	//k: load BLINN-PHONG lighting model interaction shader
	R_LoadGLSLShader("interaction_blinnphong.vert", &interactionBlinnPhongShader, GL_VERTEX_SHADER);
	if (!glConfig.textureCompressionAvailable)
		R_LoadGLSLShader("interaction_blinnphong_etc.frag", &interactionBlinnPhongShader, GL_FRAGMENT_SHADER);
	else
		R_LoadGLSLShader("interaction_blinnphong.frag", &interactionBlinnPhongShader, GL_FRAGMENT_SHADER);

	if (!R_LinkGLSLShader(&interactionBlinnPhongShader, true) && !R_ValidateGLSLProgram(&interactionBlinnPhongShader)) {
		const char *frag = !glConfig.textureCompressionAvailable ? INTERACTION_BLINNPHONG_ETC_FRAG : INTERACTION_BLINNPHONG_FRAG;
		if(R_CreateShaderProgram(&interactionBlinnPhongShader, INTERACTION_BLINNPHONG_VERT, frag, !glConfig.textureCompressionAvailable ? "interaction_blinn_phong_etc" : "interaction_blinn_phong"))
		{
			common->Printf("[Harmattan]: RB_GLSL_InitShaders(%s) -> using internal\n", !glConfig.textureCompressionAvailable ? "interaction_blinn_phong_etc" : "interaction_blinn_phong");
		}
		else
		return false;
	} else {
		RB_GLSL_GetUniformLocations(&interactionBlinnPhongShader);
	}

	//k: load diffuse cubemap shader
	R_LoadGLSLShader("diffuseCubemap.vert", &diffuseCubemapShader, GL_VERTEX_SHADER);
	R_LoadGLSLShader("diffuseCubemap.frag", &diffuseCubemapShader, GL_FRAGMENT_SHADER);

	if (!R_LinkGLSLShader(&diffuseCubemapShader, true) && !R_ValidateGLSLProgram(&diffuseCubemapShader)) {
		if(R_CreateShaderProgram(&diffuseCubemapShader, DIFFUSE_CUBEMAP_VERT, CUBEMAP_FRAG, "diffuseCubemap"))
		{
			common->Printf("[Harmattan]: RB_GLSL_InitShaders(diffuseCubemap) -> using internal\n");
		}
		else
		return false;
	} else {
		RB_GLSL_GetUniformLocations(&diffuseCubemapShader);
	}

	//k: load texgen shader
	R_LoadGLSLShader("texgen.vert", &texgenShader, GL_VERTEX_SHADER);
	R_LoadGLSLShader("texgen.frag", &texgenShader, GL_FRAGMENT_SHADER);

	if (!R_LinkGLSLShader(&texgenShader, true) && !R_ValidateGLSLProgram(&texgenShader)) {
		if(R_CreateShaderProgram(&texgenShader, TEXGEN_VERT, TEXGEN_FRAG, "texgen"))
		{
			common->Printf("[Harmattan]: RB_GLSL_InitShaders(texgen) -> using internal\n");
		}
		else
		return false;
	} else {
		RB_GLSL_GetUniformLocations(&texgenShader);
	}
	return true;
}

/*
==================
R_ReloadGLSLPrograms_f
==================
*/
void R_ReloadGLSLPrograms_f(const idCmdArgs &args)
{
	int		i;

	common->Printf("----- R_ReloadGLSLPrograms -----\n");

	if (!RB_GLSL_InitShaders()) {
		common->Printf("GLSL shaders failed to init.\n");
	}

	glConfig.allowGLSLPath = true;

	common->Printf("-------------------------------\n");
}

void R_GLSL_Init(void)
{
	glConfig.allowGLSLPath = false;

	common->Printf("---------- R_GLSL_Init ----------\n");

	if (!glConfig.GLSLAvailable) {
		common->Printf("Not available.\n");
		return;
	}

	common->Printf("Available.\n");

	common->Printf("---------------------------------\n");
}



static void R_DeleteShaderProgram(shaderProgram_t *shaderProgram)
{
	if(shaderProgram->program)
	{
		if(glIsProgram(shaderProgram->program))
			glDeleteProgram(shaderProgram->program);
	}

	if(shaderProgram->vertexShader)
	{
		if(glIsShader(shaderProgram->vertexShader))
			glDeleteShader(shaderProgram->vertexShader);
	}

	if(shaderProgram->fragmentShader)
	{
		if(glIsShader(shaderProgram->fragmentShader))
			glDeleteShader(shaderProgram->fragmentShader);
	}
	memset(shaderProgram, 0, sizeof(shaderProgram_t));
}

#define LOG_LEN 1024
static GLint R_CreateShader(GLenum type, const char *source)
{
	GLint shader = 0;
	GLint status;
	
	shader = glCreateShader(type);
	if(shader == 0)
	{
		common->Error("[Harmattan]: %s::glCreateShader(%s) error!\n", __func__, type == GL_VERTEX_SHADER ? "GL_VERTEX_SHADER" : "GL_FRAGMENT_SHADER");
		return 0;
	}

	glShaderSource(shader, 1, (const GLchar **)&source, 0);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if(!status)
	{
		GLchar log[LOG_LEN];
		glGetShaderInfoLog(shader, sizeof(GLchar) * LOG_LEN, NULL, log);
		common->Error("[Harmattan]: %s::glCompileShader(%s) -> %s!\n", __func__, type == GL_VERTEX_SHADER ? "GL_VERTEX_SHADER" : "GL_FRAGMENT_SHADER", log);
		glDeleteShader(shader);
		shader = 0;
	}

	return shader;
}

static GLint R_CreateProgram(GLint vertShader, GLint fragShader, bool needsAttributes = true)
{
	GLint program = 0;
	GLint result;

	program = glCreateProgram();
	if(program == 0)
	{
		common->Error("[Harmattan]: %s::glCreateProgram() error!\n", __func__);
		return 0;
	}

	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);

	if(needsAttributes)
	{
		glBindAttribLocation(program, 8, "attr_TexCoord");
		glBindAttribLocation(program, 9, "attr_Tangent");
		glBindAttribLocation(program, 10, "attr_Bitangent");
		glBindAttribLocation(program, 11, "attr_Normal");
		glBindAttribLocation(program, 12, "attr_Vertex");
		glBindAttribLocation(program, 13, "attr_Color");
	}

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if(!result)
	{
		GLchar log[LOG_LEN];
		glGetProgramInfoLog(program, sizeof(GLchar) * LOG_LEN, NULL, log);
		common->Error("[Harmattan]: %s::glLinkProgram() -> %s!\n", __func__, log);
		glDeleteProgram(program);
		program = 0;
	}

	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &result);
	if(!result)
	{
		GLchar log[LOG_LEN];
		glGetProgramInfoLog(program, sizeof(GLchar) * LOG_LEN, NULL, log);
		common->Error("[Harmattan]: %s::glValidateProgram() -> %s!\n", __func__, log);
		glDeleteProgram(program);
		program = 0;
	}

	return program;
}

bool R_CreateShaderProgram(shaderProgram_t *shaderProgram, const char *vert, const char *frag , const char *name)
{
	R_DeleteShaderProgram(shaderProgram);
	shaderProgram->vertexShader = R_CreateShader(GL_VERTEX_SHADER, vert);
	if(shaderProgram->vertexShader == 0)
		return false;

	shaderProgram->fragmentShader = R_CreateShader(GL_FRAGMENT_SHADER, frag);
	if(shaderProgram->fragmentShader == 0)
	{
		R_DeleteShaderProgram(shaderProgram);
		return false;
	}

	shaderProgram->program = R_CreateProgram(shaderProgram->vertexShader, shaderProgram->fragmentShader);
	if(shaderProgram->program == 0)
	{
		R_DeleteShaderProgram(shaderProgram);
		return false;
	}

	RB_GLSL_GetUniformLocations(shaderProgram);
#ifdef _HARM_SHADER_NAME
	strncpy(shaderProgram->name, name, sizeof(shaderProgram->name));
#else
	(void)(name);
#endif

	return true;
}

