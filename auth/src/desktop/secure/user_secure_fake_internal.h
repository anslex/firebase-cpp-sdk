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

#ifndef FIREBASE_AUTH_CLIENT_CPP_SRC_DESKTOP_SECURE_USER_SECURE_FAKE_INTERNAL_H_
#define FIREBASE_AUTH_CLIENT_CPP_SRC_DESKTOP_SECURE_USER_SECURE_FAKE_INTERNAL_H_

#include <string>

#include "auth/src/desktop/secure/user_secure_internal.h"

namespace firebase {
namespace auth {
namespace secure {

// Fake implementation for the secure manager of user data.
class UserSecureFakeInternal : public UserSecureInternal {
 public:
  explicit UserSecureFakeInternal(const char* secure_path);
  ~UserSecureFakeInternal() override;

  std::string LoadUserData(const std::string& app_name) override;

  void SaveUserData(const std::string& app_name,
                    const std::string& user_data) override;

  void DeleteUserData(const std::string& app_name) override;

  void DeleteAllData() override;

 private:
  std::string GetFilePath(const std::string& app_name);

  const std::string secure_path_;
};

}  // namespace secure
}  // namespace auth
}  // namespace firebase

#endif  // FIREBASE_AUTH_CLIENT_CPP_SRC_DESKTOP_SECURE_USER_SECURE_FAKE_INTERNAL_H_
