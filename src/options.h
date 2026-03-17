#pragma once

struct Options
{
    float sfx_volume = 0.5f;
    float music_volume = 0.5f;
    float sensitivity = 1.0f;
    int render_distance = 3;
    bool show_fog = true;
    bool show_gui = true;
    bool show_debug_info = false;
    bool fullscreen = false;
    bool fxaa = true;
};

class OptionsManager
{
    private:
        static Options _options;

    public:
        static void Init();
        static Options GetOptions();
        static void SetOptions(Options new_options);
        static void SaveOptions();
};
