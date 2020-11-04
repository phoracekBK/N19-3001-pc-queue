#include <stdio.h>
#include <stdlib.h>
#include <CUnit/CUnit.h>
#include <CUnit/Console.h>


int main(int argv, char ** argc)
{
	 if (CUE_SUCCESS != CU_initialize_registry())
		 return CU_get_error();



	 CU_console_run_tests();
	 CU_cleanup_registry();

	 return 0;
}
