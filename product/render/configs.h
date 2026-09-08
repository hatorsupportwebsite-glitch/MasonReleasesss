#pragma once

#include <string>
#include <vector>

namespace configs
{
	// %USERPROFILE%\Documents\MasonRecode
	std::wstring get_config_dir_w();
	std::string  get_config_dir();
	// %USERPROFILE%\Documents\MasonRecode\killsound
	std::wstring get_killsound_dir_w();

	/// Создать папку при необходимости
	bool ensure_directory();

	/// Список имён без расширения .cfg
	std::vector<std::string> list_configs();

	/// Создать пустой зашифрованный файл (имя без .cfg)
	bool create_config(const char* name_no_ext);

	/// Сохранить текущие настройки в name.cfg
	bool save_config(const char* name_no_ext);

	/// Загрузить настройки из name.cfg
	bool load_config(const char* name_no_ext);

	/// Удалить name.cfg
	bool delete_config(const char* name_no_ext);

	/// Открыть папку в проводнике
	void open_config_directory();

	/// Последняя ошибка (для UI)
	const char* last_error();

	/// Безопасное имя файла (только буквы, цифры, _, -)
	std::string sanitize_config_name(const char* name);
}
