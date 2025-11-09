#ifndef UI_H
#define UI_H

#include "shader.h"

class UIButton
{
    // ...
};

class UIMoonSettingsMenu
{
    // ...
};

class UIOptionsMenu
{
    // ...
};

class UIDeleteMoonMenu
{

};

class UIMainMenu
{
    private:
        unsigned int _lunacraft_text_vao;
        unsigned int _lunacraft_text_texture;
        unsigned int _bg_textures[5];
        unsigned int _bg_vao;
        Shader _bg_shader;
        int _current_bg = 0;
        float _current_bg_time = 0;
        // UIMoonSettingsMenu _moon_settings_menu;
        // UIDeleteMoonMenu _delete_moon_menu;
        // UIOptionsMenu _options_menu;
        // UIButton _buttons[10]; // 4 moons, 4 deletes, options, quit (there are more; just for now)

    public:
        UIMainMenu();
        void Update();
        void Render(float delta_time);
};

class UIPauseMenu
{

};

#endif
