#include "RemoteCtrlService.h"
#include "getopt.h"

int main(int argc,char **argv)  
{
	int option_index;
	int opt;
	char const *shortopt = "d:r:i:h";
	struct option long_options[] = {
		{"daemon", optional_argument, 0, 'b'},
		{"redirect", optional_argument, 0, 'r'},
		{"interval", required_argument, 0, 'i'}
	};
	
	while ((option_index = -1) ,
			   (opt=getopt_long(argc, argv,
					shortopt, long_options,
					&option_index)) != -1) 
	{
		switch(opt)
		{
			case 'd':
				
				printf("daemon %s\n", optarg);
			break;
			case 'r':
				printf("redirect %s\n", optarg);
			break;
			case 'i':
				printf("interval %s\n", optarg);
			break;
		}
	}

	//RemoteCtrlServiceOpen(1);
	//RemoteCtrlServiceClose();

	return 0;
}






