#pragma once
#include <string>
#include <vector>
struct KillSoundItem { std::string label_utf8; std::wstring path; };
namespace kill_sound { void scan_folder(std::vector<KillSoundItem>& out); }
