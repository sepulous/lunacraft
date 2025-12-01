#ifndef OPTIONS_H
#define OPTIONS_H

struct Options
{
    int render_distance = 3;
    bool show_fog = true;
    bool show_gui = true;
    bool show_debug_info = false;
    float sfx_volume = 0.5f;
    float music_volume = 0.5f;
    float sensitivity = 1.0f;
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

#endif
