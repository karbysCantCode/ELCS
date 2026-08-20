#ifndef STYLES_H
#define STYLES_H


#define STYLESHEET_TEXTEDIT \
"QTextEdit {" \
" background: #11141a;" \
" color: #e5e7eb;" \
" border: 1px solid #2d323d;" \
" border-radius: 4px;" \
" padding: 8px;" \
" font-size: 14px;" \
"}" \
"QTextEdit:focus {" \
" border: 1px solid #3d8cff;" \
"}"


#define STYLESHEET_BUTTON_NORMAL \
"QPushButton {" \
" background: #1a1d25;" \
" color: #e5e7eb;" \
" border: 1px solid #303541;" \
" border-radius: 4px;" \
" padding: 8px 18px;" \
" font-size: 14px;" \
"}" \
"QPushButton:hover {" \
" background: #222631;" \
" border: 1px solid #3c4351;" \
"}" \
"QPushButton:pressed {" \
" background: #12151b;" \
"}"


#define STYLESHEET_BUTTON_MAIN \
"QPushButton {" \
" background: #2878d4;" \
" color: white;" \
" border: 1px solid #5da0ee;" \
" border-radius: 6px;" \
" padding: 10px 24px;" \
" font-size: 14px;" \
"}" \
"QPushButton:hover {" \
" background: #3589e8;" \
" border: 1px solid #79b4f5;" \
"}" \
"QPushButton:pressed {" \
" background: #1d5fae;" \
" border: 1px solid #4c8ed0;" \
"}"


#define STYLESHEET_BUTTON_DANGER \
"QPushButton {" \
" background: #642a2a;" \
" color: #ffdddd;" \
" border: 1px solid #8b3b3b;" \
" border-radius: 4px;" \
" padding: 8px 18px;" \
" font-size: 14px;" \
"}" \
"QPushButton:hover {" \
" background: #7b3030;" \
" border: 1px solid #a64b4b;" \
"}" \
"QPushButton:pressed {" \
" background: #4d2020;" \
"}"


#define STYLESHEET_FRAME_MAIN \
"QFrame {" \
" background: #11141b;" \
"}"


#define STYLESHEET_FRAME_SECONDARY \
"QFrame {" \
" background: #171a22;" \
"}"


#define STYLESHEET_WIDGET_SECONDARY \
"QWidget {" \
" background: #171a22;" \
" border: none;" \
" border-radius: 4px;" \
"}"


#define STYLESHEET_FRAME_CARD \
"QFrame {" \
" background: #1b1f28;" \
" border-radius: 5px;" \
" border: 1px solid #2c313c;" \
"}"


#define STYLESHEET_TOOLBOX_ITEM \
"QFrame {" \
" background: #1b1f28;" \
" border-radius: 8px;" \
" border: 1px solid #2c313c;" \
"}" \
"QFrame:hover {" \
" background: #20242e;" \
" border: 1px solid #3d8cff;" \
"}"


#define STYLESHEET_TOOLBOX_ITEM_LABEL \
"QLabel {" \
" color: #e5e7eb;" \
" font-size: 13px;" \
" font-weight: 600;" \
"}"


#define STYLESHEET_OVERLAY \
"background: rgba(0,0,0,175);"


#define STYLESHEET_LABEL_TEXT \
"QLabel {" \
" color: #d5d9e0;" \
" font-size: 14px;" \
"}"


#define STYLESHEET_LABEL_TITLE \
"QLabel {" \
" color: #f3f4f6;" \
" font-size: 20px;" \
" font-weight: bold;" \
"}"


#define STYLESHEET_LABEL_SECONDARY \
"QLabel {" \
" color: #858c99;" \
" font-size: 12px;" \
"}"


#define STYLESHEET_LABEL_ERROR \
"QLabel {" \
" color: #ff6b6b;" \
" font-size: 13px;" \
" font-weight: bold;" \
"}"


#define STYLESHEET_LABEL_SUCCESS \
"QLabel {" \
" color: #5fc77b;" \
" font-size: 13px;" \
" font-weight: bold;" \
"}"


#define STYLESHEET_PROPERTY_SECTION_HEADER \
"QToolButton {" \
" background: #1b1f28;" \
" color: #e5e7eb;" \
" border: 1px solid #2c313c;" \
" border-radius: 4px;" \
" padding: 2px;" \
" text-align: left;" \
" font-size: 14px;" \
" font-weight: bold;" \
"}" \
"QToolButton:hover {" \
" background: #222631;" \
" border: 1px solid #3c4351;" \
"}" \
"QToolButton:pressed {" \
" background: #151820;" \
"}"


#define STYLESHEET_PROPERTY_SECTION_SCROLLAREA \
"QScrollArea {" \
" border: none;" \
" background: #11141b;" \
"}" \
"QScrollBar:vertical {" \
" background: #0b0d12;" \
" width: 10px;" \
" margin: 0px;" \
"}" \
"QScrollBar::handle:vertical {" \
" background: #292e38;" \
" min-height: 20px;" \
" border-radius: 5px;" \
"}" \
"QScrollBar::handle:vertical:hover {" \
" background: #383f4c;" \
"}" \
"QScrollBar::add-line:vertical," \
"QScrollBar::sub-line:vertical {" \
" height: 0px;" \
"}" \
"QScrollBar::add-page:vertical," \
"QScrollBar::sub-page:vertical {" \
" background: transparent;" \
"}"


#define STYLESHEET_PROPERTY_CONTENT \
"QWidget {" \
" background: #11141b;" \
"}"


#define STYLESHEET_PROPERTY_ROW \
"QWidget {" \
" background: #171a22;" \
" border: 1px solid #2c313c;" \
" border-radius: 4px;" \
"}" \
"QLabel {" \
" color: #d5d9e0;" \
" border: none;" \
" background: transparent;" \
" font-size: 13px;" \
"}"

#define STYLESHEET_SPINBOX \
"QSpinBox {" \
" background: #1a1d25;" \
" color: #e5e7eb;" \
" border: 1px solid #303541;" \
" border-radius: 4px;" \
" padding: 4px 6px;" \
" font-size: 13px;" \
"}" \
"QSpinBox:focus {" \
" border: 1px solid #3d8cff;" \
"}" \
"QSpinBox::up-button, QSpinBox::down-button {" \
" background: #1a1d25;" \
" border: none;" \
" width: 16px;" \
"}" \
"QSpinBox::up-button:hover, QSpinBox::down-button:hover {" \
" background: #222631;" \
"}"

#define STYLESHEET_LINEEDIT \
"QLineEdit {" \
" background: #1a1d25;" \
" color: #e5e7eb;" \
" border: 1px solid #303541;" \
" border-radius: 4px;" \
" padding: 4px 8px;" \
" font-size: 13px;" \
"}" \
"QLineEdit:focus {" \
" border: 1px solid #3d8cff;" \
"}"

#define STYLESHEET_COMBOBOX \
"QComboBox {" \
" background: #1a1d25;" \
" color: #e5e7eb;" \
" border: 1px solid #303541;" \
" border-radius: 4px;" \
" padding: 4px 8px;" \
" font-size: 13px;" \
"}" \
"QComboBox:focus {" \
" border: 1px solid #3d8cff;" \
"}" \
"QComboBox QAbstractItemView {" \
" background: #1a1d25;" \
" color: #e5e7eb;" \
" border: 1px solid #303541;" \
" selection-background-color: #222631;" \
"}"

#define STYLESHEET_MENUBAR \
"QMenuBar {" \
" background: #11141b;" \
" color: #e5e7eb;" \
" border-bottom: 1px solid #2c313c;" \
" padding: 2px;" \
"}" \
"QMenuBar::item {" \
" background: transparent;" \
" padding: 6px 12px;" \
" border-radius: 4px;" \
"}" \
"QMenuBar::item:selected {" \
" background: #222631;" \
"}" \
"QMenuBar::item:pressed {" \
" background: #1a1d25;" \
"}"




#endif // STYLES_H