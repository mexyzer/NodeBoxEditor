#include <stdlib.h>
#include <iostream>
#include <irrlicht.h>
#include "util/string.hpp"
#include "common.hpp"
#include "Editor.hpp"

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#ifndef _DEBUG
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif
#endif

#ifdef _WIN32
#include <windows.h>

bool PathExists(const char* path)
{
	return (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES);
}

#else
#include <sys/stat.h>

bool PathExists(const char* path)
{
	struct stat st;
	return (stat(path, &st) == 0);
}

#endif

int main(int argc, char *argv[]) {
	std::cerr <<
		" _   _           _        ____              _____    _ _ _             \n"
		"| \\ | | ___   __| | ___  | __ )  _____  __ | ____|__| (_) |_ ___  _ __ \n"
		"|  \\| |/ _ \\ / _` |/ _ \\ |  _ \\ / _ \\ \\/ / |  _| / _` | | __/ _ \\| '__|\n"
		"| |\\  | (_) | (_| |  __/ | |_) | (_) >  <  | |__| (_| | | || (_) | |   \n"
		"|_| \\_|\\___/ \\__,_|\\___| |____/ \\___/_/\\_\\ |_____\\__,_|_|\\__\\___/|_|   \n\n"
		<< std::endl;

	// Find the working directory
	std::cerr << "Looking for the working directory..." << std::endl;
	if (!PathExists("media/sky.jpg")) {
		chdir("../");
		if (!PathExists("media/sky.jpg")) {
			chdir("share/nodeboxeditor");
			if (!PathExists("media/sky.jpg")) {
				std::cerr << "Can't find the working directory!" << std::endl;
			} else {
				std::cerr << "Setting" << std::endl;
				editor_is_installed = true;
			}
		}
	}

	// Settings
	Configuration* conf = new Configuration();
	if (conf == NULL) {
		return EXIT_FAILURE;
	}

	// Init Settings
	conf->set("snapping", "true");
	conf->set("limiting", "true");
	conf->set("driver", "opengl");
	conf->set("hide_sidebar", "false");
	conf->set("save_directory", "");
	conf->set("always_show_position_handle", "false");
	conf->set("vsync", "true");
	conf->set("use_sleep", "false");
	conf->set("fullscreen", "false");
	conf->set("width", "896");
	conf->set("height", "520");
	if (!editor_is_installed)
		conf->load("editor.conf");
	else
		conf->load("~/.config/nodeboxeditor.conf");

	E_DRIVER_TYPE driv = irr::video::EDT_OPENGL;

	const std::string confDriver = str_to_lower(conf->get("driver"));
	if (confDriver == "directx8") {
		driv = EDT_DIRECT3D8;
	} else if (confDriver == "directx9") {
		driv = EDT_DIRECT3D9;
	} else if (confDriver == "software") {
		driv = EDT_SOFTWARE;
	}

	// Start Irrlicht
	int w = conf->getInt("width");
	int h = conf->getInt("height");

	if (w < 1) w = 896;
	if (h < 1) h = 520;

	if (!conf->getBool("vsync")) {
		std::cerr << "[WARNING] You have disabled vsync. Expect major CPU usage!" << std::endl;
	}
	irr::IrrlichtDevice* device = irr::createDevice(
		driv,
		irr::core::dimension2d<irr::u32>(w,h),
		16U,
		conf->getBool("fullscreen"),
		false,
		conf->getBool("vsync")
	);
	if (device == NULL) {
		return EXIT_FAILURE; // could not create selected driver.
	}

	// Editor
	Editor* editor = new Editor();
	editor->run(device, conf);

	if (editor_is_installed)
		conf->load("editor.conf");
	else
		conf->load("~/.config/nodeboxeditor.conf");

	return 1;
}

// Fix for Inconsistency detected by ld.so
#include <pthread.h>
void junk() {
	int i;
	i = pthread_getconcurrency();
};
