install(FILES ${PROJECT_SOURCE_DIR}/icons/start-menu.svg
  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/"
  RENAME "kiran-menu.svg")
install(FILES ${PROJECT_SOURCE_DIR}/icons/window-switcher.svg
  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/"
  RENAME "kiran-window-switcher.svg")
install(FILES ${PROJECT_SOURCE_DIR}/icons/workspace-switcher.svg
  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/"
  RENAME "kiran-workspace-switcher.svg")

install(FILES ${PROJECT_SOURCE_DIR}/icons/menu-apps-list.svg
  ${PROJECT_SOURCE_DIR}/icons/menu-favorite-apps.svg
  ${PROJECT_SOURCE_DIR}/icons/menu-recent-files.svg
  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/")

