#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <format>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ui.h"
#include "storage.h"
#include "input.h"
#include "shader.h"
#include "options.h"
#include "viewport.h"
#include "rng.h"
#include "inventory.h"
#include "sound_system.h"
#include "player.h"
#include "item.h"
#include "moon.h"
#include "dropped_item.h"

#include <stb_image/stb_image.h>
#include <stb_truetype/stb_truetype.h>

// Must compile shaders before calling this!
void UIRescale()
{
    auto viewport_dimensions = Viewport::GetDimensions();
    glm::mat4 proj = glm::ortho(0.0, (double)viewport_dimensions.x, 0.0, (double)viewport_dimensions.y, -1.0, 1.0);
    glm::mat4 ui_matrix = proj * UIGetVirtualToWindow();

    ShaderManager::UI_IMAGE_SHADER.Use();
    glUniformMatrix4fv(glGetUniformLocation(ShaderManager::UI_IMAGE_SHADER.GetID(), "u_ui_matrix"), 1, GL_FALSE, glm::value_ptr(ui_matrix));

    ShaderManager::UI_TEXT_SHADER.Use();
    glUniformMatrix4fv(glGetUniformLocation(ShaderManager::UI_TEXT_SHADER.GetID(), "u_ui_matrix"), 1, GL_FALSE, glm::value_ptr(ui_matrix));
}

glm::mat4 UIGetVirtualToWindow()
{
    auto viewport_dimensions = Viewport::GetDimensions();

    float aspect_ratio_ratio = ((float)viewport_dimensions.x / (float)viewport_dimensions.y) / (16.0f / 9.0f);
    float scale;
    if (aspect_ratio_ratio < 1.0f)
        scale = std::max((float)viewport_dimensions.x / VIRTUAL_UI_WIDTH, (float)viewport_dimensions.y / VIRTUAL_UI_HEIGHT);
    else
        scale = std::min((float)viewport_dimensions.x / VIRTUAL_UI_WIDTH, (float)viewport_dimensions.y / VIRTUAL_UI_HEIGHT);

    float scaled_virtual_width = VIRTUAL_UI_WIDTH * scale;
    float scaled_virtual_height = VIRTUAL_UI_HEIGHT * scale;
    float offset_x = ((float)viewport_dimensions.x - scaled_virtual_width)  * 0.5f;
    float offset_y = ((float)viewport_dimensions.y - scaled_virtual_height) * 0.5f;
    return glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(offset_x, offset_y, 0.0f)), glm::vec3(scale, scale, 1.0f));
}

//
// Main Menu
//

UIMainMenu::UIMainMenu()
{
    // Lunacraft logo
    lunacraft_logo_.LoadImage(Storage::IMAGES / "lunacraft.png");
    lunacraft_logo_.SetPosition({70, 875});
    lunacraft_logo_.SetSize({859, 130});

    // Background images
    int i = 1;
    for (UIImage& background_image : background_images_)
    {
        std::string image_path = std::string("main_menu_") + std::to_string(i) + ".png";
        background_image.LoadImage(Storage::IMAGES / image_path);
        background_image.SetPosition({0, 0});
        background_image.SetSize({VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT});
        i++;
    }

    // Moon buttons
    i = 0;
    for (UIButton& moon_button : moon_buttons_)
    {
        glm::vec2 button_position = {595, 705 - i*(105 + 25)};
        glm::vec2 button_size = {820, 105};
        moon_button.SetSize(button_size);
        moon_button.SetPosition(button_position);
        moon_button.SetImage(Storage::IMAGES / "ui" / "ui_button_1.png");

        std::string button_text;
        std::filesystem::path moon_folder = Storage::MOONS / (std::string("moon") + std::to_string(i));
        if (std::filesystem::exists(moon_folder))
        {
            int chunk_count = std::count_if(
                std::filesystem::directory_iterator(moon_folder),
                std::filesystem::directory_iterator{},
                [](const std::filesystem::directory_entry& e){ return e.is_regular_file(); }
            );

            if (chunk_count == 0)
            {
                button_text = std::string("Moon ") + "ABCD"[i] + " - Unexplored";
                moon_button.SetClickAction([this, i]() {
                    moon_settings_menu_.SetMoonID(i);
                    moon_settings_menu_.SetActive(true);
                });
            }
            else
            {
                float distance_traveled = glm::sqrt(chunk_count);
                std::ostringstream text;
                text << "Moon " << "ABCD"[i] << " - " << std::fixed << std::setprecision(1) << distance_traveled << " Sq km";
                button_text = text.str();
                moon_button.SetClickAction([i, this]() {
                    moon_settings_menu_.SetMoonID(i);
                    moon_settings_menu_.SetLaunchButtonClicked(true);
                });
            }
        }
        else
        {
            button_text = std::string("Moon ") + "ABCD"[i] + " - Unexplored";
            moon_button.SetClickAction([this, i]() {
                moon_settings_menu_.SetMoonID(i);
                moon_settings_menu_.SetActive(true);
            });
        }
        glm::vec2 text_size = UIText::GetTextSizeInPixels(button_text, 0.5f);
        moon_button.GetText().SetPosition({
            button_position.x + (button_size.x / 2.0f) - (text_size.x / 2.0f),
            button_position.y + (button_size.y / 2.0f) - (text_size.y / 2.0f)
        });
        moon_button.SetText(button_text, 0.5f, {0.0f, 0.0f, 0.0f, 1.0f});

        i++;
    }

    // Reset buttons
    i = 0;
    for (UIButton& reset_button : reset_buttons_)
    {
        glm::vec2 button_position = {1475, 715 - i*(80 + 50)};
        glm::vec2 button_size = {210, 80};
        reset_button.SetSize(button_size);
        reset_button.SetPosition(button_position);
        reset_button.SetImage(Storage::IMAGES / "ui" / "ui_button_2.png");

        glm::vec2 text_size = UIText::GetTextSizeInPixels("Reset", 0.425f);
        reset_button.GetText().SetPosition({
            button_position.x + (button_size.x / 2.0f) - (text_size.x / 2.0f),
            button_position.y + (button_size.y / 2.0f) - (text_size.y / 2.0f)
        });
        reset_button.SetText("Reset", 0.425f, {0.9f, 0.0f, 0.0f, 1.0f});
        reset_button.SetClickAction([this, i]() {
            reset_moon_menu_.SetMoon(i);
            reset_moon_menu_.SetActive(true);
        });

        i++;
    }

    // Options button
    glm::vec2 options_button_position = {785, 175};
    glm::vec2 options_button_size = {390, 105};
    float options_font_size = 0.6f;
    options_button_.SetSize(options_button_size);
    options_button_.SetPosition(options_button_position);
    options_button_.SetImage(Storage::IMAGES / "ui" / "ui_button_3.png");
    glm::vec2 options_text_size = UIText::GetTextSizeInPixels("Options", options_font_size);
    options_button_.GetText().SetPosition({
        options_button_position.x + (options_button_size.x / 2.0f) - (options_text_size.x / 2.0f),
        options_button_position.y + (options_button_size.y / 2.0f) - (options_text_size.y / 2.0f)
    });
    options_button_.SetText("Options", options_font_size, {0.0f, 0.0f, 0.0f, 1.0f});
    options_button_.SetClickAction([this]() { options_menu_.SetActive(true); });

    // Quit button
    glm::vec2 quit_button_position = {1195, 175};
    glm::vec2 quit_button_size = {220, 105};
    float quit_font_size = 0.6f;
    quit_button_.SetSize(quit_button_size);
    quit_button_.SetPosition(quit_button_position);
    quit_button_.SetImage(Storage::IMAGES / "ui" / "ui_button_4.png");
    glm::vec2 quit_text_size = UIText::GetTextSizeInPixels("Quit", quit_font_size);
    quit_button_.GetText().SetPosition({
        quit_button_position.x + (quit_button_size.x / 2.0f) - (quit_text_size.x / 2.0f),
        quit_button_position.y + (quit_button_size.y / 2.0f) - (quit_text_size.y / 2.0f)
    });
    quit_button_.SetText("Quit", quit_font_size, {0.0f, 0.0f, 0.0f, 1.0f});
}

void UIMainMenu::RefreshMoonButtonText()
{
    int i = 0;
    for (UIButton& moon_button : moon_buttons_)
    {
        glm::vec2 button_position = {595, 705 - i*(105 + 25)};
        glm::vec2 button_size = {820, 105};

        std::string button_text;
        std::filesystem::path moon_folder = Storage::MOONS / (std::string("moon") + std::to_string(i));
        if (std::filesystem::exists(moon_folder))
        {
            int chunk_count = std::count_if(
                std::filesystem::directory_iterator(moon_folder),
                std::filesystem::directory_iterator{},
                [](const std::filesystem::directory_entry& e){ return e.is_regular_file(); }
            );

            if (chunk_count == 0)
            {
                button_text = std::string("Moon ") + "ABCD"[i] + " - Unexplored";
                moon_button.SetClickAction([this, i]() {
                    moon_settings_menu_.SetMoonID(i);
                    moon_settings_menu_.SetActive(true);
                });
            }
            else
            {
                float distance_traveled = glm::sqrt(chunk_count);
                std::ostringstream text;
                text << "Moon " << "ABCD"[i] << " - " << std::fixed << std::setprecision(1) << distance_traveled << " Sq km";
                button_text = text.str();
                moon_button.SetClickAction([i, this]() {
                    moon_settings_menu_.SetMoonID(i);
                    moon_settings_menu_.SetLaunchButtonClicked(true);
                });
            }
        }
        else
        {
            button_text = std::string("Moon ") + "ABCD"[i] + " - Unexplored";
            moon_button.SetClickAction([this, i]() {
                moon_settings_menu_.SetMoonID(i);
                moon_settings_menu_.SetActive(true);
            });
        }
        glm::vec2 text_size = UIText::GetTextSizeInPixels(button_text, 0.5f);
        moon_button.GetText().SetPosition({
            button_position.x + (button_size.x / 2.0f) - (text_size.x / 2.0f),
            button_position.y + (button_size.y / 2.0f) - (text_size.y / 2.0f)
        });
        moon_button.SetText(button_text, 0.5f, {0.0f, 0.0f, 0.0f, 1.0f});

        i++;
    }
}

void UIMainMenu::ResetMoonSettings()
{
    moon_settings_menu_.Reset();
}

void UIMainMenu::SetLoadProgressLevel(float progress)
{
    load_moon_menu_.SetActive(progress > 0);
    load_moon_menu_.SetProgressLevel(progress);
}

bool UIMainMenu::IsQuitClicked()
{
    return quit_button_.IsClicked();
}

bool UIMainMenu::IsLaunchButtonClicked()
{
    return moon_settings_menu_.IsLaunchButtonClicked();
}

void UIMainMenu::SetLaunchButtonClicked(bool status)
{
    return moon_settings_menu_.SetLaunchButtonClicked(status);
}

std::pair<int, MoonSettings> UIMainMenu::GetMoonData()
{
    return std::pair<int, MoonSettings>{moon_settings_menu_.GetMoonID(), moon_settings_menu_.GetMoonSettings()};
}

void UIMainMenu::Update(float delta_time)
{
    if (moon_settings_menu_.IsActive())
    {
        moon_settings_menu_.Update(delta_time);
        if (moon_settings_menu_.IsLaunchButtonClicked())
            load_moon_menu_.SetActive(true);
    }
    else if (options_menu_.IsActive())
    {
        options_menu_.Update();
    }
    else if (reset_moon_menu_.IsActive())
    {
        reset_moon_menu_.Update();
        if (reset_moon_menu_.ResetClicked())
            RefreshMoonButtonText();
    }
    else if (!load_moon_menu_.IsActive()) // Player can't interact with buttons behind active menus
    {
        for (UIButton& moon_button : moon_buttons_)
            moon_button.Update();

        for (UIButton& reset_button : reset_buttons_)
            reset_button.Update();

        options_button_.Update();
        quit_button_.Update();
    }
}

void UIMainMenu::Render(float delta_time)
{
    auto viewport = Viewport::GetDimensions();
    float aspect_ratio_ratio = ((float)viewport.x / (float)viewport.y) / (16.0f / 9.0f);
    float aspect_scale = (aspect_ratio_ratio > 1.0f) ? aspect_ratio_ratio : (1.0f / aspect_ratio_ratio);

    Shader &ui_image_shader = ShaderManager::UI_IMAGE_SHADER;
    
    //
    // Background images
    //

    ui_image_shader.Use();
    ui_image_shader.SetFloat("u_darkness", 0.0f);

    // Fully visible for 8 seconds, fades out in 1 second
    const float opaque_time = 8.0f;
    const float fade_time = 1.0f;
    const float scale_speed = 0.04f;
    glm::vec2 current_position;
    glm::vec2 offset;

    if (current_background_time_ < opaque_time) // Current background is fully opaque
    {
        float scale = scale_speed * current_background_time_ + 1.0f;
        ui_image_shader.SetFloat("u_opacity", 1.0f);

        current_position = background_images_[current_background_].GetPosition();
        offset = {(VIRTUAL_UI_WIDTH * (aspect_scale * scale - 1.0f)) / 2.0f, (VIRTUAL_UI_HEIGHT * (aspect_scale * scale - 1.0f)) / 2.0f};
        background_images_[current_background_].SetSize(aspect_scale * scale * glm::vec2(VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT));
        background_images_[current_background_].SetPosition({-offset.x, -offset.y});
        background_images_[current_background_].Render();
    }
    else // Cross-fading
    {
        float scale, opacity;

        // Current
        scale = scale_speed * current_background_time_ + 1.0f;
        opacity = (opaque_time + fade_time) - current_background_time_;
        ui_image_shader.SetFloat("u_opacity", opacity);

        current_position = background_images_[current_background_].GetPosition();
        offset = {(VIRTUAL_UI_WIDTH * (aspect_scale * scale - 1.0f)) / 2.0f, (VIRTUAL_UI_HEIGHT * (aspect_scale * scale - 1.0f)) / 2.0f};
        background_images_[current_background_].SetSize(aspect_scale * scale * glm::vec2(VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT));
        background_images_[current_background_].SetPosition({-offset.x, -offset.y});
        background_images_[current_background_].Render();

        // Next
        scale = scale_speed * (current_background_time_ - opaque_time) + 1.0f;
        opacity = (current_background_time_ - opaque_time) / fade_time;
        ui_image_shader.SetFloat("u_opacity", opacity);

        current_position = background_images_[(current_background_ + 1) % 5].GetPosition();
        offset = {(VIRTUAL_UI_WIDTH * (aspect_scale * scale - 1.0f)) / 2.0f, (VIRTUAL_UI_HEIGHT * (aspect_scale * scale - 1.0f)) / 2.0f};
        background_images_[(current_background_ + 1) % 5].SetSize(aspect_scale * scale * glm::vec2(VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT));
        background_images_[(current_background_ + 1) % 5].SetPosition({-offset.x, -offset.y});
        background_images_[(current_background_ + 1) % 5].Render();

        if (current_background_time_ >= opaque_time + fade_time)
        {
            current_background_time_ = fade_time;
            background_images_[current_background_].SetSize(aspect_scale * glm::vec2{VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT});
            background_images_[current_background_].SetPosition({0, 0});
            current_background_ = (current_background_ + 1) % 5;
        }
    }

    current_background_time_ += delta_time;


    ui_image_shader.SetFloat("u_opacity", 1.0f);

    lunacraft_logo_.Render();

    for (UIButton& moon_button : moon_buttons_)
        moon_button.Render();

    for (UIButton& reset_button : reset_buttons_)
        reset_button.Render();

    options_button_.Render();
    quit_button_.Render();

    ui_image_shader.SetFloat("u_darkness", 0.0f);
    if (moon_settings_menu_.IsActive())
        moon_settings_menu_.Render();
    else if (options_menu_.IsActive())
        options_menu_.Render();
    else if (reset_moon_menu_.IsActive())
        reset_moon_menu_.Render();
    else if (load_moon_menu_.IsActive())
        load_moon_menu_.Render();
}

//
// Moon Settings Menu
//

UIMoonSettingsMenu::UIMoonSettingsMenu()
{
    // Background
    background_.LoadImage(Storage::IMAGES / "ui" / "ui_menu_bg.png", GL_NEAREST);
    float bg_width = 1200;
    float bg_height = 700;
    float bg_pos_x = (VIRTUAL_UI_WIDTH / 2.0f) - (bg_width / 2.0f);
    float bg_pos_y = (VIRTUAL_UI_HEIGHT / 2.0f) - (bg_height / 2.0f);
    background_.SetSize({bg_width, bg_height});
    background_.SetPosition({bg_pos_x, bg_pos_y});

    // Title
    glm::vec2 title_size = UIText::GetTextSizeInPixels("Moon Settings", 0.8f);
    glm::vec2 title_position = {bg_pos_x + (bg_width / 2.0f) - (title_size.x / 2.0f), bg_pos_y + bg_height - 100};
    title_.SetPosition(title_position);
    title_.SetText("Moon Settings");
    title_.SetFontSize(0.8f);
    title_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    float setting_text_align_x = 580;
    float setting_font_size = 0.4f;

    // Tree cover
    float tree_text_width = UIText::GetTextSizeInPixels("Tree Cover:", setting_font_size).x;
    tree_cover_.SetPosition({bg_pos_x + setting_text_align_x - tree_text_width, 700});
    tree_cover_.SetText("Tree Cover:");
    tree_cover_.SetFontSize(setting_font_size);
    tree_cover_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Tree cover slider
    tree_cover_slider_.SetSize({460, 20});
    tree_cover_slider_.SetPosition({bg_pos_x + setting_text_align_x + 20, 700});
    tree_cover_slider_.SetBounds({0.0f, 1.0f});
    tree_cover_slider_.SetValue(0.5f);

    // Terrain roughness
    float roughness_text_width = UIText::GetTextSizeInPixels("Terrain Roughness:", setting_font_size).x;
    roughness_.SetPosition({bg_pos_x + setting_text_align_x - roughness_text_width, 640});
    roughness_.SetText("Terrain Roughness:");
    roughness_.SetFontSize(setting_font_size);
    roughness_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Terrain roughness slider
    roughness_slider_.SetPosition({bg_pos_x + setting_text_align_x + 20, 640});
    roughness_slider_.SetSize({460, 20});
    roughness_slider_.SetBounds({0.0f, 1.0f});
    roughness_slider_.SetValue(0.5f);

    // Wildlife level
    float wildlife_text_width = UIText::GetTextSizeInPixels("Wildlife Level:", setting_font_size).x;
    wildlife_.SetPosition({bg_pos_x + setting_text_align_x - wildlife_text_width, 580});
    wildlife_.SetText("Wildlife Level:");
    wildlife_.SetFontSize(setting_font_size);
    wildlife_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Wildlife level slider
    wildlife_slider_.SetPosition({bg_pos_x + setting_text_align_x + 20, 580});
    wildlife_slider_.SetSize({460, 20});
    wildlife_slider_.SetBounds({0.0f, 1.0f});
    wildlife_slider_.SetValue(0.5f);

    // Seed
    float seed_text_width = UIText::GetTextSizeInPixels("Seed:", setting_font_size).x;
    seed_.SetPosition({bg_pos_x + setting_text_align_x - seed_text_width, 520});
    seed_.SetText("Seed:");
    seed_.SetFontSize(setting_font_size);
    seed_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Seed textbox
    seed_textbox_.SetPosition({bg_pos_x + setting_text_align_x + 20, 505});
    seed_textbox_.SetSize({460, 50});

    // Mode description
    float mode_text_width = UIText::GetTextSizeInPixels("Survive on an alien moon", 0.3f).x;
    mode_description_.SetPosition({bg_pos_x + (bg_width / 2.0f) - (mode_text_width / 2.0f), bg_pos_y + 230});
    mode_description_.SetText("Survive on an alien moon");
    mode_description_.SetFontSize(0.3f);
    mode_description_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Creative button
    creative_button_.SetToggledImage(Storage::IMAGES / "ui" / "creative_toggled.png");
    creative_button_.SetUntoggledImage(Storage::IMAGES / "ui" / "creative_untoggled.png");
    glm::vec2 creative_button_size = {200, 60};
    creative_button_.SetSize(creative_button_size);
    creative_button_.SetPosition({bg_pos_x + (bg_width / 2.0f) - (creative_button_size.x / 2.0f) - 100, bg_pos_y + 150});

    // Explore button
    explore_button_.SetToggledImage(Storage::IMAGES / "ui" / "explore_toggled.png");
    explore_button_.SetUntoggledImage(Storage::IMAGES / "ui" / "explore_untoggled.png");
    glm::vec2 explore_button_size = {200, 60};
    explore_button_.SetSize(explore_button_size);
    explore_button_.SetPosition({bg_pos_x + (bg_width / 2.0f) + (explore_button_size.x / 2.0f) - 100, bg_pos_y + 150});
    explore_button_.SetToggled(true);

    // Back button
    glm::vec2 back_button_position = {bg_pos_x + 50, bg_pos_y + 40};
    glm::vec2 back_button_size = {160, 80};
    float back_font_size = 0.4f;
    back_button_.SetPosition(back_button_position);
    back_button_.SetSize(back_button_size);
    back_button_.SetImage(Storage::IMAGES / "ui" / "ui_button_4.png");
    back_button_.SetText("Back", back_font_size, {0.0f, 0.0f, 0.0f, 1.0f});
    glm::vec2 back_text_size = UIText::GetTextSizeInPixels("Back", back_font_size);
    back_button_.GetText().SetPosition({
        back_button_position.x + (back_button_size.x / 2.0f) - (back_text_size.x / 2.0f),
        back_button_position.y + (back_button_size.y / 2.0f) - (back_text_size.y / 2.0f)
    });
    back_button_.SetClickAction([this]() { active_ = false; });

    // Launch button
    glm::vec2 launch_button_size = {240, 80};
    glm::vec2 launch_button_position = {bg_pos_x + (bg_width / 2.0f) - (launch_button_size.x / 2.0f), bg_pos_y + 40};
    float launch_font_size = 0.4f;
    launch_button_.SetPosition(launch_button_position);
    launch_button_.SetSize(launch_button_size);
    launch_button_.SetImage(Storage::IMAGES / "ui" / "ui_button_3.png");
    launch_button_.SetText("Launch!", launch_font_size, {0.0f, 0.0f, 0.0f, 1.0f});
    glm::vec2 launch_text_size = UIText::GetTextSizeInPixels("Launch!", launch_font_size);
    launch_button_.GetText().SetPosition({
        launch_button_position.x + (launch_button_size.x / 2.0f) - (launch_text_size.x / 2.0f),
        launch_button_position.y + (launch_button_size.y / 2.0f) - (launch_text_size.y / 2.0f)
    });
    launch_button_.SetClickAction([this]() {
        moon_settings_.tree_cover = tree_cover_slider_.GetValue();
        moon_settings_.terrain_roughness = roughness_slider_.GetValue();
        moon_settings_.wildlife_level = wildlife_slider_.GetValue();
        moon_settings_.is_creative = creative_button_.IsToggled();

        std::string seed_text = seed_textbox_.GetText();
        std::stringstream trimmed_seed;
        for (char c : seed_text)
            if (c != ' ')
                trimmed_seed << c;

        uint64_t seed;
        if (trimmed_seed.str().empty())
        {
            seed = RNG{}.Get<uint64_t>();
        }
        else
        {
            seed = 1469598103934665603ULL;
            for (unsigned char c : trimmed_seed.str())
            {
                seed ^= c;
                seed *= 1099511628211ULL; // FNV prime
            }
        }
        
        moon_settings_.seed = seed;

        SetActive(false);
        SetLaunchButtonClicked(true);
    });
}

void UIMoonSettingsMenu::SetMoonID(int moon_id)
{
    moon_id_ = moon_id;
}

int UIMoonSettingsMenu::GetMoonID()
{
    return moon_id_;
}

MoonSettings UIMoonSettingsMenu::GetMoonSettings()
{
    return moon_settings_;
}

void UIMoonSettingsMenu::SetActive(bool status)
{
    active_ = status;
}

bool UIMoonSettingsMenu::IsActive()
{
    return active_;
}

void UIMoonSettingsMenu::SetLaunchButtonClicked(bool status)
{
    launch_clicked_ = status;
}

bool UIMoonSettingsMenu::IsLaunchButtonClicked()
{
    return launch_clicked_;
}

void UIMoonSettingsMenu::Reset()
{
    // Reset sliders and seed textbox
    tree_cover_slider_.SetValue(0.5f);
    roughness_slider_.SetValue(0.5f);
    wildlife_slider_.SetValue(0.5f);
    seed_textbox_.SetText("");
    
    // Reset gamemode stuff
    float bg_width = 1200;
    float bg_height = 700;
    float bg_pos_x = (VIRTUAL_UI_WIDTH / 2.0f) - (bg_width / 2.0f);
    float bg_pos_y = (VIRTUAL_UI_HEIGHT / 2.0f) - (bg_height / 2.0f);
    float mode_text_width = UIText::GetTextSizeInPixels("Survive on an alien moon", 0.3f).x;
    mode_description_.SetPosition({bg_pos_x + (bg_width / 2.0f) - (mode_text_width / 2.0f), bg_pos_y + 230});
    mode_description_.SetText("Survive on an alien moon");
    explore_button_.SetToggled(true);
    creative_button_.SetToggled(false);
}

void UIMoonSettingsMenu::Update(float delta_time)
{
    back_button_.Update();
    launch_button_.Update();
    tree_cover_slider_.Update();
    roughness_slider_.Update();
    wildlife_slider_.Update();
    seed_textbox_.Update(delta_time);

    bool explore_was_toggled = explore_button_.IsToggled();
    explore_button_.Update();
    if (explore_was_toggled && !explore_button_.IsToggled()) // Prevent both buttons being untoggled
        explore_button_.SetToggled(true);
    if (explore_button_.IsToggled())
        creative_button_.SetToggled(false);

    bool creative_was_toggled = creative_button_.IsToggled();
    creative_button_.Update();
    if (creative_was_toggled && !creative_button_.IsToggled()) // Prevent both buttons being untoggled
        creative_button_.SetToggled(true);
    if (creative_button_.IsToggled())
        explore_button_.SetToggled(false);

    glm::vec2 bg_pos = background_.GetPosition();
    glm::vec2 bg_size = background_.GetSize();
    float mode_text_width;
    if (explore_button_.IsToggled())
    {
        mode_text_width = UIText::GetTextSizeInPixels("Survive on an alien moon", 0.3f).x;
        mode_description_.SetPosition({bg_pos.x + (bg_size.x / 2.0f) - (mode_text_width / 2.0f), bg_pos.y + 230});
        mode_description_.SetText("Survive on an alien moon");
    }
    else
    {
        mode_text_width = UIText::GetTextSizeInPixels("Create without limits, safely", 0.3f).x;
        mode_description_.SetPosition({bg_pos.x + (bg_size.x / 2.0f) - (mode_text_width / 2.0f), bg_pos.y + 230});
        mode_description_.SetText("Create without limits, safely");
    }
}

void UIMoonSettingsMenu::Render()
{
    background_.Render();
    title_.Render();
    tree_cover_.Render();
    tree_cover_slider_.Render();
    roughness_.Render();
    roughness_slider_.Render();
    wildlife_.Render();
    wildlife_slider_.Render();
    seed_.Render();
    seed_textbox_.Render();
    explore_button_.Render();
    creative_button_.Render();
    mode_description_.Render();
    back_button_.Render();
    launch_button_.Render();
}

//
// Load Moon Menu
//

UILoadMoonMenu::UILoadMoonMenu()
{
    // Background
    background_.LoadImage(Storage::IMAGES / "ui" / "ui_menu_bg.png", GL_NEAREST);
    float bg_width = 780;
    float bg_height = 420;
    float bg_pos_x = (VIRTUAL_UI_WIDTH / 2.0f) - (bg_width / 2.0f);
    float bg_pos_y = (VIRTUAL_UI_HEIGHT / 2.0f) - (bg_height / 2.0f);
    background_.SetSize({bg_width, bg_height});
    background_.SetPosition({bg_pos_x, bg_pos_y});

    // Title
    glm::vec2 title_size = UIText::GetTextSizeInPixels("Inserting astronaut...", 0.4f);
    glm::vec2 title_position = {bg_pos_x + (bg_width / 2.0f) - (title_size.x / 2.0f), bg_pos_y + bg_height - 100};
    title_.SetPosition(title_position);
    title_.SetText("Inserting astronaut...");
    title_.SetFontSize(0.4f);
    title_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Progress bar
    glm::vec2 progress_bar_size = {400, 20};
    progress_bar_.SetPosition({bg_pos_x + (bg_width / 2.0f) - (progress_bar_size.x / 2.0f), bg_pos_y + bg_height - 200});
    progress_bar_.SetSize(progress_bar_size);

    // Status text
    glm::vec2 status_size = UIText::GetTextSizeInPixels("Loading chunks...", 0.4f);
    glm::vec2 status_position = {bg_pos_x + (bg_width / 2.0f) - (status_size.x / 2.0f), bg_pos_y + bg_height - 300};
    status_.SetPosition(status_position);
    status_.SetText("Loading chunks...");
    status_.SetFontSize(0.4f);
    status_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});
}

void UILoadMoonMenu::SetActive(bool status)
{
    active_ = status;
}

bool UILoadMoonMenu::IsActive()
{
    return active_;
}

void UILoadMoonMenu::SetProgressLevel(float level)
{
    progress_bar_.SetLevel(level);
    int stage = level < 0.5f ? 0 : 1;
    if (stage != stage_)
    {
        float bg_width = 780;
        float bg_height = 420;
        float bg_pos_x = (VIRTUAL_UI_WIDTH / 2.0f) - (bg_width / 2.0f);
        float bg_pos_y = (VIRTUAL_UI_HEIGHT / 2.0f) - (bg_height / 2.0f);
        if (stage == 0) // Loading chunks...
        {
            glm::vec2 status_size = UIText::GetTextSizeInPixels("Loading chunks...", 0.4f);
            glm::vec2 status_position = {bg_pos_x + (bg_width / 2.0f) - (status_size.x / 2.0f), bg_pos_y + bg_height - 300};
            status_.SetPosition(status_position);
            status_.SetText("Loading chunks...");
        }
        else // Building geometry...
        {
            glm::vec2 status_size = UIText::GetTextSizeInPixels("Building geometry...", 0.4f);
            glm::vec2 status_position = {bg_pos_x + (bg_width / 2.0f) - (status_size.x / 2.0f), bg_pos_y + bg_height - 300};
            status_.SetPosition(status_position);
            status_.SetText("Building geometry...");
        }

        stage_ = stage;
    }
}

void UILoadMoonMenu::Update()
{
    
}

void UILoadMoonMenu::Render()
{
    background_.Render();
    title_.Render();
    progress_bar_.Render();
    status_.Render();
}

//
// Options Menu
//

UIOptionsMenu::UIOptionsMenu()
{
    // Background
    background_.LoadImage(Storage::IMAGES / "ui" / "ui_menu_bg.png", GL_NEAREST);
    float bg_width = 1200;
    float bg_height = 700;
    float bg_pos_x = (VIRTUAL_UI_WIDTH / 2.0f) - (bg_width / 2.0f);
    float bg_pos_y = (VIRTUAL_UI_HEIGHT / 2.0f) - (bg_height / 2.0f);
    background_.SetSize({bg_width, bg_height});
    background_.SetPosition({bg_pos_x, bg_pos_y});

    // Title
    glm::vec2 title_size = UIText::GetTextSizeInPixels("Options", 0.8f);
    glm::vec2 title_position = {bg_pos_x + (bg_width / 2.0f) - (title_size.x / 2.0f), bg_pos_y + bg_height - 100};
    title_.SetPosition(title_position);
    title_.SetText("Options");
    title_.SetFontSize(0.8f);
    title_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    Options current_options = OptionsManager::GetOptions();

    float option_text_align_x1 = 360;
    float option_text_align_x2 = 1050;
    float option_font_size = 0.4f;

    // SFX volume
    float sfx_text_width = UIText::GetTextSizeInPixels("SFX Vol:", option_font_size).x;
    sfx_volume_.SetPosition({bg_pos_x + option_text_align_x1 - sfx_text_width, 680});
    sfx_volume_.SetText("SFX Vol:");
    sfx_volume_.SetFontSize(option_font_size);
    sfx_volume_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // SFX volume slider
    sfx_volume_slider_.SetPosition({bg_pos_x + option_text_align_x1 + 25, 680});
    sfx_volume_slider_.SetSize({240, 20});
    sfx_volume_slider_.SetBounds({0.0f, 1.0f});
    sfx_volume_slider_.SetValue(current_options.sfx_volume);

    // Music volume
    float music_text_width = UIText::GetTextSizeInPixels("Music Vol:", option_font_size).x;
    music_volume_.SetPosition({bg_pos_x + option_text_align_x1 - music_text_width, 600});
    music_volume_.SetText("Music Vol:");
    music_volume_.SetFontSize(option_font_size);
    music_volume_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Music volume slider
    music_volume_slider_.SetPosition({bg_pos_x + option_text_align_x1 + 25, 600});
    music_volume_slider_.SetSize({240, 20});
    music_volume_slider_.SetBounds({0.0f, 1.0f});
    music_volume_slider_.SetValue(current_options.music_volume);

    // Sensitivity
    float sensitivity_text_width = UIText::GetTextSizeInPixels("Ctrl Sense:", option_font_size).x;
    sensitivity_.SetPosition({bg_pos_x + option_text_align_x1 - sensitivity_text_width, 520});
    sensitivity_.SetText("Ctrl Sense:");
    sensitivity_.SetFontSize(option_font_size);
    sensitivity_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Sensitivity slider
    sensitivity_slider_.SetPosition({bg_pos_x + option_text_align_x1 + 25, 520});
    sensitivity_slider_.SetSize({240, 20});
    sensitivity_slider_.SetBounds({0.0f, 1.0f});
    sensitivity_slider_.SetValue(current_options.sensitivity);

    // Render distance
    float render_distance_text_width = UIText::GetTextSizeInPixels("View Dist:", option_font_size).x;
    render_distance_.SetPosition({bg_pos_x + option_text_align_x1 - render_distance_text_width, 440});
    render_distance_.SetText("View Dist:");
    render_distance_.SetFontSize(option_font_size);
    render_distance_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Render distance slider
    render_distance_slider_.SetDiscrete(true);
    render_distance_slider_.SetPosition({bg_pos_x + option_text_align_x1 + 25, 440});
    render_distance_slider_.SetSize({240, 20});
    render_distance_slider_.SetBounds({1.0f, 12.0f});
    render_distance_slider_.SetValue(current_options.render_distance);

    // Show GUI
    float gui_text_width = UIText::GetTextSizeInPixels("Show GUI:", option_font_size).x;
    show_gui_.SetPosition({bg_pos_x + option_text_align_x2 - gui_text_width, 680});
    show_gui_.SetText("Show GUI:");
    show_gui_.SetFontSize(option_font_size);
    show_gui_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Show GUI toggle
    show_gui_toggle_.SetPosition({bg_pos_x + option_text_align_x2 + 20, 680 - 10});
    show_gui_toggle_.SetSize({40, 40});
    show_gui_toggle_.SetToggled(current_options.show_gui);

    // Show fog
    float fog_text_width = UIText::GetTextSizeInPixels("Show Fog:", option_font_size).x;
    show_fog_.SetPosition({bg_pos_x + option_text_align_x2 - fog_text_width, 600});
    show_fog_.SetText("Show Fog:");
    show_fog_.SetFontSize(option_font_size);
    show_fog_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Show fog toggle
    show_fog_toggle_.SetPosition({bg_pos_x + option_text_align_x2 + 20, 600 - 10});
    show_fog_toggle_.SetSize({40, 40});
    show_fog_toggle_.SetToggled(current_options.show_fog);

    // Show debug info
    float debug_text_width = UIText::GetTextSizeInPixels("Debug Info:", option_font_size).x;
    show_debug_.SetPosition({bg_pos_x + option_text_align_x2 - debug_text_width, 520});
    show_debug_.SetText("Debug Info:");
    show_debug_.SetFontSize(option_font_size);
    show_debug_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Show debug toggle
    show_debug_toggle_.SetPosition({bg_pos_x + option_text_align_x2 + 20, 520 - 10});
    show_debug_toggle_.SetSize({40, 40});
    show_debug_toggle_.SetToggled(current_options.show_debug_info);

    // VSync
    float vsync_text_width = UIText::GetTextSizeInPixels("VSync:", option_font_size).x;
    vsync_.SetPosition({bg_pos_x + option_text_align_x2 - vsync_text_width, 440});
    vsync_.SetText("VSync:");
    vsync_.SetFontSize(option_font_size);
    vsync_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // VSync
    vsync_toggle_.SetPosition({bg_pos_x + option_text_align_x2 + 20, 440 - 10});
    vsync_toggle_.SetSize({40, 40});
    vsync_toggle_.SetToggled(current_options.vsync);

    // Fullscreen
    float fullscreen_text_width = UIText::GetTextSizeInPixels("Fullscreen:", option_font_size).x;
    fullscreen_.SetPosition({bg_pos_x + option_text_align_x2 - fullscreen_text_width, 360});
    fullscreen_.SetText("Fullscreen:");
    fullscreen_.SetFontSize(option_font_size);
    fullscreen_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Fullscreen toggle
    fullscreen_toggle_.SetPosition({bg_pos_x + option_text_align_x2 + 20, 360 - 10});
    fullscreen_toggle_.SetSize({40, 40});
    fullscreen_toggle_.SetToggled(current_options.fullscreen);

    // Back button
    glm::vec2 back_button_position = {bg_pos_x + 50, bg_pos_y + 40};
    glm::vec2 back_button_size = {160, 80};
    float back_font_size = 0.4f;
    back_button_.SetPosition(back_button_position);
    back_button_.SetSize(back_button_size);
    back_button_.SetImage(Storage::IMAGES / "ui" / "ui_button_4.png");
    back_button_.SetText("Back", back_font_size, {0.0f, 0.0f, 0.0f, 1.0f});
    glm::vec2 back_text_size = UIText::GetTextSizeInPixels("Back", back_font_size);
    back_button_.GetText().SetPosition({
        back_button_position.x + (back_button_size.x / 2.0f) - (back_text_size.x / 2.0f),
        back_button_position.y + (back_button_size.y / 2.0f) - (back_text_size.y / 2.0f)
    });
    back_button_.SetClickAction([this]() {
        Options new_options;
        new_options.music_volume = music_volume_slider_.GetValue();
        new_options.render_distance = render_distance_slider_.GetValue();
        new_options.sensitivity = sensitivity_slider_.GetValue();
        new_options.sfx_volume = sfx_volume_slider_.GetValue();
        new_options.show_debug_info = show_debug_toggle_.IsToggled();
        new_options.show_fog = show_fog_toggle_.IsToggled();
        new_options.show_gui = show_gui_toggle_.IsToggled();
        new_options.vsync = vsync_toggle_.IsToggled();
        OptionsManager::SetOptions(new_options);
        active_ = false;
    });
}

void UIOptionsMenu::SetActive(bool status)
{
    active_ = status;
    if (active_)
    {
        // Update UI based on actual options
        Options options = OptionsManager::GetOptions();
        sfx_volume_slider_.SetValue(options.sfx_volume);
        music_volume_slider_.SetValue(options.music_volume);
        render_distance_slider_.SetValue(options.render_distance);
        sensitivity_slider_.SetValue(options.sensitivity);
        show_gui_toggle_.SetToggled(options.show_gui);
        show_fog_toggle_.SetToggled(options.show_fog);
        show_debug_toggle_.SetToggled(options.show_debug_info);
    }
}

bool UIOptionsMenu::IsActive()
{
    return active_;
}

void UIOptionsMenu::Update()
{
    back_button_.Update();
    sfx_volume_slider_.Update();
    music_volume_slider_.Update();
    sensitivity_slider_.Update();
    render_distance_slider_.Update();
    show_gui_toggle_.Update();
    show_fog_toggle_.Update();
    show_debug_toggle_.Update();
    vsync_toggle_.Update();
    fullscreen_toggle_.Update();

    Options options = OptionsManager::GetOptions();
    options.sfx_volume = sfx_volume_slider_.GetValue();
    options.music_volume = music_volume_slider_.GetValue();
    options.render_distance = render_distance_slider_.GetValue();
    options.sensitivity = sensitivity_slider_.GetValue();
    options.show_gui = show_gui_toggle_.IsToggled();
    options.show_fog = show_fog_toggle_.IsToggled();
    options.show_debug_info = show_debug_toggle_.IsToggled();
    options.vsync = vsync_toggle_.IsToggled();
    options.fullscreen = fullscreen_toggle_.IsToggled();
    OptionsManager::SetOptions(options);
}

void UIOptionsMenu::Render()
{
    background_.Render();
    title_.Render();
    sfx_volume_.Render();
    sfx_volume_slider_.Render();
    music_volume_.Render();
    music_volume_slider_.Render();
    sensitivity_.Render();
    sensitivity_slider_.Render();
    render_distance_.Render();
    render_distance_slider_.Render();
    show_gui_.Render();
    show_gui_toggle_.Render();
    show_fog_.Render();
    show_fog_toggle_.Render();
    show_debug_.Render();
    show_debug_toggle_.Render();
    vsync_.Render();
    vsync_toggle_.Render();
    fullscreen_.Render();
    fullscreen_toggle_.Render();
    back_button_.Render();
}

//
// Reset Moon Menu
//

UIResetMoonMenu::UIResetMoonMenu()
{
    // Background
    background_.LoadImage(Storage::IMAGES / "ui" / "ui_menu_bg.png", GL_NEAREST);
    float bg_width = 720;
    float bg_height = 420;
    float bg_pos_x = (VIRTUAL_UI_WIDTH / 2.0f) - (bg_width / 2.0f);
    float bg_pos_y = (VIRTUAL_UI_HEIGHT / 2.0f) - (bg_height / 2.0f);
    background_.SetSize({bg_width, bg_height});
    background_.SetPosition({bg_pos_x, bg_pos_y});

    // Title
    glm::vec2 title_size = UIText::GetTextSizeInPixels("Reset saved world?", 0.4f);
    glm::vec2 title_position = {bg_pos_x + (bg_width / 2.0f) - (title_size.x / 2.0f), bg_pos_y + bg_height - 100};
    title_.SetPosition(title_position);
    title_.SetText("Reset saved world?");
    title_.SetFontSize(0.4f);
    title_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Cancel button
    glm::vec2 cancel_button_position = {bg_pos_x + 100, bg_pos_y + 100};
    glm::vec2 cancel_button_size = {240, 80};
    float cancel_font_size = 0.4f;
    cancel_button_.SetPosition(cancel_button_position);
    cancel_button_.SetSize(cancel_button_size);
    cancel_button_.SetImage(Storage::IMAGES / "ui" / "ui_button_3.png");
    cancel_button_.SetText("Cancel", cancel_font_size, {0.0f, 0.0f, 0.0f, 1.0f});
    glm::vec2 cancel_text_size = UIText::GetTextSizeInPixels("Cancel", cancel_font_size);
    cancel_button_.GetText().SetPosition({
        cancel_button_position.x + (cancel_button_size.x / 2.0f) - (cancel_text_size.x / 2.0f),
        cancel_button_position.y + (cancel_button_size.y / 2.0f) - (cancel_text_size.y / 2.0f)
    });
    cancel_button_.SetClickAction([this]() { active_ = false; });

    // Reset button
    glm::vec2 reset_button_position = {bg_pos_x + 380, bg_pos_y + 100};
    glm::vec2 reset_button_size = {240, 80};
    float reset_font_size = 0.4f;
    reset_button_.SetPosition(reset_button_position);
    reset_button_.SetSize(reset_button_size);
    reset_button_.SetImage(Storage::IMAGES / "ui" / "ui_button_3.png");
    reset_button_.SetText("Reset", reset_font_size, {0.9f, 0.0f, 0.0f, 1.0f});
    glm::vec2 reset_text_size = UIText::GetTextSizeInPixels("Reset", reset_font_size);
    reset_button_.GetText().SetPosition({
        reset_button_position.x + (reset_button_size.x / 2.0f) - (reset_text_size.x / 2.0f),
        reset_button_position.y + (reset_button_size.y / 2.0f) - (reset_text_size.y / 2.0f)
    });
    reset_button_.SetClickAction([this]() {
        std::filesystem::path moon_path = Storage::MOONS / (std::string("moon") + std::to_string(moon_));
        std::filesystem::remove_all(moon_path);
        SetActive(false);
    });
}

void UIResetMoonMenu::SetMoon(int moon)
{
    moon_ = moon;
}

int UIResetMoonMenu::GetMoon()
{
    return moon_;
}

void UIResetMoonMenu::SetActive(bool status)
{
    active_ = status;
}

bool UIResetMoonMenu::IsActive()
{
    return active_;
}

bool UIResetMoonMenu::ResetClicked()
{
    return reset_button_.IsClicked();
}

void UIResetMoonMenu::Update()
{
    cancel_button_.Update();
    reset_button_.Update();
}

void UIResetMoonMenu::Render()
{
    background_.Render();
    title_.Render();
    cancel_button_.Render();
    reset_button_.Render();
}

//
// Pause Menu
//

UIPauseMenu::UIPauseMenu()
{
    background_.LoadImage(Storage::IMAGES / "ui" / "ui_black.png", GL_NEAREST);
    background_.SetPosition({0, 0});
    background_.SetSize({VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT});

    glm::vec2 button_size = {520, 120};
    float text_scale = 0.6f;

    glm::vec2 resume_button_position = {(VIRTUAL_UI_WIDTH / 2.0f) - (button_size.x / 2.0f), 700};
    resume_button_.SetSize(button_size);
    resume_button_.SetPosition(resume_button_position);
    resume_button_.SetImage(Storage::IMAGES / "ui" / "ui_button_3.png");
    resume_button_.SetText("Resume", text_scale, {0.0f, 0.0f, 0.0f, 1.0f});
    glm::vec2 resume_text_size = UIText::GetTextSizeInPixels("Resume", text_scale);
    resume_button_.GetText().SetPosition({
        resume_button_position.x + (button_size.x / 2.0f) - (resume_text_size.x / 2.0f),
        resume_button_position.y + (button_size.y / 2.0f) - (resume_text_size.y / 2.0f)
    });
    resume_button_.SetClickAction([this]() { SetActive(false); });

    glm::vec2 options_button_position = {(VIRTUAL_UI_WIDTH / 2.0f) - (button_size.x / 2.0f), 500};
    options_button_.SetSize(button_size);
    options_button_.SetPosition(options_button_position);
    options_button_.SetImage(Storage::IMAGES / "ui" / "ui_button_3.png");
    options_button_.SetText("Options", text_scale, {0.0f, 0.0f, 0.0f, 1.0f});
    glm::vec2 options_text_size = UIText::GetTextSizeInPixels("Options", text_scale);
    options_button_.GetText().SetPosition({
        options_button_position.x + (button_size.x / 2.0f) - (options_text_size.x / 2.0f),
        options_button_position.y + (button_size.y / 2.0f) - (options_text_size.y / 2.0f)
    });
    options_button_.SetClickAction([this]() { options_menu_.SetActive(true); });

    glm::vec2 quit_button_position = {(VIRTUAL_UI_WIDTH / 2.0f) - (button_size.x / 2.0f), 300};
    quit_button_.SetSize(button_size);
    quit_button_.SetPosition(quit_button_position);
    quit_button_.SetImage(Storage::IMAGES / "ui" / "ui_button_3.png");
    quit_button_.SetText("Save & Quit", text_scale, {0.0f, 0.0f, 0.0f, 1.0f});
    glm::vec2 quit_text_size = UIText::GetTextSizeInPixels("Save & Quit", text_scale);
    quit_button_.GetText().SetPosition({
        quit_button_position.x + (button_size.x / 2.0f) - (quit_text_size.x / 2.0f),
        quit_button_position.y + (button_size.y / 2.0f) - (quit_text_size.y / 2.0f)
    });
}

void UIPauseMenu::SetActive(bool value)
{
    active_ = value;
}

bool UIPauseMenu::IsActive()
{
    return active_;
}

bool UIPauseMenu::QuitClicked()
{
    return quit_clicked_;
}

bool UIPauseMenu::ResumeClicked()
{
    return resume_clicked_;
}

void UIPauseMenu::Update()
{
    if (options_menu_.IsActive())
    {
        options_menu_.Update();
    }
    else
    {
        resume_button_.Update();
        options_button_.Update();
        quit_button_.Update();
        resume_clicked_ = resume_button_.IsClicked();
        quit_clicked_ = quit_button_.IsClicked();
    }

    auto viewport = Viewport::GetDimensions();
    float aspect_ratio_ratio = ((float)viewport.x / (float)viewport.y) / (16.0f / 9.0f);
    float aspect_scale = (aspect_ratio_ratio > 1.0f) ? aspect_ratio_ratio : (1.0f / aspect_ratio_ratio);
    background_.SetSize(aspect_scale * glm::vec2{VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT});
    background_.SetPosition({(-VIRTUAL_UI_WIDTH * (aspect_scale - 1.0f)) / 2.0f, (-VIRTUAL_UI_HEIGHT * (aspect_scale - 1.0f)) / 2.0f});
}

void UIPauseMenu::Render()
{
    Shader &image_shader = ShaderManager::UI_IMAGE_SHADER;
    image_shader.Use();

    image_shader.SetFloat("u_opacity", 0.4f);
    background_.Render();
    image_shader.SetFloat("u_opacity", 1.0f);

    resume_button_.Render();
    options_button_.Render();
    quit_button_.Render();
    if (options_menu_.IsActive())
        options_menu_.Render();
}

//
// Debug Menu
//

UIDebugMenu::UIDebugMenu()
{
    
}

void UIDebugMenu::SetActive(bool value)
{
    active_ = value;
}

bool UIDebugMenu::IsActive()
{
    return active_;
}

void UIDebugMenu::Update(const DebugInfo& debug_info)
{
    std::stringstream debug_text;
    debug_text << "Lunacraft " << LUNACRAFT_VERSION << "\n"; // LUNACRAFT_VERSION is supplied by CMake
    debug_text << "FPS: " << debug_info.fps << "\n";
    debug_text << "Position: (" << std::fixed << std::setprecision(3) << debug_info.player_pos.x << ", " << debug_info.player_pos.y << ", " << debug_info.player_pos.z << ")\n";
    debug_text << "Seed: " << debug_info.seed;

    float text_height = UIText::GetTextSizeInPixels(debug_text.str(), 0.5f).y;
    debug_text_.SetPosition({40, 1000});
    debug_text_.SetText(debug_text.str());
    debug_text_.SetFontSize(0.5f);
    debug_text_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
}

void UIDebugMenu::Render()
{
    debug_text_.Render();
}

//
// Death screen
//

UIDeathScreen::UIDeathScreen()
{
    // Background
    background_.LoadImage(Storage::IMAGES / "ui" / "ui_menu_bg.png", GL_NEAREST);
    float bg_width = 920;
    float bg_height = 450;
    float bg_pos_x = (VIRTUAL_UI_WIDTH / 2.0f) - (bg_width / 2.0f);
    float bg_pos_y = (VIRTUAL_UI_HEIGHT / 2.0f) - (bg_height / 2.0f);
    background_.SetSize({bg_width, bg_height});
    background_.SetPosition({bg_pos_x, bg_pos_y});

    // Text
    glm::vec2 text_size = UIText::GetTextSizeInPixels("You have died.", 0.5f);
    glm::vec2 text_position = {bg_pos_x + (bg_width / 2.0f) - (text_size.x / 2.0f), bg_pos_y + bg_height - 100};
    text_.SetPosition(text_position);
    text_.SetText("You have died.");
    text_.SetFontSize(0.5f);
    text_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});

    // Ok button
    glm::vec2 ok_button_size = {360, 120};
    glm::vec2 ok_button_position = {bg_pos_x + (bg_width / 2.0f) - (ok_button_size.x / 2.0f), bg_pos_y + 100};
    float ok_font_size = 0.5f;
    ok_button_.SetPosition(ok_button_position);
    ok_button_.SetSize(ok_button_size);
    ok_button_.SetImage(Storage::IMAGES / "ui" / "ui_button_3.png");
    ok_button_.SetText("ok", ok_font_size, {0.0f, 0.0f, 0.0f, 1.0f});
    glm::vec2 ok_text_size = UIText::GetTextSizeInPixels("ok", ok_font_size);
    ok_button_.GetText().SetPosition({
        ok_button_position.x + (ok_button_size.x / 2.0f) - (ok_text_size.x / 2.0f),
        ok_button_position.y + (ok_button_size.y / 2.0f) - (ok_text_size.y / 2.0f)
    });
    ok_button_.SetClickAction([this]() { clicked_ok_ = true; });
}

void UIDeathScreen::Update()
{
    ok_button_.Update();
}

void UIDeathScreen::Render()
{
    background_.Render();
    text_.Render();
    ok_button_.Render();
}

bool UIDeathScreen::IsActive()
{
    return active_;
}

void UIDeathScreen::SetActive(bool active)
{
    active_ = active;
}

bool UIDeathScreen::ClickedOk()
{
    return clicked_ok_;
}

void UIDeathScreen::ResetClickedOk()
{
    clicked_ok_ = false;
}

//
// In-game UI
//

UIGame::UIGame()
{
    crosshair_.LoadImage(Storage::IMAGES / "ui" / "crosshair.png", GL_NEAREST);
    crosshair_.SetSize({50, 50});
    crosshair_.SetPosition({935, 515});

    alert_.SetPosition({40, 400});
    alert_.SetFontSize(0.3f);
    alert_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
}

UIPauseMenu &UIGame::GetPauseMenu()
{
    return pause_menu_;
}

UIDebugMenu &UIGame::GetDebugMenu()
{
    return debug_menu_;
}

UIInventory &UIGame::GetInventoryUI()
{
    return inventory_;
}

UIDeathScreen &UIGame::GetDeathScreen()
{
    return death_screen_;
}

void UIGame::SetAlert(std::string str)
{
    alert_.SetText(str);
    alert_active_ = true;
}

void UIGame::Update(float delta_time)
{
    if (alert_active_)
    {
        alert_time_ += delta_time;
        if (alert_time_ > 10.0f)
            alert_active_ = false;
    }
    else
    {
        alert_time_ = 0;
    }
}

void UIGame::Render()
{
    glDepthFunc(GL_LEQUAL);

    if (alert_active_)
        alert_.Render();

    if (OptionsManager::GetOptions().show_gui)
        crosshair_.Render();
        
    inventory_.Render();

    if (debug_menu_.IsActive())
        debug_menu_.Render();

    if (pause_menu_.IsActive())
        pause_menu_.Render();

    if (death_screen_.IsActive())
        death_screen_.Render();
}

//
// Inventory UI
//

UIInventory::UIInventory()
{
    hotbar_base_.LoadImage(Storage::IMAGES / "ui" / "hotbar_base.png", GL_NEAREST);
    hotbar_base_.SetSize({VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT});
    hotbar_base_.SetPosition({0, 0});

    inventory_base_.LoadImage(Storage::IMAGES / "ui" / "inventory_base.png", GL_NEAREST);
    inventory_base_.SetSize({VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT});
    inventory_base_.SetPosition({0, 0});

    scanner_slot_.first.SetPosition({362, 862});
    scanner_slot_.first.SetSize({76, 76});
    scanner_slot_.second.SetFontSize(0.3f);
    scanner_slot_.second.SetPosition({362 + 60, 862 + 10});
    scanner_slot_.second.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
    scanner_text_.SetPosition({540, 915});
    scanner_text_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
    scanner_text_.SetFontSize(0.2f);
    scanner_text_.SetText("INSERT ITEM TO\nRECEIVE\nSCANNING\nINFORMATION.");

    suit_status_text_.SetPosition({491, 20});
    suit_status_text_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
    suit_status_text_.SetFontSize(0.28f);
    suit_status_text_.SetText("Suit status:");

    suit_status_bar_.LoadImage(Storage::IMAGES / "ui" / "health_bar.png");
    suit_status_bar_.SetSize({186, 18});
    suit_status_bar_.SetPosition({735, 18});

    health_text_.SetPosition({1004, 20});
    health_text_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
    health_text_.SetFontSize(0.28f);
    health_text_.SetText("Health:");

    health_bar_.LoadImage(Storage::IMAGES / "ui" / "health_bar.png");
    health_bar_.SetSize({186, 18});
    health_bar_.SetPosition({1156, 18});

    hotbar_select_.LoadImage(Storage::IMAGES / "ui" / "inventory_select.png");
    hotbar_select_.SetSize({88, 88});
    hotbar_select_.SetPosition({477, 54});

    held_item_.LoadImage(Storage::IMAGES / "items" / "none.png");
    held_item_.SetSize({85, 85});
    held_amount_.SetFontSize(0.3f);
    held_amount_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});

    jetpack_icon_.LoadImage(Storage::IMAGES / "items" / "jetpack_t1.png", GL_NEAREST);
    jetpack_icon_.SetPosition({1488, 18});
    jetpack_icon_.SetSize({32, 32});

    jetpack_bar_bg_.LoadImage(Storage::IMAGES / "ui" / "jetpack_bar_bg.png", GL_NEAREST);
    jetpack_bar_bg_.SetPosition({1490, 56});
    jetpack_bar_bg_.SetSize({24, 100});

    jetpack_bar_.LoadImage(Storage::IMAGES / "ui" / "jetpack_bar.png", GL_NEAREST);
    jetpack_bar_.SetPosition({1490, 56});
    jetpack_bar_.SetSize({24, 100});

    
    //
    // Hotbar and inventory slots
    //

    const int INV_SLOT_SIZE = 88;
    const int INV_SLOT_MARGIN = 12;
    for (int row = 0; row < 5; row++)
    {
        for (int col = 0; col < 10; col++)
        {
            float x = 477 + (INV_SLOT_SIZE + INV_SLOT_MARGIN) * col;
            float y = 54 + (INV_SLOT_SIZE + INV_SLOT_MARGIN) * row;

            auto &[slot_image, slot_amount] = inventory_slots_[row][col];
            slot_image.SetPosition({x + 6, y});
            slot_image.SetSize({INV_SLOT_SIZE - 6, INV_SLOT_SIZE - 6});
            slot_amount.SetFontSize(0.3f);
            slot_amount.SetPosition({x + 62, y + 10});
            slot_amount.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
        }
    }

    //
    // Assembler slots
    //

    assembler_output_slot_.first.SetPosition({1468, 682});
    assembler_output_slot_.first.SetSize({76, 76});
    assembler_output_slot_.second.SetFontSize(0.3f);
    assembler_output_slot_.second.SetPosition({1468 + 60, 682 + 10});
    assembler_output_slot_.second.SetColor({1.0f, 1.0f, 1.0f, 1.0f});

    const int ASS_SLOT_SIZE = 76;
    const int ASS_SLOT_MARGIN = 13;
    for (int row = 0; row < 3; row++)
    {
        for (int col = 0; col < 3; col++)
        {
            float x = 1122 + (ASS_SLOT_SIZE + ASS_SLOT_MARGIN) * col;
            float y = 682 + (ASS_SLOT_SIZE + ASS_SLOT_MARGIN) * row;

            auto &[slot_image, slot_amount] = assembler_input_slots_[row][col];
            slot_image.SetPosition({x, y});
            slot_image.SetSize({ASS_SLOT_SIZE, ASS_SLOT_SIZE});
            slot_amount.SetFontSize(0.3f);
            slot_amount.SetPosition({x + 60, y + 10});
            slot_amount.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
        }
    }

    //
    // Spacesuit slots
    //

    // Helmet
    spacesuit_slots_[2].first.SetPosition({921, 906});
    spacesuit_slots_[2].first.SetSize({73, 73});
    spacesuit_slots_[2].second.SetFontSize(0.3f);
    spacesuit_slots_[2].second.SetPosition({921 + 60, 906 + 10});
    spacesuit_slots_[2].second.SetColor({1.0f, 1.0f, 1.0f, 1.0f});

    // Battery
    spacesuit_slots_[1].first.SetPosition({921, 793});
    spacesuit_slots_[1].first.SetSize({73, 73});
    spacesuit_slots_[1].second.SetFontSize(0.3f);
    spacesuit_slots_[1].second.SetPosition({921 + 60, 793 + 10});
    spacesuit_slots_[1].second.SetColor({1.0f, 1.0f, 1.0f, 1.0f});

    // Jetpack
    spacesuit_slots_[0].first.SetPosition({921, 658});
    spacesuit_slots_[0].first.SetSize({73, 73});
    spacesuit_slots_[0].second.SetFontSize(0.3f);
    spacesuit_slots_[0].second.SetPosition({921 + 60, 658 + 10});
    spacesuit_slots_[0].second.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
}

void UIInventory::RebuildUI(Player *player, bool force)
{
    auto &inventory = player->GetInventory();
    float suit_status = (float)player->GetSuitStatus() / 100.0f;
    float health = (float)player->GetHealth() / 100.0f;

    // Hotbar
    for (int col = 0; col < 10; col++)
    {
        auto &slot = inventory.inventory[0][col];
        auto &local_slot = local_inv_.inventory[0][col];
        if (force || local_slot != slot)
        {
            auto &[slot_image, slot_amount] = inventory_slots_[0][col];
            auto icon = GetItemIcon(slot.item);
            slot_image.LoadImage(icon.bytes, icon.width, icon.height, icon.num_channels, GL_NEAREST);
            if (slot.amount > 1)
                slot_amount.SetText(std::to_string(slot.amount));
            else
                slot_amount.SetText("");

            local_slot = slot;
        }
    }

    if (force || active_)
    {
        // Inventory
        for (int row = 1; row < 5; row++)
        {
            for (int col = 0; col < 10; col++)
            {
                auto &slot = inventory.inventory[row][col];
                auto &local_slot = local_inv_.inventory[row][col];
                if (force || local_slot != slot)
                {
                    auto &[slot_image, slot_amount] = inventory_slots_[row][col];
                    auto icon = GetItemIcon(slot.item);
                    slot_image.LoadImage(icon.bytes, icon.width, icon.height, icon.num_channels, GL_NEAREST);
                    if (slot.amount > 1)
                        slot_amount.SetText(std::to_string(slot.amount));
                    else
                        slot_amount.SetText("");

                    local_slot = slot;
                }
            }
        }

        // Spacesuit slots
        for (int i = 0; i < 3; i++)
        {
            auto &slot = inventory.spacesuit[i];
            auto &local_slot = local_inv_.spacesuit[i];
            if (force || local_slot != slot)
            {
                auto &[slot_image, slot_amount] = spacesuit_slots_[i];
                auto icon = GetItemIcon(slot.item);
                slot_image.LoadImage(icon.bytes, icon.width, icon.height, icon.num_channels, GL_NEAREST);
                if (slot.amount > 1)
                    slot_amount.SetText(std::to_string(slot.amount));
                else
                    slot_amount.SetText("");

                local_slot = slot;
            }
        }

        // Assembler slots
        for (int row = 0; row < 3; row++)
        {
            for (int col = 0; col < 3; col++)
            {
                auto &slot = inventory.assembler_input[row][col];
                auto &local_slot = local_inv_.assembler_input[row][col];
                if (force || local_slot != slot)
                {
                    auto &[slot_image, slot_amount] = assembler_input_slots_[row][col];
                    auto icon = GetItemIcon(slot.item);
                    slot_image.LoadImage(icon.bytes, icon.width, icon.height, icon.num_channels, GL_NEAREST);
                    if (slot.amount > 1)
                        slot_amount.SetText(std::to_string(slot.amount));
                    else
                        slot_amount.SetText("");

                    local_slot = slot;
                }
            }
        }

        if (force || local_inv_.assembler_output != inventory.assembler_output)
        {
            auto icon = GetItemIcon(inventory.assembler_output.item);
            assembler_output_slot_.first.LoadImage(icon.bytes, icon.width, icon.height, icon.num_channels, GL_NEAREST);
            if (inventory.assembler_output.amount > 1)
                assembler_output_slot_.second.SetText(std::to_string(inventory.assembler_output.amount));
            else
                assembler_output_slot_.second.SetText("");

            local_inv_.assembler_output = inventory.assembler_output;
        }

        // Scanner slot
        if (force || local_inv_.scanner != inventory.scanner)
        {
            auto icon = GetItemIcon(inventory.scanner.item);
            scanner_slot_.first.LoadImage(icon.bytes, icon.width, icon.height, icon.num_channels, GL_NEAREST);
            local_inv_.scanner = inventory.scanner;
        }
    }

    // Suit status and health bar
    suit_status_bar_.SetCrop({0.0f, 0.0f, suit_status, 1.0f});
    suit_status_bar_.SetSize({186 * suit_status, 18});
    health_bar_.SetCrop({0.0f, 0.0f, health, 1.0f});
    health_bar_.SetSize({186 * health, 18});

    ui_built_ = true;
}

void UIInventory::Update(Player *player)
{
    RebuildUI(player, !ui_built_);

    auto &inventory = player->GetInventory();
    float suit_status = (float)player->GetSuitStatus() / 100.0f;
    float health = (float)player->GetHealth() / 100.0f;
    float jetpack_level = (float)player->GetJetpackEnergy() / (float)player->GetMaxJetpackEnergy();

    glm::dvec2 mouse_position = Input::GetVirtualMousePosition(UIGetVirtualToWindow());

    // Update held item (UI)
    held_item_.SetPosition({mouse_position.x - (85 / 2), mouse_position.y - (85 / 2)});
    held_amount_.SetPosition(held_item_.GetPosition() + glm::vec2{60, 10});

    /////////////////////////////////////

    auto viewport = Viewport::GetDimensions();
    float aspect_ratio_ratio = ((float)viewport.x / (float)viewport.y) / (16.0f / 9.0f);
    float aspect_scale = (aspect_ratio_ratio > 1.0f) ? aspect_ratio_ratio : (1.0f / aspect_ratio_ratio);
    // // background_.SetSize(aspect_scale * glm::vec2{VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT});
    // // background_.SetPosition({(-VIRTUAL_UI_WIDTH * (aspect_scale - 1.0f)) / 2.0f, (-VIRTUAL_UI_HEIGHT * (aspect_scale - 1.0f)) / 2.0f});

    // hotbar_base_.SetSize(aspect_scale * glm::vec2{VIRTUAL_UI_WIDTH, VIRTUAL_UI_HEIGHT});
    // hotbar_base_.SetPosition({((aspect_scale - 1.0f)) / 1.0f, ((aspect_scale - 1.0f)) / 1.0f});

    if (aspect_ratio_ratio < 1.0f)
    {
        hotbar_base_.SetPosition({0, 0});
    }

    // Update suit status and health
    suit_status_bar_.SetCrop({0.0f, 0.0f, suit_status, 1.0f});
    suit_status_bar_.SetSize({186 * suit_status, 18});
    health_bar_.SetCrop({0.0f, 0.0f, health, 1.0f});
    health_bar_.SetSize({186 * health, 18});

    // Update jetpack level
    jetpack_bar_.SetSize({24, 100 * jetpack_level});

    /////////////////////////////////////

    // Update inventory
    if (!active_)
    {
        int scroll = Input::GetMouseScroll();
        if (scroll > 0) // Scroll left
        {
            if (inventory.selected_hotbar_slot == 0)
                inventory.selected_hotbar_slot = 9;
            else
                inventory.selected_hotbar_slot--;
        }
        else if (scroll < 0) // Scroll right
        {
            inventory.selected_hotbar_slot = (inventory.selected_hotbar_slot + 1) % 10;
        }

        if (Input::IsKeyPressed(GLFW_KEY_1))
            inventory.selected_hotbar_slot = 0;
        else if (Input::IsKeyPressed(GLFW_KEY_2))
            inventory.selected_hotbar_slot = 1;
        else if (Input::IsKeyPressed(GLFW_KEY_3))
            inventory.selected_hotbar_slot = 2;
        else if (Input::IsKeyPressed(GLFW_KEY_4))
            inventory.selected_hotbar_slot = 3;
        else if (Input::IsKeyPressed(GLFW_KEY_5))
            inventory.selected_hotbar_slot = 4;
        else if (Input::IsKeyPressed(GLFW_KEY_6))
            inventory.selected_hotbar_slot = 5;
        else if (Input::IsKeyPressed(GLFW_KEY_7))
            inventory.selected_hotbar_slot = 6;
        else if (Input::IsKeyPressed(GLFW_KEY_8))
            inventory.selected_hotbar_slot = 7;
        else if (Input::IsKeyPressed(GLFW_KEY_9))
            inventory.selected_hotbar_slot = 8;
        else if (Input::IsKeyPressed(GLFW_KEY_0))
            inventory.selected_hotbar_slot = 9;

        // Update selection box
        hotbar_select_.SetPosition({477 + (88 + 12) * inventory.selected_hotbar_slot, 54});
    }
    else
    {
        bool left_click = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        bool right_click = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
        if (left_click || right_click)
        {
            std::pair<UIImage, UIText> *ui_slot;
            ItemStack *clicked_slot = GetSlotUnderMouse(mouse_position, inventory, &ui_slot);
            if (inventory.held_stack.IsEmpty()) // Pick up stack
            {
                if (clicked_slot && !clicked_slot->IsEmpty() && clicked_slot != &inventory.assembler_output)
                {
                    if (left_click)
                    {
                        // Update inventory
                        inventory.held_stack = *clicked_slot;
                        *clicked_slot = {ItemID::none, 0};

                        // Update slot UI
                        auto slot_icon = GetItemIcon(ItemID::none);
                        ui_slot->first.LoadImage(slot_icon.bytes, slot_icon.width, slot_icon.height, slot_icon.num_channels, GL_NEAREST);
                        ui_slot->second.SetText("");
                    }
                    else
                    {
                        // Update inventory
                        int amount_to_take = glm::max(clicked_slot->amount / 2, 1);
                        clicked_slot->amount -= amount_to_take;
                        inventory.held_stack = {clicked_slot->item, amount_to_take};
                        if (clicked_slot->amount == 0)
                            clicked_slot->item = ItemID::none;

                        // Update slot UI
                        if (clicked_slot->amount == 0)
                        {
                            auto slot_icon = GetItemIcon(ItemID::none);
                            ui_slot->first.LoadImage(slot_icon.bytes, slot_icon.width, slot_icon.height, slot_icon.num_channels, GL_NEAREST);
                            ui_slot->second.SetText("");
                        }
                        else
                        {
                            ui_slot->second.SetText(std::to_string(clicked_slot->amount));
                        }
                    }

                    // Update held stack UI
                    auto held_item_icon = GetItemIcon(inventory.held_stack.item);
                    held_item_.LoadImage(held_item_icon.bytes, held_item_icon.width, held_item_icon.height, held_item_icon.num_channels, GL_NEAREST);
                    if (inventory.held_stack.amount > 1)
                        held_amount_.SetText(std::to_string(inventory.held_stack.amount));
                    else
                        held_amount_.SetText("");

                    // Update scanner text
                    if (clicked_slot == &inventory.scanner)
                        scanner_text_.SetText("INSERT ITEM TO\nRECEIVE\nSCANNING\nINFORMATION.");
                }
            }
            else // Place/swap stack
            {
                if (clicked_slot && clicked_slot != &inventory.assembler_output)
                {
                    if (clicked_slot->amount == 0) // Place
                    {
                        if (left_click)
                        {
                            *clicked_slot = inventory.held_stack;
                            inventory.held_stack = {ItemID::none, 0};
                        }
                        else
                        {
                            *clicked_slot = {inventory.held_stack.item, 1};
                            if (!inventory.IsCreative())
                            {
                                inventory.held_stack.amount--;
                                if (inventory.held_stack.amount < 1)
                                    inventory.held_stack = {ItemID::none, 0};
                            }
                        }

                        if (clicked_slot == &inventory.scanner)
                            SoundSystem::Play(SoundSystem::Sound::DING);
                    }
                    else // Add/swap
                    {
                        if (clicked_slot->item == inventory.held_stack.item) // Add to slot
                        {
                            if (left_click)
                            {
                                clicked_slot->amount += inventory.held_stack.amount;
                                inventory.held_stack = {ItemID::none, 0};
                            }
                            else
                            {
                                clicked_slot->amount++;
                                if (!inventory.IsCreative())
                                {
                                    inventory.held_stack.amount--;
                                    if (inventory.held_stack.amount < 1)
                                        inventory.held_stack = {ItemID::none, 0};
                                }
                            }
                        }
                        else // Swap with slot
                        {
                            ItemStack clicked_slot_copy = *clicked_slot;
                            *clicked_slot = inventory.held_stack;
                            inventory.held_stack = clicked_slot_copy;

                            if (clicked_slot == &inventory.scanner)
                                SoundSystem::Play(SoundSystem::Sound::DING);
                        }
                    }

                    // Update slot UI
                    auto slot_icon = GetItemIcon(clicked_slot->item);
                    ui_slot->first.LoadImage(slot_icon.bytes, slot_icon.width, slot_icon.height, slot_icon.num_channels, GL_NEAREST);
                    if (clicked_slot->amount > 1)
                        ui_slot->second.SetText(std::to_string(clicked_slot->amount));
                    else
                        ui_slot->second.SetText("");

                    // Update held stack UI
                    auto held_item_icon = GetItemIcon(inventory.held_stack.item);
                    held_item_.LoadImage(held_item_icon.bytes, held_item_icon.width, held_item_icon.height, held_item_icon.num_channels, GL_NEAREST);
                    if (inventory.held_stack.amount > 1)
                        held_amount_.SetText(std::to_string(inventory.held_stack.amount));
                    else
                        held_amount_.SetText("");

                    // Update scanner data
                    if (clicked_slot == &inventory.scanner && !inventory.scanner.IsEmpty())
                    {
                        auto scanner_data = GetItemScannerData(inventory.scanner.item);
                        std::string scanner_text;
                        if (ItemIsDisk(inventory.scanner.item))
                        {
                            scanner_text = std::format("{}", scanner_data[ScannerDataType::COMPOSITION]);
                        }
                        else
                        {
                            scanner_text = std::format("Type: {}\n\nComposition:\n{}\n\nValue: {}",
                                scanner_data[ScannerDataType::TYPE],
                                scanner_data[ScannerDataType::COMPOSITION],
                                scanner_data[ScannerDataType::VALUE]);
                        }
                        scanner_text_.SetText(scanner_text);
                    }
                }
            }

            // Assembler
            if (clicked_slot >= &inventory.assembler_input[0][0] && clicked_slot <= &inventory.assembler_input[3][3]) // Recompute output
            {
                auto recipe = inventory.GetRecipeMatch();
                if (recipe.empty())
                    inventory.assembler_output = {ItemID::none, 0};
                else
                    inventory.assembler_output = {recipe[0].first, recipe[0].second};

                auto icon = GetItemIcon(inventory.assembler_output.item);
                assembler_output_slot_.first.LoadImage(icon.bytes, icon.width, icon.height, icon.num_channels, GL_NEAREST);
                if (inventory.assembler_output.amount > 1)
                    assembler_output_slot_.second.SetText(std::to_string(inventory.assembler_output.amount));
                else
                    assembler_output_slot_.second.SetText("");
            }
            
            if (clicked_slot == &inventory.assembler_output && !inventory.assembler_output.IsEmpty()) // Take item (if possible)
            {
                auto recipe = inventory.GetRecipeMatch();
                if (!recipe.empty())
                {
                    // We shouldn't do anything if the item can't actually be taken in some way
                    if ((inventory.HasSpaceForItem(recipe[0].first) && Input::IsKeyHeld(GLFW_KEY_LEFT_SHIFT)) || inventory.held_stack.IsEmpty() || inventory.held_stack.item == recipe[0].first)
                    {
                        int recipe_idx = 1;
                        int min_row = 2;
                        int max_row = 0;
                        int min_col = 2;
                        int max_col = 0;
                        bool done_box = false;

                        for (int row = 2; row >= 0; row--)
                        {
                            for (int col = 2; col >= 0; col--)
                            {
                                ItemStack input = inventory.assembler_input[row][col];

                                if (!input.IsEmpty())
                                {
                                    done_box = true;
                                    min_row = std::min(min_row, row);
                                    max_row = std::max(max_row, row);
                                    min_col = std::min(min_col, col);
                                    max_col = std::max(max_col, col);
                                }
                            }
                        }

                        if (done_box)
                        {
                            for (int row = min_row; row <= max_row; row++)
                            {
                                for (int col = min_col; col <= max_col; col++)
                                {
                                    ItemStack& input_stack = inventory.assembler_input[row][col];

                                    int amount_to_take = recipe[recipe_idx].second;
                                    input_stack.amount -= amount_to_take;
                                    if (input_stack.amount < 1)
                                        input_stack.item = ItemID::none;
                                    recipe_idx++;
                                    if (recipe_idx == recipe.size())
                                        goto done_consuming_input;
                                }
                            }
                        }

                        done_consuming_input:

                        int added_slot_idx = -1;
                        if (Input::IsKeyHeld(GLFW_KEY_LEFT_SHIFT))
                        {
                            added_slot_idx = inventory.Add({recipe[0].first, recipe[0].second});
                        }
                        else if (inventory.held_stack.IsEmpty()) // Put output in hand
                        {
                            inventory.held_stack = {recipe[0].first, recipe[0].second};
                        }
                        else // Already holding output item
                        {
                            inventory.held_stack.amount += recipe[0].second;
                        }

                        if (inventory.GetRecipeMatch().empty())
                            inventory.assembler_output = {ItemID::none, 0};
                        
                        SoundSystem::Play(SoundSystem::Sound::CRAFT);

                        //
                        // Update UI
                        //

                        // Assembler
                        for (int row = 0; row < 3; row++)
                        {
                            for (int col = 0; col < 3; col++)
                            {
                                auto slot = inventory.assembler_input[row][col];
                                auto &[slot_image, slot_amount] = assembler_input_slots_[row][col];
                                auto item_icon = GetItemIcon(slot.item);
                                slot_image.LoadImage(item_icon.bytes, item_icon.width, item_icon.height, item_icon.num_channels, GL_NEAREST);
                                if (slot.amount > 1)
                                    slot_amount.SetText(std::to_string(slot.amount));
                                else
                                    slot_amount.SetText("");
                            }
                        }
                        auto output_icon = GetItemIcon(inventory.assembler_output.item);
                        assembler_output_slot_.first.LoadImage(output_icon.bytes, output_icon.width, output_icon.height, output_icon.num_channels, GL_NEAREST);
                        if (inventory.assembler_output.amount > 1)
                            assembler_output_slot_.second.SetText(std::to_string(inventory.assembler_output.amount));
                        else
                            assembler_output_slot_.second.SetText("");

                        // Held stack
                        held_item_.LoadImage(Storage::IMAGES / "items" / GetItemFile(inventory.held_stack.item), GL_NEAREST);
                        if (inventory.held_stack.amount > 1)
                            held_amount_.SetText(std::to_string(inventory.held_stack.amount));
                        else
                            held_amount_.SetText("");

                        // Inventory (if player shift-clicked)
                        if (added_slot_idx != -1)
                        {
                            int row = added_slot_idx / 10;
                            int col = added_slot_idx % 10;
                            auto added_slot = inventory.inventory[row][col];
                            auto &added_ui_slot = inventory_slots_[row][col];
                            auto icon = GetItemIcon(inventory.inventory[row][col].item);
                            added_ui_slot.first.LoadImage(icon.bytes, icon.width, icon.height, icon.num_channels, GL_NEAREST);
                            if (added_slot.amount > 1)
                                added_ui_slot.second.SetText(std::to_string(added_slot.amount));
                            else
                                added_ui_slot.second.SetText("");
                        }
                    }
                }
            }
        }
    }
}

void UIInventory::Render()
{
    Shader &image_shader = ShaderManager::UI_IMAGE_SHADER;
    image_shader.Use();
    image_shader.SetFloat("u_darkness", 0);

    //
    // Hotbar
    //

    if (OptionsManager::GetOptions().show_gui || active_)
    {
        hotbar_base_.Render();
        suit_status_text_.Render();
        suit_status_bar_.Render();
        health_text_.Render();
        health_bar_.Render();
        hotbar_select_.Render();

        // Jetpack
        jetpack_icon_.Render();
        jetpack_bar_bg_.Render();
        jetpack_bar_.Render();
        
        // Hotbar slots
        for (int col = 0; col < 10; col++)
        {
            auto &[slot_image, slot_amount] = inventory_slots_[0][col];
            slot_image.Render();
            slot_amount.Render();
        }
    }

    //
    // Rest of the inventory
    //

    if (active_)
    {
        inventory_base_.Render();
        
        // Inventory
        for (int row = 1; row < 5; row++)
        {
            for (int col = 0; col < 10; col++)
            {
                auto &[slot_image, slot_amount] = inventory_slots_[row][col];
                slot_image.Render();
                slot_amount.Render();
            }
        }

        // Assembler
        for (int row = 0; row < 3; row++)
        {
            for (int col = 0; col < 3; col++)
            {
                auto &[slot_image, slot_amount] = assembler_input_slots_[row][col];
                slot_image.Render();
                slot_amount.Render();
            }
        }
        assembler_output_slot_.first.Render();
        assembler_output_slot_.second.Render();

        // Spacesuit
        for (int i = 0; i < 3; i++)
        {
            auto &[slot_image, slot_amount] = spacesuit_slots_[i];
            slot_image.Render();
            slot_amount.Render();
        }

        // Scanner
        scanner_slot_.first.Render();
        scanner_slot_.second.Render();
        scanner_text_.Render();

        // Held stack
        held_item_.Render();
        held_amount_.Render();
    }
}

bool UIInventory::IsActive()
{
    return active_;
}

void UIInventory::SetActive(bool active)
{
    active_ = active;

    auto &held_stack = Moon::GetCurrentMoon()->GetPlayer()->GetInventory().held_stack;
    if (!active && !held_stack.IsEmpty())
    {
        auto &camera = Moon::GetCurrentMoon()->GetPlayer()->GetCamera();
        DroppedItem *dropped_item = new DroppedItem({
            .position = camera.position + 1.0f * camera.forward,
            .item = held_stack.item,
            .amount = held_stack.amount
        });
        dropped_item->SetVelocity(4.0f * camera.forward);
        Moon::GetCurrentMoon()->GetEntityManager().AddEntity(dropped_item);
        held_stack = {ItemID::none, 0};
        auto held_item_icon = GetItemIcon(ItemID::none);
        held_item_.LoadImage(held_item_icon.bytes, held_item_icon.width, held_item_icon.height, held_item_icon.num_channels, GL_NEAREST);
        held_amount_.SetText("");
    }
}

ItemStack *UIInventory::GetSlotUnderMouse(glm::dvec2 mouse_pos, Inventory &inventory, std::pair<UIImage, UIText> **out_slot)
{
    if (mouse_pos.x >= 362 && mouse_pos.x <= 362 + 76 && mouse_pos.y >= 862 && mouse_pos.y <= 862 + 76) // Scanner slot
    {
        *out_slot = &scanner_slot_;
        return &inventory.scanner;
    }
    else if (mouse_pos.x >= 921 && mouse_pos.x <= 921 + 73 && mouse_pos.y >= 658 && mouse_pos.y <= 658 + 73) // Jetpack slot
    {
        *out_slot = &spacesuit_slots_[0];
        return &inventory.spacesuit[0];
    }
    else if (mouse_pos.x >= 921 && mouse_pos.x <= 921 + 73 && mouse_pos.y >= 793 && mouse_pos.y <= 793 + 73) // Battery slot
    {
        *out_slot = &spacesuit_slots_[1];
        return &inventory.spacesuit[1];
    }
    else if (mouse_pos.x >= 921 && mouse_pos.x <= 921 + 73 && mouse_pos.y >= 906 && mouse_pos.y <= 906 + 73) // Helmet slot
    {
        *out_slot = &spacesuit_slots_[2];
        return &inventory.spacesuit[2];
    }
    else if (mouse_pos.x >= 1468 && mouse_pos.x <= 1468 + 76 && mouse_pos.y >= 682 && mouse_pos.y <= 682 + 76) // Assembler output
    {
        *out_slot = &assembler_output_slot_;
        return &inventory.assembler_output;
    }
    else
    {
        // Check inventory slots
        for (int row = 0; row < 5; row++)
        {
            for (int col = 0; col < 10; col++)
            {
                float x = 477 + (85 + 14) * col;
                float y = 56 + (85 + 14) * row;
                if (mouse_pos.x >= x && mouse_pos.x <= x + 85 && mouse_pos.y >= y && mouse_pos.y <= y + 85)
                {
                    *out_slot = &inventory_slots_[row][col];
                    return &inventory.inventory[row][col];
                }
            }
        }

        // Check assembler inputs
        for (int row = 0; row < 3; row++)
        {
            for (int col = 0; col < 3; col++)
            {
                float x = 1122 + (76 + 13) * col;
                float y = 682 + (76 + 13) * row;
                if (mouse_pos.x >= x && mouse_pos.x <= x + 76 && mouse_pos.y >= y && mouse_pos.y <= y + 76)
                {
                    *out_slot = &assembler_input_slots_[row][col];
                    return &inventory.assembler_input[row][col];
                }
            }
        }
    }

    return nullptr;
}

//
// Images
//

UIImage::UIImage(GLint filtering)
{
    float vertices[] = {
    //  Position--  UV--------
        0.0f, 0.0f, 0.0f, 0.0f, // Bottom left
        1.0f, 0.0f, 1.0f, 0.0f, // Bottom right
        1.0f, 1.0f, 1.0f, 1.0f, // Top right
        1.0f, 1.0f, 1.0f, 1.0f, // Top right
        0.0f, 1.0f, 0.0f, 1.0f, // Top left
        0.0f, 0.0f, 0.0f, 0.0f, // Bottom left
    };

    quad_.SetShader(ShaderManager::UI_IMAGE_SHADER);
    quad_.SetVertexData(vertices, 6, GL_STATIC_DRAW);

    position_ = glm::vec2(0);
    size_ = glm::vec2(0);
    crop_ = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
}

void UIImage::LoadImage(std::filesystem::path image_path, GLint filtering)
{
    int image_width, image_height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *image_data = stbi_load(reinterpret_cast<const char *>(image_path.u8string().c_str()), &image_width, &image_height, &num_channels, 0);

    quad_.SetTexture(image_data, image_width, image_height, num_channels, filtering);

    stbi_image_free(image_data);

    aspect_ratio_ = (float)image_width / (float)image_height;
}

void UIImage::LoadImage(unsigned char *bytes, int width, int height, int num_channels, GLint filtering)
{
    quad_.SetTexture(bytes, width, height, num_channels, filtering);
    aspect_ratio_ = (float)width / (float)height;
}

void UIImage::SetPosition(glm::vec2 position)
{
    position_ = position;
}

glm::vec2 UIImage::GetPosition()
{
    return position_;
}

void UIImage::SetSize(glm::vec2 size, bool preserve_aspect_ratio)
{
    if (preserve_aspect_ratio)
        size.x = size.y * aspect_ratio_;

    size_ = size;
}

glm::vec2 UIImage::GetSize()
{
    return size_;
}

void UIImage::SetCrop(glm::vec4 crop)
{
    crop_ = crop;
}

void UIImage::Render()
{
    glm::mat4 model_matrix{1.0};
    model_matrix = glm::translate(model_matrix, glm::vec3{position_, 0.0f});
    model_matrix = glm::scale(model_matrix, glm::vec3{size_, 0.0f});
    
    quad_.Render([&](Shader *shader) {
        shader->SetVec4("u_crop", crop_);
        shader->SetMat4("u_model_matrix", model_matrix);
    });
}

//
// Text
//

GLuint UIText::atlas_texture_ = 0;
stbtt_packedchar UIText::packed_chars_[95];
stbtt_aligned_quad UIText::aligned_quads_[95];

void UIText::SetupFont_()
{
    std::ifstream inputStream(Storage::FONTS / "ack.ttf", std::ios::binary);

    inputStream.seekg(0, std::ios::end);
    auto&& fontFileSize = inputStream.tellg();
    inputStream.seekg(0, std::ios::beg);

    uint8_t* fontDataBuf = new uint8_t[fontFileSize];

    inputStream.read((char*)fontDataBuf, fontFileSize);

    stbtt_fontinfo fontInfo = {};
    if(!stbtt_InitFont(&fontInfo, fontDataBuf, 0))
        std::cerr << "stbtt_InitFont() Failed!\n";

    const size_t font_atlas_width = 1024;
    const size_t font_atlas_height = 1024;
    unsigned char *font_atlas_texture_data = new uint8_t[font_atlas_width * font_atlas_height];
    
    stbtt_pack_context ctx;
    stbtt_PackBegin(
        &ctx,
        font_atlas_texture_data,
        font_atlas_width,
        font_atlas_height,
        0,                        // Stride in bytes
        1,                        // Padding between the glyphs
        nullptr
    );
    stbtt_PackFontRange(
        &ctx,                         // stbtt_pack_context
        fontDataBuf,                  // Font Atlas texture data
        0,                            // Font Index                                 
        128,                           // Size of font in pixels. (Use STBTT_POINT_SIZE(fontSize) to use points) 
        (int)' ',                     // Code point of the first charecter
        95,                           // No. of charecters to be included in the font atlas 
        packed_chars_                 // stbtt_packedchar array, this struct will contain the data to render a glyph
    );
    stbtt_PackEnd(&ctx);

    for (int i = 0; i < 95; i++)
    {
        float unused_x, unused_y;

        stbtt_GetPackedQuad(
            packed_chars_,              // Array of stbtt_packedchar
            font_atlas_width,                      // Width of the font atlas texture
            font_atlas_height,                     // Height of the font atlas texture
            i,                                   // Index of the glyph
            &unused_x, &unused_y,                  // current position of the glyph in screen pixel coordinates, (not required as we have a different corrdinate system)
            &aligned_quads_[i],         // stbtt_alligned_quad struct. (this struct mainly consists of the texture coordinates)
            0                                    // Allign X and Y position to a integer (doesn't matter because we are not using 'unusedX' and 'unusedY')
        );
    }

    delete[] fontDataBuf;
    
    // Set up font texture
    glGenTextures(1, &atlas_texture_);
    glBindTexture(GL_TEXTURE_2D, atlas_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, font_atlas_width, font_atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, font_atlas_texture_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    delete[] font_atlas_texture_data;
}

UIText::UIText()
{
    if (atlas_texture_ == 0) // Font must be initialized
    {
        UIText::SetupFont_();
    }

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // Position and UV coords
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float))); // Color
    glEnableVertexAttribArray(1);

    text_ = "";
    font_size_ = 1.0f;
    position_ = glm::vec2(0.0f, 0.0f);
    color_ = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

UIText::UIText(std::string text, float font_size, glm::vec2 position, glm::vec4 color)
{
    if (atlas_texture_ == 0) // Font must be initialized
    {
        UIText::SetupFont_();
    }

    text_ = text;
    font_size_ = font_size;
    position_ = position;
    color_ = color;

    int order[6] = { 0, 1, 2, 0, 2, 3 };
    glm::vec2 localPosition = position;
    std::vector<float> vertices(text.length() * 6 * (4 + 4));
    int vertices_index = 0;
    for (char ch : text)
    {
        // Check if the charecter glyph is in the font atlas.
        if (ch >= (int)' ' && ch <= (int)' ' + 95)
        {
            // Retrive the data that is used to render a glyph of charecter 'ch'
            stbtt_packedchar* packedChar = &packed_chars_[ch - (int)' ']; 
            stbtt_aligned_quad* alignedQuad = &aligned_quads_[ch - (int)' '];

            // The units of the fields of the above structs are in pixels, 
            // convert them to a unit of what we want be multilplying to pixelScale  
            glm::vec2 glyphSize = 
            {
                (packedChar->x1 - packedChar->x0) * font_size,
                (packedChar->y1 - packedChar->y0) * font_size
            };

            glm::vec2 glyphBoundingBoxBottomLeft = 
            {
                localPosition.x + (packedChar->xoff * font_size),
                localPosition.y - (packedChar->yoff + packedChar->y1 - packedChar->y0) * font_size
            };

            // The order of vertices of a quad goes top-right, top-left, bottom-left, bottom-right
            glm::vec2 glyphVertices[4] = 
            {
                { glyphBoundingBoxBottomLeft.x + glyphSize.x, glyphBoundingBoxBottomLeft.y + glyphSize.y },
                { glyphBoundingBoxBottomLeft.x, glyphBoundingBoxBottomLeft.y + glyphSize.y },
                { glyphBoundingBoxBottomLeft.x, glyphBoundingBoxBottomLeft.y },
                { glyphBoundingBoxBottomLeft.x + glyphSize.x, glyphBoundingBoxBottomLeft.y },
            };

            glm::vec2 glyphTextureCoords[4] = 
            {
                { alignedQuad->s1, alignedQuad->t0 },
                { alignedQuad->s0, alignedQuad->t0 },
                { alignedQuad->s0, alignedQuad->t1 },
                { alignedQuad->s1, alignedQuad->t1 },
            };

            for (int i = 0; i < 6; i++)
            {
                vertices[vertices_index] = glyphVertices[order[i]].x;
                vertices[vertices_index + 1] = glyphVertices[order[i]].y;
                vertices[vertices_index + 2] = glyphTextureCoords[order[i]].x;
                vertices[vertices_index + 3] = glyphTextureCoords[order[i]].y;
                vertices[vertices_index + 4] = color.r;
                vertices[vertices_index + 5] = color.g;
                vertices[vertices_index + 6] = color.b;
                vertices[vertices_index + 7] = color.a;
                vertices_index += 8;
            }

            localPosition.x += packedChar->xadvance * font_size;
        }
        else if (ch == '\n')
        {
            localPosition.y -= 128 * font_size;
            localPosition.x = position.x;
        }
    }

    // Set up VAO and VBO
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // Position and UV coords
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float))); // Color
    glEnableVertexAttribArray(1);
}

void UIText::Render()
{
    Shader &text_shader = ShaderManager::UI_TEXT_SHADER;
    text_shader.Use();
    text_shader.SetInt("u_font_atlas", 0);
    glBindTexture(GL_TEXTURE_2D, atlas_texture_);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, text_.length() * 6 * (4 + 4));
}

void UIText::SetText(std::string text)
{
    int order[6] = { 0, 1, 2, 0, 2, 3 };
    glm::vec2 localPosition = position_;
    std::vector<float> vertices;
    vertices.reserve(text.length() * 6 * (4 + 4));
    for (char ch : text)
    {
        // Check if the charecter glyph is in the font atlas.
        if (ch >= (int)' ' && ch <= (int)' ' + 95)
        {
            // Retrive the data that is used to render a glyph of charecter 'ch'
            stbtt_packedchar* packedChar = &packed_chars_[ch - (int)' ']; 
            stbtt_aligned_quad* alignedQuad = &aligned_quads_[ch - (int)' '];

            // The units of the fields of the above structs are in pixels, 
            // convert them to a unit of what we want be multilplying to pixelScale  
            glm::vec2 glyphSize = 
            {
                (packedChar->x1 - packedChar->x0) * font_size_,
                (packedChar->y1 - packedChar->y0) * font_size_
            };

            glm::vec2 glyphBoundingBoxBottomLeft = 
            {
                localPosition.x + (packedChar->xoff * font_size_),
                localPosition.y - (packedChar->yoff + packedChar->y1 - packedChar->y0) * font_size_
            };

            // The order of vertices of a quad goes top-right, top-left, bottom-left, bottom-right
            glm::vec2 glyphVertices[4] = 
            {
                { glyphBoundingBoxBottomLeft.x + glyphSize.x, glyphBoundingBoxBottomLeft.y + glyphSize.y },
                { glyphBoundingBoxBottomLeft.x,               glyphBoundingBoxBottomLeft.y + glyphSize.y },
                { glyphBoundingBoxBottomLeft.x,               glyphBoundingBoxBottomLeft.y },
                { glyphBoundingBoxBottomLeft.x + glyphSize.x, glyphBoundingBoxBottomLeft.y },
            };

            glm::vec2 glyphTextureCoords[4] = 
            {
                { alignedQuad->s1, alignedQuad->t0 },
                { alignedQuad->s0, alignedQuad->t0 },
                { alignedQuad->s0, alignedQuad->t1 },
                { alignedQuad->s1, alignedQuad->t1 },
            };

            for (int i = 0; i < 6; i++)
            {
                vertices.push_back(glyphVertices[order[i]].x);
                vertices.push_back(glyphVertices[order[i]].y);
                vertices.push_back(glyphTextureCoords[order[i]].x);
                vertices.push_back(glyphTextureCoords[order[i]].y);
                vertices.push_back(color_.r);
                vertices.push_back(color_.g);
                vertices.push_back(color_.b);
                vertices.push_back(color_.a);
            }

            localPosition.x += packedChar->xadvance * font_size_;
        }
        else if (ch == '\n')
        {
            localPosition.y -= 128 * font_size_;
            localPosition.x = position_.x;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    text_ = text;
}

void UIText::SetPosition(glm::vec2 position)
{
    position_ = position;
    SetText(text_);
}

void UIText::SetFontSize(float font_size)
{
    font_size_ = font_size;
    SetText(text_);
}

void UIText::SetColor(glm::vec4 color)
{
    color_ = color;
    SetText(text_);
}

glm::vec2 UIText::GetTextSizeInPixels(std::string text, float font_size)
{
    float width = 0, height = 0;
    float current_line_width = 0;
    for (int i = 0; i < text.length(); i++)
    {
        char ch = text.at(i);
        if (ch >= (int)' ' && ch <= (int)' ' + 95)
        {
            stbtt_packedchar* packedChar = &packed_chars_[ch - (int)' '];
            current_line_width += packedChar->xadvance * font_size;
        }
        
        if (ch == '\n' || i == text.length() - 1)
        {
            if (current_line_width > width)
                width = current_line_width;
            current_line_width = 0;
            height += 128 * font_size;
        }
    }

    return {width, height / 2.0f}; // I have no clue why height must be divided by two...
}

//
// Button
//

UIButton::UIButton()
{
    position_ = glm::vec2(0);
    size_ = glm::vec2(1);
    ClickAction_ = [](){};
}

void UIButton::SetImage(std::filesystem::path image_path)
{
    image_.LoadImage(image_path);
}

void UIButton::SetPosition(glm::vec2 position)
{
    position_ = position;
    text_.SetPosition(position);
    image_.SetPosition(position);
}

void UIButton::SetSize(glm::vec2 size)
{
    size_ = size;
    image_.SetSize(size);
}

void UIButton::SetText(std::string text, float font_size, glm::vec4 color)
{
    text_.SetText(text);
    text_.SetFontSize(font_size);
    text_.SetColor(color);
}

void UIButton::SetClickAction(std::function<void()> click_action)
{
    ClickAction_ = click_action;
}

bool UIButton::IsClicked()
{
    return clicked_;
}

void UIButton::Update()
{
    glm::dvec2 mouse_position = Input::GetVirtualMousePosition(UIGetVirtualToWindow());

    hovered_ = mouse_position.x >= position_.x && mouse_position.x <= position_.x + size_.x
            && mouse_position.y >= position_.y && mouse_position.y <= position_.y + size_.y;
    if (hovered_)
    {
        clicked_ = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        held_ = Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT);
    }
    else
    {
        clicked_ = false;
        held_ = false;
    }

    if (clicked_)
        ClickAction_();
}

void UIButton::Render()
{
    Shader &image_shader = ShaderManager::UI_IMAGE_SHADER;
    image_shader.Use();

    if (held_)
        image_shader.SetFloat("u_darkness", 0.2f);
    else if (hovered_)
        image_shader.SetFloat("u_darkness", 0.1f);
    else
        image_shader.SetFloat("u_darkness", 0.0f);

    image_.Render();

    ShaderManager::UI_TEXT_SHADER.Use();
    text_.Render();
}

//
// Toggle Button
//

UIToggleButton::UIToggleButton()
{
    toggled_image_.LoadImage(Storage::IMAGES / "ui" / "ui_toggle_checked.png");
    untoggled_image_.LoadImage(Storage::IMAGES / "ui" / "ui_toggle_unchecked.png");
}

void UIToggleButton::SetToggledImage(std::filesystem::path image_path)
{
    toggled_image_.LoadImage(image_path);
}

void UIToggleButton::SetUntoggledImage(std::filesystem::path image_path)
{
    untoggled_image_.LoadImage(image_path);
}

void UIToggleButton::SetPosition(glm::vec2 position)
{
    position_ = position;
    toggled_image_.SetPosition(position);
    untoggled_image_.SetPosition(position);
}

void UIToggleButton::SetSize(glm::vec2 size)
{
    size_ = size;
    toggled_image_.SetSize(size);
    untoggled_image_.SetSize(size);
}

void UIToggleButton::SetToggled(bool toggled)
{
    toggled_ = toggled;
}

bool UIToggleButton::IsToggled()
{
    return toggled_;
}

void UIToggleButton::Update()
{
    glm::dvec2 mouse_position = Input::GetVirtualMousePosition(UIGetVirtualToWindow());
    hovered_ = mouse_position.x >= position_.x && mouse_position.x <= position_.x + size_.x && mouse_position.y >= position_.y && mouse_position.y <= position_.y + size_.y;
    if (hovered_ && Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        toggled_ = !toggled_;
}

void UIToggleButton::Render()
{
    Shader &image_shader = ShaderManager::UI_IMAGE_SHADER;
    image_shader.Use();

    if (hovered_)
        image_shader.SetFloat("u_darkness", 0.1f);
    else
        image_shader.SetFloat("u_darkness", 0.0f);

    if (toggled_)
        toggled_image_.Render();
    else
        untoggled_image_.Render();
}

//
// Slider
//

UISlider::UISlider()
{
    slider_bg_middle_.LoadImage(Storage::IMAGES / "ui" / "ui_slider_bg_middle.png");
    slider_bg_right_.LoadImage(Storage::IMAGES / "ui" / "ui_slider_bg_right.png");

    slider_level_left_.LoadImage(Storage::IMAGES / "ui" / "ui_slider_level_left.png");
    slider_level_middle_.LoadImage(Storage::IMAGES / "ui" / "ui_slider_level_middle.png");

    slider_handle_.LoadImage(Storage::IMAGES / "ui" / "ui_slider_handle.png", GL_NEAREST);
    slider_handle_held_.LoadImage(Storage::IMAGES / "ui" / "ui_slider_handle_held.png");
    slider_value_text_.SetFontSize(0.5f);
    slider_value_text_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});
}

void UISlider::SetValue(float level)
{
    value_ = glm::clamp(level, value_min_, value_max_);
    if (discrete_)
        value_ = glm::round(value_);
    float f = (value_ - value_min_) / (value_max_ - value_min_);
    float handle_pos_x = position_.x + f*size_.x - 20;
    slider_level_middle_.SetSize({handle_pos_x - position_.x + 20, size_.y}, false);
    slider_handle_.SetPosition({handle_pos_x, position_.y - 16});
    slider_handle_held_.SetPosition({handle_pos_x, position_.y - 16});
    slider_value_text_.SetPosition({handle_pos_x + 30, position_.y + 30});
    if (discrete_)
    {
        slider_value_text_.SetText(std::to_string((int)value_));
    }
    else
    {
        std::ostringstream value_text;
        value_text << std::fixed << std::setprecision(2) << value_;
        slider_value_text_.SetText(value_text.str());
    }
}

float UISlider::GetValue()
{
    return value_;
}

void UISlider::SetBounds(glm::vec2 bounds)
{
    value_min_ = bounds.x;
    value_max_ = bounds.y;
}

void UISlider::SetPosition(glm::vec2 position)
{
    position_ = position;

    slider_bg_middle_.SetPosition({position.x + 10, position.y});
    slider_bg_right_.SetPosition({position.x + 10 + size_.x, position.y});

    slider_level_left_.SetPosition(position);
    slider_level_middle_.SetPosition({position.x + 10, position.y});

    slider_handle_.SetPosition({position.x - 20, position.y - 16});
    slider_handle_held_.SetPosition({position.x - 20, position.y - 16});
}

void UISlider::SetSize(glm::vec2 size)
{
    size_ = size;

    slider_bg_middle_.SetSize(size, false);
    slider_bg_middle_.SetPosition({position_.x + 10, position_.y});

    slider_bg_right_.SetSize(size, true);
    slider_bg_right_.SetPosition({position_.x + 10 + size.x, position_.y});

    slider_level_left_.SetSize(size, true);

    slider_level_middle_.SetSize(size, false);
    slider_level_middle_.SetPosition({position_.x + 10, position_.y});

    float handle_size = size.y * 2.5f;
    slider_handle_.SetSize({handle_size, handle_size});
    slider_handle_held_.SetSize({handle_size, handle_size});
}

void UISlider::SetDiscrete(bool value)
{
    discrete_ = value;
}

void UISlider::Update()
{
    glm::dvec2 mouse_pos = Input::GetVirtualMousePosition(UIGetVirtualToWindow());
    glm::vec2 handle_pos = slider_handle_.GetPosition();
    glm::vec2 handle_size = slider_handle_.GetSize();
    bool mouse_on_slider = mouse_pos.x >= position_.x - 20 && mouse_pos.x <= position_.x + size_.x + 20 && mouse_pos.y >= handle_pos.y && mouse_pos.y <= handle_pos.y + handle_size.y;

    hovered_ = mouse_pos.x >= handle_pos.x && mouse_pos.x <= handle_pos.x + handle_size.x && mouse_pos.y >= handle_pos.y && mouse_pos.y <= handle_pos.y + handle_size.y;

    if (clicked_)
        clicked_ = Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT);
    else
        clicked_ = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && mouse_on_slider;

    if (held_)
        held_ = Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT);
    else
        held_ = clicked_ && Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT) && mouse_on_slider;

    if (held_) // Still held; drag slider
    {
        // Determine new value and position of handle
        float handle_pos_x;
        if (discrete_)
        {
            float delta_x = size_.x / (value_max_ - value_min_);
            int n = glm::round(glm::clamp((float)mouse_pos.x - position_.x, 0.0f, size_.x) / delta_x);
            handle_pos_x = position_.x + n * delta_x;
            value_ = value_min_ + n;
        }
        else
        {
            handle_pos_x = glm::clamp((float)mouse_pos.x, position_.x, position_.x + size_.x);
            float t = (handle_pos_x - position_.x) / size_.x;
            value_ = glm::mix(value_min_, value_max_, t);
        }

        // Update positions
        slider_level_middle_.SetSize({handle_pos_x - position_.x, size_.y}, false);
        slider_handle_.SetPosition({handle_pos_x - 20, position_.y - 16});
        slider_handle_held_.SetPosition({handle_pos_x - 20, position_.y - 16});
        slider_value_text_.SetPosition({handle_pos_x + 30, position_.y + 30});

        // Update value text
        if (discrete_)
        {
            slider_value_text_.SetText(std::to_string((int)value_));
        }
        else
        {
            std::ostringstream value_text;
            value_text << std::fixed << std::setprecision(2) << value_;
            slider_value_text_.SetText(value_text.str());
        }
    }
}

void UISlider::Render()
{
    slider_bg_middle_.Render();
    slider_bg_right_.Render();
    slider_level_left_.Render();
    slider_level_middle_.Render();

    if (held_)
        slider_handle_held_.Render();
    else
        slider_handle_.Render();

    if (held_ || hovered_)
        slider_value_text_.Render();
}

//
// Progress Bar
//

UIProgressBar::UIProgressBar()
{
    slider_bg_.SetImage(Storage::IMAGES / "ui" / "ui_slider_bg.png");
    slider_level_.SetImage(Storage::IMAGES / "ui" / "ui_slider_level.png");
}

void UIProgressBar::SetLevel(float level)
{
    level_ = glm::clamp(level, 0.0f, 1.0f);
    slider_level_.SetSize({level_ * size_.x, size_.y});
}

void UIProgressBar::SetPosition(glm::vec2 position)
{
    position_ = position;
    slider_bg_.SetPosition(position);
    slider_level_.SetPosition(position);
}

void UIProgressBar::SetSize(glm::vec2 size)
{
    size_ = size;
    slider_bg_.SetSize(size);
    slider_level_.SetSize(size);
}

void UIProgressBar::Render()
{
    slider_bg_.Render();
    slider_level_.Render();
}


//
// Text Box
//

UITextBox::UITextBox()
{
    box_.SetImage(Storage::IMAGES / "ui" / "ui_textbox.png");
    text_.SetText("");
    text_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});
    text_.SetFontSize(0.4f);
    cursor_.SetText("1");
    cursor_.SetColor({0.0f, 0.0f, 0.0f, 1.0f});
    cursor_.SetFontSize(0.6f);
    cursor_time_ = 0;
}

void UITextBox::SetPosition(glm::vec2 position)
{
    position_ = position;
    box_.SetPosition(position);
    text_.SetPosition({position.x + 10, position.y + 10});
    cursor_.SetPosition({position.x + 10, position.y + 10});
}

void UITextBox::SetSize(glm::vec2 size)
{
    size_ = size;
    box_.SetSize(size);
}

void UITextBox::SetText(std::string text)
{
    text_.SetText(text);
    text_str_ = text;
}

std::string UITextBox::GetText()
{
    return text_str_;
}

void UITextBox::Update(float delta_time)
{
    // Text
    if (active_)
    {
        if (Input::IsKeyPressed(GLFW_KEY_BACKSPACE))
        {
            text_str_ = text_str_.substr(0, text_str_.length() - 1);
        }
        else
        {
            auto input_char = Input::GetLastCharInput();
            if (input_char > 0 && input_char <= 127)
            {
                float text_width = UIText::GetTextSizeInPixels(text_str_, 0.4f).x;
                if (text_width < 0.9f * size_.x)
                    text_str_ += (char)input_char;
            }
        }
        text_.SetText(text_str_);
        cursor_time_ = 0;
    }

    // Cursor
    float new_text_width = UIText::GetTextSizeInPixels(text_str_, 0.4f).x;
    cursor_.SetPosition({position_.x + 10 + new_text_width, position_.y + 10});
    if (active_)
    {
        cursor_time_ += delta_time;
        if (cursor_time_ > 0.8f)
            cursor_time_ = 0;
    }
    else
    {
        cursor_time_ = 0;
    }

    if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
    {
        glm::dvec2 mouse_position = Input::GetVirtualMousePosition(UIGetVirtualToWindow());
        bool mouse_on_box = mouse_position.x >= position_.x && mouse_position.x <= position_.x + size_.x && mouse_position.y >= position_.y && mouse_position.y <= position_.y + size_.y;
        active_ = mouse_on_box;
    }
}

void UITextBox::Render()
{
    box_.Render();
    text_.Render();

    if (active_ && cursor_time_ < 0.4f)
        cursor_.Render();
}
