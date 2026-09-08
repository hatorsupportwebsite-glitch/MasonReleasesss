#include "kill_sound.h"
#include "configs.h"
#include <windows.h>
#include <algorithm>
#include <utility>
// File names for the menu only. No sound playback.
void kill_sound::scan_folder(std::vector<KillSoundItem>& out)
{
	out.clear();
	configs::ensure_directory();
	std::wstring dir = configs::get_killsound_dir_w();
	if (dir.empty()) return;
	CreateDirectoryW(dir.c_str(), nullptr);
	std::wstring pat = dir + L"\\*.wav";
	WIN32_FIND_DATAW fd = {};
	HANDLE h = FindFirstFileW(pat.c_str(), &fd);
	if (h == INVALID_HANDLE_VALUE) return;
	do {
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) continue;
		std::wstring name = fd.cFileName;
		if (name.size() < 5) continue;
		KillSoundItem it;
		it.path = dir + L"\\" + name;
		std::wstring stem = name.substr(0, name.size() - 4);
		int n = WideCharToMultiByte(CP_UTF8, 0, stem.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (n > 1) {
			it.label_utf8.resize((size_t)n);
			WideCharToMultiByte(CP_UTF8, 0, stem.c_str(), -1, it.label_utf8.data(), n, nullptr, nullptr);
			it.label_utf8.pop_back();
		} else {
			it.label_utf8 = "sound";
		}
		out.push_back(std::move(it));
	} while (FindNextFileW(h, &fd));
	FindClose(h);
	std::sort(out.begin(), out.end(), [](const KillSoundItem& a, const KillSoundItem& b) {
		return _stricmp(a.label_utf8.c_str(), b.label_utf8.c_str()) < 0;
	});
}

