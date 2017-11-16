#define App_Sys
#define App_Init
#define App_Protocol
#include "App_Libraries.h"
int main(void)
{
	app_init();
	while(1)
	{
		Pro_Communication();
//		__WFI();
	}
}



