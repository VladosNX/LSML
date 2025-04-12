#ifndef ARCHIVES_H
#define ARCHIVES_H

#include <stdbool.h>

bool extract_tar_gz(const std::string& archive_path, const std::string& output_dir);

#endif
