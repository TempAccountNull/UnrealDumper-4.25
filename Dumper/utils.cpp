#include <windows.h>
#include <winternl.h>
#include "memory.h"
#include "utils.h"

bool Compare(uint8* data, uint8* sig, uint32 size)
{
    for (uint32 i = 0; i < size; i++)
    {
        if (data[i] != sig[i] && sig[i] != 0x00)
        {
            return false;
        }
    }
    return true;
}

uint8* FindSignature(void* start, void* end, const char* sig, uint32 size)
{
    for (uint8* it = static_cast<uint8*>(start); it < static_cast<uint8*>(end) - size; it++)
    {
        if (Compare(it, (uint8*)sig, size))
        {
            return it;
        }
    }
    return NULL;
}

void* FindPointer(void* start, void* end, const char* sig, uint32 size, int32 addition)
{
  uint8* address = FindSignature(start, end, sig, size);
  if (!address) return nullptr;

  int32 k;
  for (k = 0; sig[k]; k++);

  const int32 offset = *reinterpret_cast<int32*>(address + k);
  return address + k + 4 + offset + addition;
}

void IterateExSections(void* data, std::function<bool(void*, void*)> callback)
{
  PIMAGE_DOS_HEADER dos = static_cast<PIMAGE_DOS_HEADER>(data);
  PIMAGE_NT_HEADERS nt = reinterpret_cast<PIMAGE_NT_HEADERS>(static_cast<uint8*>(data) + dos->e_lfanew);

  PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt);
  for (int i = 0; i < nt->FileHeader.NumberOfSections; i++, section++)
  {
    if (section->Characteristics & IMAGE_SCN_CNT_CODE)
    {
        uint8* start = static_cast<uint8*>(data) + section->VirtualAddress;
        uint8* end = start + section->SizeOfRawData;
        if (callback(start, end)) break;
    }
  }
}

uint32 GetProccessPath(uint32 pid, wchar_t* processName, uint32 size)
{
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, NULL, pid);

  if (!QueryFullProcessImageNameW(hProcess, NULL, processName, (DWORD*)(&size))) size = NULL;

  //Close the handle.
  CloseHandle(hProcess);

  //Return the size.
  return size;
}

uint64 GetTime()
{
  LARGE_INTEGER ret;
  NtQuerySystemTime(&ret);
  return ret.QuadPart;
}
