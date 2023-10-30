// SPDX-FileCopyrightText: Copyright 2023 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <android/native_window_jni.h>
#include "common/detached_tasks.h"
#include "core/core.h"
#include "core/file_sys/registered_cache.h"
#include "core/hle/service/acc/profile_manager.h"
#include "core/perf_stats.h"
#include "jni/applets/software_keyboard.h"
#include "jni/emu_window/emu_window.h"
#include "video_core/rasterizer_interface.h"

#pragma once

class EmulationSession final {
public:
    explicit EmulationSession();
    ~EmulationSession() = default;

    static EmulationSession& GetInstance();
    const Core::System& System() const;
    Core::System& System();

    const EmuWindow_Android& Window() const;
    EmuWindow_Android& Window();
    ANativeWindow* NativeWindow() const;
    void SetNativeWindow(ANativeWindow* native_window);
    void SurfaceChanged();

    int InstallFileToNand(std::string filename, std::string file_extension);
    void InitializeGpuDriver(const std::string& hook_lib_dir, const std::string& custom_driver_dir,
                             const std::string& custom_driver_name,
                             const std::string& file_redirect_dir);

    bool IsRunning() const;
    bool IsPaused() const;
    void PauseEmulation();
    void UnPauseEmulation();
    void HaltEmulation();
    void RunEmulation();
    void ShutdownEmulation();

    const Core::PerfStatsResults& PerfStats() const;
    void ConfigureFilesystemProvider(const std::string& filepath);
    Core::SystemResultStatus InitializeEmulation(const std::string& filepath);

    bool IsHandheldOnly();
    void SetDeviceType([[maybe_unused]] int index, int type);
    void OnGamepadConnectEvent([[maybe_unused]] int index);
    void OnGamepadDisconnectEvent([[maybe_unused]] int index);
    SoftwareKeyboard::AndroidKeyboard* SoftwareKeyboard();

private:
    static void LoadDiskCacheProgress(VideoCore::LoadCallbackStage stage, int progress, int max);
    static void OnEmulationStarted();
    static void OnEmulationStopped(Core::SystemResultStatus result);

private:
    // Window management
    std::unique_ptr<EmuWindow_Android> m_window;
    ANativeWindow* m_native_window{};

    // Core emulation
    Core::System m_system;
    InputCommon::InputSubsystem m_input_subsystem;
    Common::DetachedTasks m_detached_tasks;
    Core::PerfStatsResults m_perf_stats{};
    std::shared_ptr<FileSys::VfsFilesystem> m_vfs;
    Core::SystemResultStatus m_load_result{Core::SystemResultStatus::ErrorNotInitialized};
    std::atomic<bool> m_is_running = false;
    std::atomic<bool> m_is_paused = false;
    SoftwareKeyboard::AndroidKeyboard* m_software_keyboard{};
    std::unique_ptr<Service::Account::ProfileManager> m_profile_manager;
    std::unique_ptr<FileSys::ManualContentProvider> m_manual_provider;

    // GPU driver parameters
    std::shared_ptr<Common::DynamicLibrary> m_vulkan_library;

    // Synchronization
    std::condition_variable_any m_cv;
    mutable std::mutex m_perf_stats_mutex;
    mutable std::mutex m_mutex;
};