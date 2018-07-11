#include "win_service.h"

int main(int argc, char *argv[])
{
	if (argc > 1 && strcmp(argv[1], "-install") == 0) {
		WinService::Instance().InstallService();
	}
	else if (argc > 1 && strcmp(argv[1], "-uninstall") == 0) {
		WinService::Instance().UninstallService();
	}
	else if (argc > 1 && (strcmp(argv[1], "-console") == 0 || strcmp(argv[1], "-cli") == 0)) {
		WinService::Instance().MainProcess(argc, argv);
	}
	else {
		WinService::Instance().RunService();
	}
	return 0;
}