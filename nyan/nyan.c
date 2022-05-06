
// Тут должен быть libmain
#ifdef N_CAUSEWAY
#include <stdio.h>
int main(int reason)
{
	int result;

	if (!reason) {

		/*
		** DLL initialisation.
		*/
		wprintf(L"DLL startup...\n");

		/* return zero to let the load continue */
		result = 0;

	} else {

		/*
		** DLL clean up.
		*/
		wprintf(L"DLL shutdown...\n");

		result = 1;
	}

	return(result);
}
#endif
