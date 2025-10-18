#pragma once
#include <core/def.h>
#include <string>

namespace arc
{

void P_cvt_u32(const std::string &u8_str, std::u32string *u32_tmp);
void P_cvt_u8(const std::u32string &u32_str, std::string *u8_tmp);

} // namespace arc
