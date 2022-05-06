// stdbool.h for MVSC prior to 2013, compatible with higher versions
/*
Copyright (c) 2018 Mikhail Morozov

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
*/

#ifndef STDBOOL_H
#define STDBOOL_H

	#ifndef __cplusplus
		#if __STDC_VERSION__ >= 199901L || _MSC_VER >= 1800
			#define bool _Bool
			//#error _Bool used
		#else
			#define bool char
			//#error char used
		#endif

		#define true 1
		#define false 0
	#endif // __cplusplus
		
	#define __bool_true_false_are_defined 1

#endif // STDBOOL_H
