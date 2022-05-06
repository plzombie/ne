/*
	Файл	: nyan_vis_3dlevel.с

	Описание: Вывод 3d уровней

	История	: 13.03.13	Создан

*/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "nyan_publicapi.h"
#include "nyan_text.h"

#include "nyan_vis_init.h"
#include "nyan_vis_draw.h"
#include "nyan_vis_texture.h"
#include "nyan_vis_3dlevel.h"

#include "nyan_nglapi.h"

/*
	Функция	: nvCreate3dLevel

	Описание: Создаёт уровень. Возвращает 0 - при неудаче, иначе 1. 

	История	: 29.01.13	Создан
*/
N_API unsigned int N_APIENTRY_EXPORT nvCreate3dLevel(wchar_t *filename)
{
	if(!nv_isinit) return 0;
	
	if(nv_draw_state != NV_DRAW_STATE_NO) return 0;
	
	nlPrint(LOG_FDEBUGFORMAT7, F_NVCREATE3DLEVEL, N_FNAME, filename); nlAddTab(1);
	
	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATE3DLEVEL, N_FALSE);
	return 0;
}
