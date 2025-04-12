#include <unistd.h>
#include <iostream>
#include <ncurses.h>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <system_error>
#include "archives.h"
#define TITLE_LEFT_PADDING 3
#define WHITE_ON_BLUE 1
#define BLUE_ON_BLACK 2

void initInterface() {
	initscr();
	noecho();
	curs_set(0);
	start_color();
	init_pair(WHITE_ON_BLUE, COLOR_WHITE, COLOR_BLUE);
	init_pair(BLUE_ON_BLACK, COLOR_BLUE, COLOR_BLACK);
}

void stopInterface () {
	endwin();
	curs_set(1);
	echo();
}

std::string ljust(std::string text, int len) {
	std::string result;
	for (int i = 0; i < len; i++) {
		if (i < (int) text.length()) {
			result = result + text[i];
		} else {
			result = result + " ";
		}
	}
	
	return result;
}

int makeMenu(std::vector<std::string>& options) {
	int choice = 0;
	keypad(stdscr, TRUE);
	while (true) {
		for (int i = 0; i < (int) options.size(); i++) {
			move(5 + i, 3);
			if (i == choice) {
				attron(A_BOLD);
				attron(COLOR_PAIR(BLUE_ON_BLACK));
				printw("> %s", options[i].c_str());
				attroff(A_BOLD);
				attroff(COLOR_PAIR(BLUE_ON_BLACK));
			} else {
				printw("  %s", options[i].c_str());
			}
		}
		int ch = getch();
		switch (ch) {
			case KEY_UP:
				choice--;
				if (choice < 0) choice = 0;
				break;
			case KEY_DOWN:
				choice++;
				if (choice >= (int) options.size()) choice = (int) options.size() - 1;
				break;
			case 10:
				return choice;
				break;
		}
	}
	return 0;
}

void makeTitle(std::string title = "LSML Main Menu") {
	clear();
	
	move(LINES - 3, 3);
	attron(A_ITALIC);
	printw("Use UP/DOWN arrows to move and ENTER to confirm");
	attroff(A_ITALIC);
	
	move(0, 0);
	attron(COLOR_PAIR(WHITE_ON_BLUE));
	for (int i = 0; i < TITLE_LEFT_PADDING; i++) {
		printw(" ");
	}
	printw(title.c_str());
	for (int i = 0; i < COLS - TITLE_LEFT_PADDING - (int) title.length(); i++) {
		printw(" ");
	}
	attroff(COLOR_PAIR(WHITE_ON_BLUE));
	move(3, 3);
}

void makeProgressBar(int y, int x, std::string title, int progress) {
	if (progress != -1) {
		mvprintw(y, x, "%s (%d/100%%)         ", title.c_str(), progress);
	} else {
		mvprintw(y, x, "%s (waiting)", title.c_str());
	}
	refresh();
}

int dl_progress(void* client, double dltotal, double dlnow, double ultotal, double ulnow) {
	if (dltotal > 0) makeProgressBar(3, 3, "Downloading...", static_cast<int>((dlnow / dltotal) * 100));
	return 0;
}

void newSubsystem() {
	makeTitle("New Subsystem");
	std::vector<std::string> systems = {"Arch Linux", "Debian Bookworm (12)", "Ubuntu Oracular (24.10)", "Alpine", "Cancel"};
	std::vector<std::string> urls = {
		"https://media.githubusercontent.com/media/VladosNX/lsml-rootfs/refs/heads/main/arch.tar.gz",
		"https://media.githubusercontent.com/media/VladosNX/lsml-rootfs/refs/heads/main/debian-bookworm.tar.gz",
		"https://media.githubusercontent.com/media/VladosNX/lsml-rootfs/refs/heads/main/ubuntu.tar.gz",
		"https://media.githubusercontent.com/media/VladosNX/lsml-rootfs/refs/heads/main/alpine.tar.gz"};
	int system = makeMenu(systems);
	if (system == (int) systems.size() - 1) return;
	makeTitle(systems[system]);

	mvprintw(3, 3, "Subsystem name: [                        ]");
	
	curs_set(1);
	echo();
	char name[100];
	mvgetstr(3, 20, name);

	mvprintw(5, 3, "Confirm? (y/n) ");
	curs_set(1);
	int confirming = 1;
	while (confirming == 1) {
		int confirmation = getch();
		switch (confirmation) {
			case 'y':
				confirming = 0;
				curs_set(0);
				noecho();
				break;
			case 'n':
				confirming = 0;
				curs_set(0);
				noecho();
				return newSubsystem();
				break;
		}
	}

	std::string homePath = std::getenv("HOME");
	std::string lsmlVmsPath = homePath + "/lsml";
	std::error_code ec;
	if (!std::filesystem::exists(std::filesystem::path(lsmlVmsPath), ec)) {
		mkdir(lsmlVmsPath.c_str(), 0755);
	}
	std::string nameString = name;
	std::string subsystemPath = lsmlVmsPath + "/" + nameString;
	if (mkdir(subsystemPath.c_str(), 0755) != 0) {
		makeTitle("Error while creating folder");
		mvprintw(3, 3, "Can't create folder %s", subsystemPath.c_str());
		mvprintw(4, 3, "Press ENTER to dismiss");
		noecho();
		while (getch() != 10) {};
		echo();
		return newSubsystem();
	}

	makeTitle("Downloading...");
	mvprintw(3, 3, "Downloading rootfs, please wait.");
	refresh();
	CURL *curl = curl_easy_init();
	std::string rootfsFilename = subsystemPath + "/rootfs.tar.gz";
	FILE *file = fopen(rootfsFilename.c_str(), "wb");

	curl_easy_setopt(curl, CURLOPT_URL, urls[system].c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, dl_progress);
	CURLcode code = curl_easy_perform(curl);
	if (code != CURLE_OK) {
		char title[256];
		snprintf(title, sizeof(title), "Error while downloading %s", systems[system].c_str());
		makeTitle(title);

		mvprintw(3, 3, "Can't download rootfs (%s)", curl_easy_strerror(code));
		while (getch() != 10) {};
		return newSubsystem();
	}
	fclose(file);
	curl_easy_cleanup(curl);

	std::string rootfsPath = subsystemPath + "/" + "rootfs";
	if (mkdir(rootfsPath.c_str(), 0755) != 0) {
		makeTitle("Error while creating folder");
		mvprintw(3, 3, "Can't create folder %s", rootfsPath.c_str());
		mvprintw(4, 3, "Press ENTER to dismiss");
		noecho();
		while (getch() != 10) {};
		echo();
		return newSubsystem();
	}

	if (!extract_tar_gz(rootfsFilename, rootfsPath)) {
		makeTitle("Extraction Error");
		mvprintw(3, 3, "Can't extract rootfs");
		mvprintw(4, 3, "Press ENTER to dismiss");
		while (getch() != 10) {};
		return newSubsystem();
	};

	std::string lsmlMarkPath = subsystemPath + "/.LSML_MARK";
	std::ofstream lsmlMarkFile(lsmlMarkPath);
	if (!lsmlMarkFile) {
		makeTitle("Error");
		mvprintw(3, 3, "Could not create LSML mark file");
		while (getch() != 10) {};
		return newSubsystem();
	}
	lsmlMarkFile << "LSML MARK";

	std::string startScriptPath = subsystemPath + "/start.sh";
	std::ofstream startScript(startScriptPath);
	if (!startScript.is_open()) {
		makeTitle("Error");
		mvprintw(3, 3, "Can't open start.sh script (%s)", ec.message().c_str());
		std::error_code ec(errno, std::generic_category());
		while (getch() != 10) {};
		return newSubsystem();
	}
	// startScript << "#!/bin/bash\n";
	// startScript << "set -e\n";
	// startScript << "proot -0 \\\n";
	// startScript << "	-r rootfs \\\n";
	// startScript << "	-w / \\\n";
	// startScript << "	/bin/bash --login\n";
	// startScript << "\n";
	std::string src = subsystemPath + "/rootfs/start.sh";
	std::string dst = subsystemPath + "/start.sh";
	rename(src.c_str(), dst.c_str());
	chmod(startScriptPath.c_str(), 0775);
	
	// Installation done
	makeTitle("Installation done");
	mvprintw(3, 3, "%s installed successfully!", systems[system].c_str());
	mvprintw(4, 3, "Press ENTER to return to main menu");
	while (getch() != 10) {};
	return;
}

void getSubsystems() {
	std::vector<std::string> vms;
	std::string homePath = std::getenv("HOME");
	std::string lsmlPathString = homePath + "/lsml";
	std::filesystem::path lsmlPath = lsmlPathString;

	makeTitle("Subsystems");
	
	std::error_code ec;
	if (!std::filesystem::exists(std::filesystem::path(lsmlPath), ec)) {
		mvprintw(3, 3, "You don't have any VMs");
		while (getch() != 10) {};
		return;
	}
	
	for (const auto& entry : std::filesystem::directory_iterator(lsmlPath)) {
		vms.push_back(entry.path().filename().string());
	}
	vms.push_back("Back");

	if (vms.size() == 0) {
		mvprintw(3, 3, "You don't have any VMs");
		while (getch() != 10) {};
		return;
	}
	
	int choice = makeMenu(vms);
	if (choice == (int) vms.size() - 1) {
		return;
	}

	makeTitle(vms[choice].c_str());
	
	std::string vmPath = lsmlPathString + "/" + vms[choice];
	std::vector<std::string> options = {"Start", "Back"};
	int action = makeMenu(options);
	switch (action) {
		case 0:
			stopInterface();
			for (int i = 0; i < 100; i++) {
				std::cout << "\n";
			}
			std::cout << "[\x1b[34m*\x1b[0m] You're now in " << vms[choice] << " shell - type 'exit' to leave\n";
			chdir(vmPath.c_str());
			system("./start.sh");
			std::cout << "\n\n[\x1b[32mV\x1b[0m] Subsystem exited - press ENTER to return to LSML\n";
			while (getch() != 10) {};
			initInterface();
			return;
			break;
		case 1:
			return;
			break;
	}

}

void infoAndStats() {
	makeTitle("Info / Statistics");

	// Step 1: is LSML initialized
	std::string lsmlPathString = (std::string) std::getenv("HOME") + "/lsml";
	std::filesystem::path lsmlPath = lsmlPathString;

	std::error_code ec;
	if (!std::filesystem::exists(lsmlPath, ec)) {
		mvprintw(3, 3, "LSML Directory missing");
		mvprintw(4, 3, "Press ENTER to go to main menu");
		while (getch() != 10) {};
		return;
	} else {
		int amount = 0;
		for (const auto& entry : std::filesystem::directory_iterator(lsmlPath)) {
			amount++;
		}
		mvprintw(3, 3, "VM Amount: %d", amount);
	}

	// Step 2: Counting LSML subsystems size
	uintmax_t subsystemsSize = 0;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(lsmlPath)) {
		if (entry.is_regular_file() && !entry.is_symlink()) {
			subsystemsSize += entry.file_size();
		}
	}
	int megabytes = subsystemsSize / 1024 / 1024;
	mvprintw(4, 3, "Total size: %d MB", megabytes);

	attron(COLOR_PAIR(BLUE_ON_BLACK));
	attron(A_BOLD);
	mvprintw(LINES - 5, 3, "> Back");
	attroff(COLOR_PAIR(BLUE_ON_BLACK));
	attroff(A_BOLD);
	while (getch() != 10) {};
	return;
}

void mainMenu () {
	makeTitle("Main Menu");
	std::vector<std::string> options = {"New subsystem", "Get subsystems", "Info/stats", "Exit"};
	int choice = makeMenu(options);
	switch (choice) {
		case 0:
			newSubsystem();
			break;
		case 1:
			getSubsystems();
			break;
		case 2:
			infoAndStats();
			break;
		case 3:
			stopInterface();
			exit(0);
			break;
	}
}

int main () {
	initInterface();

	while (1) mainMenu();
		
	return 0;
}
