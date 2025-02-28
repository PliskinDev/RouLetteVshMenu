#pragma once
#include "GamePatching.hpp"

namespace CODBO3
{
    static void Initialize()
    {
        using namespace GamePatching;

        switch (g_FindActiveGame.m_MenuToLoad)
        {
            case FindActiveGame::PatchedMenu::BO3Fatality:
            {
                // wait because injecting menu too fast can result in a crash
                Sleep(50000);

                const std::string& fileName = GetCurrentDir() + "modmenus/enstone/fatality_by_enstone_102_patched.bin";
                uint64_t pageTable[2]{};
                if (StartPayload(fileName.c_str(), KB(444), 0x7D0, 0x4000, pageTable))
                    vsh::ShowNotificationWithIcon(L"Fatality is now loaded", vsh::NotifyIcon::Pen, vsh::NotifySound::Trophy);
                break;
            }
        }
    }
}