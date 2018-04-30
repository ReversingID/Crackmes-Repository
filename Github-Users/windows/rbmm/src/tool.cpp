#include "lineheap.h"

class KKK : public FSER
{
  struct API_NAME 
  {
    ULONG ofs, len;
  };

  LineHeap m_sh, m_nh;
  HMODULE m_hNTDLL;
  ULONG m_nNames;
  int m_state;

  virtual NTSTATUS sci1(PBYTE pb, ULONG cb)
  {
    STATIC_ASTRING(ntka, "NTKERNELAPI");//
    PBYTE end = pb + cb, c, a;
    BOOL bFirst = TRUE;
    while (pb = (PBYTE)strnstr(end - pb, pb, sizeof(ntka) - 1, ntka))
    {
      if (bFirst)
      {
        bFirst = FALSE;
        continue;
      }

      if (c = (PBYTE)strnchr(end - pb, pb, ';'))
      {
        if ((a = (PBYTE)strnchr(c - pb, pb, '(')) && strnchr(c - a, a, ')'))
        {
          cb = RtlPointerToOffset(pb, c - 1);

          if (PVOID pv = m_nh.Allocate(cb, 1))
          {
            if (API_NAME* pan = (API_NAME*)m_sh.Allocate(sizeof(API_NAME), __alignof(API_NAME)))
            {
              memcpy(pv, pb, cb);
              pan->len = cb;
              pan->ofs = RtlPointerToOffset(m_nh.getBase(), pv);
              m_nNames++;
            }
          }
        }
        pb = c;
      }
      else
      {
        break;
      }
    }

    return 0;
  }

  void PrintNames(PVOID ImageBase, PIMAGE_NT_HEADERS pinth, PIMAGE_IMPORT_DESCRIPTOR piid, ULONG Size, BOOL bDefFile)
  {
    API_NAME* _pan = (API_NAME*)m_sh.getBase(), *pan;
    PCSTR pcsz = (PCSTR)m_nh.getBase();

    PVOID pvEnd = RtlOffsetToPointer(piid, Size);
    const IMAGE_THUNK_DATA32 * pINT;

    while (piid->Name && piid < pvEnd)
    {
      if (piid->OriginalFirstThunk && 
        (pINT = (PIMAGE_THUNK_DATA32)RtlAddressInSectionTable(pinth, ImageBase, piid->OriginalFirstThunk)))
      {
        while (ULONG Ordinal = pINT++->u1.Ordinal)
        {
          if (IMAGE_SNAP_BY_ORDINAL(Ordinal))
          {
            __debugbreak();
          }
          else
          {
            PIMAGE_IMPORT_BY_NAME piibn = (PIMAGE_IMPORT_BY_NAME)
              RtlAddressInSectionTable(pinth, ImageBase, (ULONG)Ordinal);

            PCSTR Name = (PCSTR)piibn->Name;
            if (GetProcAddress(m_hNTDLL, Name))
            {
              if (bDefFile) DbgPrint("%s=ntdll.%s\n", Name, Name);
            }
            else if (Size = m_nNames)
            {
              pan = _pan;
              SIZE_T len = strlen(Name);
              do 
              {
                if (PCSTR pc = strnstr(pan->len, pcsz + pan->ofs, len, Name))
                {
                  if (*pc >= 'A')
                  {
                    continue;
                  }
                  bDefFile ? DbgPrint("%s\n", Name) : 
                    DbgPrint("%.*s\n{\n\treturn 0;\n}\n", pan->len, pcsz + pan->ofs);
                  break;
                }
              } while (pan++, --Size);

              if (!Size)
              {
                DbgPrint("%s\n", Name);
              }
            }
          }
        }
      }
      piid++;
    }
  }

  virtual NTSTATUS sci2(PVOID ImageBase)
  {
    if (PIMAGE_NT_HEADERS pinth = RtlImageNtHeader(ImageBase))
    {
      ULONG Size;

      if (PVOID pv = RtlImageDirectoryEntryToData(ImageBase, FALSE, IMAGE_DIRECTORY_ENTRY_IMPORT, &Size))
      {
        PrintNames(ImageBase, pinth, (PIMAGE_IMPORT_DESCRIPTOR)pv, Size, FALSE);
        PrintNames(ImageBase, pinth, (PIMAGE_IMPORT_DESCRIPTOR)pv, Size, TRUE);
      }
    }

    return 0;
  }

  virtual NTSTATUS sci(PBYTE pb, ULONG cb)
  {
    switch (m_state)
    {
    case 0:
      return sci1(pb, cb);
    case 1:
      return sci2(pb);
    }
    return STATUS_NOT_IMPLEMENTED;
  }
public:
  KKK()
  {
    m_state = 0;
    m_hNTDLL = GetModuleHandle(L"ntdll");
  }

  BOOL Init(SIZE_T cb1, SIZE_T cb2)
  {
    return 0 <= m_sh.Create(cb1) && 0 <= m_nh.Create(cb2);
  }

  void SetState(int i)
  {
    m_state = i;
  }
};
  STATIC_OBJECT_ATTRIBUTES(oa1, "\\??\\C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.15063.0\\km\\ntifs.h");
  STATIC_OBJECT_ATTRIBUTES(oa2, "\\??\\C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.15063.0\\km\\ntddk.h");
  STATIC_OBJECT_ATTRIBUTES(oa3, "\\??\\C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.15063.0\\km\\wdm.h");
  STATIC_OBJECT_ATTRIBUTES(oa4, "\\??\\d:\\crackme_drv.sys");
  KKK h;
  if (h.Init(0x100000, 0x1000000))
  {
    h.MapFileR(&oa1);
    h.MapFileR(&oa2);
    h.MapFileR(&oa3);
    h.SetState(1);
    h.MapFileR(&oa4);
  }
