#include "Overlay.hpp"
#include "Games/FindActiveGame.hpp"

Overlay g_Overlay;

Overlay::Overlay()
{
#ifdef LAUNCHER_DEBUG
   sys_ppu_thread_create(&UpdateInfoThreadId, UpdateInfoThread, 0, 0xB01, 512, SYS_PPU_THREAD_CREATE_JOINABLE, "Overlay::UpdateInfoThread()");
#endif // LAUNCHER_DEBUG
}

void Overlay::OnUpdate()
{
#ifdef LAUNCHER_DEBUG
   CalculateFps();
   DrawOverlay();
#endif // LAUNCHER_DEBUG
}

void Overlay::OnShutdown()
{
#ifdef LAUNCHER_DEBUG
   if (UpdateInfoThreadId != SYS_PPU_THREAD_ID_INVALID)
   {
      m_StateRunning = false;

      sys_ppu_thread_yield();
      Sleep(refreshDelay * 1000 + 500);

      uint64_t exitCode;
      sys_ppu_thread_join(UpdateInfoThreadId, &exitCode);
   }
#endif // LAUNCHER_DEBUG
}

void Overlay::DrawOverlay()
{
#ifdef LAUNCHER_DEBUG
   wchar_t buffer[300]{};

   std::wstring overlayText = L"";

   std::wstring kernelName;
   switch (m_KernelType)
   {
      case 1: kernelName = L"CEX"; break;
      case 2: kernelName = L"DEX"; break;
      case 3: kernelName = L"DEH"; break;
      default: kernelName = L"N/A";  break;
   }

   // FPS, Temps, Vsh Ram, Fan Speed
   vsh::swprintf(buffer, 150, 
       L"FPS: %.2f\nCPU: %.0f\u2109 / GPU: %.0f\u2109\nRAM: %.1f%% %.1f / %.1f MB\nFan speed: %.0f%%\n",
       m_FPS, 
       m_CPUTemp, m_GPUTemp, 
       m_MemoryUsage.percent, m_MemoryUsage.used, m_MemoryUsage.total,
       m_FanSpeed);
   overlayText += buffer;

   // Firmware Version
   std::wstring payloadName = IsConsoleHen() ? L"PS3HEN" : IsConsoleMamba() ? L"Mamba" : L"Cobra";
   std::wstring payloadStr = m_PayloadVersion == 0 ? L"" : payloadName;
   std::wstring payloadVerStr = m_PayloadVersion == 0 ?
       L"" :
       to_wstring(m_PayloadVersion >> 8)
       + L"."
       + to_wstring((m_PayloadVersion & 0xF0) >> 4);

   if (IsConsoleHen())
       payloadVerStr += L"." + to_wstring(m_PayloadVersion & 0xF);

   overlayText += to_wstring(m_FirmwareVersion, 2)
       + L" " + kernelName
       + L" " + payloadStr
       + L" " + payloadVerStr
       + L"\n";


   // Game Name
   paf::View* gamePlugin = paf::View::Find("game_plugin");
   if (gamePlugin)
   {
       char gameTitleId[16]{};
       char gameTitleName[64]{};
       g_FindActiveGame.GetGameName(gameTitleId, gameTitleName);

       bool isTitleIdEmpty = (gameTitleId[0] == NULL) || (gameTitleId[0] == ' ');

       vsh::swprintf(buffer, 120, L"%s %c %s\n", gameTitleName, isTitleIdEmpty ? ' ' : '/', gameTitleId);
       overlayText += buffer;
   }


   g_Renderer.Text(
       overlayText,
       paf::vec2(-g_Renderer.m_ViewportWidth / 2 + g_Menu.safeArea.x + 5, g_Renderer.m_ViewportHeight / 2 - g_Menu.safeArea.y - 5),
       16.0f,
       g_Menu.colorText,
       Renderer::Left,
       Renderer::Top);
#endif // LAUNCHER_DEBUG
}

void Overlay::CalculateFps()
{
#ifdef LAUNCHER_DEBUG
   // FPS REPORTING
   // get current timing info
   float timeNow = (float)sys_time_get_system_time() * .000001f;
   float fElapsedInFrame = (float)(timeNow - m_FpsLastTime);
   m_FpsLastTime = timeNow;
   ++m_FpsFrames;
   m_FpsTimeElapsed += fElapsedInFrame;

   // report fps at appropriate interval
   if (m_FpsTimeElapsed >= m_FpsTimeReport)
   {
      m_FPS = (m_FpsFrames - m_FpsFramesLastReport) * 1.f / (float)(m_FpsTimeElapsed - m_FpsTimeLastReport);

      m_FpsTimeReport += m_FpsREPORT_TIME;
      m_FpsTimeLastReport = m_FpsTimeElapsed;
      m_FpsFramesLastReport = m_FpsFrames;
   }
#endif // LAUNCHER_DEBUG
}

void Overlay::UpdateInfoThread(uint64_t arg)
{
#ifdef LAUNCHER_DEBUG
   g_Overlay.m_StateRunning = true;

   while (g_Overlay.m_StateRunning)
   {
      Sleep(refreshDelay * 1000);

      // Using syscalls in a loop on hen will cause a black screen when launching a game
      // so in order to fix this we need to sleep 10/15 seconds when a game is launched
      if (g_Helpers.m_StateGameJustLaunched)
      {
         Sleep(15 * 1000);
         g_Helpers.m_StateGameJustLaunched = false;
      }

      g_Overlay.m_MemoryUsage = GetMemoryUsage();
      // Convert to MB
      g_Overlay.m_MemoryUsage.total /= 1024;
      g_Overlay.m_MemoryUsage.free /= 1024;
      g_Overlay.m_MemoryUsage.used /= 1024;

      g_Overlay.m_FanSpeed = GetFanSpeed();

      g_Overlay.m_CPUTemp = GetTemperatureFahrenheit(0);
      g_Overlay.m_GPUTemp = GetTemperatureFahrenheit(1);

      int ret = get_target_type(&g_Overlay.m_KernelType);
      if (ret != SUCCEEDED)
          g_Overlay.m_KernelType = 0;

      g_Overlay.m_FirmwareVersion = GetFirmwareVersion();

      g_Overlay.m_PayloadVersion = GetPayloadVersion();

   }

   sys_ppu_thread_exit(0);
#endif // LAUNCHER_DEBUG
}