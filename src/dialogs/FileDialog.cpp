#include "FileDialog.hpp"
#include "../util/string.hpp"
#include "../util/filesys.hpp"
#include "../FileFormat/FileFormat.hpp"
#include "../util/tinyfiledialogs.h"

void FileDialog_open_project(EditorState *state)
{
	std::string path = getSaveLoadDirectory(state->settings->get("save_directory"), state->settings->getBool("installed"));

	const char* filters[] = {"*.nbe"};
	const char *cfile = tinyfd_openFileDialog("Open Project",
			path.c_str(), 1, filters, 0);

	if (!cfile)
		return;

	std::string file = cfile;

	if (file == "")
		return;


	std::cerr << file.c_str() << std::endl;

	// Get file parser
	FileFormat *parser = getFromExt(file, state);
	if (!parser) {
		state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
		L"File format does not exist. (It is based on file extensions)");
		return;
	}

	// Get directory, and load#
	std::cerr << "Reading from " << file << std::endl;
	Project *tmp = parser->read(file);
	if (tmp) {
		if (state->project)
			delete state->project;
			state->project = tmp;
			state->project->SelectNode(0);
			state->Mode()->unload();
			state->menu->init();
			state->Mode()->load();
			delete parser;
			parser = NULL;
			return;
	} else {
		switch(parser->error_code) {
		case EFFE_IO_ERROR:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
				L"Failed to open the file\n\t(Does it not exist, or is it readonly?)");
			break;
		case EFFE_READ_OLD_VERSION:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
				L"This file is outdated and is not supported");
			break;
		case EFFE_READ_NEW_VERSION:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
				L"This file was created with a new version of NBE\n\t(Update your copy)");
			break;
		case EFFE_READ_PARSE_ERROR:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
				L"An error occurred while reading the file - it may be corrupted\n\t(This should never happen)");
			break;
		case EFFE_READ_WRONG_TYPE:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
				L"The file is not in the correct format\n\t(Are you opening the wrong type of file?)");
			break;
		default:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
				L"Unknown error");
			break;
		}
	}
}

void save_file(FileFormat *writer, EditorState *state, std::string file)
{
	if (!writer || !state)
		return;

	std::string after(file);

	if (after.find('.') == std::string::npos) {
		after += '.';
		after += writer->getExtension();
	}

	std::cerr << "Saving to " << after << std::endl;

	if (!writer->write(state->project, after)) {
		if (writer->error_code == EFFE_IO_ERROR) {
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to Save",
					L"Unable to open file to save to");
		} else {
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to Save",
					L"Unknown reason");
		}
	}

	delete writer;
}

void FileDialog_save_project(EditorState *state)
{
	// Get path
	std::string path = getSaveLoadDirectory(state->settings->get("save_directory"), state->settings->getBool("installed"));

	const char* filters[] = {"*.nbe"};
	std::string file = tinyfd_saveFileDialog("Save Project", path.c_str(),
			1, filters);
	std::cerr << file << std::endl;

	if (file == "")
		return;

	FileFormat *writer = getFromType(FILE_FORMAT_NBE, state);
	save_file(writer, state, file);
}

void FileDialog_export(EditorState *state, int parser)
{
	// Get path
	std::string path = getSaveLoadDirectory(state->settings->get("save_directory"), state->settings->getBool("installed"));

	const char* filters[] = {""};

	if (parser == (int)FILE_FORMAT_LUA)
		filters[0] = "*.lua";
	else if (parser == (int)FILE_FORMAT_CPP)
		filters[0] = "*.cpp";

	std::string file = tinyfd_saveFileDialog("Save Project", path.c_str(),
			1, filters);
	std::cerr << file << std::endl;

	if (file == "")
		return;

	FileFormat *writer = getFromType((FileFormatType)parser, state);
	save_file(writer, state, file);
}

void FileDialog_export_textures(EditorState *state)
{
	std::string path = getSaveLoadDirectory(state->settings->get("save_directory"),
			state->settings->getBool("installed"));

	const char *cdir = tinyfd_selectFolderDialog ("Select Folder", path.c_str());

	if (!cdir)
		return;

	// Save images
	std::string dir = trim(cdir);

	if (dir == "")
		return;

	if (*dir.rbegin() != '/')
		dir += "/";

	std::cerr << "Exporting Images to " << dir.c_str() << std::endl;
	CreateDir(dir.c_str());
	Media *media = &state->project->media;
	std::map<std::string, Media::Image*>& images = media->getList();
	for (std::map<std::string, Media::Image*>::const_iterator it = images.begin();
			it != images.end();
			++it) {
		Media::Image *image = it->second;
		if (!image->get()) {
			std::cerr << "Image->get() is NULL!" << std::endl;
			continue;
		}
		state->device->getVideoDriver()->writeImageToFile(image->get(), (dir + image->name).c_str());
	}
}

/*
	} else if (type == EFPT_IMPORT) {
		const char *filters[] = {"*.nbe"};
		const char *cfile = tinyfd_openFileDialog("Import Nodes",
				path.c_str(), 1, filters, 0);

		if (!cfile)
			return;

		std::string file = cfile;

		if (file == "")
			return;

		FileFormat *parser = getFromExt(file, state);
		if (!parser) {
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
			L"File format does not exist.");
			return;
		}

		// Get directory, and load
		std::cerr << "Reading from " << file << std::endl;
		Project *tmp = parser->read(file, state->project);
		if (tmp) {
			state->project->remesh();
		} else {
			switch(parser->error_code) {
			case EFFE_IO_ERROR:
				state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
						L"Failed to open the file\n\t(Does it not exist, or is it readonly?)");
				break;
			case EFFE_READ_OLD_VERSION:
				state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"This file is outdated and is not supported");
				break;
			case EFFE_READ_NEW_VERSION:
				state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"This file was created with a new version of NBE\n\t(Update your copy)");
				break;
			case EFFE_READ_PARSE_ERROR:
				state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"An error occurred while reading the file - it may be corrupted\n\t(This should never happen)");
				break;
			case EFFE_READ_WRONG_TYPE:
				state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"The file is not in the correct format\n\t(Are you opening the wrong type of file?)");
				break;
			default:
				state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"Unknown error");
				break;
			}
			delete parser;
			parser = NULL;
		}

	}
}

bool FileDialog::canClose()
{
	return true;
}

bool FileDialog::close()
{
	state->menu->dialog = NULL;
	delete this;
	return true;
}

void FileDialog::doSave(const SEvent &event)
{

}

void FileDialog::doText(const SEvent &event)
{
	IGUIEnvironment *guienv = state->device->getGUIEnvironment();
	if (!state->project) {
		guienv->addMessageBox(L"Unable to save",
				L"You have not yet opened a project.");
		return;
	}
	IGUIEditBox *box = NULL;
	IGUIComboBox *cb = NULL;
	if (!get_parent_box_cb(state, event, win, &box, &cb)) {
		return;
	}
	FileFormat *writer = getFromType((FileFormatType) cb->getItemData(cb->getSelected()), state);
	if (writer) {
		std::string res = writer->getAsString(state->project);
		if (res == "") {
			guienv->addMessageBox(L"Unable to generate code",
					L"Unknown reason");
		}
		IGUIWindow *dialog = guienv->addWindow(rect<irr::s32>(340, 50, 740, 350), false, L"Code Window");
		IGUIEditBox *db = guienv->addEditBox(narrow_to_wide(res).c_str(), rect<s32>(5, 25, 395, 295), false, dialog);
		db->setMultiLine(true);
		db->setWordWrap(false);
		delete writer;
	} else {
		state->device->getGUIEnvironment()->addMessageBox(L"Unable to save",
				L"File format does not exist.");
	}

	close();
}

void FileDialog::doOpen(const SEvent &event)
{
	IGUIEditBox *box = NULL;
	IGUIComboBox *cb = NULL;
	if (!get_parent_box_cb(state, event, win, &box, &cb)) {
		return;
	}

	// Get file parser
	FileFormat *parser = getFromType((FileFormatType) cb->getItemData(cb->getSelected()), state);
	if (!parser) {
		state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
				L"File format does not exist.");
		return;
	}

	// Get file name
	irr::core::stringc t = box->getText();
	std::string after(t.c_str(), t.size());
	if (after.find('.') == std::string::npos) {
		after += '.';
		after += parser->getExtension();
	}

	// Get directory, and load
	std::string dir = getSaveLoadDirectory(state->settings->get("save_directory"), state->settings->getBool("installed"));
	std::cerr << "Reading from " << dir + after << std::endl;
	Project *tmp = parser->read(dir + after);
	if (tmp) {
		if (state->project)
			delete state->project;

		win->remove();
		win = NULL;
		state->project = tmp;
		state->project->SelectNode(0);
		state->Mode()->unload();
		state->menu->init();
		state->Mode()->load();
		delete parser;
		parser = NULL;
		close();
		return;
	} else {
		switch(parser->error_code) {
		case EFFE_IO_ERROR:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"Failed to open the file\n\t(Does it not exist, or is it readonly?)");
			break;
		case EFFE_READ_OLD_VERSION:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"This file is outdated and is not supported");
			break;
		case EFFE_READ_NEW_VERSION:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"This file was created with a new version of NBE\n\t(Update your copy)");
			break;
		case EFFE_READ_PARSE_ERROR:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"An error occurred while reading the file - it may be corrupted\n\t(This should never happen)");
			break;
		case EFFE_READ_WRONG_TYPE:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"The file is not in the correct format\n\t(Are you opening the wrong type of file?)");
			break;
		default:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"Unknown error");
			break;
		}
		delete parser;
		parser = NULL;
	}
}

void FileDialog::doImport(const SEvent &event)
{

	IGUIEditBox *box = NULL;
	IGUIComboBox *cb = NULL;
	if (!get_parent_box_cb(state, event, win, &box, &cb)) {
		return;
	}

	// Get file parser
	FileFormat *parser = getFromType((FileFormatType) cb->getItemData(cb->getSelected()), state);
	if (!parser) {
		state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
				L"File format does not exist.");
		return;
	}

	// Get file name
	irr::core::stringc t = box->getText();
	std::string after(t.c_str(), t.size());
	if (after.find('.') == std::string::npos) {
		after += '.';
		after += parser->getExtension();
	}

	// Get directory, and load
	std::string dir = getSaveLoadDirectory(state->settings->get("save_directory"), state->settings->getBool("installed"));
	std::cerr << "Reading from " << dir + after << std::endl;
	Project *tmp = parser->read(dir + after, state->project);
	if (tmp) {
		state->project->remesh();
		close();
	} else {
		switch(parser->error_code) {
		case EFFE_IO_ERROR:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"Failed to open the file\n\t(Does it not exist, or is it readonly?)");
			break;
		case EFFE_READ_OLD_VERSION:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"This file is outdated and is not supported");
			break;
		case EFFE_READ_NEW_VERSION:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"This file was created with a new version of NBE\n\t(Update your copy)");
			break;
		case EFFE_READ_PARSE_ERROR:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"An error occurred while reading the file - it may be corrupted\n\t(This should never happen)");
			break;
		case EFFE_READ_WRONG_TYPE:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"The file is not in the correct format\n\t(Are you opening the wrong type of file?)");
			break;
		default:
			state->device->getGUIEnvironment()->addMessageBox(L"Unable to open",
					L"Unknown error");
			break;
		}
		delete parser;
		parser = NULL;
	}
}
*/