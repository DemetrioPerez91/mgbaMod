include(ExportDirectory)
set(SOURCE_FILES
	commandline.c
	proxy-backend.c
	thread-proxy.c
	video-backend.c
	video-logger.c)

if(ENABLE_VFS)
	list(APPEND SOURCE_FILES
		updater.c)
endif()

set(GUI_FILES
	gui/Cloud/logToFile.c
	gui/Cloud/cleanupFileName.c
	gui/Cloud/upload.c
	gui/cloud/encoding.c
	gui/cloud/download.c
	gui/cheats.c
	gui/gui-config.c
	gui/gui-runner.c
	gui/remap.c)

source_group("Extra features" FILES ${SOURCE_FILES})
source_group("Extra GUI source" FILES ${GUI_FILES})

export_directory(EXTRA SOURCE_FILES)
export_directory(EXTRA_GUI GUI_FILES)
