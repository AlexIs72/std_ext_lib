#include "filesystem.h"

#include "misc.h"

std::string std::ext::misc::gen_uuid()
{
    return std::ext::filesystem::file_get_contents("/proc/sys/kernel/random/uuid");
}