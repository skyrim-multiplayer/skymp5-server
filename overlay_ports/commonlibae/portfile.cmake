vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Ryan-rsm-McKenzie/CommonLibSSE
    REF 0e9d380b90950eb3ece1e5b95e3b6a379ee03f8e
    SHA512 0
    HEAD_REF master
    PATCHES
      patches/objectrefr-make_moverefr_public.patch
      patches/variable-make_members_public.patch
      patches/stackframe-uncomment_top_args.patch
      patches/extradatalist-make_members_public.patch
      patches/expand-alias.patch
)

vcpkg_configure_cmake(SOURCE_PATH ${SOURCE_PATH})

vcpkg_install_cmake()
vcpkg_fixup_cmake_targets(CONFIG_PATH "lib/cmake/commonlibsse")
vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
