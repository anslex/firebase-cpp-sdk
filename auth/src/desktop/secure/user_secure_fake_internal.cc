// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "auth/src/desktop/secure/user_secure_fake_internal.h"

#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif  // defined(_WIN32)

#include <errno.h>

#include <cstdio>
#include <fstream>
#include <string>

namespace firebase {
namespace auth {
namespace secure {

#if defined(_WIN32)
static const char kDirectorySeparator[] = "\\";
#define unlink _unlink
#define mkdir(x, y) _mkdir(x)
#define rmdir _rmdir
#else
static const char kDirectorySeparator[] = "/";
#endif  // defined(_WIN32)

static const char kFileExtension[] = ".authbin";

UserSecureFakeInternal::UserSecureFakeInternal(const char* secure_path)
    : secure_path_(secure_path) {}

UserSecureFakeInternal::~UserSecureFakeInternal() {}

std::string UserSecureFakeInternal::LoadUserData(const std::string& app_name) {
  std::string output;
  std::string filename = GetFilePath(app_name);

  std::ifstream infile;
  infile.open(filename, std::ios::binary);
  if (infile.fail()) {
    LogDebug("LoadUserData: Failed to read %s", filename.c_str());
    return "";
  }

  infile.seekg(0, std::ios::end);
  size_t length = infile.tellg();
  infile.seekg(0, std::ios::beg);
  output.resize(length);
  infile.read(&*output.begin(), length);
  if (infile.fail()) {
    return "";
  }
  infile.close();
  return output;
}

void UserSecureFakeInternal::SaveUserData(const std::string& app_name,
                                          const std::string& user_data) {
  // Make the directory in case it doesn't already exist, ignoring errors.
  if (mkdir(secure_path_.c_str(), 0700) < 0) {
    int error = errno;
    if (error != 0 && error != EEXIST) {
      LogWarning("SaveUserData: Couldn't create directory %s: %d",
                 secure_path_.c_str(), error);
    }
  }

  std::string filename = GetFilePath(app_name);

  std::ofstream ofile(filename, std::ios::binary);
  ofile.write(user_data.c_str(), user_data.length());
  ofile.close();
}

void UserSecureFakeInternal::DeleteUserData(const std::string& app_name) {
  std::string filename = GetFilePath(app_name);
  std::ifstream infile;
  infile.open(filename, std::ios::binary);
  if (infile.fail()) {
    return;
  }
  infile.close();
#if defined(_WIN32)
  DeleteFile(filename.c_str());
#else
  unlink(filename.c_str());
#endif  // defined(_WIN32)
}

void UserSecureFakeInternal::DeleteAllData() {
  std::vector<std::string> files_to_delete;
#if defined(_WIN32)
  std::string file_spec =
      secure_path_ + kDirectorySeparator + "*" + kFileExtension;
  WIN32_FIND_DATA file_data;
  HANDLE handle = FindFirstFile(file_spec.c_str(), &file_data);
  if (INVALID_HANDLE_VALUE == handle) {
    DWORD error = GetLastError();
    if (error != ERROR_FILE_NOT_FOUND) {
      LogWarning("DeleteAllData: Couldn't find file list matching %s: %d",
                 file_spec.c_str(), error);
    }
    return;
  }
  do {
    std::string file_path =
        secure_path_ + kDirectorySeparator + file_data.cFileName;
    files_to_delete.push_back(file_path);
  } while (FindNextFile(handle, &file_data));
  FindClose(handle);
#else
  // These are data types defined in the "dirent" header
  DIR* the_folder = opendir(secure_path_.c_str());
  if (!the_folder) {
    return;
  }
  struct dirent* next_file;

  while ((next_file = readdir(the_folder)) != nullptr) {
    // Only delete files matching the file extension.
    if (strcasestr(next_file->d_name, kFileExtension) !=
        next_file->d_name + strlen(next_file->d_name) -
            strlen(kFileExtension)) {
      continue;
    }
    // Build the path for each file in the folder
    std::string file_path =
        secure_path_ + kDirectorySeparator + next_file->d_name;
    files_to_delete.push_back(file_path);
  }
  closedir(the_folder);
#endif
  for (int i = 0; i < files_to_delete.size(); ++i) {
    if (unlink(files_to_delete[i].c_str()) == -1) {
      int error = errno;
      if (error != 0) {
        LogWarning("DeleteAllData: Couldn't remove file %s: %d",
                   files_to_delete[i].c_str(), error);
      }
    }
  }
  // Remove the directory if it's empty, ignoring errors.
  if (rmdir(secure_path_.c_str()) == -1) {
    int error = errno;
    LogDebug("DeleteAllData: Couldn't remove directory %s: %d",
             secure_path_.c_str(), error);
  }
}

std::string UserSecureFakeInternal::GetFilePath(const std::string& app_name) {
  std::string filepath =
      secure_path_ + kDirectorySeparator + app_name + kFileExtension;
  return filepath;
}

}  // namespace secure
}  // namespace auth
}  // namespace firebase
