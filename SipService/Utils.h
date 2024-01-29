#pragma once

std::string LocalTime(time_t time);

std::string GenerateRandomString(int n);

std::string SSRC_Hex(std::string ssrc);

std::string ToUtf8String(const std::string &input);

std::string ToMbcsString(const std::string &input);

std::string GetCurrentModuleDirectory();
