import os
import shutil
import build_utils


def get_supported_targets(platform):
    if platform == 'win32':
        return ['win32', 'win10', 'android']
    else:
        return ['macos', 'ios', 'android']


def get_dependencies_for_target(target):
    return []


def build_for_target(target, working_directory_path, root_project_path):
    if target == 'win32':
        _build_win32(working_directory_path, root_project_path)
    elif target == 'win10':
        _build_win10(working_directory_path, root_project_path)
    elif target == 'macos':
        _build_macos(working_directory_path, root_project_path)
    elif target == 'ios':
        _build_ios(working_directory_path, root_project_path)
    elif target == 'android':
        _build_android(working_directory_path, root_project_path)


def get_download_info():
    return 'https://www.lua.org/ftp/lua-5.3.3.tar.gz'


def _download_and_extract(working_directory_path):
    source_folder_path = os.path.join(working_directory_path, 'lua_source')

    url = get_download_info()
    build_utils.download_and_extract(
        url,
        working_directory_path,
        source_folder_path,
        build_utils.get_url_file_name_no_ext(url))

    return source_folder_path


@build_utils.run_once
def _patch_sources(source_folder_path, working_directory_path):
    shutil.copyfile(
        'CMakeLists.txt',
        os.path.join(source_folder_path, 'CMakeLists.txt'))


def _build_win32(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    _patch_sources(source_folder_path, working_directory_path)

    build_utils.build_and_copy_libraries_win32_cmake(
        os.path.join(working_directory_path, 'gen'),
        source_folder_path,
        root_project_path,
        'lua.sln', 'lua',
        'lua.lib', 'lua.lib',
        'lua_wind.lib', 'lua_win.lib',
        'lua_wind.lib', 'lua_win.lib')

    _copy_headers(source_folder_path, root_project_path)


def _build_win10(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    _patch_sources(source_folder_path, working_directory_path)

    build_utils.build_and_copy_libraries_win10_cmake(
        os.path.join(working_directory_path, 'gen'),
        source_folder_path,
        root_project_path,
        'lua.sln', 'lua',
        'lua.lib', 'lua.lib',
        'lua_wind.lib', 'lua_win.lib',
        'lua_wind.lib', 'lua_win.lib',
        'lua_wind.lib', 'lua_win.lib')

    _copy_headers(source_folder_path, root_project_path)


def _build_macos(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    _patch_sources(source_folder_path, working_directory_path)

    build_utils.build_and_copy_libraries_macos_cmake(
        os.path.join(working_directory_path, 'gen'),
        source_folder_path,
        root_project_path,
        'lua.xcodeproj', 'lua',
        'liblua.a',
        'liblua_macos.a')

    _copy_headers(source_folder_path, root_project_path)


def _build_ios(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    _patch_sources(source_folder_path, working_directory_path)

    build_utils.build_and_copy_libraries_ios_cmake(
        os.path.join(working_directory_path, 'gen'),
        source_folder_path,
        root_project_path,
        'lua.xcodeproj', 'lua',
        'liblua.a',
        'liblua_ios.a')

    _copy_headers(source_folder_path, root_project_path)


def _build_android(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    _patch_sources(source_folder_path, working_directory_path)

    build_utils.build_and_copy_libraries_android_cmake(
        os.path.join(working_directory_path, 'gen'),
        source_folder_path,
        root_project_path,
        'liblua.a',
        'liblua.a')

    _copy_headers(source_folder_path, root_project_path)


def _copy_headers(source_folder_path, root_project_path):
    include_path = os.path.join(root_project_path, 'Libs/lua/include')
    build_utils.copy_files(source_folder_path, include_path, 'src/*.h')
