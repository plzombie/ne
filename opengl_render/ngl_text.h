/*
	Файл	: ngl_text.h

	Описание: Весь текст

	История	: 04.08.12	Создан

*/

#include "../nyan/nyan_text.h"

#define NGL_MAXVERTEXATTRIBS L"Maximum vertex attribs"
#define NGL_MAXPROGRAMLOCALPARAMS L"Maximum program local params"
#define NGL_MAXPROGRAMENVPARAMS L"Maximum program env params"
#define NGL_MAXPROGRAMMATRICES L"Maximum program matrices"
#define NGL_MAXPROGRAMTEMPORARIES L"Maximum program temporaries"
#define NGL_MAXPROGRAMPARAMS L"Maximum program params"
#define NGL_MAXPROGRAMADDRESSREGS L"Maximum address regs"

#define NGL_SUPPORTNPOT L"Support NPOT textures"
#define NGL_SUPPORTBGRA L"Support BGRA textures"
#define NGL_SUPPORTABGR L"Support ABGR textures"
#define NGL_SUPPORTCMYKA L"Support CMYKA textures"
#define NGL_SUPPORTPP L"Support packed pixels"
#define NGL_SUPPORTVERTEXPROGRAM L"Support vertex programs"
#define NGL_SUPPORTTEXTUREEDGECLAMP L"Support CLAMP_TO_EDGE_EXT"
#define NGL_SUPPORTWGLGETEXTSTR L"Support wglGetExtensionsStringARB"
#define NGL_MAXTEXSIZE L"Max texture size"

#define NGL_TOTALGLERRORS L"Total OpenGL errors"

// Названия функций

#define F_NGLCLEAR L"nglClear()"

#define F_NGLTEXLOADTOGL L"nglTexLoadToGL()"
#define F_NGLTEXUPDATETOGL L"nglTexUpdateToGL()"
#define F_NGLUPDATETEXTURE L"nglUpdateTexture()"

#define F_NGLBATCH2DDRAW L"nglBatch2dDraw()"
#define F_NGLBATCH3DDRAWMESH L"nglBatch3dDrawMesh()"
#define F_NGLBATCH2DBEGIN L"nglBatch2dBegin()"
#define F_NGLBATCH2DEND L"nglBatch2dEnd()"
#define F_NGLBATCHSETMODELVIEWMATRIX L"nglBatchSetModelviewMatrix()"
#define F_NGLBATCH3DSETAMBIENTLIGHT L"nglBatch3dSetAmbientLight()"
#define F_NGLBATCH3DBEGIN L"nglBatch3dBegin()"
#define F_NGLBATCH3DEND L"nglBatch3dEnd()"
#define F_NGLREADSCREEN L"nglReadScreen()"

#define F_NGLSHADERCREATEARBVERTEXPROG L"nglShaderCreateARBVertexProg()"

#define F_NGLCATCHOPENGLERROR L"nglCatchOpenGLError"

#define F_NGLADDSEPARATERCONTEXTIFNEEDED L"nglAddSeparateRContextIfNeeded"

// Сообщения об ошибках

#define ERR_OPENGL L"%ls: OpenGL error %d"

#define ERR_FAILEDTOCHOOSEPIXELFORMAT L"Failed to choose Pixel Format"
#define ERR_FAILEDTOSETPIXELFORMAT L"Failed to set Pixel Format"

#define ERR_COULDNTOPENDISPLAY L"Could not open X display"
#define ERR_FAILEDTOQUERYXF86VIDMODEEXT L"Failed to query XF86VIDMODE extension"
#define ERR_FAILEDTOQUERYGLXEXT L"Failed to query GLX extension"
#define ERR_COULDNTGETVISUAL L"Could not get visual\n"

#define ERR_FAILEDTOCREATERC L"Failed to create Rendering Context"
#define ERR_FAILEDTODELETERC L"Failed to delete Rendering Context"
#define ERR_FAILEDTOMAKECURRENTRC L"Failed to make current Rendering Context"
#define ERR_FAILEDTOCREATESEPARATERC L"Failed to create separate Rendering Contexts"
#define ERR_FAILEDTODELETESEPARATERC L"Failed to delete separate Rendering Contexts"
#define ERR_UNSUPPORTEDNGLCOLORFORMAT L"Unsupported nglcolorformat parameter"

#define ERR_CANTCOMPILESHADER L"%ls: Can't compile shader. Error string '%ls', position %d"

#include "../commonsrc/render/ngl_text_common.h"
