#ifndef UI_H
#define UI_H

#include "shader.h"

class UIScreenImage
{
    private:
        unsigned int _vao;
        unsigned int _vbo;
        unsigned int _texture;

    public:
        UIScreenImage();
        UIScreenImage(const char *image_path, float x, float y, float width, float height);
        void SetImage(const char *image_path);
        void Render();
};

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
        UIScreenImage _lunacraft_text;
        UIScreenImage _background_images[5];
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
