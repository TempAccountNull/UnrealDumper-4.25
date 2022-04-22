#include <fmt/core.h>
#include "dumper.h"
#include "utils.h"

int main(const int argc, char* argv[])
{
    uint64 start, end, time;
    Dumper* dumper = Dumper::GetInstance();

    start = GetTime();

    switch (dumper->Init(argc, argv))
    {
		case STATUS::WINDOW_NOT_FOUND:    puts("[ERR]: Can't find UE4 window");                        return STATUS::FAILED;
		case STATUS::PROCESS_NOT_FOUND:   puts("[ERR]: Can't find process");                           return STATUS::FAILED;
		case STATUS::READER_ERROR:        puts("[ERR]: Can't init reader");                            return STATUS::FAILED;
		case STATUS::CANNOT_GET_PROCNAME: puts("[ERR]: Can't get process name");                       return STATUS::FAILED;
		case STATUS::ENGINE_NOT_FOUND:    puts("[ERR]: Can't find offsets for this game");             return STATUS::FAILED;
		case STATUS::ENGINE_FAILED:       puts("[ERR]: Can't init engine for this game");              return STATUS::FAILED;
		case STATUS::MODULE_NOT_FOUND:    puts("[ERR]: Can't enumerate modules (protected process?)"); return STATUS::FAILED;
		case STATUS::CANNOT_READ:         puts("[ERR]: Can't read process memory");                    return STATUS::FAILED;
		case STATUS::INVALID_IMAGE:       puts("[ERR]: Can't get executable sections");                return STATUS::FAILED;
		case STATUS::SUCCESS:                                                                          break;
    default:                                                                                           return STATUS::FAILED;
    }
    end = GetTime();
    time = (end - start) / 10000;
    fmt::print("[INFO]: Init time: {} ms\n", time);

    start = GetTime();

    switch (dumper->Dump())
    {
		case STATUS::FILE_NOT_OPEN:      puts("[ERR]: Can't open file");          return STATUS::FAILED;
		case STATUS::ZERO_PACKAGES:      puts("[ERR]: Size of packages is zero"); return STATUS::FAILED;
		case STATUS::SUCCESS:                                                     break;
    default:                                                                      return STATUS::FAILED;
    }
    end = GetTime();
    time = (end - start) / 10000;
    fmt::print("[Success] Dump time: {} ms\n", time);

    return NULL;
}
