#ifndef STYLES_H
#define STYLES_H

#define STYLESHEET_TEXTEDIT \
"QTextEdit {" \
" background: #3a3a3a;" \
" color: white;" \
" border: 1px solid #555;" \
" border-radius: 4px;" \
" padding: 8px;" \
" font-size: 14px;" \
"}" \
"QTextEdit:focus {" \
" border: 1px solid #4da6ff;" \
"}"

#define STYLESHEET_BUTTON_NORMAL \
"QPushButton {" \
" background: #444444;" \
" color: white;" \
" border: 1px solid #555555;" \
" border-radius: 4px;" \
" padding: 8px 18px;" \
" font-size: 14px;" \
"}" \
"QPushButton:hover {" \
" background: #555555;" \
"}" \
"QPushButton:pressed {" \
" background: #333333;" \
"}"


#define STYLESHEET_BUTTON_MAIN \
"QPushButton {" \
" background: #2d8cff;" \
" color: white;" \
" border: 1px solid #a3ccff;" \
" border-radius: 4px;" \
" padding: 8px 18px;" \
" font-size: 14px;" \
"}" \
"QPushButton:hover {" \
" background: #4da6ff;" \
"}" \
"QPushButton:pressed {" \
" background: #1c6dcc;" \
"}"

#define STYLESHEET_BUTTON_DANGER \
"QPushButton {" \
" background: #d9534f;" \
" color: white;" \
" border: 1px solid #ffbab8;" \
" border-radius: 4px;" \
" padding: 8px 18px;" \
" font-size: 14px;" \
"}" \
"QPushButton:hover {" \
" background: #e57373;" \
"}" \
"QPushButton:pressed {" \
" background: #b52b27;" \
"}"

#define STYLESHEET_FRAME_MAIN \
"QFrame {" \
" background: #2b2b2b;" \
"}"

#define STYLESHEET_FRAME_SECONDARY \
"QFrame {" \
" background: #333333;" \
"}"

#define STYLESHEET_FRAME_CARD \
"QFrame {" \
" background: #3a3a3a;" \
" border-radius: 4px;" \
" border: 1px solid #555555;" \
"}"

#define STYLESHEET_OVERLAY \
"background: rgba(0,0,0,120);"

#define STYLESHEET_LABEL_TEXT \
"QLabel {" \
" color: #dddddd;" \
" font-size: 14px;" \
"}"

#define STYLESHEET_LABEL_TITLE \
"QLabel {" \
" color: white;" \
" font-size: 20px;" \
" font-weight: bold;" \
"}"

#define STYLESHEET_LABEL_SECONDARY \
"QLabel {" \
" color: #aaaaaa;" \
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
" color: #4caf50;" \
" font-size: 13px;" \
" font-weight: bold;" \
"}"

#define STYLESHEET_PROPERTY_SECTION_HEADER \
"QToolButton {" \
" background: #3a3a3a;" \
" color: white;" \
" border: 1px solid #555555;" \
" border-radius: 4px;" \
" padding: 2px;" \
" text-align: left;" \
" font-size: 14px;" \
" font-weight: bold;" \
"}" \
"QToolButton:hover {" \
" background: #454545;" \
"}" \
"QToolButton:pressed {" \
" background: #2f2f2f;" \
"}"


#define STYLESHEET_PROPERTY_SECTION_SCROLLAREA \
"QScrollArea {" \
" border: none;" \
" background: #2b2b2b;" \
"}" \
"QScrollBar:vertical {" \
" background: #2b2b2b;" \
" width: 10px;" \
" margin: 0px;" \
"}" \
"QScrollBar::handle:vertical {" \
" background: #555555;" \
" min-height: 20px;" \
" border-radius: 5px;" \
"}" \
"QScrollBar::handle:vertical:hover {" \
" background: #666666;" \
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
" background: #2b2b2b;" \
"}"


#define STYLESHEET_PROPERTY_ROW \
"QWidget {" \
" background: #333333;" \
" border: 1px solid #555555;" \
" border-radius: 4px;" \
"}" \
"QLabel {" \
" color: #dddddd;" \
" border: none;" \
" background: transparent;" \
" font-size: 13px;" \
"}"
#endif // STYLES_H