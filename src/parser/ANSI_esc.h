#pragma once
#include <string>
#include <iostream>
#include <list>
#include <stdexcept>

class ANSI_ESC {
private:
    std::string pos(int n, std::string tail) const
    {
        return CSI + std::to_string(n) + tail;
    }

    std::string pos(int n, int m, std::string tail) const
    {
        return CSI + std::to_string(n) + ";" + std::to_string(m) + tail;
    }

public:
    // Control characters
    const std::string BEL = "\x07";
    const std::string BS = "\x08";
    const std::string HT = "\x09";
    const std::string LF = "\x0a";
    const std::string VT = "\x0b";
    const std::string FF = "\x0c";
    const std::string CR = "\x0d";
    const std::string ESC = "\x1b";
    const std::string DEL = "\x7f";
    const std::string CSI = ESC + "[";
    const std::string DCS = ESC + "P";
    const std::string OSC = ESC + "]";

    // Cursor control
    const std::string HIDE_CURSOR = CSI + "?25l";
    const std::string SHOW_CURSOR = CSI + "?25h";
    const std::string HOME = CSI + "H";

    // Cursor movement
    std::string pos(int line, int column) const { return pos(line, column, "H"); }
    std::string up(int lines) const { return pos(lines, "A"); }
    std::string down(int lines) const { return pos(lines, "B"); }  // Fixed typo in method name (was 'dowm')
    std::string right(int columns) const { return pos(columns, "C"); }  // Changed parameter name to 'columns' for clarity
    std::string left(int columns) const { return pos(columns, "D"); }  // Changed parameter name to 'columns' for clarity
    std::string downBeginning(int lines) const { return pos(lines, "E"); }
    std::string upBeginning(int lines) const { return pos(lines, "F"); }
    std::string column(int col) const { return pos(col, "G"); }  // Fixed missing return

    // More cursor control
    const std::string GETPOS = CSI + "6n";
    const std::string UP1 = ESC + "M";
    const std::string SAVE_CUR_DEC = ESC + "7";
    const std::string RESTORE_CUR_DEC = ESC + "8";  // Added missing ESC
    const std::string SAVE_CUR_SCO = CSI + "s";
    const std::string RESTORE_CUR_SCO = CSI + "u";

    // Erase operations
    const std::string ERASE_IN_DISPLAY = CSI + "J";
    const std::string ERASE_DOWN_DISPLAY = CSI + "0J";
    const std::string ERASE_UP_DISPLAY = CSI + "1J";
    const std::string ERASE_ALL_DISPLAY = CSI + "2J";
    const std::string ERASE_SAVED_LINES = CSI + "3J";  // Renamed from ERASE_SAVED_LINE for accuracy
    const std::string ERASE_IN_LINE = CSI + "K";
    const std::string ERASE_CURSOR_EOL = CSI + "0K";
    const std::string ERASE_START_CURSOR = CSI + "1K";
    const std::string ERASE_LINE = CSI + "2K";

    /// <summary>
    /// Use this to format graphic ops
    /// listed below
    /// </summary>
    std::string gr(std::list<std::string> ops) const
    {
        if (ops.empty()) {
            return CSI + "m";  // Return reset if no ops provided
        }

        std::string str = CSI;
        for (const auto& op : ops) {
            str += op + ";";
        }
        str.pop_back();  // More efficient than substr
        str += "m";
        return str;
    }

    /// <summary>
    /// Use this to format a single op
    /// </summary>
    std::string gr(std::string op) const
    {
        return CSI + op + "m";
    }

    /// <summary>
    /// graphic ops
    /// </summary>
    const std::string RESET_ALL = "0";
    const std::string BOLD = "1";
    const std::string BOLD_OFF = "22";
    const std::string DIM = "2";
    const std::string DIM_OFF = "22";
    const std::string ITALIC = "3";
    const std::string ITALIC_OFF = "23";
    const std::string UNDERLINE = "4";
    const std::string UNDERLINE_OFF = "24";
    const std::string BLINK = "5";
    const std::string BLINK_OFF = "25";
    const std::string INVERSE = "7";
    const std::string INVERSE_OFF = "27";
    const std::string HIDDEN = "8";
    const std::string HIDDEN_OFF = "28";
    const std::string STRIKETHROUGH = "9";
    const std::string STRIKETHROUGH_OFF = "29";

    // Colors
    const std::string BLACK_FOREGROUND = "30";
    const std::string BLACK_BACKGROUND = "40";
    const std::string RED_FOREGROUND = "31";
    const std::string RED_BACKGROUND = "41";
    const std::string GREEN_FOREGROUND = "32";
    const std::string GREEN_BACKGROUND = "42";
    const std::string YELLOW_FOREGROUND = "33";
    const std::string YELLOW_BACKGROUND = "43";
    const std::string BLUE_FOREGROUND = "34";
    const std::string BLUE_BACKGROUND = "44";
    const std::string MAGENTA_FOREGROUND = "35";
    const std::string MAGENTA_BACKGROUND = "45";
    const std::string CYAN_FOREGROUND = "36";
    const std::string CYAN_BACKGROUND = "46";
    const std::string WHITE_FOREGROUND = "37";
    const std::string WHITE_BACKGROUND = "47";
    const std::string BRIGHT_BLACK_FOREGROUND = "90";
    const std::string BRIGHT_BLACK_BACKGROUND = "100";
    const std::string BRIGHT_RED_FOREGROUND = "91";
    const std::string BRIGHT_RED_BACKGROUND = "101";
    const std::string BRIGHT_GREEN_FOREGROUND = "92";
    const std::string BRIGHT_GREEN_BACKGROUND = "102";
    const std::string BRIGHT_YELLOW_FOREGROUND = "93";
    const std::string BRIGHT_YELLOW_BACKGROUND = "103";
    const std::string BRIGHT_BLUE_FOREGROUND = "94";
    const std::string BRIGHT_BLUE_BACKGROUND = "104";
    const std::string BRIGHT_MAGENTA_FOREGROUND = "95";
    const std::string BRIGHT_MAGENTA_BACKGROUND = "105";
    const std::string BRIGHT_CYAN_FOREGROUND = "96";
    const std::string BRIGHT_CYAN_BACKGROUND = "106";
    const std::string BRIGHT_WHITE_FOREGROUND = "97";
    const std::string BRIGHT_WHITE_BACKGROUND = "107";

    const std::string FOREGROUND = "38";
    const std::string BACKGROUND = "48";
    const std::string _256_COLORS = "5";
    const std::string _24BIT_COLOR = "2";

    // provided for convenience
    std::string foreground(int c) const
    {
        if (c < 0 || c > 255) {
            throw std::out_of_range("foreground color must be in the range 0-255");
        }
        return gr({ FOREGROUND, _256_COLORS, std::to_string(c) });
    }

    std::string background(int c) const
    {
        if (c < 0 || c > 255) {
            throw std::out_of_range("background color must be in the range 0-255");
        }
        return gr({ BACKGROUND, _256_COLORS, std::to_string(c) });
    }

    std::string rgb_foreground(int r, int g, int b) const
    {
        if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
            throw std::out_of_range("RGB values must be in the range 0-255");
        }
        return gr({ FOREGROUND, _24BIT_COLOR, std::to_string(r), std::to_string(g), std::to_string(b) });
    }

    std::string rgb_background(int r, int g, int b) const
    {
        if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
            throw std::out_of_range("RGB values must be in the range 0-255");
        }
        return gr({ BACKGROUND, _24BIT_COLOR, std::to_string(r), std::to_string(g), std::to_string(b) });
    }

    // Screen modes
    const std::string SCREEN_MODE_40_25_MONO_TEXT = "0";
    const std::string SCREEN_MODE_40_25_COLOR_TEXT = "1";
    const std::string SCREEN_MODE_80_25_MONO_TEXT = "2";
    const std::string SCREEN_MODE_80_25_COLOR_TEXT = "3";
    const std::string SCREEN_MODE_320_200_4COLOR_GR = "4";
    const std::string SCREEN_MODE_320_200_MONO_GR = "5";
    const std::string SCREEN_MODE_640_200_MONO_GR = "6";
    const std::string SCREEN_MODE_LINE_WRAP = "7";
    const std::string SCREEN_MODE_320_200_COLOR_GR = "13";
    const std::string SCREEN_MODE_640_200_16COLOR_GR = "14";
    const std::string SCREEN_MODE_640_350_2COLOR_GR = "15";
    const std::string SCREEN_MODE_640_350_16COLOR_GR = "16";
    const std::string SCREEN_MODE_640_480_MONO_GR = "17";
    const std::string SCREEN_MODE_640_480_16COLOR_GR = "18";
    const std::string SCREEN_MODE_320_200_256COLOR_GR = "19";

    std::string screenmode(std::string mode) const
    {
        return CSI + "=" + mode + "h";
    }

    std::string reset_screenmode(std::string mode) const
    {
        return CSI + "=" + mode + "l";
    }

    // More screen control
    const std::string INVISIBLE_CURSOR = CSI + "?25l";
    const std::string VISIBLE_CURSOR = CSI + "?25h";
    const std::string RESTORE_SCREEN = CSI + "?47l";
    const std::string SAVE_SCREEN = CSI + "?47h";
    const std::string ENABLE_ALT_BUFFER = CSI + "?1049h";
    const std::string DISABLE_ALT_BUFFER = CSI + "?1049l";
};