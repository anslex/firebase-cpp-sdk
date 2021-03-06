# Copyright 2018 Google
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# CMake file for the firebase_app library

cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

include(binary_to_array)
include(download_pod_headers)

project(firebase_app NONE)
enable_language(C)
enable_language(CXX)

# Define how to generate google_services_resource_(source/header)
binary_to_array("google_services_resource"
    "${CMAKE_CURRENT_LIST_DIR}/google_services.fbs"
    "firebase::fbs"
    "${FIREBASE_GEN_FILE_DIR}/app")

# Define the resource builds needed for Android
binary_to_array("app_resources"
    "${CMAKE_CURRENT_LIST_DIR}/app_resources/build/dexed.jar"
    "firebase_app"
    "${FIREBASE_GEN_FILE_DIR}/app")
binary_to_array("google_api_resources"
    "${CMAKE_CURRENT_LIST_DIR}/google_api_resources/build/dexed.jar"
    "google_api"
    "${FIREBASE_GEN_FILE_DIR}/app")
binary_to_array("invites_resources"
    "${CMAKE_CURRENT_LIST_DIR}/invites_resources/build/dexed.jar"
    "firebase_invites"
    "${FIREBASE_GEN_FILE_DIR}/app")

# Generate build_type_generated.h
file(MAKE_DIRECTORY ${FIREBASE_GEN_FILE_DIR}/app/src)
set(build_type_header ${FIREBASE_GEN_FILE_DIR}/app/src/build_type_generated.h)
add_custom_command(
    OUTPUT ${build_type_header}
    COMMAND python "${FIREBASE_SCRIPT_DIR}/build_type_header.py"
        "--build_type=head"
        "--output_file=${build_type_header}"
    COMMENT "Generating build_type_generated header"
)

# Generate version.h
set(version_header_dir ${FIREBASE_GEN_FILE_DIR}/app/src/include/firebase)
set(version_header ${version_header_dir}/version.h)
file(MAKE_DIRECTORY ${version_header_dir})
add_custom_command(
    OUTPUT ${version_header}
    COMMAND python "${FIREBASE_SCRIPT_DIR}/version_header.py"
        "--input_file=${FIREBASE_SCRIPT_DIR}/cpp_sdk_version.json"
        "--output_file=${version_header}"
        "--build_type=released"
    COMMENT "Generating version header"
)

# Build the google_services_generated.h header from the flatbuffer schema file.
set(FLATBUFFERS_FLATC_SCHEMA_EXTRA_ARGS
    "--no-union-value-namespacing"
    "--gen-object-api"
    "--cpp-ptr-type" "flatbuffers::unique_ptr")
build_flatbuffers("${CMAKE_CURRENT_LIST_DIR}/google_services.fbs"
                  ""
                  "app_generated_includes"
                  "${FIREBASE_FLATBUFFERS_DEPENDENCIES}"
                  "${FIREBASE_GEN_FILE_DIR}/app"
                  ""
                  "")

set(log_common_SRCS
    src/log.cc)
set(log_android_SRCS
    src/log_android.cc
    src/util_android.cc)
set(log_ios_SRCS
    src/log_ios.mm)
set(log_desktop_SRCS
    src/log_stdio.cc)
if(ANDROID)
  set(log_SRCS
      "${log_common_SRCS}"
      "${log_android_SRCS}")
elseif(IOS)
  set(log_SRCS
      "${log_common_SRCS}"
      "${log_ios_SRCS}")
else()
  set(log_SRCS
      "${log_common_SRCS}"
      "${log_desktop_SRCS}")
endif()

set(common_SRCS
    ${google_services_resource_source}
    ${google_services_resource_header}
    src/app_common.cc
    src/app_options.cc
    src/callback.cc
    src/cleanup_notifier.cc
    src/function_registry.cc
    src/future_manager.cc
    src/path.cc
    src/reference_counted_future_impl.cc
    src/scheduler.cc
    src/thread_cpp11.cc
    src/thread_pthread.cc
    src/time.cc
    src/util.cc
    src/variant.cc)

set(invites_SRCS
    src/invites/cached_receiver.cc
    src/invites/invites_receiver_internal.cc)

set(app_android_SRCS
    src/app_android.cc
    src/google_play_services/availability_android.cc
    ${app_resources_source}
    ${google_api_resources_source}
    ${invites_resources_source}
    src/invites/android/invites_receiver_internal_android.cc
    src/invites/android/invites_android_helper.cc)
set(app_ios_SRCS
    src/app_ios.mm
    src/util_ios.mm
    src/invites/ios/invites_receiver_internal_ios.mm
    src/invites/ios/invites_ios_startup.mm)
set(app_desktop_SRCS
    src/app_desktop.cc
    src/invites/stub/invites_receiver_internal_stub.cc
    src/variant_util.cc)
if(ANDROID)
  set(app_platform_SRCS
      "${app_android_SRCS}")
elseif(IOS)
  set(app_platform_SRCS
      "${app_ios_SRCS}")
else()
  set(app_platform_SRCS
      "${app_desktop_SRCS}")
endif()

set(internal_HDRS
    src/include/firebase/app.h
    src/include/firebase/future.h
    src/include/firebase/internal/common.h
    src/include/firebase/internal/future_impl.h
    src/include/firebase/log.h
    src/include/firebase/util.h
    src/include/firebase/variant.h
    src/include/google_play_services/availability.h
    ${version_header})

set(utility_common_HDRS
    src/app_common.h
    src/assert.h
    ${build_type_header}
    src/callback.h
    src/cleanup_notifier.h
    src/function_registry.h
    src/future_manager.h
    src/log.h
    src/mutex.h
    src/optional.h
    src/path.h
    src/pthread_condvar.h
    src/reference_counted_future_impl.h
    src/scheduler.h
    src/semaphore.h
    src/thread.h
    src/time.h
    src/util.h)
set(utility_android_HDRS)
set(utility_ios_HDRS)
set(utility_desktop_HDRS
    src/variant_util.h
    src/invites/cached_receiver.h
    src/invites/invites_receiver_internal.h
    src/invites/receiver_interface.h
    src/invites/sender_receiver_interface.h)
if(ANDROID)
  set(utility_HDRS
      "${utility_common_HDRS}"
      "${utility_android_HDRS}")
elseif(IOS)
  set(utility_HDRS
      "${utility_common_HDRS}"
      "${utility_ios_HDRS}")
else()
  set(utility_HDRS
      "${utility_common_HDRS}"
      "${utility_desktop_HDRS}")
endif()

set(app_android_HDRS
    ${app_resources_header}
    ${google_api_resources_header}
    ${invites_resources_header}
    src/invites/android/invites_android_helper.h
    src/invites/android/invites_receiver_internal_android.h)
set(app_ios_HDRS
    src/invites/ios/invites_receiver_internal_ios.h)
set(app_desktop_HDRS
    src/invites/stub/invites_receiver_internal_stub.h)
if(ANDROID)
  set(app_platform_HDRS
      "${app_android_HDRS}")
elseif(IOS)
  set(app_platform_HDRS
      "${app_ios_HDRS}")
else()
  set(app_platform_HDRS
      "${app_desktop_HDRS}")
endif()

add_library(firebase_app STATIC
    ${log_SRCS}
    ${common_SRCS}
    ${invites_SRCS}
    ${app_platform_SRCS}
    ${internal_HDRS}
    ${utility_HDRS}
    ${app_platform_HDRS}
    ${FIREBASE_GEN_FILE_DIR}/app/google_services_generated.h
    memory/atomic.h
    meta/type_traits.h
    meta/move.h
    memory/unique_ptr.h
    memory/shared_ptr.h)

target_include_directories(firebase_app
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/include
    ${FIREBASE_GEN_FILE_DIR}
  PRIVATE
    ${FIREBASE_CPP_SDK_ROOT_DIR}
    ${FLATBUFFERS_SOURCE_DIR}/include
)
target_compile_definitions(firebase_app
  PRIVATE
    -DINTERNAL_EXPERIMENTAL=1
)
# firebase_app has a dependency on flatbuffers, which needs to be included.
target_link_libraries(firebase_app
  PRIVATE
    flatbuffers
)
# Automatically include headers that might not be declared.
if(MSVC)
  add_definitions(/FI"assert.h" /FI"string.h" /FI"stdint.h")
else()
  add_definitions(-include assert.h -include string.h)
endif()

if(IOS)
  # Enable Automatic Reference Counting (ARC).
  set_property(
    TARGET firebase_app
    APPEND_STRING PROPERTY
    COMPILE_FLAGS "-fobjc-arc")

  # Setup the target for getting the Firebase iOS SDK headers through Cocoapods.
  set(pod_target_name "download_app_pod_headers")
  set(pods_dir "${PROJECT_BINARY_DIR}/Pods")
  set(pod_list "")
  list(APPEND pod_list "'Firebase/Core', '6.0.0'")
  list(APPEND pod_list "'Firebase/DynamicLinks', '6.0.0'")

  setup_pod_headers_target("${pod_target_name}" "${pods_dir}" "${pod_list}")

  # Add the Cocoapod headers to the include directories
  set(base_header_dir "${pods_dir}/Pods/Headers/Public")
  target_include_directories(firebase_app
    PRIVATE
      ${base_header_dir}/FirebaseCore
      ${base_header_dir}/FirebaseDynamicLinks
      ${base_header_dir}/FirebaseInstanceID
  )

  # Add a dependency to downloading the headers onto app.
  add_dependencies(firebase_app ${pod_target_name})
endif()

# Add the rest subdirectory, so that other libraries can access it
add_subdirectory(rest)
