#ifndef COLOR_HPP
#define COLOR_HPP

#include <algorithm>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <windows.h>

namespace hue
{
    constexpr int DEFAULT_COLOR = 7;
    constexpr int BAD_COLOR = -256;

    const std::map<std::string, int> CODES = {
        {"black",            0}, {"k",   0},
        {"blue",             1}, {"b",   1},
        {"green",            2}, {"g",   2},
        {"aqua",             3}, {"a",   3},
        {"red",              4}, {"r",   4},
        {"purple",           5}, {"p",   5},
        {"yellow",           6}, {"y",   6},
        {"white",            7}, {"w",   7},
        {"grey",             8}, {"e",   8},
        {"light blue",       9}, {"lb",  9},
        {"light green",     10}, {"lg", 10},
        {"light aqua",      11}, {"la", 11},
        {"light red",       12}, {"lr", 12},
        {"light purple",    13}, {"lp", 13},
        {"light yellow",    14}, {"ly", 14},
        {"bright white",    15}, {"bw", 15}
    };

    const std::map<int, std::string> NAMES = {
        { 0, "black"},
        { 1, "blue"},
        { 2, "green"},
        { 3, "aqua"},
        { 4, "red"},
        { 5, "purple"},
        { 6, "yellow"},
        { 7, "white"},
        { 8, "grey"},
        { 9, "light blue"},
        {10, "light green"},
        {11, "light aqua"},
        {12, "light red"},
        {13, "light purple"},
        {14, "light yellow"},
        {15, "bright white"}
    };

    inline bool is_good(int c)
    {
        return 0 <= c && c < 256;
    }

    inline int itoc(int c)
    {
        return is_good(c) ? c : BAD_COLOR;
    }

    inline int itoc(int a, int b)
    {
        return itoc(a + b * 16);
    }

    // std::string to color
    int stoc(std::string a)
    {
        // convert s to lowercase, and format variants like  "light_blue"
        std::transform(a.begin(), a.end(), a.begin(), [](char c)
            {
                if ('A' <= c && c <= 'Z')
                    c = c - 'A' + 'a';
                else if (c == '_' || c == '-')
                    c = ' ';
                return c;
            });

        // operator[] on std::map is non-const, use std::map::at instead
        return (CODES.find(a) != CODES.end()) ? CODES.at(a) : BAD_COLOR;
    }

    int stoc(std::string a, std::string b)
    {
        return itoc(stoc(a), stoc(b));
    }

    std::string ctos(int c)
    {
        return (0 <= c && c < 256) ?
            "(text) " + NAMES.at(c % 16) + " + " +
            "(background) " + NAMES.at(c / 16) :
            "BAD COLOR";
    }

    int get()
    {
        CONSOLE_SCREEN_BUFFER_INFO i;
        return GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &i) ?
            i.wAttributes : BAD_COLOR;
    }

    int get_text()
    {
        return (get() != BAD_COLOR) ? get() % 16 : BAD_COLOR;
    }

    int get_background()
    {
        return (get() != BAD_COLOR) ? get() / 16 : BAD_COLOR;
    }

    void set(int c)
    {
        if (is_good(c))
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
    }

    void set(int a, int b)
    {
        set(a + b * 16);
    }

    void set(std::string a, std::string b)
    {
        set(stoc(a) + stoc(b) * 16);
    }

    void set_text(std::string a)
    {
        set(stoc(a), get_background());
    }

    void set_background(std::string b)
    {
        set(get_text(), stoc(b));
    }

    void reset()
    {
        set(DEFAULT_COLOR);
    }

    int invert(int c)
    {
        if (is_good(c)) {
            int a = c % 16;
            int b = c / 16;
            return b + a * 16;
        }
        else
            return BAD_COLOR;
    }

    std::ostream& reset(std::ostream& os) { reset();                return os; }
    std::ostream& black(std::ostream& os) { set_text("k");          return os; }
    std::ostream& blue(std::ostream& os) { set_text("b");          return os; }
    std::ostream& green(std::ostream& os) { set_text("g");          return os; }
    std::ostream& aqua(std::ostream& os) { set_text("a");          return os; }
    std::ostream& red(std::ostream& os) { set_text("r");          return os; }
    std::ostream& purple(std::ostream& os) { set_text("p");          return os; }
    std::ostream& yellow(std::ostream& os) { set_text("y");          return os; }
    std::ostream& white(std::ostream& os) { set_text("w");          return os; }
    std::ostream& grey(std::ostream& os) { set_text("e");          return os; }
    std::ostream& light_blue(std::ostream& os) { set_text("lb");         return os; }
    std::ostream& light_green(std::ostream& os) { set_text("lg");         return os; }
    std::ostream& light_aqua(std::ostream& os) { set_text("la");         return os; }
    std::ostream& light_red(std::ostream& os) { set_text("lr");         return os; }
    std::ostream& light_purple(std::ostream& os) { set_text("lp");         return os; }
    std::ostream& light_yellow(std::ostream& os) { set_text("ly");         return os; }
    std::ostream& bright_white(std::ostream& os) { set_text("bw");         return os; }
    std::ostream& on_black(std::ostream& os) { set_background("k");    return os; }
    std::ostream& on_blue(std::ostream& os) { set_background("b");    return os; }
    std::ostream& on_green(std::ostream& os) { set_background("g");    return os; }
    std::ostream& on_aqua(std::ostream& os) { set_background("a");    return os; }
    std::ostream& on_red(std::ostream& os) { set_background("r");    return os; }
    std::ostream& on_purple(std::ostream& os) { set_background("p");    return os; }
    std::ostream& on_yellow(std::ostream& os) { set_background("y");    return os; }
    std::ostream& on_white(std::ostream& os) { set_background("w");    return os; }
    std::ostream& on_grey(std::ostream& os) { set_background("e");    return os; }
    std::ostream& on_light_blue(std::ostream& os) { set_background("lb");   return os; }
    std::ostream& on_light_green(std::ostream& os) { set_background("lg");   return os; }
    std::ostream& on_light_aqua(std::ostream& os) { set_background("la");   return os; }
    std::ostream& on_light_red(std::ostream& os) { set_background("lr");   return os; }
    std::ostream& on_light_purple(std::ostream& os) { set_background("lp");   return os; }
    std::ostream& on_light_yellow(std::ostream& os) { set_background("ly");   return os; }
    std::ostream& on_bright_white(std::ostream& os) { set_background("bw");   return os; }
    std::ostream& black_on_black(std::ostream& os) { set("k", "k");   return os; }
    std::ostream& black_on_blue(std::ostream& os) { set("k", "b");   return os; }
    std::ostream& black_on_green(std::ostream& os) { set("k", "g");   return os; }
    std::ostream& black_on_aqua(std::ostream& os) { set("k", "a");   return os; }
    std::ostream& black_on_red(std::ostream& os) { set("k", "r");   return os; }
    std::ostream& black_on_purple(std::ostream& os) { set("k", "p");   return os; }
    std::ostream& black_on_yellow(std::ostream& os) { set("k", "y");   return os; }
    std::ostream& black_on_white(std::ostream& os) { set("k", "w");   return os; }
    std::ostream& black_on_grey(std::ostream& os) { set("k", "e");   return os; }
    std::ostream& black_on_light_blue(std::ostream& os) { set("k", "lb");  return os; }
    std::ostream& black_on_light_green(std::ostream& os) { set("k", "lg");  return os; }
    std::ostream& black_on_light_aqua(std::ostream& os) { set("k", "la");  return os; }
    std::ostream& black_on_light_red(std::ostream& os) { set("k", "lr");  return os; }
    std::ostream& black_on_light_purple(std::ostream& os) { set("k", "lp");  return os; }
    std::ostream& black_on_light_yellow(std::ostream& os) { set("k", "ly");  return os; }
    std::ostream& black_on_bright_white(std::ostream& os) { set("k", "bw");  return os; }
    std::ostream& blue_on_black(std::ostream& os) { set("b", "k");   return os; }
    std::ostream& blue_on_blue(std::ostream& os) { set("b", "b");   return os; }
    std::ostream& blue_on_green(std::ostream& os) { set("b", "g");   return os; }
    std::ostream& blue_on_aqua(std::ostream& os) { set("b", "a");   return os; }
    std::ostream& blue_on_red(std::ostream& os) { set("b", "r");   return os; }
    std::ostream& blue_on_purple(std::ostream& os) { set("b", "p");   return os; }
    std::ostream& blue_on_yellow(std::ostream& os) { set("b", "y");   return os; }
    std::ostream& blue_on_white(std::ostream& os) { set("b", "w");   return os; }
    std::ostream& blue_on_grey(std::ostream& os) { set("b", "e");   return os; }
    std::ostream& blue_on_light_blue(std::ostream& os) { set("b", "lb");  return os; }
    std::ostream& blue_on_light_green(std::ostream& os) { set("b", "lg");  return os; }
    std::ostream& blue_on_light_aqua(std::ostream& os) { set("b", "la");  return os; }
    std::ostream& blue_on_light_red(std::ostream& os) { set("b", "lr");  return os; }
    std::ostream& blue_on_light_purple(std::ostream& os) { set("b", "lp");  return os; }
    std::ostream& blue_on_light_yellow(std::ostream& os) { set("b", "ly");  return os; }
    std::ostream& blue_on_bright_white(std::ostream& os) { set("b", "bw");  return os; }
    std::ostream& green_on_black(std::ostream& os) { set("g", "k");   return os; }
    std::ostream& green_on_blue(std::ostream& os) { set("g", "b");   return os; }
    std::ostream& green_on_green(std::ostream& os) { set("g", "g");   return os; }
    std::ostream& green_on_aqua(std::ostream& os) { set("g", "a");   return os; }
    std::ostream& green_on_red(std::ostream& os) { set("g", "r");   return os; }
    std::ostream& green_on_purple(std::ostream& os) { set("g", "p");   return os; }
    std::ostream& green_on_yellow(std::ostream& os) { set("g", "y");   return os; }
    std::ostream& green_on_white(std::ostream& os) { set("g", "w");   return os; }
    std::ostream& green_on_grey(std::ostream& os) { set("g", "e");   return os; }
    std::ostream& green_on_light_blue(std::ostream& os) { set("g", "lb");  return os; }
    std::ostream& green_on_light_green(std::ostream& os) { set("g", "lg");  return os; }
    std::ostream& green_on_light_aqua(std::ostream& os) { set("g", "la");  return os; }
    std::ostream& green_on_light_red(std::ostream& os) { set("g", "lr");  return os; }
    std::ostream& green_on_light_purple(std::ostream& os) { set("g", "lp");  return os; }
    std::ostream& green_on_light_yellow(std::ostream& os) { set("g", "ly");  return os; }
    std::ostream& green_on_bright_white(std::ostream& os) { set("g", "bw");  return os; }
    std::ostream& aqua_on_black(std::ostream& os) { set("a", "k");   return os; }
    std::ostream& aqua_on_blue(std::ostream& os) { set("a", "b");   return os; }
    std::ostream& aqua_on_green(std::ostream& os) { set("a", "g");   return os; }
    std::ostream& aqua_on_aqua(std::ostream& os) { set("a", "a");   return os; }
    std::ostream& aqua_on_red(std::ostream& os) { set("a", "r");   return os; }
    std::ostream& aqua_on_purple(std::ostream& os) { set("a", "p");   return os; }
    std::ostream& aqua_on_yellow(std::ostream& os) { set("a", "y");   return os; }
    std::ostream& aqua_on_white(std::ostream& os) { set("a", "w");   return os; }
    std::ostream& aqua_on_grey(std::ostream& os) { set("a", "e");   return os; }
    std::ostream& aqua_on_light_blue(std::ostream& os) { set("a", "lb");  return os; }
    std::ostream& aqua_on_light_green(std::ostream& os) { set("a", "lg");  return os; }
    std::ostream& aqua_on_light_aqua(std::ostream& os) { set("a", "la");  return os; }
    std::ostream& aqua_on_light_red(std::ostream& os) { set("a", "lr");  return os; }
    std::ostream& aqua_on_light_purple(std::ostream& os) { set("a", "lp");  return os; }
    std::ostream& aqua_on_light_yellow(std::ostream& os) { set("a", "ly");  return os; }
    std::ostream& aqua_on_bright_white(std::ostream& os) { set("a", "bw");  return os; }
    std::ostream& red_on_black(std::ostream& os) { set("r", "k");   return os; }
    std::ostream& red_on_blue(std::ostream& os) { set("r", "b");   return os; }
    std::ostream& red_on_green(std::ostream& os) { set("r", "g");   return os; }
    std::ostream& red_on_aqua(std::ostream& os) { set("r", "a");   return os; }
    std::ostream& red_on_red(std::ostream& os) { set("r", "r");   return os; }
    std::ostream& red_on_purple(std::ostream& os) { set("r", "p");   return os; }
    std::ostream& red_on_yellow(std::ostream& os) { set("r", "y");   return os; }
    std::ostream& red_on_white(std::ostream& os) { set("r", "w");   return os; }
    std::ostream& red_on_grey(std::ostream& os) { set("r", "e");   return os; }
    std::ostream& red_on_light_blue(std::ostream& os) { set("r", "lb");  return os; }
    std::ostream& red_on_light_green(std::ostream& os) { set("r", "lg");  return os; }
    std::ostream& red_on_light_aqua(std::ostream& os) { set("r", "la");  return os; }
    std::ostream& red_on_light_red(std::ostream& os) { set("r", "lr");  return os; }
    std::ostream& red_on_light_purple(std::ostream& os) { set("r", "lp");  return os; }
    std::ostream& red_on_light_yellow(std::ostream& os) { set("r", "ly");  return os; }
    std::ostream& red_on_bright_white(std::ostream& os) { set("r", "bw");  return os; }
    std::ostream& purple_on_black(std::ostream& os) { set("p", "k");   return os; }
    std::ostream& purple_on_blue(std::ostream& os) { set("p", "b");   return os; }
    std::ostream& purple_on_green(std::ostream& os) { set("p", "g");   return os; }
    std::ostream& purple_on_aqua(std::ostream& os) { set("p", "a");   return os; }
    std::ostream& purple_on_red(std::ostream& os) { set("p", "r");   return os; }
    std::ostream& purple_on_purple(std::ostream& os) { set("p", "p");   return os; }
    std::ostream& purple_on_yellow(std::ostream& os) { set("p", "y");   return os; }
    std::ostream& purple_on_white(std::ostream& os) { set("p", "w");   return os; }
    std::ostream& purple_on_grey(std::ostream& os) { set("p", "e");   return os; }
    std::ostream& purple_on_light_blue(std::ostream& os) { set("p", "lb");  return os; }
    std::ostream& purple_on_light_green(std::ostream& os) { set("p", "lg");  return os; }
    std::ostream& purple_on_light_aqua(std::ostream& os) { set("p", "la");  return os; }
    std::ostream& purple_on_light_red(std::ostream& os) { set("p", "lr");  return os; }
    std::ostream& purple_on_light_purple(std::ostream& os) { set("p", "lp");  return os; }
    std::ostream& purple_on_light_yellow(std::ostream& os) { set("p", "ly");  return os; }
    std::ostream& purple_on_bright_white(std::ostream& os) { set("p", "bw");  return os; }
    std::ostream& yellow_on_black(std::ostream& os) { set("y", "k");   return os; }
    std::ostream& yellow_on_blue(std::ostream& os) { set("y", "b");   return os; }
    std::ostream& yellow_on_green(std::ostream& os) { set("y", "g");   return os; }
    std::ostream& yellow_on_aqua(std::ostream& os) { set("y", "a");   return os; }
    std::ostream& yellow_on_red(std::ostream& os) { set("y", "r");   return os; }
    std::ostream& yellow_on_purple(std::ostream& os) { set("y", "p");   return os; }
    std::ostream& yellow_on_yellow(std::ostream& os) { set("y", "y");   return os; }
    std::ostream& yellow_on_white(std::ostream& os) { set("y", "w");   return os; }
    std::ostream& yellow_on_grey(std::ostream& os) { set("y", "e");   return os; }
    std::ostream& yellow_on_light_blue(std::ostream& os) { set("y", "lb");  return os; }
    std::ostream& yellow_on_light_green(std::ostream& os) { set("y", "lg");  return os; }
    std::ostream& yellow_on_light_aqua(std::ostream& os) { set("y", "la");  return os; }
    std::ostream& yellow_on_light_red(std::ostream& os) { set("y", "lr");  return os; }
    std::ostream& yellow_on_light_purple(std::ostream& os) { set("y", "lp");  return os; }
    std::ostream& yellow_on_light_yellow(std::ostream& os) { set("y", "ly");  return os; }
    std::ostream& yellow_on_bright_white(std::ostream& os) { set("y", "bw");  return os; }
    std::ostream& white_on_black(std::ostream& os) { set("w", "k");   return os; }
    std::ostream& white_on_blue(std::ostream& os) { set("w", "b");   return os; }
    std::ostream& white_on_green(std::ostream& os) { set("w", "g");   return os; }
    std::ostream& white_on_aqua(std::ostream& os) { set("w", "a");   return os; }
    std::ostream& white_on_red(std::ostream& os) { set("w", "r");   return os; }
    std::ostream& white_on_purple(std::ostream& os) { set("w", "p");   return os; }
    std::ostream& white_on_yellow(std::ostream& os) { set("w", "y");   return os; }
    std::ostream& white_on_white(std::ostream& os) { set("w", "w");   return os; }
    std::ostream& white_on_grey(std::ostream& os) { set("w", "e");   return os; }
    std::ostream& white_on_light_blue(std::ostream& os) { set("w", "lb");  return os; }
    std::ostream& white_on_light_green(std::ostream& os) { set("w", "lg");  return os; }
    std::ostream& white_on_light_aqua(std::ostream& os) { set("w", "la");  return os; }
    std::ostream& white_on_light_red(std::ostream& os) { set("w", "lr");  return os; }
    std::ostream& white_on_light_purple(std::ostream& os) { set("w", "lp");  return os; }
    std::ostream& white_on_light_yellow(std::ostream& os) { set("w", "ly");  return os; }
    std::ostream& white_on_bright_white(std::ostream& os) { set("w", "bw");  return os; }
    std::ostream& grey_on_black(std::ostream& os) { set("e", "k");   return os; }
    std::ostream& grey_on_blue(std::ostream& os) { set("e", "b");   return os; }
    std::ostream& grey_on_green(std::ostream& os) { set("e", "g");   return os; }
    std::ostream& grey_on_aqua(std::ostream& os) { set("e", "a");   return os; }
    std::ostream& grey_on_red(std::ostream& os) { set("e", "r");   return os; }
    std::ostream& grey_on_purple(std::ostream& os) { set("e", "p");   return os; }
    std::ostream& grey_on_yellow(std::ostream& os) { set("e", "y");   return os; }
    std::ostream& grey_on_white(std::ostream& os) { set("e", "w");   return os; }
    std::ostream& grey_on_grey(std::ostream& os) { set("e", "e");   return os; }
    std::ostream& grey_on_light_blue(std::ostream& os) { set("e", "lb");  return os; }
    std::ostream& grey_on_light_green(std::ostream& os) { set("e", "lg");  return os; }
    std::ostream& grey_on_light_aqua(std::ostream& os) { set("e", "la");  return os; }
    std::ostream& grey_on_light_red(std::ostream& os) { set("e", "lr");  return os; }
    std::ostream& grey_on_light_purple(std::ostream& os) { set("e", "lp");  return os; }
    std::ostream& grey_on_light_yellow(std::ostream& os) { set("e", "ly");  return os; }
    std::ostream& grey_on_bright_white(std::ostream& os) { set("e", "bw");  return os; }
    std::ostream& light_blue_on_black(std::ostream& os) { set("lb", "k");  return os; }
    std::ostream& light_blue_on_blue(std::ostream& os) { set("lb", "b");  return os; }
    std::ostream& light_blue_on_green(std::ostream& os) { set("lb", "g");  return os; }
    std::ostream& light_blue_on_aqua(std::ostream& os) { set("lb", "a");  return os; }
    std::ostream& light_blue_on_red(std::ostream& os) { set("lb", "r");  return os; }
    std::ostream& light_blue_on_purple(std::ostream& os) { set("lb", "p");  return os; }
    std::ostream& light_blue_on_yellow(std::ostream& os) { set("lb", "y");  return os; }
    std::ostream& light_blue_on_white(std::ostream& os) { set("lb", "w");  return os; }
    std::ostream& light_blue_on_grey(std::ostream& os) { set("lb", "e");  return os; }
    std::ostream& light_blue_on_light_blue(std::ostream& os) { set("lb", "lb"); return os; }
    std::ostream& light_blue_on_light_green(std::ostream& os) { set("lb", "lg"); return os; }
    std::ostream& light_blue_on_light_aqua(std::ostream& os) { set("lb", "la"); return os; }
    std::ostream& light_blue_on_light_red(std::ostream& os) { set("lb", "lr"); return os; }
    std::ostream& light_blue_on_light_purple(std::ostream& os) { set("lb", "lp"); return os; }
    std::ostream& light_blue_on_light_yellow(std::ostream& os) { set("lb", "ly"); return os; }
    std::ostream& light_blue_on_bright_white(std::ostream& os) { set("lb", "bw"); return os; }
    std::ostream& light_green_on_black(std::ostream& os) { set("lg", "k");  return os; }
    std::ostream& light_green_on_blue(std::ostream& os) { set("lg", "b");  return os; }
    std::ostream& light_green_on_green(std::ostream& os) { set("lg", "g");  return os; }
    std::ostream& light_green_on_aqua(std::ostream& os) { set("lg", "a");  return os; }
    std::ostream& light_green_on_red(std::ostream& os) { set("lg", "r");  return os; }
    std::ostream& light_green_on_purple(std::ostream& os) { set("lg", "p");  return os; }
    std::ostream& light_green_on_yellow(std::ostream& os) { set("lg", "y");  return os; }
    std::ostream& light_green_on_white(std::ostream& os) { set("lg", "w");  return os; }
    std::ostream& light_green_on_grey(std::ostream& os) { set("lg", "e");  return os; }
    std::ostream& light_green_on_light_blue(std::ostream& os) { set("lg", "lb"); return os; }
    std::ostream& light_green_on_light_green(std::ostream& os) { set("lg", "lg"); return os; }
    std::ostream& light_green_on_light_aqua(std::ostream& os) { set("lg", "la"); return os; }
    std::ostream& light_green_on_light_red(std::ostream& os) { set("lg", "lr"); return os; }
    std::ostream& light_green_on_light_purple(std::ostream& os) { set("lg", "lp"); return os; }
    std::ostream& light_green_on_light_yellow(std::ostream& os) { set("lg", "ly"); return os; }
    std::ostream& light_green_on_bright_white(std::ostream& os) { set("lg", "bw"); return os; }
    std::ostream& light_aqua_on_black(std::ostream& os) { set("la", "k");  return os; }
    std::ostream& light_aqua_on_blue(std::ostream& os) { set("la", "b");  return os; }
    std::ostream& light_aqua_on_green(std::ostream& os) { set("la", "g");  return os; }
    std::ostream& light_aqua_on_aqua(std::ostream& os) { set("la", "a");  return os; }
    std::ostream& light_aqua_on_red(std::ostream& os) { set("la", "r");  return os; }
    std::ostream& light_aqua_on_purple(std::ostream& os) { set("la", "p");  return os; }
    std::ostream& light_aqua_on_yellow(std::ostream& os) { set("la", "y");  return os; }
    std::ostream& light_aqua_on_white(std::ostream& os) { set("la", "w");  return os; }
    std::ostream& light_aqua_on_grey(std::ostream& os) { set("la", "e");  return os; }
    std::ostream& light_aqua_on_light_blue(std::ostream& os) { set("la", "lb"); return os; }
    std::ostream& light_aqua_on_light_green(std::ostream& os) { set("la", "lg"); return os; }
    std::ostream& light_aqua_on_light_aqua(std::ostream& os) { set("la", "la"); return os; }
    std::ostream& light_aqua_on_light_red(std::ostream& os) { set("la", "lr"); return os; }
    std::ostream& light_aqua_on_light_purple(std::ostream& os) { set("la", "lp"); return os; }
    std::ostream& light_aqua_on_light_yellow(std::ostream& os) { set("la", "ly"); return os; }
    std::ostream& light_aqua_on_bright_white(std::ostream& os) { set("la", "bw"); return os; }
    std::ostream& light_red_on_black(std::ostream& os) { set("lr", "k");  return os; }
    std::ostream& light_red_on_blue(std::ostream& os) { set("lr", "b");  return os; }
    std::ostream& light_red_on_green(std::ostream& os) { set("lr", "g");  return os; }
    std::ostream& light_red_on_aqua(std::ostream& os) { set("lr", "a");  return os; }
    std::ostream& light_red_on_red(std::ostream& os) { set("lr", "r");  return os; }
    std::ostream& light_red_on_purple(std::ostream& os) { set("lr", "p");  return os; }
    std::ostream& light_red_on_yellow(std::ostream& os) { set("lr", "y");  return os; }
    std::ostream& light_red_on_white(std::ostream& os) { set("lr", "w");  return os; }
    std::ostream& light_red_on_grey(std::ostream& os) { set("lr", "e");  return os; }
    std::ostream& light_red_on_light_blue(std::ostream& os) { set("lr", "lb"); return os; }
    std::ostream& light_red_on_light_green(std::ostream& os) { set("lr", "lg"); return os; }
    std::ostream& light_red_on_light_aqua(std::ostream& os) { set("lr", "la"); return os; }
    std::ostream& light_red_on_light_red(std::ostream& os) { set("lr", "lr"); return os; }
    std::ostream& light_red_on_light_purple(std::ostream& os) { set("lr", "lp"); return os; }
    std::ostream& light_red_on_light_yellow(std::ostream& os) { set("lr", "ly"); return os; }
    std::ostream& light_red_on_bright_white(std::ostream& os) { set("lr", "bw"); return os; }
    std::ostream& light_purple_on_black(std::ostream& os) { set("lp", "k");  return os; }
    std::ostream& light_purple_on_blue(std::ostream& os) { set("lp", "b");  return os; }
    std::ostream& light_purple_on_green(std::ostream& os) { set("lp", "g");  return os; }
    std::ostream& light_purple_on_aqua(std::ostream& os) { set("lp", "a");  return os; }
    std::ostream& light_purple_on_red(std::ostream& os) { set("lp", "r");  return os; }
    std::ostream& light_purple_on_purple(std::ostream& os) { set("lp", "p");  return os; }
    std::ostream& light_purple_on_yellow(std::ostream& os) { set("lp", "y");  return os; }
    std::ostream& light_purple_on_white(std::ostream& os) { set("lp", "w");  return os; }
    std::ostream& light_purple_on_grey(std::ostream& os) { set("lp", "e");  return os; }
    std::ostream& light_purple_on_light_blue(std::ostream& os) { set("lp", "lb"); return os; }
    std::ostream& light_purple_on_light_green(std::ostream& os) { set("lp", "lg"); return os; }
    std::ostream& light_purple_on_light_aqua(std::ostream& os) { set("lp", "la"); return os; }
    std::ostream& light_purple_on_light_red(std::ostream& os) { set("lp", "lr"); return os; }
    std::ostream& light_purple_on_light_purple(std::ostream& os) { set("lp", "lp"); return os; }
    std::ostream& light_purple_on_light_yellow(std::ostream& os) { set("lp", "ly"); return os; }
    std::ostream& light_purple_on_bright_white(std::ostream& os) { set("lp", "bw"); return os; }
    std::ostream& light_yellow_on_black(std::ostream& os) { set("ly", "k");  return os; }
    std::ostream& light_yellow_on_blue(std::ostream& os) { set("ly", "b");  return os; }
    std::ostream& light_yellow_on_green(std::ostream& os) { set("ly", "g");  return os; }
    std::ostream& light_yellow_on_aqua(std::ostream& os) { set("ly", "a");  return os; }
    std::ostream& light_yellow_on_red(std::ostream& os) { set("ly", "r");  return os; }
    std::ostream& light_yellow_on_purple(std::ostream& os) { set("ly", "p");  return os; }
    std::ostream& light_yellow_on_yellow(std::ostream& os) { set("ly", "y");  return os; }
    std::ostream& light_yellow_on_white(std::ostream& os) { set("ly", "w");  return os; }
    std::ostream& light_yellow_on_grey(std::ostream& os) { set("ly", "e");  return os; }
    std::ostream& light_yellow_on_light_blue(std::ostream& os) { set("ly", "lb"); return os; }
    std::ostream& light_yellow_on_light_green(std::ostream& os) { set("ly", "lg"); return os; }
    std::ostream& light_yellow_on_light_aqua(std::ostream& os) { set("ly", "la"); return os; }
    std::ostream& light_yellow_on_light_red(std::ostream& os) { set("ly", "lr"); return os; }
    std::ostream& light_yellow_on_light_purple(std::ostream& os) { set("ly", "lp"); return os; }
    std::ostream& light_yellow_on_light_yellow(std::ostream& os) { set("ly", "ly"); return os; }
    std::ostream& light_yellow_on_bright_white(std::ostream& os) { set("ly", "bw"); return os; }
    std::ostream& bright_white_on_black(std::ostream& os) { set("bw", "k");  return os; }
    std::ostream& bright_white_on_blue(std::ostream& os) { set("bw", "b");  return os; }
    std::ostream& bright_white_on_green(std::ostream& os) { set("bw", "g");  return os; }
    std::ostream& bright_white_on_aqua(std::ostream& os) { set("bw", "a");  return os; }
    std::ostream& bright_white_on_red(std::ostream& os) { set("bw", "r");  return os; }
    std::ostream& bright_white_on_purple(std::ostream& os) { set("bw", "p");  return os; }
    std::ostream& bright_white_on_yellow(std::ostream& os) { set("bw", "y");  return os; }
    std::ostream& bright_white_on_white(std::ostream& os) { set("bw", "w");  return os; }
    std::ostream& bright_white_on_grey(std::ostream& os) { set("bw", "e");  return os; }
    std::ostream& bright_white_on_light_blue(std::ostream& os) { set("bw", "lb"); return os; }
    std::ostream& bright_white_on_light_green(std::ostream& os) { set("bw", "lg"); return os; }
    std::ostream& bright_white_on_light_aqua(std::ostream& os) { set("bw", "la"); return os; }
    std::ostream& bright_white_on_light_red(std::ostream& os) { set("bw", "lr"); return os; }
    std::ostream& bright_white_on_light_purple(std::ostream& os) { set("bw", "lp"); return os; }
    std::ostream& bright_white_on_light_yellow(std::ostream& os) { set("bw", "ly"); return os; }
    std::ostream& bright_white_on_bright_white(std::ostream& os) { set("bw", "bw"); return os; }
}


namespace dye
{
    template<typename T>
    using bar = typename std::conditional<std::is_same<T, const char*>::value, std::string, T>::type;

    template<typename T> class colorful;
    template<typename T> class item;

    template<typename T>
    class colorful : private std::list<item<T>>
    {
    public:
        using std::list<item<T>>::list;

        colorful<T>& operator+=(const colorful<T>& rhs)
        {
            this->insert(this->end(), rhs.begin(), rhs.end());
            return *this;
        }

        colorful<T>& operator+=(colorful<T>&& rhs)
        {
            this->splice(this->end(), std::move(rhs));
            return *this;
        }

        colorful<T>& operator+=(T t)
        {
            this->push_back(std::move(t));
            return *this;
        }

        void push_front(T t)
        {
            this->std::list<item<T>>::push_front(item<T>(std::move(t)));
        }

        void push_back(T t)
        {
            this->std::list<item<T>>::push_back(item<T>(std::move(t)));
        }

        colorful<T>& invert()
        {
            for (auto& elem : *this)
                elem.invert();
            return *this;
        }

        template<typename U>
        friend std::ostream& operator<<(std::ostream&, const colorful<U>&);

        template<typename U>
        friend colorful<U> invert(colorful<U> col);
    };

    template<typename T>
    colorful<T> operator+(colorful<T> lhs, colorful<T> rhs)
    {
        colorful<T> res(std::move(lhs));
        return res += rhs;
    }

    template<typename T>
    colorful<T> operator+(colorful<T> lhs, std::string rhs)
    {
        colorful<T> res(std::move(lhs));
        res.push_back(std::move(rhs));
        return res;
    }

    template<typename T>
    colorful<T> operator+(const std::string& lhs, colorful<T> rhs)
    {
        colorful<T> res(std::move(rhs));
        res.push_front(std::move(lhs));
        return res;
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& os, const colorful<T>& colorful)
    {
        for (const auto& elem : colorful)
            os << elem;
        return os;
    }

    template<typename T>
    colorful<T> invert(colorful<T> col)
    {
        colorful<T> res(std::move(col));
        for (auto& elem : res)
            elem.invert();
        return res;
    }

    template<typename T>
    class item
    {
        T thing;
        int color;

    public:
        item(T t) : thing(std::move(t)), color(hue::get()) {}
        item(T t, int a) : thing(std::move(t)), color(hue::itoc(a)) {}
        item(T t, int a, int b) : thing(std::move(t)), color(hue::itoc(a, b)) {}
        item(T t, std::string a) : thing(std::move(t)), color(hue::stoc(a)) {}
        item(T t, std::string a, std::string b) : thing(std::move(t)), color(hue::stoc(a, b)) {}

        item<T>& invert()
        {
            color = hue::invert(color);
            return *this;
        }

        template<typename U>
        friend class colorful;

        template<typename U>
        friend std::ostream& operator<<(std::ostream&, const item<U>&);
    };

    template<typename T>
    std::ostream& operator<<(std::ostream& os, const item<T>& it)
    {
        hue::set(it.color);
        os << it.thing;
        hue::reset();
        return os;
    }

    template<typename T> using R = colorful<bar<T>>;
    template<typename T> using S = item<bar<T>>;

    template<typename T> R<T> colorize(T t, std::string a) { return R<T> { S<T>(t, a) }; }
    template<typename T> R<T> vanilla(T t) { return R<T> { S<T>(t) }; }
    template<typename T> R<T> black(T t) { return R<T> { S<T>(t, "k") }; }
    template<typename T> R<T> blue(T t) { return R<T> { S<T>(t, "b") }; }
    template<typename T> R<T> green(T t) { return R<T> { S<T>(t, "g") }; }
    template<typename T> R<T> aqua(T t) { return R<T> { S<T>(t, "a") }; }
    template<typename T> R<T> red(T t) { return R<T> { S<T>(t, "r") }; }
    template<typename T> R<T> purple(T t) { return R<T> { S<T>(t, "p") }; }
    template<typename T> R<T> yellow(T t) { return R<T> { S<T>(t, "y") }; }
    template<typename T> R<T> white(T t) { return R<T> { S<T>(t, "w") }; }
    template<typename T> R<T> grey(T t) { return R<T> { S<T>(t, "e") }; }
    template<typename T> R<T> light_blue(T t) { return R<T> { S<T>(t, "lb") }; }
    template<typename T> R<T> light_green(T t) { return R<T> { S<T>(t, "lg") }; }
    template<typename T> R<T> light_aqua(T t) { return R<T> { S<T>(t, "la") }; }
    template<typename T> R<T> light_red(T t) { return R<T> { S<T>(t, "lr") }; }
    template<typename T> R<T> light_purple(T t) { return R<T> { S<T>(t, "lp") }; }
    template<typename T> R<T> light_yellow(T t) { return R<T> { S<T>(t, "ly") }; }
    template<typename T> R<T> bright_white(T t) { return R<T> { S<T>(t, "bw") }; }
    template<typename T> R<T> on_black(T t) { return R<T> { S<T>(t, "k", "k") }; }
    template<typename T> R<T> on_blue(T t) { return R<T> { S<T>(t, "k", "b") }; }
    template<typename T> R<T> on_green(T t) { return R<T> { S<T>(t, "k", "g") }; }
    template<typename T> R<T> on_aqua(T t) { return R<T> { S<T>(t, "k", "a") }; }
    template<typename T> R<T> on_red(T t) { return R<T> { S<T>(t, "k", "r") }; }
    template<typename T> R<T> on_purple(T t) { return R<T> { S<T>(t, "k", "p") }; }
    template<typename T> R<T> on_yellow(T t) { return R<T> { S<T>(t, "k", "y") }; }
    template<typename T> R<T> on_white(T t) { return R<T> { S<T>(t, "k", "w") }; }
    template<typename T> R<T> on_grey(T t) { return R<T> { S<T>(t, "k", "e") }; }
    template<typename T> R<T> on_light_blue(T t) { return R<T> { S<T>(t, "k", "lb") }; }
    template<typename T> R<T> on_light_green(T t) { return R<T> { S<T>(t, "k", "lg") }; }
    template<typename T> R<T> on_light_aqua(T t) { return R<T> { S<T>(t, "k", "la") }; }
    template<typename T> R<T> on_light_red(T t) { return R<T> { S<T>(t, "k", "lr") }; }
    template<typename T> R<T> on_light_purple(T t) { return R<T> { S<T>(t, "k", "lp") }; }
    template<typename T> R<T> on_light_yellow(T t) { return R<T> { S<T>(t, "k", "ly") }; }
    template<typename T> R<T> on_bright_white(T t) { return R<T> { S<T>(t, "k", "bw") }; }
    template<typename T> R<T> black_on_black(T t) { return R<T> { S<T>(t, "k", "k") }; }
    template<typename T> R<T> black_on_blue(T t) { return R<T> { S<T>(t, "k", "b") }; }
    template<typename T> R<T> black_on_green(T t) { return R<T> { S<T>(t, "k", "g") }; }
    template<typename T> R<T> black_on_aqua(T t) { return R<T> { S<T>(t, "k", "a") }; }
    template<typename T> R<T> black_on_red(T t) { return R<T> { S<T>(t, "k", "r") }; }
    template<typename T> R<T> black_on_purple(T t) { return R<T> { S<T>(t, "k", "p") }; }
    template<typename T> R<T> black_on_yellow(T t) { return R<T> { S<T>(t, "k", "y") }; }
    template<typename T> R<T> black_on_white(T t) { return R<T> { S<T>(t, "k", "w") }; }
    template<typename T> R<T> black_on_grey(T t) { return R<T> { S<T>(t, "k", "e") }; }
    template<typename T> R<T> black_on_light_blue(T t) { return R<T> { S<T>(t, "k", "lb") }; }
    template<typename T> R<T> black_on_light_green(T t) { return R<T> { S<T>(t, "k", "lg") }; }
    template<typename T> R<T> black_on_light_aqua(T t) { return R<T> { S<T>(t, "k", "la") }; }
    template<typename T> R<T> black_on_light_red(T t) { return R<T> { S<T>(t, "k", "lr") }; }
    template<typename T> R<T> black_on_light_purple(T t) { return R<T> { S<T>(t, "k", "lp") }; }
    template<typename T> R<T> black_on_light_yellow(T t) { return R<T> { S<T>(t, "k", "ly") }; }
    template<typename T> R<T> black_on_bright_white(T t) { return R<T> { S<T>(t, "k", "bw") }; }
    template<typename T> R<T> blue_on_black(T t) { return R<T> { S<T>(t, "b", "k") }; }
    template<typename T> R<T> blue_on_blue(T t) { return R<T> { S<T>(t, "b", "b") }; }
    template<typename T> R<T> blue_on_green(T t) { return R<T> { S<T>(t, "b", "g") }; }
    template<typename T> R<T> blue_on_aqua(T t) { return R<T> { S<T>(t, "b", "a") }; }
    template<typename T> R<T> blue_on_red(T t) { return R<T> { S<T>(t, "b", "r") }; }
    template<typename T> R<T> blue_on_purple(T t) { return R<T> { S<T>(t, "b", "p") }; }
    template<typename T> R<T> blue_on_yellow(T t) { return R<T> { S<T>(t, "b", "y") }; }
    template<typename T> R<T> blue_on_white(T t) { return R<T> { S<T>(t, "b", "w") }; }
    template<typename T> R<T> blue_on_grey(T t) { return R<T> { S<T>(t, "b", "e") }; }
    template<typename T> R<T> blue_on_light_blue(T t) { return R<T> { S<T>(t, "b", "lb") }; }
    template<typename T> R<T> blue_on_light_green(T t) { return R<T> { S<T>(t, "b", "lg") }; }
    template<typename T> R<T> blue_on_light_aqua(T t) { return R<T> { S<T>(t, "b", "la") }; }
    template<typename T> R<T> blue_on_light_red(T t) { return R<T> { S<T>(t, "b", "lr") }; }
    template<typename T> R<T> blue_on_light_purple(T t) { return R<T> { S<T>(t, "b", "lp") }; }
    template<typename T> R<T> blue_on_light_yellow(T t) { return R<T> { S<T>(t, "b", "ly") }; }
    template<typename T> R<T> blue_on_bright_white(T t) { return R<T> { S<T>(t, "b", "bw") }; }
    template<typename T> R<T> green_on_black(T t) { return R<T> { S<T>(t, "g", "k") }; }
    template<typename T> R<T> green_on_blue(T t) { return R<T> { S<T>(t, "g", "b") }; }
    template<typename T> R<T> green_on_green(T t) { return R<T> { S<T>(t, "g", "g") }; }
    template<typename T> R<T> green_on_aqua(T t) { return R<T> { S<T>(t, "g", "a") }; }
    template<typename T> R<T> green_on_red(T t) { return R<T> { S<T>(t, "g", "r") }; }
    template<typename T> R<T> green_on_purple(T t) { return R<T> { S<T>(t, "g", "p") }; }
    template<typename T> R<T> green_on_yellow(T t) { return R<T> { S<T>(t, "g", "y") }; }
    template<typename T> R<T> green_on_white(T t) { return R<T> { S<T>(t, "g", "w") }; }
    template<typename T> R<T> green_on_grey(T t) { return R<T> { S<T>(t, "g", "e") }; }
    template<typename T> R<T> green_on_light_blue(T t) { return R<T> { S<T>(t, "g", "lb") }; }
    template<typename T> R<T> green_on_light_green(T t) { return R<T> { S<T>(t, "g", "lg") }; }
    template<typename T> R<T> green_on_light_aqua(T t) { return R<T> { S<T>(t, "g", "la") }; }
    template<typename T> R<T> green_on_light_red(T t) { return R<T> { S<T>(t, "g", "lr") }; }
    template<typename T> R<T> green_on_light_purple(T t) { return R<T> { S<T>(t, "g", "lp") }; }
    template<typename T> R<T> green_on_light_yellow(T t) { return R<T> { S<T>(t, "g", "ly") }; }
    template<typename T> R<T> green_on_bright_white(T t) { return R<T> { S<T>(t, "g", "bw") }; }
    template<typename T> R<T> aqua_on_black(T t) { return R<T> { S<T>(t, "a", "k") }; }
    template<typename T> R<T> aqua_on_blue(T t) { return R<T> { S<T>(t, "a", "b") }; }
    template<typename T> R<T> aqua_on_green(T t) { return R<T> { S<T>(t, "a", "g") }; }
    template<typename T> R<T> aqua_on_aqua(T t) { return R<T> { S<T>(t, "a", "a") }; }
    template<typename T> R<T> aqua_on_red(T t) { return R<T> { S<T>(t, "a", "r") }; }
    template<typename T> R<T> aqua_on_purple(T t) { return R<T> { S<T>(t, "a", "p") }; }
    template<typename T> R<T> aqua_on_yellow(T t) { return R<T> { S<T>(t, "a", "y") }; }
    template<typename T> R<T> aqua_on_white(T t) { return R<T> { S<T>(t, "a", "w") }; }
    template<typename T> R<T> aqua_on_grey(T t) { return R<T> { S<T>(t, "a", "e") }; }
    template<typename T> R<T> aqua_on_light_blue(T t) { return R<T> { S<T>(t, "a", "lb") }; }
    template<typename T> R<T> aqua_on_light_green(T t) { return R<T> { S<T>(t, "a", "lg") }; }
    template<typename T> R<T> aqua_on_light_aqua(T t) { return R<T> { S<T>(t, "a", "la") }; }
    template<typename T> R<T> aqua_on_light_red(T t) { return R<T> { S<T>(t, "a", "lr") }; }
    template<typename T> R<T> aqua_on_light_purple(T t) { return R<T> { S<T>(t, "a", "lp") }; }
    template<typename T> R<T> aqua_on_light_yellow(T t) { return R<T> { S<T>(t, "a", "ly") }; }
    template<typename T> R<T> aqua_on_bright_white(T t) { return R<T> { S<T>(t, "a", "bw") }; }
    template<typename T> R<T> red_on_black(T t) { return R<T> { S<T>(t, "r", "k") }; }
    template<typename T> R<T> red_on_blue(T t) { return R<T> { S<T>(t, "r", "b") }; }
    template<typename T> R<T> red_on_green(T t) { return R<T> { S<T>(t, "r", "g") }; }
    template<typename T> R<T> red_on_aqua(T t) { return R<T> { S<T>(t, "r", "a") }; }
    template<typename T> R<T> red_on_red(T t) { return R<T> { S<T>(t, "r", "r") }; }
    template<typename T> R<T> red_on_purple(T t) { return R<T> { S<T>(t, "r", "p") }; }
    template<typename T> R<T> red_on_yellow(T t) { return R<T> { S<T>(t, "r", "y") }; }
    template<typename T> R<T> red_on_white(T t) { return R<T> { S<T>(t, "r", "w") }; }
    template<typename T> R<T> red_on_grey(T t) { return R<T> { S<T>(t, "r", "e") }; }
    template<typename T> R<T> red_on_light_blue(T t) { return R<T> { S<T>(t, "r", "lb") }; }
    template<typename T> R<T> red_on_light_green(T t) { return R<T> { S<T>(t, "r", "lg") }; }
    template<typename T> R<T> red_on_light_aqua(T t) { return R<T> { S<T>(t, "r", "la") }; }
    template<typename T> R<T> red_on_light_red(T t) { return R<T> { S<T>(t, "r", "lr") }; }
    template<typename T> R<T> red_on_light_purple(T t) { return R<T> { S<T>(t, "r", "lp") }; }
    template<typename T> R<T> red_on_light_yellow(T t) { return R<T> { S<T>(t, "r", "ly") }; }
    template<typename T> R<T> red_on_bright_white(T t) { return R<T> { S<T>(t, "r", "bw") }; }
    template<typename T> R<T> purple_on_black(T t) { return R<T> { S<T>(t, "p", "k") }; }
    template<typename T> R<T> purple_on_blue(T t) { return R<T> { S<T>(t, "p", "b") }; }
    template<typename T> R<T> purple_on_green(T t) { return R<T> { S<T>(t, "p", "g") }; }
    template<typename T> R<T> purple_on_aqua(T t) { return R<T> { S<T>(t, "p", "a") }; }
    template<typename T> R<T> purple_on_red(T t) { return R<T> { S<T>(t, "p", "r") }; }
    template<typename T> R<T> purple_on_purple(T t) { return R<T> { S<T>(t, "p", "p") }; }
    template<typename T> R<T> purple_on_yellow(T t) { return R<T> { S<T>(t, "p", "y") }; }
    template<typename T> R<T> purple_on_white(T t) { return R<T> { S<T>(t, "p", "w") }; }
    template<typename T> R<T> purple_on_grey(T t) { return R<T> { S<T>(t, "p", "e") }; }
    template<typename T> R<T> purple_on_light_blue(T t) { return R<T> { S<T>(t, "p", "lb") }; }
    template<typename T> R<T> purple_on_light_green(T t) { return R<T> { S<T>(t, "p", "lg") }; }
    template<typename T> R<T> purple_on_light_aqua(T t) { return R<T> { S<T>(t, "p", "la") }; }
    template<typename T> R<T> purple_on_light_red(T t) { return R<T> { S<T>(t, "p", "lr") }; }
    template<typename T> R<T> purple_on_light_purple(T t) { return R<T> { S<T>(t, "p", "lp") }; }
    template<typename T> R<T> purple_on_light_yellow(T t) { return R<T> { S<T>(t, "p", "ly") }; }
    template<typename T> R<T> purple_on_bright_white(T t) { return R<T> { S<T>(t, "p", "bw") }; }
    template<typename T> R<T> yellow_on_black(T t) { return R<T> { S<T>(t, "y", "k") }; }
    template<typename T> R<T> yellow_on_blue(T t) { return R<T> { S<T>(t, "y", "b") }; }
    template<typename T> R<T> yellow_on_green(T t) { return R<T> { S<T>(t, "y", "g") }; }
    template<typename T> R<T> yellow_on_aqua(T t) { return R<T> { S<T>(t, "y", "a") }; }
    template<typename T> R<T> yellow_on_red(T t) { return R<T> { S<T>(t, "y", "r") }; }
    template<typename T> R<T> yellow_on_purple(T t) { return R<T> { S<T>(t, "y", "p") }; }
    template<typename T> R<T> yellow_on_yellow(T t) { return R<T> { S<T>(t, "y", "y") }; }
    template<typename T> R<T> yellow_on_white(T t) { return R<T> { S<T>(t, "y", "w") }; }
    template<typename T> R<T> yellow_on_grey(T t) { return R<T> { S<T>(t, "y", "e") }; }
    template<typename T> R<T> yellow_on_light_blue(T t) { return R<T> { S<T>(t, "y", "lb") }; }
    template<typename T> R<T> yellow_on_light_green(T t) { return R<T> { S<T>(t, "y", "lg") }; }
    template<typename T> R<T> yellow_on_light_aqua(T t) { return R<T> { S<T>(t, "y", "la") }; }
    template<typename T> R<T> yellow_on_light_red(T t) { return R<T> { S<T>(t, "y", "lr") }; }
    template<typename T> R<T> yellow_on_light_purple(T t) { return R<T> { S<T>(t, "y", "lp") }; }
    template<typename T> R<T> yellow_on_light_yellow(T t) { return R<T> { S<T>(t, "y", "ly") }; }
    template<typename T> R<T> yellow_on_bright_white(T t) { return R<T> { S<T>(t, "y", "bw") }; }
    template<typename T> R<T> white_on_black(T t) { return R<T> { S<T>(t, "w", "k") }; }
    template<typename T> R<T> white_on_blue(T t) { return R<T> { S<T>(t, "w", "b") }; }
    template<typename T> R<T> white_on_green(T t) { return R<T> { S<T>(t, "w", "g") }; }
    template<typename T> R<T> white_on_aqua(T t) { return R<T> { S<T>(t, "w", "a") }; }
    template<typename T> R<T> white_on_red(T t) { return R<T> { S<T>(t, "w", "r") }; }
    template<typename T> R<T> white_on_purple(T t) { return R<T> { S<T>(t, "w", "p") }; }
    template<typename T> R<T> white_on_yellow(T t) { return R<T> { S<T>(t, "w", "y") }; }
    template<typename T> R<T> white_on_white(T t) { return R<T> { S<T>(t, "w", "w") }; }
    template<typename T> R<T> white_on_grey(T t) { return R<T> { S<T>(t, "w", "e") }; }
    template<typename T> R<T> white_on_light_blue(T t) { return R<T> { S<T>(t, "w", "lb") }; }
    template<typename T> R<T> white_on_light_green(T t) { return R<T> { S<T>(t, "w", "lg") }; }
    template<typename T> R<T> white_on_light_aqua(T t) { return R<T> { S<T>(t, "w", "la") }; }
    template<typename T> R<T> white_on_light_red(T t) { return R<T> { S<T>(t, "w", "lr") }; }
    template<typename T> R<T> white_on_light_purple(T t) { return R<T> { S<T>(t, "w", "lp") }; }
    template<typename T> R<T> white_on_light_yellow(T t) { return R<T> { S<T>(t, "w", "ly") }; }
    template<typename T> R<T> white_on_bright_white(T t) { return R<T> { S<T>(t, "w", "bw") }; }
    template<typename T> R<T> grey_on_black(T t) { return R<T> { S<T>(t, "e", "k") }; }
    template<typename T> R<T> grey_on_blue(T t) { return R<T> { S<T>(t, "e", "b") }; }
    template<typename T> R<T> grey_on_green(T t) { return R<T> { S<T>(t, "e", "g") }; }
    template<typename T> R<T> grey_on_aqua(T t) { return R<T> { S<T>(t, "e", "a") }; }
    template<typename T> R<T> grey_on_red(T t) { return R<T> { S<T>(t, "e", "r") }; }
    template<typename T> R<T> grey_on_purple(T t) { return R<T> { S<T>(t, "e", "p") }; }
    template<typename T> R<T> grey_on_yellow(T t) { return R<T> { S<T>(t, "e", "y") }; }
    template<typename T> R<T> grey_on_white(T t) { return R<T> { S<T>(t, "e", "w") }; }
    template<typename T> R<T> grey_on_grey(T t) { return R<T> { S<T>(t, "e", "e") }; }
    template<typename T> R<T> grey_on_light_blue(T t) { return R<T> { S<T>(t, "e", "lb") }; }
    template<typename T> R<T> grey_on_light_green(T t) { return R<T> { S<T>(t, "e", "lg") }; }
    template<typename T> R<T> grey_on_light_aqua(T t) { return R<T> { S<T>(t, "e", "la") }; }
    template<typename T> R<T> grey_on_light_red(T t) { return R<T> { S<T>(t, "e", "lr") }; }
    template<typename T> R<T> grey_on_light_purple(T t) { return R<T> { S<T>(t, "e", "lp") }; }
    template<typename T> R<T> grey_on_light_yellow(T t) { return R<T> { S<T>(t, "e", "ly") }; }
    template<typename T> R<T> grey_on_bright_white(T t) { return R<T> { S<T>(t, "e", "bw") }; }
    template<typename T> R<T> light_blue_on_black(T t) { return R<T> { S<T>(t, "lb", "k") }; }
    template<typename T> R<T> light_blue_on_blue(T t) { return R<T> { S<T>(t, "lb", "b") }; }
    template<typename T> R<T> light_blue_on_green(T t) { return R<T> { S<T>(t, "lb", "g") }; }
    template<typename T> R<T> light_blue_on_aqua(T t) { return R<T> { S<T>(t, "lb", "a") }; }
    template<typename T> R<T> light_blue_on_red(T t) { return R<T> { S<T>(t, "lb", "r") }; }
    template<typename T> R<T> light_blue_on_purple(T t) { return R<T> { S<T>(t, "lb", "p") }; }
    template<typename T> R<T> light_blue_on_yellow(T t) { return R<T> { S<T>(t, "lb", "y") }; }
    template<typename T> R<T> light_blue_on_white(T t) { return R<T> { S<T>(t, "lb", "w") }; }
    template<typename T> R<T> light_blue_on_grey(T t) { return R<T> { S<T>(t, "lb", "e") }; }
    template<typename T> R<T> light_blue_on_light_blue(T t) { return R<T> { S<T>(t, "lb", "lb") }; }
    template<typename T> R<T> light_blue_on_light_green(T t) { return R<T> { S<T>(t, "lb", "lg") }; }
    template<typename T> R<T> light_blue_on_light_aqua(T t) { return R<T> { S<T>(t, "lb", "la") }; }
    template<typename T> R<T> light_blue_on_light_red(T t) { return R<T> { S<T>(t, "lb", "lr") }; }
    template<typename T> R<T> light_blue_on_light_purple(T t) { return R<T> { S<T>(t, "lb", "lp") }; }
    template<typename T> R<T> light_blue_on_light_yellow(T t) { return R<T> { S<T>(t, "lb", "ly") }; }
    template<typename T> R<T> light_blue_on_bright_white(T t) { return R<T> { S<T>(t, "lb", "bw") }; }
    template<typename T> R<T> light_green_on_black(T t) { return R<T> { S<T>(t, "lg", "k") }; }
    template<typename T> R<T> light_green_on_blue(T t) { return R<T> { S<T>(t, "lg", "b") }; }
    template<typename T> R<T> light_green_on_green(T t) { return R<T> { S<T>(t, "lg", "g") }; }
    template<typename T> R<T> light_green_on_aqua(T t) { return R<T> { S<T>(t, "lg", "a") }; }
    template<typename T> R<T> light_green_on_red(T t) { return R<T> { S<T>(t, "lg", "r") }; }
    template<typename T> R<T> light_green_on_purple(T t) { return R<T> { S<T>(t, "lg", "p") }; }
    template<typename T> R<T> light_green_on_yellow(T t) { return R<T> { S<T>(t, "lg", "y") }; }
    template<typename T> R<T> light_green_on_white(T t) { return R<T> { S<T>(t, "lg", "w") }; }
    template<typename T> R<T> light_green_on_grey(T t) { return R<T> { S<T>(t, "lg", "e") }; }
    template<typename T> R<T> light_green_on_light_blue(T t) { return R<T> { S<T>(t, "lg", "lb") }; }
    template<typename T> R<T> light_green_on_light_green(T t) { return R<T> { S<T>(t, "lg", "lg") }; }
    template<typename T> R<T> light_green_on_light_aqua(T t) { return R<T> { S<T>(t, "lg", "la") }; }
    template<typename T> R<T> light_green_on_light_red(T t) { return R<T> { S<T>(t, "lg", "lr") }; }
    template<typename T> R<T> light_green_on_light_purple(T t) { return R<T> { S<T>(t, "lg", "lp") }; }
    template<typename T> R<T> light_green_on_light_yellow(T t) { return R<T> { S<T>(t, "lg", "ly") }; }
    template<typename T> R<T> light_green_on_bright_white(T t) { return R<T> { S<T>(t, "lg", "bw") }; }
    template<typename T> R<T> light_aqua_on_black(T t) { return R<T> { S<T>(t, "la", "k") }; }
    template<typename T> R<T> light_aqua_on_blue(T t) { return R<T> { S<T>(t, "la", "b") }; }
    template<typename T> R<T> light_aqua_on_green(T t) { return R<T> { S<T>(t, "la", "g") }; }
    template<typename T> R<T> light_aqua_on_aqua(T t) { return R<T> { S<T>(t, "la", "a") }; }
    template<typename T> R<T> light_aqua_on_red(T t) { return R<T> { S<T>(t, "la", "r") }; }
    template<typename T> R<T> light_aqua_on_purple(T t) { return R<T> { S<T>(t, "la", "p") }; }
    template<typename T> R<T> light_aqua_on_yellow(T t) { return R<T> { S<T>(t, "la", "y") }; }
    template<typename T> R<T> light_aqua_on_white(T t) { return R<T> { S<T>(t, "la", "w") }; }
    template<typename T> R<T> light_aqua_on_grey(T t) { return R<T> { S<T>(t, "la", "e") }; }
    template<typename T> R<T> light_aqua_on_light_blue(T t) { return R<T> { S<T>(t, "la", "lb") }; }
    template<typename T> R<T> light_aqua_on_light_green(T t) { return R<T> { S<T>(t, "la", "lg") }; }
    template<typename T> R<T> light_aqua_on_light_aqua(T t) { return R<T> { S<T>(t, "la", "la") }; }
    template<typename T> R<T> light_aqua_on_light_red(T t) { return R<T> { S<T>(t, "la", "lr") }; }
    template<typename T> R<T> light_aqua_on_light_purple(T t) { return R<T> { S<T>(t, "la", "lp") }; }
    template<typename T> R<T> light_aqua_on_light_yellow(T t) { return R<T> { S<T>(t, "la", "ly") }; }
    template<typename T> R<T> light_aqua_on_bright_white(T t) { return R<T> { S<T>(t, "la", "bw") }; }
    template<typename T> R<T> light_red_on_black(T t) { return R<T> { S<T>(t, "lr", "k") }; }
    template<typename T> R<T> light_red_on_blue(T t) { return R<T> { S<T>(t, "lr", "b") }; }
    template<typename T> R<T> light_red_on_green(T t) { return R<T> { S<T>(t, "lr", "g") }; }
    template<typename T> R<T> light_red_on_aqua(T t) { return R<T> { S<T>(t, "lr", "a") }; }
    template<typename T> R<T> light_red_on_red(T t) { return R<T> { S<T>(t, "lr", "r") }; }
    template<typename T> R<T> light_red_on_purple(T t) { return R<T> { S<T>(t, "lr", "p") }; }
    template<typename T> R<T> light_red_on_yellow(T t) { return R<T> { S<T>(t, "lr", "y") }; }
    template<typename T> R<T> light_red_on_white(T t) { return R<T> { S<T>(t, "lr", "w") }; }
    template<typename T> R<T> light_red_on_grey(T t) { return R<T> { S<T>(t, "lr", "e") }; }
    template<typename T> R<T> light_red_on_light_blue(T t) { return R<T> { S<T>(t, "lr", "lb") }; }
    template<typename T> R<T> light_red_on_light_green(T t) { return R<T> { S<T>(t, "lr", "lg") }; }
    template<typename T> R<T> light_red_on_light_aqua(T t) { return R<T> { S<T>(t, "lr", "la") }; }
    template<typename T> R<T> light_red_on_light_red(T t) { return R<T> { S<T>(t, "lr", "lr") }; }
    template<typename T> R<T> light_red_on_light_purple(T t) { return R<T> { S<T>(t, "lr", "lp") }; }
    template<typename T> R<T> light_red_on_light_yellow(T t) { return R<T> { S<T>(t, "lr", "ly") }; }
    template<typename T> R<T> light_red_on_bright_white(T t) { return R<T> { S<T>(t, "lr", "bw") }; }
    template<typename T> R<T> light_purple_on_black(T t) { return R<T> { S<T>(t, "lp", "k") }; }
    template<typename T> R<T> light_purple_on_blue(T t) { return R<T> { S<T>(t, "lp", "b") }; }
    template<typename T> R<T> light_purple_on_green(T t) { return R<T> { S<T>(t, "lp", "g") }; }
    template<typename T> R<T> light_purple_on_aqua(T t) { return R<T> { S<T>(t, "lp", "a") }; }
    template<typename T> R<T> light_purple_on_red(T t) { return R<T> { S<T>(t, "lp", "r") }; }
    template<typename T> R<T> light_purple_on_purple(T t) { return R<T> { S<T>(t, "lp", "p") }; }
    template<typename T> R<T> light_purple_on_yellow(T t) { return R<T> { S<T>(t, "lp", "y") }; }
    template<typename T> R<T> light_purple_on_white(T t) { return R<T> { S<T>(t, "lp", "w") }; }
    template<typename T> R<T> light_purple_on_grey(T t) { return R<T> { S<T>(t, "lp", "e") }; }
    template<typename T> R<T> light_purple_on_light_blue(T t) { return R<T> { S<T>(t, "lp", "lb") }; }
    template<typename T> R<T> light_purple_on_light_green(T t) { return R<T> { S<T>(t, "lp", "lg") }; }
    template<typename T> R<T> light_purple_on_light_aqua(T t) { return R<T> { S<T>(t, "lp", "la") }; }
    template<typename T> R<T> light_purple_on_light_red(T t) { return R<T> { S<T>(t, "lp", "lr") }; }
    template<typename T> R<T> light_purple_on_light_purple(T t) { return R<T> { S<T>(t, "lp", "lp") }; }
    template<typename T> R<T> light_purple_on_light_yellow(T t) { return R<T> { S<T>(t, "lp", "ly") }; }
    template<typename T> R<T> light_purple_on_bright_white(T t) { return R<T> { S<T>(t, "lp", "bw") }; }
    template<typename T> R<T> light_yellow_on_black(T t) { return R<T> { S<T>(t, "ly", "k") }; }
    template<typename T> R<T> light_yellow_on_blue(T t) { return R<T> { S<T>(t, "ly", "b") }; }
    template<typename T> R<T> light_yellow_on_green(T t) { return R<T> { S<T>(t, "ly", "g") }; }
    template<typename T> R<T> light_yellow_on_aqua(T t) { return R<T> { S<T>(t, "ly", "a") }; }
    template<typename T> R<T> light_yellow_on_red(T t) { return R<T> { S<T>(t, "ly", "r") }; }
    template<typename T> R<T> light_yellow_on_purple(T t) { return R<T> { S<T>(t, "ly", "p") }; }
    template<typename T> R<T> light_yellow_on_yellow(T t) { return R<T> { S<T>(t, "ly", "y") }; }
    template<typename T> R<T> light_yellow_on_white(T t) { return R<T> { S<T>(t, "ly", "w") }; }
    template<typename T> R<T> light_yellow_on_grey(T t) { return R<T> { S<T>(t, "ly", "e") }; }
    template<typename T> R<T> light_yellow_on_light_blue(T t) { return R<T> { S<T>(t, "ly", "lb") }; }
    template<typename T> R<T> light_yellow_on_light_green(T t) { return R<T> { S<T>(t, "ly", "lg") }; }
    template<typename T> R<T> light_yellow_on_light_aqua(T t) { return R<T> { S<T>(t, "ly", "la") }; }
    template<typename T> R<T> light_yellow_on_light_red(T t) { return R<T> { S<T>(t, "ly", "lr") }; }
    template<typename T> R<T> light_yellow_on_light_purple(T t) { return R<T> { S<T>(t, "ly", "lp") }; }
    template<typename T> R<T> light_yellow_on_light_yellow(T t) { return R<T> { S<T>(t, "ly", "ly") }; }
    template<typename T> R<T> light_yellow_on_bright_white(T t) { return R<T> { S<T>(t, "ly", "bw") }; }
    template<typename T> R<T> bright_white_on_black(T t) { return R<T> { S<T>(t, "bw", "k") }; }
    template<typename T> R<T> bright_white_on_blue(T t) { return R<T> { S<T>(t, "bw", "b") }; }
    template<typename T> R<T> bright_white_on_green(T t) { return R<T> { S<T>(t, "bw", "g") }; }
    template<typename T> R<T> bright_white_on_aqua(T t) { return R<T> { S<T>(t, "bw", "a") }; }
    template<typename T> R<T> bright_white_on_red(T t) { return R<T> { S<T>(t, "bw", "r") }; }
    template<typename T> R<T> bright_white_on_purple(T t) { return R<T> { S<T>(t, "bw", "p") }; }
    template<typename T> R<T> bright_white_on_yellow(T t) { return R<T> { S<T>(t, "bw", "y") }; }
    template<typename T> R<T> bright_white_on_white(T t) { return R<T> { S<T>(t, "bw", "w") }; }
    template<typename T> R<T> bright_white_on_grey(T t) { return R<T> { S<T>(t, "bw", "e") }; }
    template<typename T> R<T> bright_white_on_light_blue(T t) { return R<T> { S<T>(t, "bw", "lb") }; }
    template<typename T> R<T> bright_white_on_light_green(T t) { return R<T> { S<T>(t, "bw", "lg") }; }
    template<typename T> R<T> bright_white_on_light_aqua(T t) { return R<T> { S<T>(t, "bw", "la") }; }
    template<typename T> R<T> bright_white_on_light_red(T t) { return R<T> { S<T>(t, "bw", "lr") }; }
    template<typename T> R<T> bright_white_on_light_purple(T t) { return R<T> { S<T>(t, "bw", "lp") }; }
    template<typename T> R<T> bright_white_on_light_yellow(T t) { return R<T> { S<T>(t, "bw", "ly") }; }
    template<typename T> R<T> bright_white_on_bright_white(T t) { return R<T> { S<T>(t, "bw", "bw") }; }
}

#endif
