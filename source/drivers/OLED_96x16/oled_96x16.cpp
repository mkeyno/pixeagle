
// OLED runtime driver module.


#include "OLED_96x16.hpp"
#include <px4_platform_common/getopt.h>
#include <px4_platform_common/log.h>
#include <px4_platform_common/module.h>


static OLED_96x16 *g_instance = nullptr;

static void usage(){
	PRINT_MODULE_DESCRIPTION(
								R"DESCR_STR(
								Description
								Driver for the OLED_96x16 display.
								Examples
								$ oled_96x16 start -b 1 -a 0x3c
								$ oled_96x16 status
								$ oled_96x16 test -t "Test Text"
								$ oled_96x16 stop
								)DESCR_STR");
	PRINT_MODULE_USAGE_NAME("oled_96x16", "driver");
	PRINT_MODULE_USAGE_COMMAND("start");
	PRINT_MODULE_USAGE_PARAM_INT('b', 1, 1, PX4_I2C_BUS_MAX_BUS_ITEMS, "I2C bus", false);
	PRINT_MODULE_USAGE_PARAM_INT('a', OLED_I2C_ADDR, 0x08, 0x7F, "I2C address", false);
	PRINT_MODULE_USAGE_PARAM_FLAG('X', "External bus", true);
	PRINT_MODULE_USAGE_COMMAND("status");
	PRINT_MODULE_USAGE_COMMAND("test");
	PRINT_MODULE_USAGE_PARAM_STRING('t', nullptr, nullptr, "Text to display", false);
	PRINT_MODULE_USAGE_COMMAND("stop");
}

extern "C" __EXPORT int oled_96x16_main(int argc, char *argv[]){
	
	int 		myoptind = 1;
	int 		ch;
	const char 	*myoptarg = nullptr;
	int 		bus = PX4_I2C_BUS_EXPANSION; // Default to external
	int 		addr = OLED_I2C_ADDR;
	bool 		external = true;
	
	while ((ch = px4_getopt(argc, argv, "b:a:X", &myoptind, &myoptarg)) != EOF) {
				switch (ch) {
							case 'b':				bus = strtol(myoptarg, nullptr, 10);				break;
							case 'a':				addr = strtol(myoptarg, nullptr, 16);				break;
							case 'X':				external = true;				break;
							default:				usage();				return -1;
							}
				}
	
	if (myoptind >= argc) {
							usage();
							return -1;
							}
	const char *verb = argv[myoptind];
	
	if (!strcmp(verb, "start")) {
									if (g_instance != nullptr) {
																	PX4_WARN("Already started");
																	return -1;
																	}
									int device_bus = external ? -bus : bus;
									g_instance = new OLED_96x16(device_bus, 400000, addr);
									if (g_instance == nullptr) {
																PX4_ERR("Allocation failed");
																return -1;
																}
									if (g_instance->init() != PX4_OK) {
																		PX4_ERR("Init failed");
																		delete g_instance;
																		g_instance = nullptr;
																		return -1;
																		}
									return 0;
									}
	if (!strcmp(verb, "status")) {
									if (g_instance == nullptr) {
									PX4_WARN("Not running");
									return -1;
									}
									g_instance->print_status();
									return 0;
									}
	if (!strcmp(verb, "test")) {
								if (g_instance == nullptr) {
															PX4_WARN("Not running");
															return -1;
															}
								const char *text = "Hello PX4";
								myoptind++;
								while ((ch = px4_getopt(argc, argv, "t:", &myoptind, &myoptarg)) != EOF) {
												switch (ch) {
																case 't':
																text = myoptarg;
																break;
																}
												}
								g_instance->print_auto_wrap(text);
								return 0;
								}
	if (!strcmp(verb, "stop")) {
								if (g_instance == nullptr) {
															PX4_WARN("Not running");
															return -1;
															}
								delete g_instance;
								g_instance = nullptr;
								return 0;
								}
	usage();
	return -1;
}