# Generate help documents
SET(DOC_DIR "${CMAKE_SOURCE_DIR}/app/resources/documentation")
SET(DOC_HELP_SOURCE_DIR "${DOC_DIR}/help")
SET(DOC_HELP_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/help")

SET(PANDOC_TEMPLATE_PATH "${DOC_HELP_SOURCE_DIR}/template.html")
SET(PANDOC_INPUT_PATH    "${DOC_HELP_SOURCE_DIR}/index.md")
SET(PANDOC_OUTPUT_PATH   "${DOC_HELP_TARGET_DIR}/index.html")

FIX_WIN32_PATH(PANDOC_TEMPLATE_PATH)
FIX_WIN32_PATH(PANDOC_INPUT_PATH)
FIX_WIN32_PATH(PANDOC_OUTPUT_PATH)

ADD_CUSTOM_TARGET(GenerateHelp
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DOC_HELP_TARGET_DIR}"
    COMMAND pandoc -s --toc --toc-depth=2 --template "${PANDOC_TEMPLATE_PATH}" -o "${PANDOC_OUTPUT_PATH}" "${PANDOC_INPUT_PATH}"
    COMMAND ${CMAKE_COMMAND} -DINPUT="${DOC_HELP_TARGET_DIR}/index.html" -DOUTPUT="${DOC_HELP_TARGET_DIR}/index.html" -P "${CMAKE_SOURCE_DIR}/cmake/TransformKeyboardShortcuts.cmake"
)