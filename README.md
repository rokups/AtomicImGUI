AtomicImGUI
===========

[imgui](https://github.com/ocornut/imgui) integration for [AtomicGameEngine](https://github.com/AtomicGameEngine/AtomicGameEngine/).

# How to use

```cpp
// Create subsystem
auto imgui = new AtomicImGUI(context_);
// Register a font (optional, default font is always created)
const unsigned short range[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
imgui->AddFont("UI/fontawesome-webfont.ttf", 0, true, range);
// Draw GUI
SubscribeToEvent(E_IMGUIFRAME, [&](StringHash, VariantMap&) {
        ImGui::Begin("Example");
        ImGui::Text("%s Search", ICON_FA_SEARCH);
        ImGui::End();
});
```
