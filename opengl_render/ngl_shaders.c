/*
	Файл	: ngl_shaders.c

	Описание: Функции, облегчающие работу с шейдерами

	История	: 15.08.12	Создан

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../extclib/mbstowcsl.h"

#ifdef N_WINDOWS
	#include <windows.h>
#endif
#include <GL/glu.h>
#include "../forks/gl/glext.h"

#include "ngl.h"
#include "ngl_text.h"

#include "ngl_init.h"
#include "ngl_shaders.h"

/*
	Функция	: nglShaderCreateARBVertexProg

	Описание: Загружает вершинный шейдер через ARB_vertex_program

	История	: 10.07.12	Создан

*/
unsigned int nglShaderCreateARBVertexProg(const char *text)
{
	GLuint id;

	nglCatchOpenGLError(F_NGLSHADERCREATEARBVERTEXPROG);// (~_~)

	funcptr_glGenProgramsARB(1, &id);
	funcptr_glBindProgramARB(GL_VERTEX_PROGRAM_ARB, id);
	funcptr_glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(text), text);

	if(nglCatchOpenGLError(F_NGLSHADERCREATEARBVERTEXPROG) == GL_INVALID_OPERATION) {
		GLint errpos;
		wchar_t *errwstr;

		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errpos);

		errwstr = ngl_ea->nAllocMemory(sizeof(wchar_t)*(strlen((char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB))+1));
		if(errwstr) {
			mbstowcsl(errwstr, (char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB), strlen((char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB))+1);
			ngl_ea->nlPrint(ERR_CANTCOMPILESHADER, F_NGLSHADERCREATEARBVERTEXPROG, errwstr, errpos);
			ngl_ea->nFreeMemory(errwstr);
		}

		funcptr_glDeleteProgramsARB(1,&id);

		return 0;
	}
	return id;
}
