# adding audio transport library, audiorw library (for standalone transport script only),
# and native file dialog library that is build locally

# IMPORTANT: run get-and-build-native-file-dialog before building
set(app_link_libs audio_transport audiorw nfd)
set(app_linker_flags -L${app_path})
set(app_include_dirs ${app_path})
