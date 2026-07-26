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
" border: none;" \
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
" border: none;" \
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
" border: none;" \
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
#endif // STYLES_H