#pragma once

#include <map>
#include <string>

std::map<wchar_t, char> wchar_t_map = {
        {L'А', 128},
        {L'Б', 129},
        {L'В', 130},
        {L'Г', 131},
        {L'Д', 132},
        {L'Е', 133},
        {L'Ж', 134},
        {L'З', 135},
        {L'И', 136},
        {L'Й', 137},
        {L'К', 138},
        {L'Л', 139},
        {L'М', 140},
        {L'Н', 141},
        {L'О', 142},
        {L'П', 143},
        {L'Р', 144},
        {L'С', 145},
        {L'Т', 146},
        {L'У', 147},
        {L'Ф', 148},
        {L'Х', 149},
        {L'Ц', 150},
        {L'Ч', 151},
        {L'Ш', 152},
        {L'Щ', 153},
        {L'Ъ', 154},
        {L'І', 155},
        {L'Ь', 156},
        {L'Є', 157},
        {L'Ю', 158},
        {L'Я', 159},
        {L'а', 160},
        {L'б', 161},
        {L'в', 162},
        {L'г', 163},
        {L'д', 164},
        {L'е', 165},
        {L'ж', 166},
        {L'з', 167},
        {L'и', 168},
        {L'й', 169},
        {L'к', 170},
        {L'л', 171},
        {L'м', 172},
        {L'н', 173},
        {L'о', 174},
        {L'п', 175},

        {L'р', 224},
        {L'с', 225},
        {L'т', 226},
        {L'у', 227},
        {L'ф', 228},
        {L'х', 229},
        {L'ц', 230},
        {L'ч', 231},
        {L'ш', 232},
        {L'щ', 233},
        {L'ъ', 234},
        {L'і', 235},
        {L'ь', 236},
        {L'є', 237},
        {L'ю', 238},
        {L'я', 239},
};

char* toMappedBytes(std::wstring s)
{
    char* output = new char[s.size() + 1];
    for(int i = 0; i < s.size(); i++) {
        wchar_t c = s.at(i);
        if (wchar_t_map.count(c) > 0) {
            output[i] = wchar_t_map.at(c);
        } else {
            output[i] = (char) s.at(i);
        }
    }
    output[s.size()] = 0;

    return output;
}