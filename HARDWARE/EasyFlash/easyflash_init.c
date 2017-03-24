#include "easyflash_init.h"
#include "easyflash.h"
#include "sys.h"
#include <stdio.h>
#include <stdlib.h>
#include "usart.h"


void test_env(void) {
    uint32_t i_boot_times = NULL;
    char *c_old_boot_times, c_new_boot_times[11] = {0};

    /* get the boot count number from Env */
    c_old_boot_times = ef_get_env("boot_times");
   // RT_ASSERT(c_old_boot_times);
		if(!c_old_boot_times)
		printf("assert failed \n");
    i_boot_times = atol(c_old_boot_times);
    /* boot count +1 */
    i_boot_times ++;
    printf("The system now boot %d times\n", i_boot_times);
    /* interger to string */
    sprintf(c_new_boot_times,"%ld", i_boot_times);
    /* set and store the boot count number to Env */
    ef_set_env("boot_times", c_new_boot_times);
    ef_save_env();
}
